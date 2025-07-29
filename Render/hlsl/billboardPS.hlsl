#include "LightHelper.hlsli"


// ==========================
// GLOBALS
// ==========================
Texture2DArray gTreeMapArray : register(t1);
Texture2D      gTextures[22] : register(t2);
SamplerState   gSampleType   : register(s0);


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

cbuffer cbRarelyChanged   : register(b1)
{
    // some flags for controlling the rendering process and
    // params which are changed very rarely

    float3 gFogColor;            // what is the color of fog?
    float  gFogStart;            // how far from camera the fog starts?
    float  gFogRange;            // how far from camera the object is fully fogged?

    int    gNumOfDirLights;      // current number of directional light sources

    int    gFogEnabled;          // turn on/off the fog effect
    int    gTurnOnFlashLight;    // turn on/off the flashlight
    int    gAlphaClipping;       // turn on/off alpha clipping
};


// ==========================
// TYPEDEFS
// ==========================
struct PS_INPUT
{
    float4 posH   : SV_POSITION;
    float3 posW   : POSITION;
    float  translucency : TRANSLUCENCY;
    float3 color   : COLOR;
    float3 normalW : NORMAL;
    float2 tex    : TEXCOORD;
    uint   primID : SV_PrimitiveID;
};


// ==========================
// PIXEL SHADER
// ==========================
float4 PS(PS_INPUT pin) : SV_TARGET
{
/*
    // the toEye vector is used in lighting
    float3 toEyeW = gEyePosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;

    // default to multiplicative identity
    //float4 texColor = float4(1, 1, 1, 1);

    // sample texture
    float3 uvw = float3(pin.tex, pin.primID % 4);
    float4 texColor = gTreeMapArray.Sample(gSampleType, uvw);

    if (gAlphaClipping)
    {
        clip(texColor.a - 0.1f);
    }

    //
    // fogging
    //
    if (gFogEnabled)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        texColor = lerp(texColor, float4(gFogColor, 1.0f), fogLerp);
    }
    */

    // default to multiplicative identity
   //float4 texColor = float4(1, 1, 1, 1);

   // sample texture
   float4 texColor = gTextures[0].Sample(gSampleType, pin.tex);
   float3 finalColor = texColor.xyz * pin.color;

   return float4(finalColor, pin.translucency);
}
