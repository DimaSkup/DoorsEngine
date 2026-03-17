//==================================================================================
// Desc:  a pixel shader for a CRT Curvature + Scanlines post effect
//        (Retro CRT display simulation)
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
    const float resY      = GetPostFxParam(POST_FX_PARAM_SCREEN_HEIGHT);
    const float curvature = GetPostFxParam(POST_FX_PARAM_CRT_CURVATURE);

    // berrel distortion
    float2 uv = pin.tex;
    uv = uv * 2.0 - 1.0;
    float r2 = dot(uv, uv);
    uv *= 1.0 + curvature * r2;
    uv = uv * 0.5 + 0.5;

    float4 color = gScreenTex.Sample(gSamLinearClamp, uv);

    // add horizontal scanlines
    float scan = sin(uv.y * resY * 3.14159);
    color.rgb *= 0.8 + 0.2 * scan;

    return saturate(color);
}
