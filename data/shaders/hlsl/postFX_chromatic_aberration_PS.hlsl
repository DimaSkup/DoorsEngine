//==================================================================================
// Desc:  a pixel shader for a simple chromatic aberration post effect
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
    float strength = GetPostFxParam(POST_FX_PARAM_CHROMATIC_ABERRATION_STRENGTH);
    float2 offset  = (pin.tex - 0.5) * strength;

    float r = gScreenTex.Sample(gSamLinearClamp, pin.tex + offset).r;
    float g = gScreenTex.Sample(gSamLinearClamp, pin.tex).g;
    float b = gScreenTex.Sample(gSamLinearClamp, pin.tex - offset).b;

    return float4(r, g, b, 1.0);
}
