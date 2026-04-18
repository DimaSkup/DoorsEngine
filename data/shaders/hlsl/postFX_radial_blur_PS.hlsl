//==================================================================================
// Desc:  a pixel shader for a radial blur post effect
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
    const float cx       = GetPostFxParam(POST_FX_PARAM_RADIAL_BLUR_CX);
    const float cy       = GetPostFxParam(POST_FX_PARAM_RADIAL_BLUR_CY);
    const int   samples  = GetPostFxParam(POST_FX_PARAM_RADIAL_BLUR_SAMPLES);
    const float strength = GetPostFxParam(POST_FX_PARAM_RADIAL_BLUR_STRENGTH);

    float2 dir = pin.tex - float2(cx, cy);
    float4 sum = 0;

    for (int i = 0; i < samples; ++i)
    {
        float t = float(i) / (samples - 1);
        sum += gScreenTex.Sample(gSamLinearClamp, pin.tex - dir * t * strength);
    }

    return sum / samples;
}
