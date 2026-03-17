//==================================================================================
// Desc:  a pixel shader for a posterization post effect
//        (reduces the number of color levels to create a stylized "cartoon" look)
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
    float levels = GetPostFxParam(POST_FX_PARAM_POSTERIZATION_LEVELS);
    float4 c     = gScreenTex.Sample(gSamLinearClamp, pin.tex);
    c.rgb        = floor(c.rgb * levels) / (levels - 1);

    return c;
}
