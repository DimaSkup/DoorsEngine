//==================================================================================
// Desc:   a vertex shader for fullscreen rendering of a quad
//==================================================================================

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

VS_OUT VS(uint id : SV_VertexID)
{
    // generate fullscreen triangle in clip space

    float2 verts[3] = {
        float2(-1, -1),
        float2(-1,  3),
        float2( 3, -1)
    };

    VS_OUT vout;
    vout.posH = float4(verts[id], 0, 1);

    // prevent NDC Y-axis mismatch
    vout.tex   = 0.5f * (verts[id] + 1.0);
    vout.tex.y = 1.0f - vout.tex.y;

    return vout;
}
