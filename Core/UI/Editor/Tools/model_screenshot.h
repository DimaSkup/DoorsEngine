/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: model_screenshot.h
    Desc:     make a screenshot of a model from different views (or from only one)
              and create a texture atlas of these screenshots

    Created:  04.11.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <math/vec3.h>
#include <math/vec4.h>

namespace UI
{

class IFacadeEngineToUI;

//---------------------------------------------------------

class ModelScreenshot
{
public:
    ModelScreenshot() {}

    void Init(IFacadeEngineToUI* pFacade);
    void Shutdown();
    void Render(bool* pOpen);

private:
    void DrawNumFramesSelector();

private:
    IFacadeEngineToUI* pFacade_ = nullptr;

    uint  frames_                = 16;      // by default make a texture atlas with 16 views
    int   selectedModelIdx_      = -1;      // make screenshoot of this model
    bool  renderModelsNamesList_ = false;

};

} // namespace
