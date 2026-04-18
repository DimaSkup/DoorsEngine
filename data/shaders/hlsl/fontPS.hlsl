//==================================================================================
// Desc:  a pixel shader for text rendering
//==================================================================================

//---------------------------
// GLOBALS
//---------------------------
Texture2D    fontTexture    : register(t20);
SamplerState gSamLinearWrap : register(s3);

//---------------------------
// CONST BUFFERS
//---------------------------
cbuffer PixelBuffer : register(b3)
{
    float4 gPixelColor;        // pixel colour value for the font
};

//---------------------------
// TYPEDEFS
//---------------------------
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex      : TEXCOORD0;
};

//---------------------------
// Pixel Shader
//---------------------------
float4 PS(PixelInputType input) : SV_TARGET
{
    return gPixelColor * fontTexture.Sample(gSamLinearWrap, input.tex);
}
