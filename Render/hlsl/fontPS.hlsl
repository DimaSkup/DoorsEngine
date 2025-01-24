//////////////////////////////////
// Filename: font.ps
//////////////////////////////////

//
// GLOBALS
//
Texture2D shaderTexture;
SamplerState SampleType;

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
	// sample the texture pixel at this location and mix it with the global pixel color
	return gPixelColor * shaderTexture.Sample(SampleType, input.tex);
}