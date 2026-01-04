//==================================================================================
// Desc:  a vertex shader used for terrain's depth pre-pass
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"

//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    float3 posL  : POSITION;     // vertex position in local space
};

struct VS_OUT
{
    float4 posH  : SV_POSITION;  // homogeneous position
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    // transform to homogeneous clip space
    vout.posH = mul(float4(vin.posL, 1.0), gViewProj);

    return vout;
}
