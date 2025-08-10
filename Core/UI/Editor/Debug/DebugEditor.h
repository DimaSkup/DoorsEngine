// =================================================================================
// Filename:     DebugEditor.h
// Description:  editor parts to control the debugging:
//               turn on/off showing of the normals, binormals, bounding boxes,
//               switching the wireframe or fill mode, etc.
// 
// Created:      01.01.25
// =================================================================================
#pragma once

#include <Assert.h>
#include <log.h>
#include <UICommon/IFacadeEngineToUI.h>
#include <imgui.h>


namespace UI
{
	
class DebugEditor
{
private:
	IFacadeEngineToUI* pFacade_ = nullptr;

public:
	DebugEditor() {}

	void Initialize(IFacadeEngineToUI* pFacade)
	{
		// the facade interface is used to contact with the rest of the engine
		CAssert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
		pFacade_ = pFacade;
	}

    ///////////////////////////////////////////////////////

	void Draw()
	{
        // render options to control the debug visualization properties;
        // when choose some option we switch shader debug state (or turn it off)

		if (pFacade_ == nullptr)
		{
			LogErr("ptr to the facade interface == nullptr");
			return;
		}

		ImGui::Text("show as colors:");

		const char* items[] =
		{ 
			"default", 
			"normals",
			"tangents",
			"bumped normals",
			"only lighting",
			"only directed lighting",   // for instance: sun
			"only point lighting",      // for instance: light bulb, candle
			"only spot lighting",       // for instance: flashlight
			"only diffuse map",
			"only normal map",
            "wireframe",
            "material ambient",
            "material diffuse",
            "material specular",
            "material reflection",
		};

        static int debugOption = 0;
        bool anyBtnWasPressed = false;
        
		for (int i = 0; i < (int)ARRAYSIZE(items); ++i)
			anyBtnWasPressed |= ImGui::RadioButton(items[i], &debugOption, i);

		if (anyBtnWasPressed)
			pFacade_->SwitchDebugState(debugOption);
	}
};

} // namespace UI
