#include "LightHelper.hlsli"


// ==========================
// GLOBALS
// ==========================
TextureCube  gCubeMap       : register(t0);
Texture2D    gPerlinNoise   : register(t1);
Texture2D    gTextures[22]  : register(t10);
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

    float3 gSkyColorCenter;
    float  padding0;
    float3 gSkyColorApex;
    float  padding1;
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
    float4 texColor = gTextures[1].Sample(gBasicSampler, pin.tex);
    texColor *= float4(pin.color, 1.0f);
    texColor *= pin.translucency;

    return texColor;
   
}
