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
	pointLightController_.Initialize(pFacade);
	spotLightController_.Initialize(pFacade);
}

///////////////////////////////////////////////////////////

void EditorController::SetSelectedEntt(const uint32_t enttID)
{
	// set that we have selected some entity and load its data

	using enum StatesGUI::SelectedEnttType;
	StatesGUI& states = *pStatesGUI_;

	// we deselected the chosen entt so we won't render any control panel
	// until we won't select any other
	if (enttID == 0)
	{
		states.selectedEnttType_ = NONE;
		states.selectedEnttID_ = 0;
		return;
	}
	// we have chosen some entt
	else
	{
		std::string enttName;
		int lightType;

		states.selectedEnttID_ = enttID;
		pFacade_->GetEnttNameByID(enttID, enttName);

		// if we selected the sky entt
		if (enttName == "sky")
		{
			states.selectedEnttType_ = SKY;
			skyController_.LoadEnttData(enttID);
		}
		// if we selected any light source
		else if (pFacade_->IsEnttLightSource(enttID, lightType))
		{
			if (lightType == 0)
			{
				states.selectedEnttType_ = DIRECTED_LIGHT;
				// directedLightController_.LoadEnttData(enttID);
			}
			else if (lightType == 1)
			{
				states.selectedEnttType_ = POINT_LIGHT;
				pointLightController_.LoadEnttData(enttID);
			}
			else if (lightType == 2)
			{
				states.selectedEnttType_ = SPOT_LIGHT;
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
			states.selectedEnttType_ = MODEL;
			modelController_.LoadEnttData(enttID);
		}
	}
}

///////////////////////////////////////////////////////////

void EditorController::Render()
{
	//  render a panel for controlling properties of the chosen entity

	using enum StatesGUI::SelectedEnttType;

	// we want the next editor panel to be visible
	static bool isOpen = true;
	ImGui::SetNextItemOpen(isOpen);
    StatesGUI& states = *pStatesGUI_;
    StatesGUI::EntityID enttID = states.selectedEnttID_;
	StatesGUI::SelectedEnttType type = states.selectedEnttType_;
    std::string enttName;

    pFacade_->GetEnttNameByID(enttID, enttName);

	switch (type)
	{
		case SKY:
		{
			// render a panel for changing properties of the sky (since it is the chosen entity)
			if (isOpen = ImGui::CollapsingHeader("SkyEditor", ImGuiTreeNodeFlags_SpanFullWidth))
				viewSky_.Render(skyController_.GetModel());
			break;
		}
		case DIRECTED_LIGHT:
		{
			ImGui::Text("DIR LIGHT SOURCE IS CHOSEN");
			break;
		}
		case POINT_LIGHT:
		{
			ImGui::Text("Entity ID (point light): %d", enttID);
            ImGui::Text("Entity Name: %s", enttName.c_str());

			if (isOpen = ImGui::CollapsingHeader("Point Light Properties", ImGuiTreeNodeFlags_SpanFullWidth))
				viewLight_.Render(pointLightController_.GetModel());
			break;
		}
		case SPOT_LIGHT:
		{
			ImGui::Text("enttID (spot light): %d", enttID);
            ImGui::Text("Entity Name: %s", enttName.c_str());

			if (isOpen = ImGui::CollapsingHeader("Spotlight Properties", ImGuiTreeNodeFlags_SpanFullWidth))
				viewLight_.Render(spotLightController_.GetModel());
			break;
		}
		case MODEL:
		{
			// render a panel for changing properties of the chosen entity 
			ImGui::Text("enttID (model): %d", enttID);
            ImGui::Text("Entity Name: %s", enttName.c_str());

			if (isOpen = ImGui::CollapsingHeader("EntityEditor", ImGuiTreeNodeFlags_SpanFullWidth))
				viewModel_.Render(modelController_.GetModel());
			break;
		}
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
	using enum StatesGUI::SelectedEnttType;

	StatesGUI::SelectedEnttType enttType = pStatesGUI_->selectedEnttType_;
	EditorCmdType cmdType = EditorCmdType::INVALID_CMD;

	// generate a command and call executor according to the entity type
	switch (enttType)
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
			Log::Error("unknown entity type for translation: " + std::to_string(enttType));
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
	using enum StatesGUI::SelectedEnttType;

	StatesGUI::SelectedEnttType enttType = pStatesGUI_->selectedEnttType_;
	EditorCmdType cmdType  = EditorCmdType::INVALID_CMD;

	switch (enttType)
	{
		case DIRECTED_LIGHT:
		{
			cmdType = CHANGE_DIR_LIGHT_ROTATION;
			break;
		}
		case SPOT_LIGHT:
		{
			cmdType = CHANGE_SPOT_LIGHT_DIRECTION;
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
			Log::Error("unknown entity type for rotation: " + std::to_string(enttType));
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
	using enum StatesGUI::SelectedEnttType;

	EditorCmdType cmdType = EditorCmdType::INVALID_CMD;

	switch (pStatesGUI_->selectedEnttType_)
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
			Log::Error("unknown entity type for scaling: " + std::to_string(pStatesGUI_->selectedEnttType_));
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

	if ((pCmd == nullptr) || (pFacade_ == nullptr) || (!pStatesGUI_->IsSelectedAnyEntt()))
	{
		Log::Error("can't execute a command of type: " + std::to_string(pCmd->type_));
		return;
	}

	const uint32_t enttID = pStatesGUI_->selectedEnttID_;

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
		}
		case CHANGE_POINT_LIGHT_AMBIENT:
		case CHANGE_POINT_LIGHT_DIFFUSE:
		case CHANGE_POINT_LIGHT_SPECULAR:
		case CHANGE_POINT_LIGHT_POSITION:
		case CHANGE_POINT_LIGHT_RANGE:
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			pointLightController_.UndoCommand(pCmd, enttID);
		}
	}
}

} // namespace UI
