////////////////////////////////////////////////////////////////////
// Filename:     skyDomePixel.hlsl
// Description:  a HLSL pixel shader for rendering of the sky dome
// Created:      16.04.23
////////////////////////////////////////////////////////////////////

//////////////////////////////////
// GLOBALS
//////////////////////////////////
Texture2D shaderTexture : TEXTURE : register(t0);
SamplerState sampleType : SAMPLER : register(s0);

cbuffer ColorBuffer
{
	float4 apexColor;
	float4 centerColor;
};

//////////////////////////////////
// TYPEDEFS
//////////////////////////////////
struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 domePosition : TEXCOORD1;
};

//////////////////////////////////
// PIXEL SHADER
//////////////////////////////////
float4 PS(PS_INPUT input): SV_TARGET
{
	float height;
	float4 color;  // output color
	bool useTexture = false;

	if (useTexture)
	{
		// sample the pixel color from the texture using the sampler at this texture coordinate location
		color = shaderTexture.Sample(sampleType, input.tex);
		return color;
	}

	// determine the position on the sky dome where this pixel is located
	height = input.domePosition.y;

	// the value ranges from -1.0f to +1.0f so change it to only positive values
	if (height < 0.0f)
	{
		height = 0.0f;
	}

	// determine the gradient colour by interpolating between the apex and center 
	// based on the height of the pixel in the sky dome
	color = lerp(centerColor, apexColor, height);

	return color;
}