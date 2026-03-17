//==================================================================================
// const buffer for vertex shaders: container for world inverse transposed matrix
//==================================================================================

cbuffer cbWorldInvTranspose : register(b3)
{
    matrix gWorldInvTranspose;
}
