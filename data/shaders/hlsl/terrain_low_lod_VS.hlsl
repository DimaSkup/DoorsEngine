//==================================================================================
// Desc:  a vertex shader for terrain rendering
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_weather.hlsli"


TextureCube  gCubeMap       : register(t0);
SamplerState gSkySampler    : register(s1);

//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    float3   posL       : POSITION;     // vertex position in local space
};

struct VS_OUT
{
    float4   posH       : SV_POSITION;  // homogeneous position
    float4   fogColor   : COLOR;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    const float4 skyMap = gCubeMap.SampleLevel(gSkySampler, float3(0, -490, 0), 0);

    VS_OUT vout;

    // transform to homogeneous clip space
    vout.posH = mul(float4(vin.posL, 1.0), gViewProj);

    // blend sky texture pixel with fixed fog color
    vout.fogColor = skyMap * float4(gFixedFogColor, 1.0);

    return vout;
}
