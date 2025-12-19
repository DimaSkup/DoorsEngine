//==================================================================================
// Desc:  a pixel shader for a pixelation (low-resolution look) post effect
//==================================================================================
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
    const float resX    = GetPostFxParam(POST_FX_PARAM_SCREEN_WIDTH);
    const float resY    = GetPostFxParam(POST_FX_PARAM_SCREEN_HEIGHT);
    const float pixelSz = GetPostFxParam(POST_FX_PARAM_PIXELATION_PIXEL_SIZE);

    const float2 aspect = float2(resX, resY) / pixelSz;
    const float2 uv     = floor(pin.tex * aspect) / aspect;

    return gScreenTex.Sample(gSamLinearClamp, uv);
}
