//==================================================================================
// Desc:  a pixel shader for a color shirt / hue rotation post effect
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
// HELPERS
//---------------------------
float3 HueShift(const float3 color, const float angle)
{
    const float3 k   = float3(0.57735, 0.57735, 0.57735);
    const float cosA = cos(angle);
    const float sinA = sin(angle);

    return color * cosA + cross(k, color) * sinA + k * dot(k, color) * (1.0 - cosA);
}

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float4 color = gScreenTex.Sample(gSamLinearClamp, pin.tex);
    const float  hue   = GetPostFxParam(POST_FX_PARAM_COLOR_SHIFT_HUE);

    return float4(HueShift(color.rgb, hue), 1.0);
}
