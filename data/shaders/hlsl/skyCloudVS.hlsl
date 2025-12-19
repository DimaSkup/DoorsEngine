// =================================================================================
// Filename:  skyCloudVS.hlsl
// Desc:      vertex shader for clouds rendering
// =================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    float3 posL  : POSITION;     // vertex position in local space
    float2 tex   : TEXCOORD;
};

struct VS_OUT
{
    float4 posH  : SV_POSITION;       // vertex position in homogeneous space
    float3 posW  : POSITION;          // vertex position in world space
    float2 tex   : TEXCOORD;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.posW = vin.posL + gCamPosW;

    vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);
    vout.posH.z = vout.posH.w;

    vout.tex = vin.tex;

    return vout;
}
