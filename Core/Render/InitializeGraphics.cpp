// =================================================================================
// Filename:     InitializeGraphics.cpp
// Description:  there are functions for initialization of DirectX
//               and graphics parts of the engine;
//
// Created:      02.12.22
// =================================================================================
#include <CoreCommon/pch.h>
#include "InitializeGraphics.h"
#include <shellapi.h>

using namespace DirectX;


namespace Core
{

InitializeGraphics::InitializeGraphics()
{
    LogMsg("initialize graphics stuff");
}


// =================================================================================
//                                PUBLIC FUNCTIONS
// =================================================================================
bool InitializeGraphics::InitializeDirectX(
    D3DClass& d3d,
    HWND hwnd,
    const Settings& settings)
{
    // THIS FUNC initializes the DirectX stuff 
    // (device, deviceContext, swapChain, rasterizerState, viewport, etc)

    try 
    {
        bool result = d3d.Initialize(
            hwnd,
            settings.GetBool("VSYNC_ENABLED"),
            settings.GetBool("FULL_SCREEN"),
            settings.GetBool("ENABLE_4X_MSAA"),
            settings.GetFloat("NEAR_Z"),
            settings.GetFloat("FAR_Z"));         // how far we can see

        CAssert::True(result, "can't initialize the Direct3D");

        // setup the rasterizer state to default params
        d3d.SetRS({ eRenderState::CULL_BACK, eRenderState::FILL_SOLID });
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't initialize DirectX");
        return false;
    }

    return true;
}


} // namespace Core
