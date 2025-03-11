#include "LightHelper.hlsli"


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
	uint               instanceID        : SV_InstanceID;

	// data per vertex
	float3   posL       : POSITION;     // vertex position in local space
	float3   normalL    : NORMAL;       // vertex normal in local space
};

struct VS_OUT
{
	float4   posH       : SV_POSITION;  // homogeneous position
};



//
// VERTEX SHADER
//
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

	// transform pos from local to world space
	float3 posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

	// transform to homogeneous clip space
	vout.posH = mul(float4(posW, 1.0f), gViewProj);

	//float4 clipPosition = UnityObjectToClipPos(position);
	float3 clipNormal = mul((float3x3) gViewProj, mul((float3x3) vin.world, vin.normalL));

	float outlineWidth = 10;
	vout.posH.xyz += normalize(clipNormal) * outlineWidth;

	return vout;
}
