// =================================================================================
// Filename:       EntityEditorController.cpp
// Created:        01.01.25
// =================================================================================
#include "EntityEditorController.h"

#include "../../../Common/Assert.h"
#include "../../../Common/log.h"



void EntityEditorController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Assert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
	pFacade_ = pFacade;

	// initialize the entity editor model with default values
	Vec3 position(0, 0, 0);
	Vec4 dirQuat(0, 0, 0, 1);
	float uniScale = 0.0f;

	enttModel_.Update(0, position, dirQuat, uniScale);
}

///////////////////////////////////////////////////////////

void EntityEditorController::Update(const uint32_t entityID)
{
	// initialize the entity editor model
	Vec3 position(0, 0, 0);
	Vec4 dirQuat(0, 0, 0, 1);
	float uniScale = 0.0f;

	if (pFacade_->GatherEnttData(entityID, position, dirQuat, uniScale))
		enttModel_.Update(entityID, position, dirQuat, uniScale);
	else
		Log::Error("can't gather data for the entity editor :(");
}

///////////////////////////////////////////////////////////

void EntityEditorController::Render()
{
	float cameraViewMatrix[16]{0};
	float cameraProjMatrix[16]{0};

	uint32_t cameraID;
	pFacade_->GetEnttIDByName("editor_camera", cameraID);
	pFacade_->GetCameraViewAndProj(cameraID, cameraViewMatrix, cameraProjMatrix);

	enttView_.Draw(&enttModel_, cameraViewMatrix, cameraProjMatrix); 
}

///////////////////////////////////////////////////////////

void EntityEditorController::Execute(ICommand* pCommand)
{
	if ((pCommand == nullptr) ||
		(pFacade_ == nullptr) || 
		(enttModel_.GetEntityID() == 0))
	{
		Log::Error("can't execute a command of type: " + std::to_string(pCommand->type_));
		return;
	}

	using enum EntityEditorCmdType;   // for not writing "EntityEditorCmdType::" before each case
	Model::Entity& data = enttModel_;

	// execute changes according to the command type
	switch (pCommand->type_)
	{
		case CHANGE_POSITION:
		{
			const Vec3 newPos = pCommand->GetVec3();

			if (pFacade_->SetEnttPosition(data.GetEntityID(), newPos))
			{
				data.SetPosition(newPos);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_ROTATION:
		{
			const Vec4 newRotation = pCommand->GetVec4();

			if (pFacade_->SetEnttRotation(data.GetEntityID(), newRotation))
			{
				data.SetRotation(newRotation);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_SCALE:
		{
			const float newUniScale = pCommand->GetFloat();

			if (pFacade_->SetEnttUniScale(data.GetEntityID(), newUniScale))
			{
				data.SetUniformScale(newUniScale);
				// TODO: store the command into the events history
			}
			break;
		}
	}
}