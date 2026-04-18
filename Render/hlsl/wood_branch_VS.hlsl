//==================================================================================
// Filename:  wood_branch_VS.hlsl
// Desc:      vertex shader for rendering a tree's wood material
//==================================================================================
#include "helpers/noise_rand.hlsli"
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "types/vs_in_basic_inst.hlsli"
#include "types/vs_out_basic_inst.hlsli"


//---------------------------
// calculate swaying by wind
//---------------------------
void CalcWindSwaying(const float3 vOrigToVert, inout float3 posW)
{
    float swayMag = dot(vOrigToVert.xz, vOrigToVert.xz);
	
	// prevent swaying of branches near tree bark
	if (swayMag < 0.25)
		return;
	
	// significantly reduce sway magnitude
    swayMag *= 0.03;

    const float swayFactor = sin(gGameTime + noise(posW)) * swayMag;

	// apply a displacement by wind to vertex in world 
    posW += (gWindDir * swayFactor);
}

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.material = vin.material;

    // transform pos from local to world space
    vout.posW = mul(float4(vin.posL, 1.0), vin.world).xyz;

	
	// use vector from origin (0,0,0) to vertex position:
	// - for branch swaying
	const float3 vOrigToVert = mul(vin.posL, (float3x3)vin.world);
	
	CalcWindSwaying(vOrigToVert, vout.posW);
	
    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

    // interpolating normal can unnormalize it, so normalize it
    vout.normalW = mul(vin.normalL, (float3x3)vin.world);

    // calculate the tangent and normalize it
    vout.tangentW = mul(vin.tangentL, vin.world);

    vout.texU = vin.tex.x;
    vout.texV = vin.tex.y;

    return vout;
}
