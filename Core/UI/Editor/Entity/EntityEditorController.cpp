// =================================================================================
// Filename:       EntityEditorController.cpp
// Created:        01.01.25
// =================================================================================
#include "EntityEditorController.h"

#include "../../../Common/Assert.h"
#include "../../../Common/log.h"



EntityEditorController::EntityEditorController() :
	enttView_(this),
	skyView_(this) 
{
}

// =================================================================================
// public API
// =================================================================================

void EntityEditorController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Assert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
	pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void EntityEditorController::SetSelectedEntt(const uint32_t entityID)
{
	// set that we have selected some entity and load its data

	// we deselected the chosen entt so we won't render any control panel
	// until we won't select any other
	if (entityID == 0)
	{
		isSelectedAnyEntt_ = false;
		selectedEnttType_ = NONE;
		return;
	}
	// we have chosen some entt
	else
	{
		isSelectedAnyEntt_ = true;

		std::string enttName;
		pFacade_->GetEnttNameByID(entityID, enttName);

		// if we selected the sky entt
		if (enttName == "sky")
		{
			selectedEnttType_ = SKY;
			LoadSkyEnttData(entityID);
		}
		// we have selected some another entt: model, light, camera, etc
		else
		{
			selectedEnttType_ = USUAL;
			LoadUsualEnttData(entityID);
		}
	}
}

///////////////////////////////////////////////////////////

void EntityEditorController::Render()
{
	//  render a panel for controlling properties of the chosen entity

	// we want the next editor panel to be visible
	static bool isOpen = true;
	ImGui::SetNextItemOpen(isOpen);

	if (selectedEnttType_ == USUAL)   // model, camera, light src
	{
		float cameraViewMatrix[16]{ 0 };
		float cameraProjMatrix[16]{ 0 };

		uint32_t cameraID = pFacade_->GetEnttIDByName("editor_camera");
		pFacade_->GetCameraViewAndProj(cameraID, cameraViewMatrix, cameraProjMatrix);

		// render a panel for changing properties of the chosen entity 
		if (isOpen = ImGui::CollapsingHeader("EntityEditor", ImGuiTreeNodeFlags_SpanFullWidth))
			enttView_.Render(&enttModel_, cameraViewMatrix, cameraProjMatrix);
	}
	else if (selectedEnttType_ == SKY)
	{
		// render a panel for changing properties of the sky (since it is the chosen entity)
		if (isOpen = ImGui::CollapsingHeader("SkyEditor", ImGuiTreeNodeFlags_SpanFullWidth))
			skyView_.Render(&skyModel_);
	}
}

///////////////////////////////////////////////////////////

void EntityEditorController::Execute(ICommand* pCommand)
{
	if ((pCommand == nullptr) ||
		(pFacade_ == nullptr) || 
		(!isSelectedAnyEntt_))
	{
		Log::Error("can't execute a command of type: " + std::to_string(pCommand->type_));
		return;
	}

	switch (pCommand->type_)
	{
		case CHANGE_POSITION:
		case CHANGE_ROTATION:
		case CHANGE_SCALE:
		{
			ExecuteUsualEnttCommand(pCommand);
			break;
		}

		case CHANGE_SKY_COLOR_CENTER:
		case CHANGE_SKY_COLOR_APEX:
		case CHANGE_SKY_OFFSET:
		{
			ExecuteSkyCommand(pCommand);
		}
	}
}


// =================================================================================
// private API: entities data loading 
// =================================================================================

void EntityEditorController::LoadSkyEnttData(const uint32_t skyEnttID)
{
	// load data of currently selected entity by ID (which is actually the sky entt)
	ColorRGB center;
	ColorRGB apex;
	Vec3 offset;

	if (pFacade_->GatherSkyData(skyEnttID, center, apex, offset))
	{
		skyModel_.SetColorCenter(center);
		skyModel_.SetColorApex(apex);
		skyModel_.SetSkyOffset(offset);
	}
	else
	{
		Log::Error("can't gather data for the sky editor model for unknown reason");
	}
}

///////////////////////////////////////////////////////////

void EntityEditorController::LoadUsualEnttData(const uint32_t enttID)
{
	// load data of currently selected entity by ID
	Vec3 position(0, 0, 0);
	Vec4 dirQuat(0, 0, 0, 1);
	float uniScale = 0.0f;

	if (pFacade_->GatherEnttData(enttID, position, dirQuat, uniScale))
		enttModel_.SetSelectedEnttData(enttID, position, dirQuat, uniScale);
}


// =================================================================================
// private API: commands executors
// =================================================================================

void EntityEditorController::ExecuteUsualEnttCommand(ICommand* pCommand)
{
	// execute the entity changes according to the input command

	Model::Entity& data = enttModel_;
	const uint32_t selectedEntt = data.GetSelectedEntityID();


	// execute changes according to the command type
	switch (pCommand->type_)
	{
		case CHANGE_POSITION:
		{
			const Vec3 newPos = pCommand->GetVec3();

			if (pFacade_->SetEnttPosition(selectedEntt, newPos))
			{
				data.SetPosition(newPos);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_ROTATION:
		{
			const Vec4 newRotation = pCommand->GetVec4();

			if (pFacade_->SetEnttRotation(selectedEntt, newRotation))
			{
				data.SetRotation(newRotation);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_SCALE:
		{
			const float newUniScale = pCommand->GetFloat();

			if (pFacade_->SetEnttUniScale(selectedEntt, newUniScale))
			{
				data.SetUniformScale(newUniScale);
				// TODO: store the command into the events history
			}
			break;
		}
		default:
		{
			Log::Error("Unknown command to execute: " + std::to_string(pCommand->type_));
		}
	} // switch
}

///////////////////////////////////////////////////////////

void EntityEditorController::ExecuteSkyCommand(ICommand* pCommand)
{
	// execute the sky changes according to the command type
	switch (pCommand->type_)
	{
		// change the sky horizon color
		case CHANGE_SKY_COLOR_CENTER:
		{
			const ColorRGB newColorCenter = pCommand->GetColorRGB();

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
			const ColorRGB newColorApex = pCommand->GetColorRGB();

			if (pFacade_->SetSkyColorApex(newColorApex))
			{
				skyModel_.SetColorApex(newColorApex);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_SKY_OFFSET:
		{
			const Vec3 newOffset = pCommand->GetVec3();

			if (pFacade_->SetSkyOffset(newOffset))
			{
				skyModel_.SetSkyOffset(newOffset);
				// TODO: store the command into the events history
			}
			break;
		}
		default:
		{
			Log::Error("Unknown command to execute: " + std::to_string(pCommand->type_));
		}
	}; // switch
}