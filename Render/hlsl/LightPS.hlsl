#include "LightHelper.hlsli"


//
// GLOBALS
//
Texture2D    gTextures[22] : register(t0);
SamplerState gSampleType   : register(s0);


//
// CONSTANT BUFFERS
//
cbuffer cbPerFrame    : register(b0)
{
	// light sources data
	DirectionalLight  gDirLights[3];
	PointLight        gPointLights[25];
	SpotLight         gSpotLights;

	// eye position in world space
	float3            gEyePosW;         

	int               gCurrNumPointLights;
};

cbuffer cbRareChanged : register(b1)
{
	// some flags for controlling the rendering process and
	// params which are changed very rarely

	float3 gFogColor;            // what is the color of fog?
	float  gFogStart;            // how far from camera the fog starts?
	float  gFogRange;            // how far from camera the object is fully fogged?

	int    gNumOfDirLights;      // current number of directional light sources

	int   gFogEnabled;          // turn on/off the fog effect
	int   gTurnOnFlashLight;    // turn on/off the flashlight
	int   gAlphaClipping;       // turn on/off alpha clipping
}

//
// TYPEDEFS
//
struct PS_IN
{
	float4x4 material  : MATERIAL;
	float4   posH      : SV_POSITION;  // homogeneous position
	float3   posW      : POSITION;     // position in world
	float3   normalW   : NORMAL;       // normal in world
	float3   tangentW  : TANGENT;      // tangent in world
	float3   binormalW : BINORMAL;     // binormal in world
	float2   tex       : TEXCOORD;
};

struct PS_OUT
{
	float4 color : SV_Target;
	float  depth : SV_Depth;
};


//
// PIXEL SHADER
//
float4 PS(PS_IN pin) : SV_Target
{
	float4 textureColor  = gTextures[1].Sample(gSampleType, pin.tex);
	
	// execute alpha clipping
	if (gAlphaClipping)
		clip(textureColor.a - 0.1f);

	float4 specularColor = gTextures[2].Sample(gSampleType, pin.tex);
	float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;
	float4 roughnessMap = gTextures[16].Sample(gSampleType, pin.tex);


	normalMap = (normalMap * 2.0f) - 1.0f;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, pin.normalW, pin.tangentW);


	// --------------------  LIGHT   --------------------

	// a vector in the world space from vertex to eye pos
	float3 toEyeW = normalize(gEyePosW - pin.posW);

	// start with a sum of zero
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// sum the light contribution from each light source (ambient, diffuse, specular)
	float4 A, D, S;

	// sum the light contribution from each directional light source
	for (int i = 0; i < gNumOfDirLights; ++i)
	{
		ComputeDirectionalLight(
			(Material)pin.material,
			gDirLights[i],
			pin.normalW,  // bumpedNormalW, 
			toEyeW,
			specularColor.x,
			A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;
	}


	// sum the light contribution from each point light source
	for (i = 0; i < gCurrNumPointLights; ++i)
	{
		ComputePointLight(
			(Material)pin.material,
			gPointLights[i],
			pin.posW,
			pin.normalW, //.bumpedNormalW, // pin.normalW,
			toEyeW,
			specularColor.x,
			A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;
	}
	
	// sum the light contribution from each spot light source
	if (gTurnOnFlashLight)
	{
		ComputeSpotLight(
			(Material)pin.material, 
			gSpotLights,
			pin.posW,
			pin.normalW, //bumpedNormalW, // pin.normalW,
			toEyeW,
			specularColor.x,
			A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;
	}
	
	
	// modulate with late add
	float4 litColor = textureColor * (ambient + diffuse + spec);

	litColor *= roughnessMap;

	// ---------------------  FOG  ----------------------

	if (gFogEnabled)
	{
		float distToEye = length(gEyePosW - pin.posW);
		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		// blend the fog color and the lit color
		litColor = lerp(litColor, float4(gFogColor, 1.0f), fogLerp);
	}


	// common to take alpha from diffuse material and texture
	litColor.a = ((Material)pin.material).diffuse.a * textureColor.a;

	// render depth value as color
	//return float4(pin.posH.z, pin.posH.z, pin.posH.z, 1.0f);

	return litColor;
	
}


