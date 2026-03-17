//==================================================================================
// Filename:  cb_camera.hlsli
// Desc:      COMMON const buffer, container for camera params
//==================================================================================

cbuffer cbCameraParams : register(b11)
{
    matrix gView;
    matrix gProj;
    matrix gInvProj;
    matrix gInvView;
    float3 gCamPosW;     // camera position in world
    float  gNearZ;
    float  gFarZ;
}
