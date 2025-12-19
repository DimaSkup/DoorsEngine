// =================================================================================
// Filename:     SkyDomeVS.hlsl
// Description:  vertex shader for sky rendering
// =================================================================================
#include "const_buffers/cbvs_world_view_proj.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_INPUT
{ 
	float3 posL   : POSITION;       // position of the vertex in local space
};

struct VS_OUTPUT
{
	float4 posH   : SV_POSITION;    // homogeneous position
	float3 posL   : POSITION;       // position of the vertex in local space
};

//---------------------------
// Vertex Shader
//---------------------------
VS_OUTPUT VS(VS_INPUT vin)
{
	VS_OUTPUT vout;

	// set z = w so that z/w = 1 (i.e., skydome always on far plane)
	vout.posH = mul(float4(vin.posL, 1.0f), gWorldViewProj).xyww;

	// use local vertex position as cubemap lookup vector
	vout.posL = vin.posL;

	return vout;
}
