//==================================================================================
// Desc:  a pixel shader for a brightness/contrast adjustment post effect
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
    const float contrast   = GetPostFxParam(POST_FX_PARAM_CONTRAST);
    const float brightness = GetPostFxParam(POST_FX_PARAM_BRIGHTNESS);

    float4 c = gScreenTex.Sample(gSamLinearClamp, pin.tex);
    c.rgb = (c.rgb - 0.5) * contrast + 0.5;
    c.rgb += brightness;

    return c;
}
