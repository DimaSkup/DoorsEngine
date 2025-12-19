// =================================================================================
// Filename:     IEditorController.h
// Description:  declares a pure virtual interface for each editor controller
// =================================================================================
#pragma once

#include "icommand.h"
#include <stdint.h>

namespace UI
{

class IEditorController
{
public:
	// execute command and store this change into the events history
	virtual void ExecCmd(const ICommand* pCommand) = 0;

	// undo/alt_undo an event from the events history
	virtual void UndoCmd(const ICommand* pCommand, const EntityID entityId) = 0;
};

}
