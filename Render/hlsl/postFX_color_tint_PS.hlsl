//==================================================================================
// Desc:  a pixel shader for a color tint post effect
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
    float4 c = gScreenTex.Sample(gSamLinearClamp, pin.tex);

    const float tintR     = GetPostFxParam(POST_FX_PARAM_COLOR_TINT_R);
    const float tintG     = GetPostFxParam(POST_FX_PARAM_COLOR_TINT_G);
    const float tintB     = GetPostFxParam(POST_FX_PARAM_COLOR_TINT_B);
    const float intensity = GetPostFxParam(POST_FX_PARAM_COLOR_TINT_INTENSITY);

    const float3 tint = float3(tintR, tintG, tintB);
    c.rgb = lerp(c.rgb, c.rgb * tint, intensity);
    
    return c;
}
