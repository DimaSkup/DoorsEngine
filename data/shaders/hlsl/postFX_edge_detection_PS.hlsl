//==================================================================================
// Desc:  a pixel shader for an edge detection (sobel filter) post effect
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
    const float texelSizeX = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_X);
    const float texelSizeY = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_Y);
    const float2 texelSize = float2(texelSizeX, texelSizeY);

    float3 tl = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2(-1, -1)).rgb;
    float3  t = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2( 0, -1)).rgb;
    float3 tr = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2( 1, -1)).rgb;
    float3 l  = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2(-1,  0)).rgb;
    float3 r  = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2( 1,  0)).rgb;
    float3 bl = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2(-1,  1)).rgb;
    float3 b  = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2( 0,  1)).rgb;
    float3 br = gScreenTex.Sample(gSamLinearClamp, pin.tex + texelSize * float2( 1,  1)).rgb;

    float3 gx = tr + 2 * r + br - (tl + 2 * l + bl);
    float3 gy = bl + 2 * b + br - (tl + 2 * t + tr);
    float3 edge = sqrt(gx * gx + gy * gy);
    float intensity = dot(edge, float3(0.333, 0.333, 0.333));
    return float4(intensity.xxx, 1);
}
