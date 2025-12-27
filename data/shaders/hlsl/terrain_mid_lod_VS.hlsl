//==================================================================================
// Desc:  a vertex shader for terrain rendering
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_weather.hlsli"


TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);
Texture2D    gSplatMap      : register(t113);

SamplerState gSkySampler    : register(s1);
SamplerState gSamLinearWrap : register(s3);

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
	float4   splatMap   : COLOR0;
	float4   fogColor   : COLOR1;
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
    vout.posH = mul(float4(vout.posW, 1.0), gViewProj);

    // interpolating normal
    vout.normalW = vin.normalL;

    // output vertex texture attributes for interpolation across triangle
    vout.texU = vin.tex.x;
    vout.texV = vin.tex.y;

	// sample splat map per vertex
	vout.splatMap = gSplatMap.SampleLevel(gSamLinearWrap, vin.tex, 2);
	
	// blend sky texture pixel with fixed fog color
	const float4 skyMap = gCubeMap.SampleLevel(gSkySampler, float3(0, -490, 0), 0);
	vout.fogColor       = skyMap * float4(gFixedFogColor, 1.0);

    return vout;
}
