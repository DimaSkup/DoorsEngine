//==================================================================================
// Filename:  texturePS.hlsl
// Desc:      pixel shader for texturing
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"


//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4   posH       : SV_POSITION;    // homogeneous position
    float3   posW       : POSITION;       // position in world
    float2   tex        : TEXCOORD;
};


//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
    float4 finalColor     = gTextures[1].Sample(gBasicSampler, pin.tex);
    float4 skyBottomColor = gCubeMap.Sample(gSkySampler, float3(0, -490, 0));
    
    if (gAlphaClipping)
        clip(finalColor.a - 0.1f);
    

    if (gFogEnabled)
    {
        // the toEye vector is used to define a distance from camera to pixel
        float3 toEyeW = gCamPosW - pin.posW.xyz;
        float distToEye = length(toEyeW);

        // normalize
        toEyeW /= distToEye;

        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend sky pixel color with fixed fog color
        float4 fogColor = skyBottomColor * float4(gFixedFogColor, 1.0);

        // blend the fog color and the texture color
        finalColor = lerp(finalColor, fogColor, fogLerp);
    }


    return finalColor;
}
