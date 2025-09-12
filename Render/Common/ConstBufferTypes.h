// ================================================================================================
// Filename:      ConstBufferTypes.h
// Description:   contains declarations of constant buffer types for shaders;
// 
// Created:       28.11.24
// ================================================================================================
#pragma once
#include <DirectXMath.h>
#include "MaterialLightTypes.h"

namespace Render
{

// =================================================================================
// ENUMS
// =================================================================================
enum eDebugState
{
    DBG_TURN_OFF,               // turn off the debug shader and use the default shader
    DBG_SHOW_NORMALS,
    DBG_SHOW_TANGENTS,
    DBG_SHOW_BUMPED_NORMALS,
    DBG_SHOW_ONLY_LIGTHING,
    DBG_SHOW_ONLY_DIRECTED_LIGHTING,
    DBG_SHOW_ONLY_POINT_LIGHTING,
    DBG_SHOW_ONLY_SPOT_LIGHTING,
    DBG_SHOW_ONLY_DIFFUSE_MAP,
    DBG_SHOW_ONLY_NORMAL_MAP,
    DBG_WIREFRAME,
    DBG_SHOW_MATERIAL_AMBIENT,
    DBG_SHOW_MATERIAL_DIFFUSE,
    DBG_SHOW_MATERIAL_SPECULAR,
    DBG_SHOW_MATERIAL_REFLECTION,
};

namespace ConstBufType
{
    struct InstancedData
    {
        DirectX::XMMATRIX  world;
        DirectX::XMMATRIX  worldInvTranspose;
        MaterialColors     matColors;
    };

    __declspec(align(16)) struct InstancedDataBillboards
    {
        MaterialColors    matColors;
        DirectX::XMFLOAT3 posW;        // billboard position in world
        DirectX::XMFLOAT2 size;        // billboard size
    };

    // ----------------------------------------------------

    struct DebugMode
    {
        int debugType = eDebugState::DBG_SHOW_NORMALS;
    };


    //===============================================================

    struct ViewProj
    {
        // a structure for vertex shader data which is changed each frame
        DirectX::XMMATRIX  viewProj;
    };


    struct WorldAndViewProj
    {
        DirectX::XMMATRIX world    = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX viewProj = DirectX::XMMatrixIdentity();
    };


    struct WorldViewProj
    {
        DirectX::XMMATRIX worldViewProj = DirectX::XMMatrixIdentity();
    };


    struct WorldViewOrtho
    {
        DirectX::XMMATRIX worldViewOrtho = DirectX::XMMatrixIdentity();
    };

    struct Position
    {
        DirectX::XMFLOAT3 posW = {-1,-1,-1};
    };

    //===============================================================


    struct MaterialData
    {
        // material data for the pixel shader
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;
        DirectX::XMFLOAT4 reflect;
    };

    struct cbpsPerFrame
    {
        // a structure for pixel shader data which is changed each frame

        DirLight           dirLights[3];
        PointLight         pointLights[25];
        SpotLight          spotLights[25];
        DirectX::XMFLOAT3  cameraPos          = {0,0,0};
        float              time               = 0;         // game time
        int                currNumPointLights = 0;
        int                currNumSpotLights  = 0;
    };

    // ----------------------------------------------------

    struct cbpsRareChanged
    {
        // a structure for pixel shader data which is rarely changed

        DirectX::XMFLOAT3  fogColor = { 0.5f, 0.5f, 0.5f };
        float              fogStart = 15.0f;               // how far from camera the fog starts?
        float              fogRange = 250.0f;              // how far from camera the object is fully fogged?

        int                numOfDirLights = 1;           // current number of directional light sources

        int                fogEnabled = true;        // turn on/off the fog effect     
        int                turnOnFlashLight = false;       // turn on/off the flashlight
        int                alphaClipping = false;       // do we use alpha clipping?

        // sky dome params
        DirectX::XMFLOAT3 colorCenter_{ 1,1,1 };
        float             padding1_ = 1.0f;
        DirectX::XMFLOAT3 colorApex_{ 1,1,1 };
        float             padding2_ = 1.0f;
    };

    // ----------------------------------------------------



    struct ConstBuf_FontPixelColor
    {
        DirectX::XMFLOAT3 pixelColor = {1,0,0};
        float padding = 0.0f;
    };


    // =======================================================
    // const buffers for GEOMETRY shader
    // =======================================================
    struct GS_PerFrame
    {
        DirectX::XMMATRIX viewProj;
        DirectX::XMFLOAT3 cameraPosW;
        float time = 0;
    };

    struct GrassParams
    {
        float distGrassFullSize = 0;   // distance around camera where grass planes are in full size
        float distGrassVisible  = 40;   // after this distance we don't see any grass planes
    };
};


} // namespace
