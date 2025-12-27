//==================================================================================
// const buffer for vertex shaders: container for world + view * proj matrix
//==================================================================================

cbuffer cbWorldAndViewProj : register(b0)
{
    matrix gWorld;
    matrix gViewProj;
}
