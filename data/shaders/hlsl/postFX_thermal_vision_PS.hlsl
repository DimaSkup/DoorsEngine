//==================================================================================
// Desc:  a pixel shader for a thermal vision post effect
//==================================================================================

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
    float4 color   = gScreenTex.Sample(gSamLinearClamp, pin.tex);
    float lum      = dot(color.rgb, float3(0.299, 0.587, 0.114));

    float3 heatmap = lerp(float3(0,0,1), float3(1,0,0), lum);
    heatmap        = lerp(heatmap, float3(1, 1, 0), lum * lum);

    return float4(heatmap, 1);
}
