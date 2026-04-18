//==================================================================================
// Filename:  fooliageVS.hlsl
// Desc:      vertex shader for foliage rendering
//==================================================================================
#include "helpers/fog.hlsli"
#include "helpers/noise_rand.hlsli"

#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cb_weather.hlsli"

#include "types/vs_in_basic_inst.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_OUT
{
    float4x4 material           : MATERIAL;
    float4   posH               : SV_POSITION;  // homogeneous position
    float3   posW               : POSITION;     // position in world
	float    texU               : TEXCOORD0;
    float3   origToPosL         : NORMAL0;      // vec origin to local pos
    float3   normalW            : NORMAL1;      // normal in world
	float    texV               : TEXCOORD1;
    float4   tangentW           : TANGENT;      // tangent in world
    float3   fogColor           : COLOR;
};

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

    const float swayFactor = sin(gGameTime * gWindSpeed + noise(posW)) * swayMag;

	// apply a displacement by wind to vertex in world 
    posW += (gWindDir * swayFactor);
}

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

	vout.fogColor = GetFogColor();
	
    vout.material = vin.material;

    // transform pos from local to world space
    vout.posW = mul(float4(vin.posL, 1.0), vin.world).xyz;

	// use vector from origin (0,0,0) to vertex position:
	// - for branch swaying
	// - for prettier lighting (normals are looking outside of tree)
	const float3 vOrigToVert = mul(vin.posL, (float3x3)vin.world);

	
	CalcWindSwaying(vOrigToVert, vout.posW);
	
    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0), gViewProj);
	
	vout.origToPosL = float3(vOrigToVert.x, 2, vOrigToVert.z);
    vout.normalW    = mul(vin.normalL, (float3x3)vin.world);

    // calculate the tangent
    vout.tangentW = mul(vin.tangentL, vin.world);

    vout.texU = vin.tex.x;
    vout.texV = vin.tex.y;

    return vout;
}
