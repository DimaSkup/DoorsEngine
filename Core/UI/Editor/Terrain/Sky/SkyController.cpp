// =================================================================================
// Filename:      SkyController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "SkyController.h"

#include <UICommon/events_history.h>
#include <UICommon/icommand.h>
#include <UICommon/editor_cmd.h>
#include <UICommon/IFacadeEngineToUI.h>

#include <imgui.h>

#pragma warning (disable : 4996)

namespace UI
{
    
SkyController::SkyController() : skyView_(this)
{
}

//---------------------------------------------------------
// Desc:   just initialize the sky controller
// Args:   - pFacade: a virtual interface btw the GUI and engine (facade pattern)
//                    which is used to contact with the rest of the engine's parts
//---------------------------------------------------------
void SkyController::Initialize(IFacadeEngineToUI* pFacade)
{
    CAssert::NotNullptr(pFacade, "ptr to the facade == nullptr");
    pFacade_ = pFacade;

    LoadSkyEnttData();
}

//---------------------------------------------------------
// Desc:   load/reload data of currently selected entity by ID
//         (which is actually the sky entt)
//---------------------------------------------------------
void SkyController::LoadSkyEnttData()
{
    ColorRGB center;
    ColorRGB apex;
    Vec3 offset;

    const EntityID id = pFacade_->GetEnttIdByName("sky");

    if (pFacade_->GetSkyData(id, center, apex, offset))
        skyModel_.Init(id, center, apex, offset);

    else
        LogErr("can't gather data for the sky editor model for unknown reason");
}

//---------------------------------------------------------
// Desc:  render a panel for changing properties of the sky
//---------------------------------------------------------
void SkyController::Draw()
{
    ImGui::Text("Entity ID:   %d", skyModel_.GetID());
    ImGui::Text("Entity name: sky");
    
    skyView_.Render(skyModel_);
}

//---------------------------------------------------------
// Desc:   execute a command and store this change into the events history
// Args:   - pCmd:  a ptr to the commands interface (command pattern)
//---------------------------------------------------------
void SkyController::ExecCmd(const ICommand* pCmd)
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
            LogErr(LOG, "Unknown command (sky change) to execute (cmd: %d)", pCmd->type_);
        }
    }; // switch
}

//---------------------------------------------------------
// Desc:   undo/alt_undo an event (related to sky) from the events history
// Args:   - pCmd:      a ptr to the commands interface (command pattern)
//         - entityID:  an ID of the sky entity
//---------------------------------------------------------
void SkyController::UndoCmd(const ICommand* pCmd, const uint32 entityID)
{

}

} // namespace UI
