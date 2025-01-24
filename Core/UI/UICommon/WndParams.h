#pragma once

#include <imgui.h>

// params of windows
struct WndParams
{
	ImVec2 size_ = { 0,0 };   // dimensions
	ImVec2 pos_  = { 0,0 };   // upper left pos of the window
};