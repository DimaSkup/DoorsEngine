//==================================================================================
// Filename:  decalVS.hlsl
// Desc:      a vertex shader for 3d decals rendering
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"

//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    float3 posW : POSITION;   // decal's vertex is already in the world space
    float2 tex  : TEXCOORD;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.posH = mul(float4(vin.posW, 1.0), gViewProj);

    vout.tex = vin.tex;

    return vout;
}
