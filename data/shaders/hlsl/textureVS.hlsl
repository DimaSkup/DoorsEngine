//==================================================================================
// Filename:  texturePS.hlsl
// Desc:      vertex shader for texturing
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    // data per instance
    row_major matrix   world             : WORLD;
    row_major matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;
    row_major float4x4 material          : MATERIAL;
    uint               instanceID        : SV_InstanceID;

    // data per vertex
    float3   posL                        : POSITION;       // vertex position in local space
    float2   tex                         : TEXCOORD;
};

struct VS_OUT
{
    float4   posH       : SV_POSITION;    // homogeneous position
    float3   posW       : POSITION;       // position in world
    float2   tex        : TEXCOORD;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

	// transform pos from local to world space
	vout.posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

	// transform to homogeneous clip space
	vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);
	
	// output vertex attributes for interpolation across triangle
	vout.tex = vin.tex;

	return vout;
}
