//==================================================================================
// Desc:  a pixel shader for a swirl distortion post effect
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
    const float  cx     = GetPostFxParam(POST_FX_PARAM_SWIRL_DISTORT_CX);
    const float  cy     = GetPostFxParam(POST_FX_PARAM_SWIRL_DISTORT_CY);
    const float2 center = float2(cx, cy);
    const float  radius = GetPostFxParam(POST_FX_PARAM_SWIRL_DISTORT_RADIUS);
    const float  angle  = GetPostFxParam(POST_FX_PARAM_SWIRL_DISTORT_ANGLE);


    float2 uv = pin.tex;
    float2 delta = uv - center;
    float dist = length(delta);

    if (dist < radius)
    {
        float theta = atan2(delta.y, delta.x) + (radius - dist) / radius * angle;
        uv = center + float2(cos(theta), sin(theta)) * dist;
    }

    return gScreenTex.Sample(gSamLinearClamp, uv);
}
