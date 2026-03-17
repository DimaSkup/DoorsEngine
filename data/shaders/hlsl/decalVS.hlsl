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
    float3 posW         : POSITION;   // position in world space
    float2 tex          : TEXCOORD;
    float3 normW        : NORMAL;     // normal vec in world space
    float  translucency : TRANSLUCENCY;
};

struct VS_OUT
{
    float4 posH         : SV_POSITION;
    float3 posW         : POSITION;
    float2 tex          : TEXCOORD;
    float3 normW        : NORMAL;
    float  translucency : TRANSLUCENCY;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.posH  = mul(float4(vin.posW, 1.0), gViewProj);
    vout.posW  = vin.posW;
    vout.tex   = vin.tex;
    vout.normW = vin.normW;
    vout.translucency = vin.translucency;

    return vout;
}
