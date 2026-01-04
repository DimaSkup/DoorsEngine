//==================================================================================
// Desc:  a vertex shader for 2D sprites rendering
//==================================================================================
#include "const_buffers/cbvs_sprite.hlsli"
#include "const_buffers/cbvs_world_view_ortho.hlsli"

//---------------------------
// TYPEDEFS
//---------------------------
struct VS_OUT
{
    float2 pos  : POSITION;
    float2 size : SIZE;
    matrix WVO  : WORLD_VIEW_ORTHO;
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(uint id : SV_VertexID)
{
    VS_OUT vout;

    vout.pos.x  = gSpriteLeft;
    vout.pos.y  = gSpriteTop;
    vout.size.x = gSpriteWidth;
    vout.size.y = gSpriteHeight;
    vout.WVO    = gWorldViewOrtho;

    return vout;
}
