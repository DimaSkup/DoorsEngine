//==================================================================================
// Filename:  cb_view_proj.hlsli
// Desc:      COMMON const buffer, container for view * projection matrix
//==================================================================================

cbuffer cbViewProj : register(b9)
{
    matrix gViewProj;
};
