// *********************************************************************************
// Filename:    DebugVS.hlsl
// Description: this is a vertex shader for debugging
// 
// Created:     24.11.24
// *********************************************************************************


//
// CONSTANT BUFFERS
//
cbuffer cbVSPerFrame : register(b0)
{
	matrix gViewProj;
};

//
// TYPEDEFS
//
struct VS_IN
{
	// data per instance
	row_major matrix   world             : WORLD;
	row_major matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;
	row_major float4x4 material          : MATERIAL;
	uint               instanceID        : SV_InstanceID;

	// data per vertex
	float3 posL        : POSITION;     // vertex position in local space
	float2 tex         : TEXCOORD;
	float3 normalL     : NORMAL;       // vertex normal in local space
	float3 tangentL    : TANGENT;      // tangent in local space
};

struct VS_OUT
{
	float4x4 material  : MATERIAL;
	float4   posH      : SV_POSITION;  // homogeneous position
	float3   posW      : POSITION;     // position in world
	float3   normalW   : NORMAL;       // normal in world
	float3   tangentW  : TANGENT;      // tangent in world
	float2   tex       : TEXCOORD;
    uint     instanceID : SV_InstanceID;
};


//
// VERTEX SHADER
//
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

	vout.material = vin.material;

	// transform pos from local space to world space
	vout.posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

	// transform to homogeneous clip space
	vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

	// interpolating of normal can unnormalize it, so normalize it back
	vout.normalW = normalize(mul(vin.normalL, (float3x3)vin.worldInvTranspose));

	// calculate the tangent
	vout.tangentW = normalize(mul(vin.tangentL, (float3x3)vin.worldInvTranspose));

    vout.tex        = vin.tex;
    vout.instanceID = vin.instanceID;

	return vout;
}
