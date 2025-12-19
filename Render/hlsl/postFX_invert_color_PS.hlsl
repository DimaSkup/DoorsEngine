//==================================================================================
// Desc:  a pixel shader for a colors inversion post effect
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
    return float4(1.0 - color.rgb, 1.0);
}
