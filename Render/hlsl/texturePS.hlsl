#include "LightHelper.hlsli"


//////////////////////////////////
// GLOBALS
//////////////////////////////////
TextureCube  gCubeMap       : register(t0);
Texture2D    gPerlinNoise   : register(t1);
Texture2D    gTextures[22]  : register(t10);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);


//////////////////////////////////
// CONSTANT BUFFERS
//////////////////////////////////
cbuffer cbPerFrame    : register(b0)
{
    // light sources data
    DirectionalLight  gDirLights[3];
    PointLight        gPointLights[25];
    SpotLight         gSpotLights[25];
    float3            gEyePosW;                // eye position in world space
    float             gTime;
    int               gCurrNumPointLights;
    int               gCurrNumSpotLights;
};


cbuffer cbRareChanged : register(b1)
{
    // some flags for controlling the rendering process and
    // params which are changed very rarely

    float3 gFixedFogColor;       // what is the color of fog?
    float  gFogStart;            // how far from camera the fog starts?
    float  gFogRange;            // how far from camera the object is fully fogged?

    int    gNumOfDirLights;      // current number of directional light sources

    int    gFogEnabled;          // turn on/off the fog effect
    int    gTurnOnFlashLight;    // turn on/off the flashlight
    int    gAlphaClipping;       // turn on/off alpha clipping

    float3 gSkyColorCenter;
    float  padding0;
    float3 gSkyColorApex;
    float  padding1;
};


//////////////////////////////////
// TYPEDEFS
//////////////////////////////////
struct PS_IN
{
    float4   posH       : SV_POSITION;    // homogeneous position
    float3   posW       : POSITION;       // position in world
    float2   tex        : TEXCOORD;
};


//////////////////////////////////
// PIXEL SHADER
//////////////////////////////////
float4 PS(PS_IN pin) : SV_TARGET
{
	float4 finalColor = gTextures[1].Sample(gBasicSampler, pin.tex);

	
	if (gAlphaClipping)
		clip(finalColor.a - 0.1f);
	

	if (gFogEnabled)
	{
		// the toEye vector is used to define a distance from camera to pixel
		float3 toEyeW = gEyePosW - pin.posW.xyz;
		float distToEye = length(toEyeW);

        // normalize
        toEyeW /= distToEye;

        // TEMP: hacky fix for the vector to sample the proper pixel of sky
        float3 vec = -toEyeW;
        vec.y -= 490;


		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend sky pixel color with fixed fog color
        float4 skyBottomColor = gCubeMap.Sample(gSkySampler, vec);
        float4 fogColor = skyBottomColor * float4(gFixedFogColor, 1.0f);

		// blend the fog color and the texture color
		finalColor = lerp(finalColor, fogColor, fogLerp);
	}


	return finalColor;
}
