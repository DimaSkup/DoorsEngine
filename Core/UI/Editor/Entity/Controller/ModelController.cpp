// =================================================================================
// Filename:      ModelController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include "ModelController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <format>


namespace UI
{

ModelController::ModelController()
{
}

///////////////////////////////////////////////////////////

void ModelController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
	pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void ModelController::ExecuteCommand(const ICommand* pCmd, const uint32_t enttID)
{
	// execute the entity changes according to the input command
	switch (pCmd->type_)
	{
		case CHANGE_MODEL_POSITION:
		{
			ExecChangePosition(enttID, pCmd->GetVec3());
			break;
		}
		case CHANGE_MODEL_ROTATION:
		{
			ExecChangeRotationQuat(enttID, pCmd->GetVec4());
			break;
		}
		case CHANGE_MODEL_SCALE:
		{
			ExecChangeUniformScale(enttID, pCmd->GetFloat());
			break;
		}
		default:
		{
			Core::Log::Error("Unknown command to execute: " + std::to_string(pCmd->type_));
		}
	}
}

///////////////////////////////////////////////////////////

void ModelController::UndoCommand(const ICommand* pCmd, const uint32_t enttID)
{
	switch (pCmd->type_)
	{
		case CHANGE_MODEL_POSITION:
		{
			UndoChangePosition(enttID, pCmd->GetVec3());
			break;
		}
		case CHANGE_MODEL_ROTATION:
		{
			UndoChangeRotation(enttID, pCmd->GetVec4());
			break;
		}
		case CHANGE_MODEL_SCALE:
		{
			UndoChangeScale(enttID, pCmd->GetFloat());
			break;
		}
		default:
		{
			Core::Log::Error("unknown undo command for entity (model): " + std::to_string(enttID));
			return;
		}
	}
}

///////////////////////////////////////////////////////////

void ModelController::LoadEnttData(const uint32_t enttID)
{
	// load/reload data of currently selected entity by ID
	// so we will be able to see refreshed data in the editor
	Vec3 position;
	Vec4 rotQuat;
	float uniScale = 0.0f;

	if (pFacade_->GetEnttData(enttID, position, rotQuat, uniScale))
		enttModel_.SetData(position, rotQuat, uniScale);
}


// =================================================================================
// Private API: commands executors
// =================================================================================

void ModelController::ExecChangePosition(
	const uint32_t enttID,
	const Vec3& newPos)
{
	// execute some command and store it into the events history

	Vec3 oldPos = pFacade_->GetEnttPosition(enttID);

	if (pFacade_->SetEnttPosition(enttID, newPos))
	{
		// update editor fields
		enttModel_.SetPosition(newPos);

		// generate an "undo" command and store it into the history
		const CmdChangeVec3 undoCmd(CHANGE_MODEL_POSITION, oldPos);
		const std::string msg = std::format("changed posisition of entt (type: model; id: {})", enttID);
		gEventsHistory.Push(undoCmd, msg, enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change position of entt (type: model; id: {})", enttID));
	}
}

///////////////////////////////////////////////////////////

void ModelController::ExecChangeRotationQuat(const uint32_t enttID, const Vec4& rotQuat)
{
	// update rotation for entity by ID and store this event into the history

	Vec4 oldRotationQuat = pFacade_->GetEnttRotationQuat(enttID);

	if (pFacade_->SetEnttRotationQuat(enttID, rotQuat))
	{
		// update editor fields
		enttModel_.SetRotation(rotQuat);

		// generate an "undo" command and store it into the history
		const CmdChangeVec4 undoCmd(CHANGE_MODEL_ROTATION, oldRotationQuat);
		const std::string msg = std::format("changed rotation of entt (type: model; id: {})", enttID);
		gEventsHistory.Push(undoCmd, msg, enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change rotation of entt (type: model; id: {})", enttID));
	}
}

///////////////////////////////////////////////////////////

void ModelController::ExecChangeUniformScale(const uint32_t enttID, const float uniformScale)
{
	// update uniform scale for entity by ID and store this event into the history

	float oldUniformScale = pFacade_->GetEnttScale(enttID);

	if (pFacade_->SetEnttUniScale(enttID, uniformScale))
	{
		// update editor fields
		enttModel_.SetUniformScale(uniformScale);

		// generate an "undo" command and store it into the history
		const CmdChangeFloat undoCmd(CHANGE_MODEL_SCALE, oldUniformScale);
		const std::string msg = std::format("changed uniform scale of entt (type: model; id: {})", enttID);
		gEventsHistory.Push(undoCmd, msg, enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change scale of entt (type: model; id: {})", enttID));
	}
}


// =================================================================================
// Private API: Undo / alternative undo
// =================================================================================

void ModelController::UndoChangePosition(const uint32_t enttID, const Vec3& pos)
{
	// undo change of the entity model position
	// (execute the reverted command - just set the prev position)

	if (pFacade_->SetEnttPosition(enttID, pos))            // update entity
		enttModel_.SetPosition(pos);                       // update editor fields
}

///////////////////////////////////////////////////////////

void ModelController::UndoChangeRotation(const uint32_t enttID, const Vec4& rotQuat)
{
	if (pFacade_->SetEnttRotationQuat(enttID, rotQuat))    // update entity
		enttModel_.SetRotation(rotQuat);                   // update editor fields
}

///////////////////////////////////////////////////////////

void ModelController::UndoChangeScale(const uint32_t enttID, const float uniformScale)
{
	if (pFacade_->SetEnttUniScale(enttID, uniformScale))   // update entity
		enttModel_.SetUniformScale(uniformScale);          // update editor fields
}

} // namespace UI