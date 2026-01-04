// =================================================================================
// Filename:     billboardGS.hlsl
// Description:  a geometry shader (GS) to expand a point sprite into a y-axis
//               aligned billboard that faces the camera
// =================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct GS_IN
{
    float3   posW          : POSITION;       // particle (point) center position in world
    float    translucency  : TRANSLUCENCY;
    float3   color         : COLOR;
    float2   sizeW         : SIZE;           // width and height of the particle
};

struct GS_OUT
{
    float4   posH          : SV_POSITION;
    float3   posW          : POSITION;       // particle center position in world
    float    translucency  : TRANSLUCENCY;
    float3   color         : COLOR;
    float    normalX       : NORMAL_X;
    float2   tex           : TEXCOORD;
    float    normalY       : NORMAL_Y;
    float    normalZ       : NORMAL_Z;
    uint     primID        : SV_PrimitiveID;
};


//---------------------------
// GEOMETRY SHADER
//---------------------------
[maxvertexcount(4)]
void GS(
    point GS_IN gin[1],
    uint primID : SV_PrimitiveID,
    inout TriangleStream<GS_OUT> triStream)
{
    // we expand each point into a quad (4 vertices), so the maximum number
    // of vertices we output per geometry shader invocation is 4

    float2 texC[4] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f)
    };

    // compute the local coordinate system of the sprite relative to the world space
    // such that the billboard is aligned to the y-axis and faces the eye
    float3 up       = float3(0.0f, 1.0f, 0.0f);
    float3 look     = normalize(gCamPosW - gin[0].posW);
    float3 rightVec = normalize(cross(up, look));
    
    up = cross(look, rightVec);

    
    // compute triangle strip vertices (quad) in world space
    float halfWidth  = 0.5f * gin[0].sizeW.x;
    float halfHeight = 0.5f * gin[0].sizeW.y;

    
    float4 v[4];
    v[0] = float4(gin[0].posW + halfWidth * rightVec - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].posW + halfWidth * rightVec + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].posW - halfWidth * rightVec - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].posW - halfWidth * rightVec + halfHeight * up, 1.0f);

    
    // transform quad vertices to clip space and output them as a triangle strip
    GS_OUT gout;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.posH           = mul(v[i], gViewProj);
        gout.posW           = v[i].xyz;
        gout.translucency   = gin[0].translucency;
        gout.color          = gin[0].color;
        gout.normalX        = look.x;
        gout.tex            = texC[i];
        gout.normalY        = look.y;
        gout.normalZ        = look.z;
        gout.primID         = primID;

        triStream.Append(gout);
    }
}
