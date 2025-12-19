//==================================================================================
// Filename:    debug_model_VS.hlsl
// Description: this is a vertex shader for debugging
// 
// Created:     24.11.24
//==================================================================================
#include "const_buffers/cbvs_world_and_view_proj.hlsli"
#include "const_buffers/cbvs_world_inverse_transpose.hlsli"
#include "const_buffers/cbvs_skinned.hlsli"

  

//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    // data per vertex
    float3 posL             : POSITION;     // vertex position in local space
    float2 tex              : TEXCOORD;
    float3 normalL          : NORMAL;       // vertex normal in local space
    float4 tangentL         : TANGENT;      // tangent in local space
    float4 weights          : WEIGHTS;
    uint4 boneIds          : BONEINDICES;
};

struct VS_OUT
{
    float4   posH           : SV_POSITION;  // homogeneous position
    float3   posW           : POSITION;     // position in world
    float    texU           : TEXCOORD0;
    float3   normalW        : NORMAL;       // normal in world
    float    texV           : TEXCOORD1;
    float4   tangentW       : TANGENT;      // tangent in world
    float4   weights        : WEIGHTS;
    uint4    boneIds        : BONEINDICES;
    uint     currBoneId     : TEXCOORD2;
};

//---------------------------
//---------------------------
cbuffer cbDebug : register(b8)
{
	uint gCurrBoneId;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    const matrix boneTransform = 
		gBoneTransforms[vin.boneIds.x] * vin.weights.x +
        gBoneTransforms[vin.boneIds.y] * vin.weights.y +
		gBoneTransforms[vin.boneIds.z] * vin.weights.z +
		gBoneTransforms[vin.boneIds.w] * vin.weights.w;
	
	
	// assume no nonuniform scaling when transforming normals, so
	// that we don't have to use the inverse-transpose	
	const float4 posL  = mul(float4(vin.posL, 1.0), boneTransform);
	const float3 normL = mul(vin.normalL,           (float3x3)boneTransform);
	const float3 tangL = mul(vin.tangentL.xyz,      (float3x3)boneTransform); 
	
	
    // transform position to world and homogeneous clip space
    vout.posW = mul(posL, gWorld).xyz;
    vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

    // transform normal and tangent to world space
    vout.normalW      = mul(normL, (float3x3)gWorldInvTranspose).xyz;
    vout.tangentW.xyz = mul(tangL, (float3x3)gWorld);
	
	// pass tangent handedness
	vout.tangentW.w = vin.tangentL.w;

    vout.texU = vin.tex.x;
    vout.texV = vin.tex.y;

    vout.weights = vin.weights;
    /*
    vout.weights.x = vin.weights.x;
    vout.weights.y = vin.weights.y;
    vout.weights.z = vin.weights.z;
    vout.weights.w = vin.weights.w;
    //vout.weights.w = 1.0f - vout.weights.x - vout.weights.y - vout.weights.z;
    */

    vout.boneIds = vin.boneIds;
    vout.currBoneId = gCurrBoneId;

    return vout;
}
