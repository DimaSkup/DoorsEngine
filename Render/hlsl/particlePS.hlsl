#include "LightHelper.hlsli"


// ==========================
// GLOBALS
// ==========================
TextureCube  gCubeMap       : register(t0);
Texture2D    gPerlinNoise   : register(t1);
Texture2D    gTexture       : register(t10);
SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);


// ==========================
// CONSTANT BUFFERS
// ==========================
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
}


// ==========================
// TYPEDEFS
// ==========================
struct PS_INPUT
{
    float4   posH          : SV_POSITION;
    float3   posW          : POSITION;       // particle center position in world
    float    translucency  : TRANSLUCENCY;
    float3   color         : COLOR;
	float    normalX       : NORMAL_X;
    float2   tex           : TEXCOORD;
	float    normalY       : NORMAL_Y;
	float    normalZ       : NORMAL_Z;
    uint     primID        : SV_PrimitiveID;
};


// ==========================
// PIXEL SHADER
// ==========================
float4 PS(PS_INPUT pin) : SV_TARGET
{
	//return float4(pin.normalX, pin.normalY, pin.normalZ, 1.0f);

/*
    // sample texture
    float3 uvw = float3(pin.tex, pin.primID % 4);
    float4 texColor = gTreeMapArray.Sample(gSampleType, uvw);

    if (gAlphaClipping)
    {
        clip(texColor.a - 0.1f);
    }
    */

    // the toEye vector is used in lighting
    float3 toEyeW = gEyePosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;
	

	
	

    float4 texColor = gTexture.Sample(gBasicSampler, pin.tex);
    texColor *= float4(pin.color, 1.0f);
	texColor *= pin.translucency;
	//texColor.a = pin.translucency;
	
   
    //
    // fogging
    //
    //if (gFogEnabled)
	if (false)
    {
        	// TEMP: hacky fix for the vector to sample the proper pixel of sky
        float3 vec = -toEyeW;
        vec.y -= 990;
		float4 skyTexColor = gCubeMap.Sample(gSkySampler, vec);
		

        // blend sky pixel color with fixed fog color
        
        float4 fogColor    = float4(gFixedFogColor, 1.0f);
	
   
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        texColor = lerp(texColor, fogColor, fogLerp);
    }
    

    return texColor;
}
