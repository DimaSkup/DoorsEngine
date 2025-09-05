// =================================================================================
// Filename:     SkyDomePS.hlsl
// Description:  pixel shader which is used to shade sky dome
// =================================================================================


//
// GLOBALS
//
// nonnumeric values cannot be added to a cbuffer
TextureCube  gCubeMap       : register(t0);
SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);


//
// CONSTANT BUFFERS
//


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


//
// TYPEDEFS
//
struct PS_INPUT
{
    float4 posH   : SV_POSITION;    // homogeneous position
    float3 posL   : POSITION;       // position of the vertex in local space
};


//
// PIXEL SHADER
//
float4 PS(PS_INPUT pin) : SV_TARGET
{
    float height = pin.posL.y;

    if (height < 0.0f)
    {
        height = 0.0f;
    }

    // determine the gradient colour by interpolating between the apex and center 
    // based on the height of the pixel in the sky dome
    float3 color = lerp(gSkyColorCenter, gSkyColorApex, height);

    float4 texColor = gCubeMap.Sample(gSkySampler, pin.posL);
    return texColor * float4(color, 1.0f);
};
