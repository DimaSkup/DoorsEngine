//==================================================================================
// Filename:  wireframeGS.hlsl
// Desc:      geometry shader for rendering wireframes
//==================================================================================

//---------------------------
// TYPEDEFS
//---------------------------
struct GS_IN
{
    float4 posH : SV_POSITION;
};

struct GS_OUT
{
    float4 posH : SV_POSITION;
};

//---------------------------
// GEOMETRY SHADER
//---------------------------
[maxvertexcount(6)]
void GS(triangle GS_IN gin[3], inout LineStream<GS_OUT> gout)
{
    GS_OUT v;

    // edge 0-1
    v.posH = gin[0].posH;  gout.Append(v);
    v.posH = gin[1].posH;  gout.Append(v);
    gout.RestartStrip();

    // edge 1-2
    v.posH = gin[1].posH;  gout.Append(v);
    v.posH = gin[2].posH;  gout.Append(v);
    gout.RestartStrip();

    // edge 2-0
    v.posH = gin[2].posH;  gout.Append(v);
    v.posH = gin[0].posH;  gout.Append(v);
}
