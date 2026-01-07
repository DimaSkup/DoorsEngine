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
        MaterialColors     matColors;
    };

    __declspec(align(16)) struct InstancedDataBillboards
    {
        MaterialColors    matColors;
        DirectX::XMFLOAT3 posW;        // billboard position in world
        DirectX::XMFLOAT2 size;        // billboard size
    };

    // ----------------------------------------------------

    struct Debug
    {
        uint currBoneId = 0;
    };


    // ----------------------------------------------------

    // container for different weather params (is used in different shader stages: VS, GS)
    struct Weather
    {
        DirectX::XMFLOAT3 windDir   = { 0.707f, 0.0f, 0.707f };  // normalized direction of wind
        float windSpeed             = 1.0f;
        float waveAmplitude         = 0.3f;   // control how far blades bend
        float windStrength          = 2.0f;   // control how far blades bend
        float turbulence            = 0.5f;   // adds jitter / small ripples (noise)
        float gustDecay             = 0.5f;
        float gustPower             = 0.1f;      
        float waveFrequency         = 0.5f;   // controls wavelength; higher = faster small waves
        float bendScale             = 0.5f;
        float swayDistance          = 30.0f;  // from us (camera) and to this distance the swaying is linearly goes down
                                              // after this distance we see no swaying

        DirectX::XMFLOAT3  fogColor = { 0.5f, 0.5f, 0.5f };
        float              fogStart = 15.0f;                 // how far from camera the fog starts?

        DirectX::XMFLOAT3  skyColorCenter = { 1,1,1 };       // near horizon
        float              fogRange = 250.0f;          // how far from camera the object is fully fogged?

        DirectX::XMFLOAT3  skyColorApex = { 1,1,1 };       // top color
    };

    // ----------------------------------------------------

    // container for bone transformation matrices which are used for model skinning (animation)
    struct Skinned
    {
        DirectX::XMMATRIX boneTransforms[MAX_NUM_BONES_PER_CHARACTER];
    };

    // ----------------------------------------------------

    // 2D sprite
    struct Sprite
    {
        float left   = 0;
        float top    = 0;
        float width  = 100;
        float height = 100;
    };

    // ----------------------------------------------------

    struct Sky
    {
        float cloud1TranslationX = 0;
        float cloud1TranslationZ = 0;
        float cloud2TranslationX = 0;
        float cloud2TranslationZ = 0;
        float brightness = 1;
        float padding0;
        float padding1;
        float padding2;
    };

    // ----------------------------------------------------

    struct CameraParams
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX invView;
        DirectX::XMMATRIX invProj;

        DirectX::XMFLOAT3 cameraPosW = { 0,0,0 };
        float nearPlane = 0.01f;
        float farPlane  = 100.0f;
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


    struct WorldInvTranspose
    {
        DirectX::XMMATRIX worldInvTranspose = DirectX::XMMatrixIdentity();
    };


    struct Position
    {
        DirectX::XMFLOAT3 posW = {-1,-1,-1};
    };

    struct Time
    {
        float deltaTime = 0;
        float gameTime  = 0;
        float pad0;
        float pad1;
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
        int                currNumDirLights   = 1;   // current number of directed lights 
        int                currNumPointLights = 0;   // current number of point lights
        int                currNumSpotLights  = 0;   // current number of spotlights
    };

    // ----------------------------------------------------

    struct cbpsRareChanged
    {
        // a structure for pixel shader data which is rarely changed
        int                debugType      = 0;               // (int type in shader) by default render as usual
        int                fogEnabled       = true;          // turn on/off the fog effect     
        int                turnOnFlashLight = false;         // turn on/off the flashlight
        int                alphaClipping    = false;         // do we use alpha clipping?
    };

    // ----------------------------------------------------

    struct ConstBuf_FontPixelColor
    {
        DirectX::XMFLOAT3 pixelColor = {1,0,0};
        float padding = 0.0f;
    };

    // ----------------------------------------------------

    struct PostFx
    {
        float data[256];       // for meaning of each index look at enum ePostFxParam
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
        float distGrassFullSize = 0;    // distance around camera where grass planes are in full size
        float distGrassVisible  = 40;   // after this distance we don't see any grass planes
        float pad0 = NAN;
        float pad1 = NAN;
    };


};


} // namespace
