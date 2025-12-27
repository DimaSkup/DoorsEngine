//==================================================================================
// Desc:  a pixel shader for an old TV distortion (wobble + noise) post effect
//==================================================================================
#include "const_buffers/cbps_post_fx2.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "helpers/noise_rand.hlsli"
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
    const float strength = GetPostFxParam(POST_FX_PARAM_OLD_TV_DISTORT_STRENGTH);

    float2 uv = pin.tex;
    uv.y += sin(uv.x * gGameTime * 35) * strength;

    float4 color = gScreenTex.Sample(gSamLinearClamp, uv);
    color.rgb += noise(uv * gGameTime);

    return saturate(color);
}
