// =================================================================================
// Filename:       EditorController.cpp
// Created:        01.01.25
// =================================================================================
#include "EditorController.h"

#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <CoreCommon/MathHelper.h>
#include <UICommon/EditorCommands.h>
#include <UICommon/EventsHistory.h>

#include <imgui.h>
#include <ImGuizmo.h>
#include <format>

using namespace Core;

namespace UI
{

EditorController::EditorController(StatesGUI* pStatesGUI) :
    viewModel_(this),
    viewSky_(this),
    viewLight_(this),
    pStatesGUI_(pStatesGUI)
{
    Assert::NotNullptr(pStatesGUI, "input ptr to the GUI states container == nullptr");
}


// =================================================================================
// public API
// =================================================================================

void EditorController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    Assert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
    pFacade_ = pFacade;

    modelController_.Initialize(pFacade);
    skyController_.Initialize(pFacade);
    directedLightController_.Initialize(pFacade);
    pointLightController_.Initialize(pFacade);
    spotLightController_.Initialize(pFacade);
}

///////////////////////////////////////////////////////////

void EditorController::SetSelectedEntt(const EntityID enttID)
{
    // set that we have selected some entity and load its data

    // if we deselected the chosen entt so we won't render any control panel
    // until we won't select any other
    if (enttID == 0)
    {
        selectedEnttData_.id   = 0;
        selectedEnttData_.type = eSelectedEnttType::NONE;
        selectedEnttData_.name = "";
        return;
    }
    // we have chosen some entt
    else
    {
        int lightType;

        // get selected entity ID and name
        selectedEnttData_.id = enttID;
        pFacade_->GetEnttNameByID(enttID, selectedEnttData_.name);

        // get names of added components
        pFacade_->GetEntityAddedComponentsNames(enttID, selectedEnttData_.componentsNames);


        // ------------------------------------------------
        // define entity type and load its specific data

        // if we selected the sky entt
        if (selectedEnttData_.name == "sky")
        {
            selectedEnttData_.type = SKY;
            skyController_.LoadEnttData(enttID);
        }
        // if we selected any light source
        else if (pFacade_->IsEnttLightSource(enttID, lightType))
        {
            if (lightType == 0)
            {
                selectedEnttData_.type = DIRECTED_LIGHT;
                directedLightController_.LoadEnttData(enttID);
            }
            else if (lightType == 1)
            {
                selectedEnttData_.type = POINT_LIGHT;
                pointLightController_.LoadEnttData(enttID);
            }
            else if (lightType == 2)
            {
                selectedEnttData_.type = SPOT_LIGHT;
                spotLightController_.LoadEnttData(enttID);
            }
            else
            {
                Log::Error("unknown light type");
                return;
            }
        }
        // we selected any model entity
        else
        {
            selectedEnttData_.type = MODEL;
            modelController_.LoadEnttData(enttID);
        }
    }
}

///////////////////////////////////////////////////////////

void RenderEntityIdAndName(const EntityID id, const char* enttName, const char* enttType)
{
    ImGui::Text("Entity ID:   %d", id);
    ImGui::Text("Entity name: %s", enttName);
    ImGui::Text("Entity type: %s", enttType);
}

///////////////////////////////////////////////////////////

void EditorController::Render()
{
    //  render a panel for controlling properties of the chosen entity

    const EntityID enttID = selectedEnttData_.id;

    // we have no entity as selected
    if (enttID == 0)
        return;

    // we want the next editor panel to be visible
    static bool isOpen = true;
    ImGui::SetNextItemOpen(isOpen);
    
    switch (selectedEnttData_.type)
    {
        case SKY:
        {
            RenderEntityIdAndName(enttID, selectedEnttData_.name.c_str(), "sky entity");

            // render a panel for changing properties of the sky (since it is the chosen entity)
            if (isOpen = ImGui::CollapsingHeader("SkyEditor", ImGuiTreeNodeFlags_SpanFullWidth))
                viewSky_.Render(skyController_.GetModel());
            break;
        }
        case DIRECTED_LIGHT:
        {
            RenderEntityIdAndName(enttID, selectedEnttData_.name.c_str(), "directed light");

            if (ImGui::CollapsingHeader("Directed light properties", ImGuiTreeNodeFlags_SpanFullWidth))
                viewLight_.Render(directedLightController_.GetModel());

            break;
        }
        case POINT_LIGHT:
        {
            RenderEntityIdAndName(enttID, selectedEnttData_.name.c_str(), "point light");

            if (isOpen = ImGui::CollapsingHeader("Point Light Properties", ImGuiTreeNodeFlags_SpanFullWidth))
                viewLight_.Render(pointLightController_.GetModel());
            break;
        }
        case SPOT_LIGHT:
        {
            RenderEntityIdAndName(enttID, selectedEnttData_.name.c_str(), "spotlight");

            if (isOpen = ImGui::CollapsingHeader("Spotlight Properties", ImGuiTreeNodeFlags_SpanFullWidth))
                viewLight_.Render(spotLightController_.GetModel());
            break;
        }
        case MODEL:
        {
            RenderEntityIdAndName(enttID, selectedEnttData_.name.c_str(), "model");

            if (isOpen = ImGui::CollapsingHeader("EntityEditor", ImGuiTreeNodeFlags_SpanFullWidth))
                viewModel_.Render(modelController_.GetModel());
            break;
        }
    }

    // render a list of components names which are added to the entity
    if (enttID != 0)
    {
        ImGui::Separator();
        ImGui::Text("Added components:");

        for (const std::string& name : selectedEnttData_.componentsNames)
            ImGui::Text(name.c_str());
    }
}



// =================================================================================
// For manipulation with gizmos
// =================================================================================

void EditorController::UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world)
{
    // when we used a gizmo to modify a world of the selected entity
    // we call this method to actual update of the world properties

    int transformType = pStatesGUI_->gizmoOperation_;
    DirectX::XMVECTOR scale, rotQuat, translation;
    XMMatrixDecompose(&scale, &rotQuat, &translation, world);

    // execute transformation of the selected entity according to the transformation type
    switch (transformType)
    {
        case ImGuizmo::OPERATION::TRANSLATE:
        {
            TranslateSelectedEntt(translation);
            break;
        }
        case ImGuizmo::OPERATION::ROTATE:
        {
            RotateSelectedEntt(rotQuat);
            break;
        }
        case ImGuizmo::OPERATION::SCALE:
        {
            ScaleSelectedEntt(DirectX::XMVectorGetX(scale));
            break;
        }
        default:
        {
            Log::Error("Unknown ImGuizmo::OPERATION: " + std::to_string(transformType));
        }
    }
}

///////////////////////////////////////////////////////////

void EditorController::TranslateSelectedEntt(const DirectX::XMVECTOR& translation)
{
    EditorCmdType cmdType = EditorCmdType::INVALID_CMD;

    // generate a command and call executor according to the entity type
    switch (selectedEnttData_.type)
    {
        case SKY:
        {
            break;
        }
        case POINT_LIGHT:
        {
            cmdType = CHANGE_POINT_LIGHT_POSITION;
            break;
        }
        case SPOT_LIGHT:
        {
            cmdType = CHANGE_SPOT_LIGHT_POSITION;
            break;
        }
        case CAMERA:
        {
            break;
        }
        case MODEL:
        {
            cmdType = CHANGE_MODEL_POSITION;
            break;
        }
        default:
        {
            Log::Error("unknown entity type for translation: " + std::to_string(selectedEnttData_.type));
            return;
        }
    }

    // execute responsible command for rotation
    CmdChangeVec3 cmd(cmdType, translation);
    Execute(&cmd);
}

///////////////////////////////////////////////////////////

void EditorController::RotateSelectedEntt(const DirectX::XMVECTOR& quat)
{
    EditorCmdType cmdType = EditorCmdType::INVALID_CMD;

    switch (selectedEnttData_.type)
    {
        case DIRECTED_LIGHT:
        {
            cmdType = CHANGE_DIR_LIGHT_DIRECTION_BY_QUAT;
            break;
        }
        case SPOT_LIGHT:
        {
            cmdType = CHANGE_SPOT_LIGHT_DIRECTION_BY_QUAT;  // rotate using quaternion
            break;
        }
        case CAMERA:
        {
            break;
        }
        case MODEL:
        {
            cmdType = CHANGE_MODEL_ROTATION;
            break;
        }
        default:
        {
            Log::Error("unknown entity type for rotation: " + std::to_string(selectedEnttData_.type));
            return;
        }
    }

    // execute responsible command for rotation
    CmdChangeVec4 cmd(cmdType, Vec4(quat));
    Execute(&cmd);
}

///////////////////////////////////////////////////////////

void EditorController::ScaleSelectedEntt(const float uniformScale)
{
    EditorCmdType cmdType = EditorCmdType::INVALID_CMD;

    switch (selectedEnttData_.type)
    {
        case MODEL:
        {
            cmdType = CHANGE_MODEL_SCALE;
            break;
        }
        case POINT_LIGHT:
        {
            // scale range
        }
        default:
        {
            Log::Error("unknown entity type for scaling: " + std::to_string(selectedEnttData_.type));
            return;
        }
    }

    // execute responsible command for rotation
    CmdChangeFloat cmd(cmdType, uniformScale);
    Execute(&cmd);
}


// =================================================================================
// private API: commands executors
// =================================================================================

void EditorController::Execute(const ICommand* pCmd)
{
    // execute a command according to its type for the currently selected entity

    if ((pCmd == nullptr) || (pFacade_ == nullptr) || (!selectedEnttData_.IsSelectedAnyEntt()))
    {
        Log::Error("can't execute a command of type: " + std::to_string(pCmd->type_));
        return;
    }

    const uint32_t enttID = selectedEnttData_.id;

    switch (pCmd->type_)
    {
        case CHANGE_MODEL_POSITION:
        case CHANGE_MODEL_ROTATION:
        case CHANGE_MODEL_SCALE:
        {
            modelController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        case CHANGE_SKY_COLOR_CENTER:
        case CHANGE_SKY_COLOR_APEX:
        case CHANGE_SKY_OFFSET:
        {
            skyController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        case CHANGE_DIR_LIGHT_AMBIENT:
        case CHANGE_DIR_LIGHT_DIFFUSE:
        case CHANGE_DIR_LIGHT_SPECULAR:
        case CHANGE_DIR_LIGHT_DIRECTION:
        case CHANGE_DIR_LIGHT_DIRECTION_BY_QUAT:
        {
            directedLightController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        case CHANGE_POINT_LIGHT_AMBIENT:
        case CHANGE_POINT_LIGHT_DIFFUSE:
        case CHANGE_POINT_LIGHT_SPECULAR:
        case CHANGE_POINT_LIGHT_POSITION:
        case CHANGE_POINT_LIGHT_RANGE:
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            pointLightController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        case CHANGE_SPOT_LIGHT_AMBIENT:
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        case CHANGE_SPOT_LIGHT_SPECULAR:
        case CHANGE_SPOT_LIGHT_POSITION:
        case CHANGE_SPOT_LIGHT_DIRECTION:
        case CHANGE_SPOT_LIGHT_DIRECTION_BY_QUAT:
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            spotLightController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        default:
        {
            Log::Error("unknown type of command: " + std::to_string(pCmd->type_));
            return;
        }
    }
}

///////////////////////////////////////////////////////////

void EditorController::Undo(const ICommand* pCmd, const uint32_t enttID)
{
    // undo/alt_undo an event from the events history for entity by ID

    if ((pCmd == nullptr) || (pFacade_ == nullptr) || (enttID == 0))
    {
        Log::Error("can't execute a command of type: " + std::to_string(pCmd->type_));
        return;
    }

    switch (pCmd->type_)
    {
        case CHANGE_MODEL_POSITION:
        case CHANGE_MODEL_ROTATION:
        case CHANGE_MODEL_SCALE:
        {
            modelController_.UndoCommand(pCmd, enttID);
            break;
        }
        case CHANGE_SKY_COLOR_CENTER:
        case CHANGE_SKY_COLOR_APEX:
        case CHANGE_SKY_OFFSET:
        {
            skyController_.UndoCommand(pCmd, enttID);
            break;
        }
        case CHANGE_DIR_LIGHT_AMBIENT:
        case CHANGE_DIR_LIGHT_DIFFUSE:
        case CHANGE_DIR_LIGHT_SPECULAR:
        case CHANGE_DIR_LIGHT_DIRECTION:
        {
            directedLightController_.UndoCommand(pCmd, enttID);
            break;
        }
        case CHANGE_POINT_LIGHT_AMBIENT:
        case CHANGE_POINT_LIGHT_DIFFUSE:
        case CHANGE_POINT_LIGHT_SPECULAR:
        case CHANGE_POINT_LIGHT_POSITION:
        case CHANGE_POINT_LIGHT_RANGE:
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            pointLightController_.UndoCommand(pCmd, enttID);
            break;
        }
        case CHANGE_SPOT_LIGHT_AMBIENT:
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        case CHANGE_SPOT_LIGHT_SPECULAR:
        case CHANGE_SPOT_LIGHT_POSITION:
        case CHANGE_SPOT_LIGHT_DIRECTION:
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            spotLightController_.UndoCommand(pCmd, enttID);
            break;
        }

    }
}

} // namespace UI
