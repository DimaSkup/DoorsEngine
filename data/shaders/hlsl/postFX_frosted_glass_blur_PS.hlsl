//==================================================================================
// Desc:  a pixel shader for a frosted glass blur post effect
//==================================================================================
#include "helpers/noise_rand.hlsli"
#include "const_buffers/cbps_post_fx2.hlsli"
#include "const_buffers/post_fx_params_enum.hlsli"

//---------------------------
// GLOBALS
//---------------------------
Texture2D    gScreenTex      : register(t12);
SamplerState gSamLinearClamp : register(s2);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float  texelSizeX = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_X);
    const float  texelSizeY = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_Y);
    const float2 texelSize  = float2(texelSizeX, texelSizeY);
    const float  strength   = GetPostFxParam(POST_FX_PARAM_FROST_GLASS_BLUR_STRENGTH);

    const float2 offset    = (rand(pin.tex) - 0.5) * texelSize * strength;
    return gScreenTex.Sample(gSamLinearClamp, pin.tex + offset);
}
