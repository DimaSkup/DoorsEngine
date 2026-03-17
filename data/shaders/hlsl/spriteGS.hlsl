
#include "const_buffers/cb_view_proj.hlsli"

//---------------------------
// TYPEDEFS
//---------------------------
struct GS_IN
{
    float2 pos  : POSITION;
    float2 size : SIZE;
    matrix WVO  : WORLD_VIEW_ORTHO;
};

struct GS_OUT
{
    float4 posH   : SV_POSITION;
    float2 tex    : TEXCOORD;
    uint   primID : SV_PrimitiveID;
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

    const float2 texC[4] =
    {
        float2(1,0),
        float2(1,1),
        float2(0,0),
        float2(0,1)
    };

    const float width = gin[0].size.x;
    const float height = gin[0].size.y;



    float2 pos;
    pos.x = (float)(-(1600 >> 1) + gin[0].pos.x);   // posX
    pos.y = (float)(+(900 >> 1) - gin[0].pos.y);   // posY

    // left, right, top, bottom
    float l = pos.x;
    float r = l + gin[0].size.x;
    float t = pos.y;
    float b = t - gin[0].size.y;


    /*
        2D screen coordinates:

         *------- +Y --------*
         |                   |
        -x        0,0       +x
         |                   |
         *------- -Y --------*
    */

    float4 v[4];
    v[0] = float4(r, t, 0, 1);
    v[1] = float4(r, b, 0, 1);
    v[2] = float4(l, t, 0, 1);
    v[3] = float4(l, b, 0, 1);

    /*
    v[0] = float4(200, 100, 0, 1);
    v[1] = float4(200, -100, 0, 1);
    v[2] = float4(-200, 100, 0, 1);
    v[3] = float4(-200, -100, 0, 1);
    */

    /*
    v[0] = float4(pos.x + width, pos.y - height, 0.0, 1.0);  // BR
    v[1] = float4(pos.x + width, pos.y,          0.0, 1.0);  // UR
    v[2] = float4(pos.x,         pos.y,          0.0, 1.0);  // UL
    v[3] = float4(pos.x,         pos.y + height, 0.0, 1.0);  // BL
*/

    GS_OUT gout;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.posH = mul(v[i], gin[0].WVO);
        gout.tex = texC[i];
        gout.primID = primID;

        triStream.Append(gout);
    }
}
