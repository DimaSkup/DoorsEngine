
//
// GLOBALS
//
cbuffer PerFrameBuffer : register(b2)
{
	matrix gWorldViewProj;
};


//
// TYPEDEFS
//
struct VS_INPUT
{
	float2 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};


//
// Vertex Shader
//
VS_OUT VS(VS_INPUT vin)
{
	VS_OUT vout;

	// calculate the vertex position against the world, view, and projection matrices
	vout.pos = mul(float4(vin.pos, 0.0f, 1.0f), gWorldViewProj);
   
	// set the texture coordinates for the pixel shader
	vout.tex = vin.tex;

	return vout;
}
