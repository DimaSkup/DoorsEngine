//////////////////////////////////
// CONST BUFFERS
//////////////////////////////////
cbuffer cbPerFrame : register(b0)
{
    matrix gViewProj;
};

//////////////////////////////////
// TYPEDEFS
//////////////////////////////////
struct VS_IN
{
    // data per instance
    row_major matrix   world             : WORLD;
    row_major matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;
    row_major float4x4 material          : MATERIAL;
    uint               instanceID        : SV_InstanceID;

    // data per vertex
    float3   posL                        : POSITION;       // vertex position in local space
};

struct VS_OUT
{
	float4 posH   : SV_POSITION;    // homogeneous position
};


////////////////////////////////////
// Vertex Shader
//////////////////////////////////
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

    // transform position to homogeneous clip space
	float4 posW = mul(float4(vin.posL, 1.0f), vin.world);
	vout.posH = mul(posW, gViewProj);

	return vout;
}
