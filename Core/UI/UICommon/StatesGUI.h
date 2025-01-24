#pragma once

#include "WndParams.h"

struct StatesGUI
{
	// flags to show windows
	bool showWndEngineOptions_ = false;
	bool showWndForEnttCreation_ = false;


	WndParams leftPanelParams_;
	WndParams sceneSpaceParams_;
	WndParams centerBottomPanelParams_;
	WndParams rightPanelParams_;
};