// =================================================================================
// Filename:      SkyController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "SkyController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <imgui.h>

#pragma warning (disable : 4996)

namespace UI
{
	
SkyController::SkyController()
{
}

///////////////////////////////////////////////////////////

void SkyController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	CAssert::NotNullptr(pFacade, "ptr to the facade == nullptr");
	pFacade_ = pFacade;

#if 0
    // if we selected the sky entt
    if (selectedEnttData_.name == "sky")
    {
        selectedEnttData_.type = SKY;
        skyController_.LoadEnttData(enttID);
    }
#endif
}

///////////////////////////////////////////////////////////

void SkyController::LoadEnttData(const uint32_t skyEnttID)
{
	// load/reload data of currently selected entity by ID (which is actually the sky entt)
	ColorRGB center;
	ColorRGB apex;
	Vec3 offset;

	if (pFacade_->GetSkyData(skyEnttID, center, apex, offset))
	{
		skyModel_.SetColorCenter(center);
		skyModel_.SetColorApex(apex);
		skyModel_.SetSkyOffset(offset);
	}
	else
	{
		LogErr("can't gather data for the sky editor model for unknown reason");
	}
}

///////////////////////////////////////////////////////////
#if 0
void RenderEntityIdAndName(const EntityID id, const std::string& enttName)
{
    ImGui::Text("Entity ID:   %d", id);
    ImGui::Text("Entity name: %s", enttName.c_str());
}

///////////////////////////////////////////////////////////

void SkyController::Render(const EntityID id, const std::string& enttName)
{
    RenderEntityIdAndName(id, enttName.c_str());

    // render a panel for changing properties of the sky (since it is the chosen entity)
    if (isOpen = ImGui::CollapsingHeader("SkyEditor", ImGuiTreeNodeFlags_SpanFullWidth))
        viewSky_.Render(GetModel());
    break;
}
#endif


// =================================================================================
// private API: commands executors
// =================================================================================

void SkyController::ExecuteCommand(const ICommand* pCmd, const uint32_t enttID)
{
	// execute the sky changes according to the command type
	switch (pCmd->type_)
	{
		// change the sky horizon color
	case CHANGE_SKY_COLOR_CENTER:
	{
		const ColorRGB newColorCenter = pCmd->GetColorRGB();

		if (pFacade_->SetSkyColorCenter(newColorCenter))
		{
			skyModel_.SetColorCenter(newColorCenter);
			// TODO: store the command into the events history
		}
		break;
	}
	// change the sky top color
	case CHANGE_SKY_COLOR_APEX:
	{
		const ColorRGB newColorApex = pCmd->GetColorRGB();

		if (pFacade_->SetSkyColorApex(newColorApex))
		{
			skyModel_.SetColorApex(newColorApex);
			// TODO: store the command into the events history
		}
		break;
	}
	case CHANGE_SKY_OFFSET:
	{
		const Vec3 newOffset = pCmd->GetVec3();

		if (pFacade_->SetSkyOffset(newOffset))
		{
			skyModel_.SetSkyOffset(newOffset);
			// TODO: store the command into the events history
		}
		break;
	}
	default:
	{
        sprintf(g_String, "Unknown command (sky change) to execute (cmd: %d; sky_entt_id: %ld)", pCmd->type_, enttID);
        LogErr(g_String);
	}
	}; // switch
}

} // namespace UI
