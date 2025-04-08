// =================================================================================
// Filename:     MainMenuBar.h
// Description:  handles and renders a main menu bar of the engine's GUI
//               also handles and render GUI elements which are related to the
//               main menu bar or can be called using this bar;
// 
// Created:      08.01.25 by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IFacadeEngineToUI.h>
#include <UICommon/StatesGUI.h>

namespace UI
{

class MainMenuBar
{
public:
    void RenderBar(StatesGUI& states);
    void RenderWndEngineOptions (bool* pOpen);
  
private:
    void ShowOptionsGui();
    void ShowFontSelector();
};

}
