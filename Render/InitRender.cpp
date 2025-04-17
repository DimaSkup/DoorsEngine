// *********************************************************************************
// Filename:      InitRender.cpp
// Description:   implementation of functional for initialization 
//                of the Render module elements
// 
// Created:       30.08.24
// *********************************************************************************
#include "InitRender.h"
#include "Common/log.h"

#pragma warning (disable : 4996)

namespace Render
{

bool InitRender::InitializeShaders(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    ShadersContainer& shadersContainer,
    const DirectX::XMMATRIX& WVO)                 // world * base_view * ortho
{
    // initializes all the shader classes (color, texture, light, etc.)
    // and the HLSL shaders as well

    LogMsgf("\n");
    LogMsgf("%s%s", YELLOW, "---------------------------------------------------------");
    LogMsgf("%s%s", YELLOW, "                INITIALIZATION: SHADERS                  ");
    LogMsgf("%s%s", YELLOW, "---------------------------------------------------------");
    LogDbg("shaders initialization: start");

    try
    {
        bool result = false;
        char vsPath[64]{ '\0' };            // buffer for path to vertex shader
        char psPath[64]{ '\0' };            // buffer for path to pixel shader
        char gsPath[64]{ '\0' };            // buffer for path to geometry shader
        const char* pathToDir = "shaders/"; // path to direction with shader files (.cso)



        result = shadersContainer.colorShader_.Initialize(pDevice, "shaders/colorVS.cso", "shaders/colorPS.cso");
        Assert::True(result, "can't initialize the color shader class");

        result = shadersContainer.textureShader_.Initialize(pDevice, "shaders/textureVS.cso", "shaders/texturePS.cso");
        Assert::True(result, "can't initialize the texture shader class");

        result = shadersContainer.lightShader_.Initialize(pDevice, "shaders/LightVS.cso", "shaders/LightPS.cso");
        Assert::True(result, "can't initialize the light shader class");


        result = shadersContainer.fontShader_.Initialize(pDevice, WVO, "shaders/fontVS.cso", "shaders/fontPS.cso");
        Assert::True(result, "can't initialize the font shader class");

        result = shadersContainer.debugShader_.Initialize(pDevice, "shaders/DebugVS.cso", "shaders/DebugPS.cso");
        Assert::True(result, "can't initialize the debug shader");

        result = shadersContainer.skyDomeShader_.Initialize(pDevice, "shaders/SkyDomeVS.cso", "shaders/SkyDomePS.cso");
        Assert::True(result, "can't initialize the sky dome shader");


        result = shadersContainer.outlineShader_.Initialize(pDevice, "shaders/OutlineVS.cso", "shaders/OutlinePS.cso");
        Assert::True(result, "can't initialize the outline shader");

        result = shadersContainer.billboardShader_.Initialize(pDevice, "shaders/billboardVS.cso", "shaders/billboardPS.cso", "shaders/billboardGS.cso");
        Assert::True(result, "can't initialize the billboard shader");

        
        LogDbg("shaders initialization: finished successfully");
        LogMsgf("%s%s", YELLOW, "---------------------------------------------------------");
        LogMsg("\n");
    }
    catch (LIB_Exception& e) 
    {
        LogErr(e, true);
        return false;
    }

    return true;
}

} // namespace Render
