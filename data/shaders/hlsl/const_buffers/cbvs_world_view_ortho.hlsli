//==================================================================================
// Filename:  cbvs_world_view_ortho.hlsli
// Desc:      vertex shaders' const buffer for world * basic_view * ortho matrix
//==================================================================================

cbuffer PerFrameBuffer : register(b2)
{
    matrix gWorldViewOrtho;
};
