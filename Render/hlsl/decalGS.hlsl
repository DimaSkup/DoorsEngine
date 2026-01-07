//==================================================================================
// Filename:  decalGS.hlsl
// Desc:      a geometry shader for 3d decals rendering
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
    float2 tex  : TEXCOORD;
};

//---------------------------
// GEOMETRY SHADER
//---------------------------
[maxvertexcount(4)]
void GS(
    
)
