//==================================================================================
// Filename:    debug_model_VS.hlsl
// Description: this is a vertex shader for debugging
// 
// Created:     24.11.24
//==================================================================================
#include "const_buffers/cbvs_world_and_view_proj.hlsli"
#include "const_buffers/cbvs_world_inverse_transpose.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
	// data per vertex
	float3 posL        : POSITION;     // vertex position in local space
	float2 tex         : TEXCOORD;
	float3 normalL     : NORMAL;       // vertex normal in local space
	float4 tangentL    : TANGENT;      // tangent in local space
};

struct VS_OUT
{
	matrix   world     : WORLD_MATRIX;
	
	float4   posH      : SV_POSITION;  // homogeneous position
	float3   posW      : POSITION;     // position in world
	float    texU      : TEXCOORD0;
	float3   normalW   : NORMAL;       // normal in world
	float    texV      : TEXCOORD1;
	float4   tangentW  : TANGENT;      // tangent in world
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

	vout.world = gWorld;
	 
    // transform pos from local to world space
    vout.posW     = mul(float4(vin.posL, 1.0f), gWorld).xyz;

    // transform to homogeneous clip space
    vout.posH     = mul(float4(vout.posW, 1.0f), gViewProj);

    // transform normal and tangent to world space
    vout.normalW  = mul(vin.normalL, (float3x3)gWorld);
    vout.tangentW = mul(vin.tangentL, gWorld);

    vout.texU = vin.tex.x;
    vout.texV = vin.tex.y;

    return vout;
}
