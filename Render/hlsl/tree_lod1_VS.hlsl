//==================================================================================
// Desc:  a vertex shader which is used as default (texturing + lighting)
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cb_weather.hlsli"


//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
SamplerState gSkySampler    : register(s1);

//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    // data per instance
    row_major matrix   world             : WORLD;
    row_major float4x4 material          : MATERIAL;
    uint               instanceID        : SV_InstanceID;

    // data per vertex
    float3   posL                        : POSITION;       // vertex position in local space
    float2   tex                         : TEXCOORD;
};

struct VS_OUT
{
    float4x4 material          : MATERIAL;
    matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;

    float4   posH       : SV_POSITION;    // homogeneous position
    float3   posW       : POSITION;       // position in world
    float2   tex        : TEXCOORD;
	float4   fogColor   : COLOR;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.material          = vin.material;
    vout.worldInvTranspose = vin.world;

    // transform pos from local to world space
    vout.posW = mul(float4(vin.posL, 1.0), vin.world).xyz;

    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0), gViewProj);

    vout.tex = vin.tex;
	
	// blend sky pixel color with fixed fog color
	const float4 skyBottomColor = gCubeMap.SampleLevel(
		gSkySampler, 
		float3(0, -490, 0),
		0);
    vout.fogColor = skyBottomColor * float4(gFixedFogColor, 1.0);


    return vout;
}
