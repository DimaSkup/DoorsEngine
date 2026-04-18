//==================================================================================
// Desc:  a vertex shader for grass rendering
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "helpers/fog.hlsli"

//--------------------------------
// TYPEDEFS
//--------------------------------
struct VS_IN
{
    float3 posW         : POSITION;
    float2 tex          : TEXCOORD;
    float3 normal       : NORMAL;
};

struct VS_OUT
{
    float4 posH         : SV_POSITION;
    float3 posW         : POSITION;
    float2 tex          : TEXCOORD;
    float3 normal       : NORMAL;
    float3 fogColor     : COLOR;
};

//--------------------------------
// VERTEX SHADER
//--------------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.fogColor = GetFogColor();

    vout.posW = vin.posW;
    
    // transform to homogeneous clip space
    vout.posH = mul(float4(vin.posW, 1.0), gViewProj);

    // transform normal vector
    vout.normal = vin.normal;

    // pass texture coords
    vout.tex = vin.tex;
    
    return vout;
}
