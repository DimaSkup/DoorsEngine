//==================================================================================
// Desc:  vertex shader for rendering trees of LOD2
//        (only directed lighting + light map)
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "const_buffers/cb_camera.hlsli"


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
    float4   posH       : SV_POSITION;    // homogeneous position
    float3   matAmbient : COLOR0;
    float    texU       : TEXCOORD0;
    float3   matDiffuse : COLOR1;
    float    texV       : TEXCOORD1;
    float3   posWL      : NORMAL;         // transformed position but translated back to local space
    float    fogLerp    : TEXCOORD2;
    float3   fogColor   : COLOR2;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.matAmbient = vin.material[0].xyz;
    vout.matDiffuse = vin.material[1].xyz;

    // transform pos from local to world space
    float3 posW   = mul(float4(vin.posL, 1.0), vin.world).xyz;
    
    // transform to homogeneous clip space
    vout.posH     = mul(float4(posW, 1.0), gViewProj);

    vout.texU     = vin.tex.x;
    vout.texV     = vin.tex.y;
    
    // normalize a vector from origin to vertex (vertex is translated back to local space)
    const float3 posOffset      = float3(vin.world[3][0], vin.world[3][1], vin.world[3][2]);
    vout.posWL                  = normalize(posW - posOffset);

    // blend sky bottom pixel color with fixed fog color
    const float3 skyBottomColor = gCubeMap.SampleLevel(gSkySampler, float3(0, -490, 0), 0).xyz;
    vout.fogColor               = skyBottomColor * gFixedFogColor;

    // calculate the fog interpolation factor
    const float distToEye       = length(gCamPosW - posW);
    vout.fogLerp                = saturate((distToEye - gFogStart) / gFogRange);
    
    
    return vout;
}
