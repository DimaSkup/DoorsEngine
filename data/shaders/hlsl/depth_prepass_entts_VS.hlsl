//==================================================================================
// Desc:  a vertex shader used for entities depth pre-pass 
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    // data per instance
    row_major matrix world       : WORLD;
    uint             instanceID  : SV_InstanceID;

    // data per vertex
    float3           posL        : POSITION;       // vertex position in local space
};

struct VS_OUT
{
    float4 posH  : SV_POSITION;    // homogeneous position
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    // transform pos from local to world space
    float3 posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

    // transform to homogeneous clip space
    vout.posH = mul(float4(posW, 1.0f), gViewProj);

    return vout;
}
