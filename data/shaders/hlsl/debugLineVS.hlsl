// =================================================================================
// Filename:  debugLineVS.hlsl
// Desc:      vertex shader for rendering debug shapes (AABB, OBB, spheres, etc.)
// =================================================================================
#include "const_buffers/cb_view_proj.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    float3   posW   : POSITION;       // vertex position in world space
    uint     color  : COLOR;
};

struct VS_OUT
{
    float4   posH   : SV_POSITION;    // homogeneous position
    uint     color  : COLOR;
};

//---------------------------
// Vertex Shader
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.posH  = mul(float4(vin.posW, 1.0f), gViewProj);
    vout.color = vin.color;

    return vout;
}
