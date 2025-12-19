//==================================================================================
// Desc:  a pixel shader for a gaussian blur (simple 9-tap) post effect
//        (for a basic blur, do two passes: one horizontal and one vertical)
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
// Horizontal and vertical pass
//---------------------------
float4 HorizontalPass(const float2 uv)
{
    const float texelSizeX = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_X);
    

    const float weights[5] = { 0.227027, 0.316216, 0.070270, 0.016216, 0.001595 };
    float4 color = gScreenTex.Sample(gSamLinearClamp, uv) * weights[0];

    [unroll]
    for (int i = 1; i < 5; ++i)
    {
        color += gScreenTex.Sample(gSamLinearClamp, uv + float2(texelSizeX * i, 0)) * weights[i];
        color += gScreenTex.Sample(gSamLinearClamp, uv - float2(texelSizeX * i, 0)) * weights[i];
    }

    return color;
}

//---------------------------

float4 VerticalPass(const float2 uv)
{
    const float texelSizeY = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_Y);

    const float weights[5] = { 0.227027, 0.316216, 0.070270, 0.016216, 0.001595 };
    float4 color = gScreenTex.Sample(gSamLinearClamp, uv) * weights[0];

    [unroll]
    for (int i = 1; i < 5; ++i)
    {
        color += gScreenTex.Sample(gSamLinearClamp, uv + float2(0, texelSizeY * i)) * weights[i];
        color += gScreenTex.Sample(gSamLinearClamp, uv - float2(0, texelSizeY * i)) * weights[i];
    }

    return color;
}

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    float4 horizColor = HorizontalPass(pin.tex);
    float4 vertColor  = VerticalPass(pin.tex);

    return horizColor * vertColor;
}
