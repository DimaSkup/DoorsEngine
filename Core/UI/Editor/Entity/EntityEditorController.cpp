// =================================================================================
// Filename:       EntityEditorController.cpp
// Created:        01.01.25
// =================================================================================
#include "EntityEditorController.h"

#include "../../../Common/Assert.h"
#include "../../../Common/log.h"



EntityEditorController::EntityEditorController() :
	enttView_(this),
	skyView_(this),
	lightView_(this)
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
		selectedEnttID_ = 0;
		return;
	}
	// we have chosen some entt
	else
	{
		isSelectedAnyEntt_ = true;
		selectedEnttID_ = entityID;

		std::string enttName;
		int lightType;

		pFacade_->GetEnttNameByID(entityID, enttName);

		// if we selected the sky entt
		if (enttName == "sky")
		{
			selectedEnttType_ = SKY;
			LoadSkyEnttData(entityID);
		}
		else if (pFacade_->IsEnttLightSource(entityID, lightType))
		{
			if (lightType == 0)
				selectedEnttType_ = DIRECTED_LIGHT;
			else if (lightType == 1)
				selectedEnttType_ = POINT_LIGHT;
			else if (lightType == 2)
				selectedEnttType_ = SPOT_LIGHT;
			else
			{
				Log::Error("unknown light type");
				return;
			}

			LoadLightEnttData(selectedEnttID_);
		}
		else
		{
			selectedEnttType_ = MODEL;
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

	switch (selectedEnttType_)
	{
		case MODEL:
		{
			// render a panel for changing properties of the chosen entity 
			ImGui::Text("EntityID (model): %d", selectedEnttType_);

			if (isOpen = ImGui::CollapsingHeader("EntityEditor", ImGuiTreeNodeFlags_SpanFullWidth))
				enttView_.Render(&enttModel_);
			break;
		}
		case DIRECTED_LIGHT:
		{
			ImGui::Text("DIR LIGHT SOURCE IS CHOSEN");
			break;
		}
		case POINT_LIGHT:
		{
			ImGui::Text("EntityID (point light): %d", selectedEnttType_);

			if (isOpen = ImGui::CollapsingHeader("Point Light Properties", ImGuiTreeNodeFlags_SpanFullWidth))
				lightView_.Render(&pointLightModel_);
			break;
		}
		case SPOT_LIGHT:
		{
			ImGui::Text("SPOT LIGHT SOURCE IS CHOSEN");
			break;
		}
		case SKY:
		{
			// render a panel for changing properties of the sky (since it is the chosen entity)
			if (isOpen = ImGui::CollapsingHeader("SkyEditor", ImGuiTreeNodeFlags_SpanFullWidth))
				skyView_.Render(&skyModel_);
			break;
		}
	}
}

void EntityEditorController::UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world)
{
	// when we used a gizmo to modify a world of the selected entity
	// we call this method to actual update of the world properties

	DirectX::XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, world);

	// according to the selected entity type we update its world properties
	if (selectedEnttType_ == MODEL)
	{
		pFacade_->SetEnttTransformation(selectedEnttID_, translation, rotQuat, DirectX::XMVectorGetX(scale));
	}
	else if (selectedEnttType_ == DIRECTED_LIGHT)
	{
		
	}
	else if (selectedEnttType_ == POINT_LIGHT)
	{
		Execute(new CmdEntityChangeVec3(CHANGE_POINT_LIGHT_POSITION, Vec3(translation)));
	}
	else if (selectedEnttType_ == SPOT_LIGHT)
	{

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
		case CHANGE_POINT_LIGHT_AMBIENT:
		case CHANGE_POINT_LIGHT_DIFFUSE:
		case CHANGE_POINT_LIGHT_SPECULAR:
		case CHANGE_POINT_LIGHT_POSITION:
		case CHANGE_POINT_LIGHT_RANGE:
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			ExecutePointLightCommand(pCommand);
		}
	}
}


// =================================================================================
// private API: entities data loading 
// =================================================================================

void EntityEditorController::LoadSkyEnttData(const uint32_t skyEnttID)
{
	// load/reload data of currently selected entity by ID (which is actually the sky entt)
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
	// load/reload data of currently selected entity by ID
	Vec3 position;
	Vec4 dirQuat;
	float uniScale = 0.0f;

	if (pFacade_->GatherEnttData(enttID, position, dirQuat, uniScale))
		enttModel_.SetData(position, dirQuat, uniScale);
}

///////////////////////////////////////////////////////////

void EntityEditorController::LoadLightEnttData(const uint32_t enttID)
{
	// load/reload data of currently selected entity by ID (which is a light source)

	switch (selectedEnttType_)
	{
		case DIRECTED_LIGHT:
		{
			break;
		}
		case POINT_LIGHT:
		{
			Model::EntityPointLight& model = pointLightModel_;

			pFacade_->GetEnttPointLightData(
				selectedEnttID_,
				model.ambient_,
				model.diffuse_,
				model.specular_,
				model.position_,
				model.range_,
				model.attenuation_);

			break;
		}
		case SPOT_LIGHT:
		{
			break;
		}
		default:
		{
			Log::Error("Unknown entity type: " + std::to_string(selectedEnttType_));
		}
	}
}



// =================================================================================
// private API: commands executors
// =================================================================================

void EntityEditorController::ExecuteUsualEnttCommand(ICommand* pCommand)
{
	// execute the entity changes according to the input command

	Model::Entity& data = enttModel_;


	// execute changes according to the command type
	switch (pCommand->type_)
	{
		case CHANGE_POSITION:
		{
			const Vec3 newPos = pCommand->GetVec3();

			if (pFacade_->SetEnttPosition(selectedEnttID_, newPos))
			{
				data.SetPosition(newPos);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_ROTATION:
		{
			const Vec4 newRotation = pCommand->GetVec4();

			if (pFacade_->SetEnttRotation(selectedEnttID_, newRotation))
			{
				data.SetRotation(newRotation);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_SCALE:
		{
			const float newUniScale = pCommand->GetFloat();

			if (pFacade_->SetEnttUniScale(selectedEnttID_, newUniScale))
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

///////////////////////////////////////////////////////////

void EntityEditorController::ExecutePointLightCommand(ICommand* pCommand)
{
	switch (pCommand->type_)
	{
		case CHANGE_POINT_LIGHT_AMBIENT:
		{
			ColorRGBA color = pCommand->GetColorRGBA();
			pFacade_->SetPointLightAmbient(selectedEnttID_, color);
			pointLightModel_.ambient_ = color;
			// TODO: save this command into events history
			break;
		}
		case CHANGE_POINT_LIGHT_DIFFUSE:
		{
			ColorRGBA color = pCommand->GetColorRGBA();
			pFacade_->SetPointLightDiffuse(selectedEnttID_, color);
			pointLightModel_.diffuse_ = color;
			// TODO: save this command into events history
			break;
		}
		case CHANGE_POINT_LIGHT_SPECULAR:
		{
			ColorRGBA color = pCommand->GetColorRGBA();
			pFacade_->SetPointLightSpecular(selectedEnttID_, color);
			pointLightModel_.specular_ = color;
			// TODO: save this command into events history
			break;
		}
		case CHANGE_POINT_LIGHT_POSITION:
		{
			Vec3 pos = pCommand->GetVec3();

			pFacade_->SetPointLightPos(selectedEnttID_, pos);
			pFacade_->SetEnttPosition(selectedEnttID_, pos);
			pointLightModel_.position_ = pos;
			// TODO: save this command into events history
			break;
		}
		case CHANGE_POINT_LIGHT_RANGE:
		{
			float range = pCommand->GetFloat();

			pFacade_->SetPointLightRange(selectedEnttID_, range);
			pointLightModel_.range_ = range;
			// TODO: save this command into events history
			break;
		}
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			Vec3 att = pCommand->GetVec3();
			
			pFacade_->SetPointLightAttenuation(selectedEnttID_, att);
			pointLightModel_.attenuation_ = att;
			// TODO: save this command into events history
			break;
		}
	}

	
}