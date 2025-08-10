//////////////////////////////////
// Filename: font.ps
//////////////////////////////////

//
// GLOBALS
//
Texture2D    fontTexture    : register(t2);
SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);


//
// CONST BUFFERS
//
cbuffer PixelBuffer : register(b3)
{
    float4 gPixelColor;        // pixel colour value for the font
};

//
// TYPEDEFS
//
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex      : TEXCOORD0;
};


//
// Pixel Shader
//
float4 PS(PixelInputType input) : SV_TARGET
{
    return gPixelColor * fontTexture.Sample(gBasicSampler, input.tex);
}
