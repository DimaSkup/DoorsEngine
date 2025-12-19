//==================================================================================
// Desc:  a pixel shader for a grayscale post effect
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
    const float4 color = gScreenTex.Sample(gSamLinearClamp, pin.tex);
    const float gray   = dot(color.rgb, float3(0.299, 0.587, 0.114));

    return float4(gray.xxx, 1.0);
}
