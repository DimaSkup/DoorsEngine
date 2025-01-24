/////////////////////////////////////////////////////////////////////
// Filename:      skyPlanePixel.hlsl
// Description:   a pixel shader for rendering of the sky plane model
//
// Create:        27.06.23
/////////////////////////////////////////////////////////////////////



//////////////////////////////////
// GLOBALS
//////////////////////////////////
Texture2D cloudTexture1 : register(t0);
Texture2D cloudTexture2 : register(t1);
SamplerState sampleType;

cbuffer SkyBuffer
{
	float firstTranslationX;
	float firstTranslationZ;
	float secondTranslationX;
	float secondTranslationZ;
	float brightness;
	float3 padding;
};

//////////////////////////////////
// TYPEDEFS
//////////////////////////////////
struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

//////////////////////////////////
// Pixel shader
//////////////////////////////////
float4 PS(PS_INPUT input): SV_TARGET
{
	float2 sampleLocation;
	float4 textureColor1;
	float4 textureColor2;
	float4 finalColor;

	// translate the position where we sample the pixel from using
	// the first texture translation values
	sampleLocation.x = input.tex.x + firstTranslationX;
	sampleLocation.y = input.tex.y + firstTranslationZ;

	// sample the pixel color from the first cloud texture using the sampler at this 
	// texture coordinate location
	textureColor1 = cloudTexture1.Sample(sampleType, sampleLocation);

	// translation the position where we sample the pixel from using 
	// the second texture translation values
	sampleLocation.x = input.tex.x + secondTranslationX;
	sampleLocation.y = input.tex.y + secondTranslationZ;

	// sample the pixel color from the first cloud texture using the sampler at this 
	// texture coordinate location
	textureColor2 = cloudTexture2.Sample(sampleType, sampleLocation);

	// combine the two cloud textures evenly
	finalColor = lerp(textureColor1, textureColor2, 0.5f);

	// reduce brightness of the combined cloud textures by the input brightness value
	finalColor *= brightness;

	return finalColor;

}