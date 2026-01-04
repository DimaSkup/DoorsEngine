//==================================================================================
// Filename:  woodVS.hlsl
// Desc:      vertex shader for rendering a wood material
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "types/vs_in_basic_inst.hlsli"
#include "types/vs_out_basic_inst.hlsli"


//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.material = vin.material;

    // transform pos from local to world space
    vout.posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

/*
    // --- branch swaying ---
    const float2 localPosXZ = mul(vin.posL, (float3x3)vin.world).xz;

    //float swayFactor  = dot(localPosXZ, localPosXZ);
    swayFactor *= (0.03 * (swayFactor > 0.25));

    float sway = sin(gGameTime + vout.posW.x + vout.posW.z) * swayFactor;
    float3 windDir = float3(0.707, 0, 0.707);
    float3 disp = windDir * sway;

    vout.posW += disp;
	*/

    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

    // interpolating normal can unnormalize it, so normalize it
    vout.normalW = mul(vin.normalL, (float3x3)vin.worldInvTranspose);

    // calculate the tangent and normalize it
    vout.tangentW = mul(vin.tangentL, vin.world);

    vout.texU = vin.tex.x;
    vout.texV = vin.tex.y;

    return vout;
}
