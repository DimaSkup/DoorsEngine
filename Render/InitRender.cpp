// *********************************************************************************
// Filename:      InitRender.cpp
// Description:   implementation of functional for initialization 
//                of the Render module elements
// 
// Created:       30.08.24
// *********************************************************************************
#include "InitRender.h"
#include <log.h>
#include <CAssert.h>

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

    SetConsoleColor(YELLOW);
    LogMsg("\n");
    LogMsg("---------------------------------------------------------");
    LogMsg("                INITIALIZATION: SHADERS                  ");
    LogMsg("---------------------------------------------------------");
    LogDbg(LOG, "shaders initialization: start");

    try
    {
        bool result = false;
        //char vsPath[64]{ '\0' };            // buffer for path to vertex shader
        //char psPath[64]{ '\0' };            // buffer for path to pixel shader
        //char gsPath[64]{ '\0' };            // buffer for path to geometry shader
        const char* pathToDir = "shaders/"; // path to direction with shader files (.cso)


        // color / texture / light
        result = shadersContainer.colorShader_.Initialize(pDevice, "shaders/colorVS.cso", "shaders/colorPS.cso");
        CAssert::True(result, "can't initialize the color shader class");

        result = shadersContainer.textureShader_.Initialize(pDevice, "shaders/textureVS.cso", "shaders/texturePS.cso");
        CAssert::True(result, "can't initialize the texture shader class");

        result = shadersContainer.lightShader_.Initialize(pDevice, "shaders/LightVS.cso", "shaders/LightPS.cso");
        CAssert::True(result, "can't initialize the light shader class");
      

        // font / debug / sky_dome
        result = shadersContainer.fontShader_.Initialize(pDevice, WVO, "shaders/fontVS.cso", "shaders/fontPS.cso");
        CAssert::True(result, "can't initialize the font shader class");

        result = shadersContainer.debugShader_.Initialize(pDevice, "shaders/DebugVS.cso", "shaders/DebugPS.cso");
        CAssert::True(result, "can't initialize the debug shader");

        result = shadersContainer.skyDomeShader_.Initialize(pDevice, "shaders/SkyDomeVS.cso", "shaders/SkyDomePS.cso");
        CAssert::True(result, "can't initialize the sky dome shader");
        

        // outline / billboard / material_icon
        result = shadersContainer.outlineShader_.Initialize(pDevice, "shaders/OutlineVS.cso", "shaders/OutlinePS.cso");
        CAssert::True(result, "can't initialize the outline shader");

        result = shadersContainer.billboardShader_.Initialize(pDevice, "shaders/billboardVS.cso", "shaders/billboardPS.cso", "shaders/billboardGS.cso");
        CAssert::True(result, "can't initialize the billboard shader");

        result = shadersContainer.materialIconShader_.Initialize(pDevice, "shaders/MaterialIconVS.cso", "shaders/MaterialIconPS.cso");
        if (!result)
            LogErr("can't initialize the material icon shader");


        // terrain
        result = shadersContainer.terrainShader_.Initialize(pDevice, "shaders/TerrainVS.cso", "shaders/TerrainPS.cso");
        CAssert::True(result, "can't initialize the terrain shader");

        
        LogDbg(LOG, "shaders initialization: finished successfully");
        SetConsoleColor(YELLOW);
        LogMsg("---------------------------------------------------------\n");
        SetConsoleColor(RESET);

    }
    catch (EngineException& e) 
    {
        LogErr(e, true);
        return false;
    }

    return true;
}

} // namespace Render
