// =================================================================================
// Filename:     DebugEditor.h
// Description:  editor parts to control the debugging:
//               turn on/off showing of the normals, binormals, bounding boxes,
//               switching the wireframe or fill mode, etc.
// 
// Created:      01.01.25
// =================================================================================
#pragma once

#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
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
		Core::Assert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
		pFacade_ = pFacade;
	}

	void Draw()
	{
		if (pFacade_ == nullptr)
		{
			Core::Log::Error("ptr to the facade interface == nullptr");
			return;
		}

		//ImGui::SeparatorText("Debug");

		// switch shader debug state (or turn it off)
		bool anyBtnWasPressed = false;
		static int debugOption = 0;

		ImGui::Text("show as colors:");

		const char* items[] =
		{ 
			"default", 
			"normals",
			"tangents",
			"binormals",
			"bumped normals",
			"only lighting",
			"only directed lighting",   // for instance: sun
			"only point lighting",      // for instance: light bulb, candle
			"only spot lighting",       // for instance: flashlight
			"only diffuse map",
			"only normal map",
		};

		const int numItems = ARRAYSIZE(items);

		for (int i = 0; i < numItems; ++i)
			anyBtnWasPressed |= ImGui::RadioButton(items[i], &debugOption, i);

		if (anyBtnWasPressed)
		{
			pFacade_->SwitchDebugState(debugOption);
		}

	}
};

} // namespace UI