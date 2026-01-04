//==================================================================================
// Desc:  a vertex shader for terrain rendering
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
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
    // data per vertex
    float3   posL       : POSITION;     // vertex position in local space
    float2   tex        : TEXCOORD;
    float3   normalL    : NORMAL;       // vertex normal in local space
};

struct VS_OUT
{
    float4   posH       : SV_POSITION;  // homogeneous position
    float3   posW       : POSITION;     // position in world
    float    texU       : TEXCOORD0;
    float3   normalW    : NORMAL;       // normal in world
    float    texV       : TEXCOORD1;
	float4   fogColor   : COLOR0;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    // transform pos from local to world space
    vout.posW = vin.posL;

    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

    // interpolating normal
    vout.normalW = vin.normalL;

    // output vertex texture attributes for interpolation across triangle
	vout.texU = vin.tex.x;
	vout.texV = vin.tex.y;
    
	const float4 skyMap = gCubeMap.SampleLevel(gSkySampler, float3(0, -490, 0), 2);
	        
	// blend sky texture pixel with fixed fog color
    vout.fogColor = skyMap * float4(gFixedFogColor, 1.0f);

    return vout;
}
