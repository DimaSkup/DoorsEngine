//==================================================================================
// Filename:  cbvs_world_view_proj.hlsli
// Desc:      vertex shaders' const buffer for world * view * projection matrix
//==================================================================================

cbuffer cbWVP : register(b1)
{
    matrix gWorldViewProj;
};
