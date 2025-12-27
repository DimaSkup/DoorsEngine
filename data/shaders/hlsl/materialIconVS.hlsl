#include "const_buffers/cbvs_world_and_view_proj.hlsli"
#include "const_buffers/cbvs_world_inverse_transpose.hlsli"
#include "types/vs_in_basic.hlsli"
#include "types/vs_out_basic.hlsli"


//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    // transform pos from local to world space
    vout.posW     = mul(float4(vin.posL, 1.0f), gWorld).xyz;

    // transform to homogeneous clip space
    vout.posH     = mul(float4(vout.posW, 1.0f), gViewProj);

    // transform normal and tangent to world space
    vout.normalW  = mul(vin.normalL, (float3x3)gWorldInvTranspose);
    vout.tangentW = mul(vin.tangentL, gWorldInvTranspose);

    vout.texU = vin.tex.x;
    vout.texV = vin.tex.y;

    return vout;
}
