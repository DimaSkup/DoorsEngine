//==================================================================================
// Filename:  tree_lod_GS.hlsl
// Desc:      geometry shader for generation a billboard 
//            which will serve us as a lod of some plant
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"

// how many different billboard images we have for this lod
#define NUM_BILLBOARD_VIEWS 16.0


//---------------------------
// TYPEDEFS
//---------------------------
struct GS_IN
{
    float4x4 material           : MATERIAL;
	matrix   world              : WORLD;

    float3   posW               : POSITION;       // origin position in world
    float3   fogColor           : COLOR;
	float    blend              : BLEND;         // blend factor between two billboards
    float2   sizeW              : TEXCOORD0;
	float2   stage              : TEXCOORD1;     // indices of left and right billboard images
};

struct GS_OUT
{
	float4x4 material          : MATERIAL;
    matrix   world             : WORLD;
	
    float4  posH               : SV_POSITION;
	float3  fogColor           : COLOR;          
	float   blend              : BLEND;          // blend factor between two billboards
    float3  posW               : POSITION;       // billboard corner point in world
    float2  tex0               : TEXCOORD0;
	float2  tex1               : TEXCOORD1;
    uint    primID             : SV_PrimitiveID;
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

	float stageL = gin[0].stage.x;
	float stageR = gin[0].stage.y;

    float2 tex0[4] =
    {
        float2(0, 1),   
        float2(0, 0),   
        float2(1, 1),   
        float2(1, 0),   
    };
	
	float2 tex1[4] =
    {
        float2(0, 1),   
        float2(0, 0),   
        float2(1, 1),   
        float2(1, 0),   
    };
	
	const float du = 1.0 / NUM_BILLBOARD_VIEWS;
	
	tex0[0].x = stageL * du;
	tex0[1].x = tex0[0].x;
	
	tex0[2].x = stageL * du + du;
	tex0[3].x = tex0[2].x;
	
	if (stageR >= NUM_BILLBOARD_VIEWS)
		stageR = 0.0;
	
	tex1[0].x = stageR * du;
	tex1[1].x = tex1[0].x;
	
	tex1[2].x = stageR * du + du;
	tex1[3].x = tex1[2].x;

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
		gout.blend              = gin[0].blend;
        gout.posW   			= v[i].xyz;
        gout.tex0   			= tex0[i];
		gout.tex1   			= tex1[i];
        gout.primID 			= primID;

        triStream.Append(gout);
    }
}
