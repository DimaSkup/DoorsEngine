//==================================================================================
// Filename:  tree_lod_GS.hlsl
// Desc:      geometry shader for generation a billboard 
//            which will serve us as a lod of some plant
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"


//---------------------------
// TYPEDEFS
//---------------------------
struct GS_IN
{
    float4x4 material   : MATERIAL;
	matrix   world      : WORLD;
    float3   posW       : POSITION;       // origin position in world
    float4   fogColor   : COLOR;
    float2   sizeW      : TEXCOORD0;
};

struct GS_OUT
{
	float4x4 material   : MATERIAL;
	matrix   world      : WORLD;
    float4   posH    	: SV_POSITION;
	float4   fogColor	: COLOR;
    float3   posW    	: POSITION;         // billboard corner point in world
	float3   normal     : NORMAL;
    float2   tex     	: TEXCOORD0;
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

	const int numBillboardViews = 16;
	const float inv = 1.0 / numBillboardViews;
	
	// billboard image index is stored in w-component of fog
	const float stage = gin[0].fogColor.w;
	
	const float tu0 = stage * inv;
	const float tu1 = tu0 + inv;
	
    float2 tex[4] =
    {
        float2(tu0, 1),   
        float2(tu0, 0),   
        float2(tu1, 1),   
        float2(tu1, 0),   
    };

    // compute the local coordinate system of the billboard relative to the world space
    // such that the billboard is aligned to the Y-axis and faces the eye
    const float3 up     = float3(0, 1, 0);
    const float3 look   = normalize(gCamPosW - gin[0].posW);
    const float3 right  = cross(up, look);


    // compute billboard's triangles in world space (size is encoded
    const float3 halfW  = (0.5 * gin[0].sizeW.x) * right;
    const float3 height = float3(0, gin[0].sizeW.y, 0);

    float4 v[4];
    v[0] = float4(gin[0].posW + halfW,          1.0);  // BR
    v[1] = float4(gin[0].posW + halfW + height, 1.0);  // TR
    v[2] = float4(gin[0].posW - halfW,          1.0);  // BL
    v[3] = float4(gin[0].posW - halfW + height, 1.0);  // TL


    // transform quad vertices to clip space and output them as a triangle strip
    GS_OUT gout;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
		gout.material          	= gin[0].material;
		gout.world              = gin[0].world;
        gout.posH   			= mul(v[i], gViewProj);
		gout.fogColor 			= gin[0].fogColor;
        gout.posW   			= v[i].xyz;
		gout.normal             = -look;
        gout.tex    			= tex[i];

        triStream.Append(gout);
    }
}
