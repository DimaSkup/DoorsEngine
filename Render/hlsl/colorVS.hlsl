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
    float3             posL       : POSITION;       // position of the vertex in local space

	// data per instance
	row_major matrix   world      : WORLD;
	uint               instanceID : SV_InstanceID;
};

struct VS_OUTPUT
{
	float4 posH   : SV_POSITION;    // homogeneous position
};


//
// Vertex Shader
//
VS_OUTPUT VS(VS_INPUT vin)
{
	VS_OUTPUT vout;

    // transform to homogeneous clip space
	vout.posH = mul(float4(vin.posL, 1.0f), vin.world * gViewProj);

	return vout;
}
