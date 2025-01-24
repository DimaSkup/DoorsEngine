// =================================================================================
// Filename:     FogEditorCommands.h
// 
// Description:  defines commands according to which we execute 
//               updating of the fog using the editor
// 
// Created:      31.12.24
// =================================================================================
#pragma once

#include "../../UICommon/ICommand.h"

enum FogEditorCmdType
{
	CHANGE_FOG_COLOR,     
	CHANGE_FOG_START,     // distance where for starts
	CHANGE_FOG_RANGE,     // distance after which the objects are fully fogged
};

///////////////////////////////////////////////////////////

class CmdFogChangeColor : public ICommand
{
public:
	CmdFogChangeColor(const FogEditorCmdType type, const ColorRGB& color)
		: ICommand(type), color_(color) {}

	virtual ColorRGB GetColorRGB() const { return color_; }

	ColorRGB color_;
};

///////////////////////////////////////////////////////////

class CmdFogChangeFloat : public ICommand
{
public:
	CmdFogChangeFloat(const FogEditorCmdType type, const float value)
		: ICommand(type), value_(value) {}

	virtual float GetFloat() const { return value_; }

	float value_ = 0.0f;
};

///////////////////////////////////////////////////////////