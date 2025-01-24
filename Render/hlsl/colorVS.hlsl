////////////////////////////////////////////////////////////////////////////////
// Filename:    colorVertex.hlsl
// Description: it is a Vertex Shader
////////////////////////////////////////////////////////////////////////////////



//
// CONSTANT BUFFERS
//
cbuffer cbPerFrame : register(b0)
{
	matrix gViewProj;
};

//
// TYPEDEFS
//
struct VS_INPUT
{ 
	// data per instance
	row_major matrix   world             : WORLD;
	row_major matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;
	row_major matrix   texTransform      : TEX_TRANSFORM;
	row_major float4x4 material          : MATERIAL;
	uint               instanceID        : SV_InstanceID;

	float3 posL   : POSITION;       // position of the vertex in local space
	float4 color  : COLOR;          // color of the vertex
};

struct VS_OUTPUT
{
	float4 posH   : SV_POSITION;    // homogeneous position
	float3 posW   : POSITION;       // position in world
	float4 color  : COLOR;          // color of the vertex
};


//
// Vertex Shader
//
VS_OUTPUT VS(VS_INPUT vin)
{
	VS_OUTPUT vout;

	// transform pos from local to world space
	vout.posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

	// transform to homogeneous clip space
	vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

	// output vertex attributes for interpolation across triangle
	vout.color = vin.color;

	return vout;
}