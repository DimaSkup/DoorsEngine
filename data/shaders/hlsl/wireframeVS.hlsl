//==================================================================================
// Filename:  wireframeVS.hlsl
// Desc:      vertex shader for rendering wireframes
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"


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
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;
    vout.posH = mul(float4(vin.posL, 1.0), vin.world);
    vout.posH = mul(vout.posH, gViewProj);

    // a little offset towards us to prevent z-fighting of the line and already rendered geometry
    vout.posH.z -= 0.00001; 

    return vout;
}
