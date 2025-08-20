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

    struct cbvsPerFrame
    {
        // a structure for vertex shader data which is changed each frame
        DirectX::XMMATRIX  viewProj;
    };

    struct Position
    {
        DirectX::XMFLOAT3 posW;
    };

    // ----------------------------------------------------

    struct WorldViewProj
    {
        DirectX::XMMATRIX world    = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX viewProj = DirectX::XMMatrixIdentity();
    };

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
    };

    // ----------------------------------------------------

    
    struct cbvsPerFrame_SkyDome
    {
        // const buffer data structure for the VS of the sky dome shader
        DirectX::XMMATRIX worldViewProj_ = DirectX::XMMatrixIdentity();
    };

    struct cbpsRareChanged_SkyDome
    {
        // const buffer data structure for the PS of the sky dome shader

        DirectX::XMFLOAT3 colorCenter_{ 1,1,1 };
        float             padding1_ = 1.0f;
        DirectX::XMFLOAT3 colorApex_{ 1,1,1 };
        float             padding2_ = 1.0f;
    };



    // ----------------------------------------------------
    
    struct ConstantMatrixBuffer_FontVS
    {
        // a constant matrix buffer structure for the font vertex shader
        DirectX::XMMATRIX worldViewProj;
    };

    struct ConstantPixelBuffer_FontPS
    {
        // a constant buffer which contains colours are used inside the font pixel shader
        DirectX::XMFLOAT3 pixelColor;         // UI text colour
        float padding;
    };


    // =======================================================
    // const buffers for GEOMETRY shader
    // =======================================================
    struct GeometryShaderConstBuf_PerFrame
    {
        DirectX::XMMATRIX viewProj;
        DirectX::XMFLOAT3 cameraPosW;
    };

    struct GeomertyShaderConstBuf_PerObject
    {
        MaterialColors    matColors;
    };

    struct GeometryShaderConstBuf_Fixed
    {
        DirectX::XMFLOAT2 topLeft;       // 0 1
        DirectX::XMFLOAT2 bottomLeft;    // 0 0
        DirectX::XMFLOAT2 topRight;      // 1 1
        DirectX::XMFLOAT2 bottomRight;   // 1 0
    };
};


} // namespace
