#include "LightHelper.hlsli"


//////////////////////////////////
// GLOBALS
//////////////////////////////////
Texture2D    gTextures[128] : register(t0);
SamplerState gSampleType   : register(s0);


//////////////////////////////////
// CONSTANT BUFFERS
//////////////////////////////////
cbuffer cbPerFrame        : register(b0)
{
	// light sources data
	DirectionalLight  gDirLights[3];
	PointLight        gPointLights[25];
	SpotLight         gSpotLights[25];
	float3            gEyePosW;                // eye position in world space  
	int               gCurrNumPointLights;
	int               gCurrNumSpotLights;
};

cbuffer cbRarelyChanged   : register(b1)
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
};


//////////////////////////////////
// TYPEDEFS
//////////////////////////////////
struct PS_INPUT
{
	float4 posH        : SV_POSITION;  // homogeneous position of the vertex
	float3 posW        : POSITION;     // world position of the vertex
	float2 tex         : TEXCOORD0;
};

//////////////////////////////////
// PIXEL SHADER
//////////////////////////////////
float4 PS(PS_INPUT pin) : SV_TARGET
{

	/////////////////////////  TEXTURE  ////////////////////////

	// Sample the pixel color from the texture using the sampler
	// at this texture coordinate location
	float4 finalColor = gTextures[1].Sample(gSampleType, pin.tex);
	float4 lightMapColor = gTextures[10].Sample(gSampleType, pin.tex);

	// the pixels with black (or lower that 0.1f) alpha values will be refected by
	// the clip function and not draw (this is used for rendering wires/fence/etc.);
	//
	// if the pixel was rejected we just return from the pixel shader since 
	// any further computations have no sense

	if (gAlphaClipping)
		clip(finalColor.a - 0.1f);
	

	/////////////////////////   FOG   ///////////////////////////

	if (true)
	{
		// the toEye vector is used in lighting
		float3 toEye = gEyePosW - pin.posW.xyz;
		float distToEye = length(toEye);
		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		finalColor *= lightMapColor;

		// blend the fog color and the lit color
		finalColor = lerp(finalColor, float4(gFogColor, 1.0f), fogLerp);
	}

	/////////////////////////////////////////////////////////////

	return finalColor;
}
