//==================================================================================
// Desc:  a pixel shader for a heat distortion / mirage post effect
//==================================================================================
#include "const_buffers/cb_time.hlsli"
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
    const float strength = GetPostFxParam(POST_FX_PARAM_HEAT_DISTORT_STRENGTH);

    float2 uv = pin.tex;
    float wave = sin(uv.y * 100 + gGameTime * 5.0) * 0.5 + 0.5;
    uv.x += wave * strength;

    return gScreenTex.Sample(gSamLinearClamp, uv);
}
