// =================================================================================
// Filename:     SkyEditorCommands.h
// Description:  defines commands according to which we execute 
//               updating of the sky using the sky editor
// =================================================================================
#pragma once

#include "../../UICommon/ICommand.h"

enum SkyEditorCmdType
{
	CHANGE_COLOR_CENTER,    // change the horizon color of the sky gradient
	CHANGE_COLOR_APEX,      // change the top color of the sky gradient
	CHANGE_SKY_OFFSET,      // change offset of the sky mesh
};

///////////////////////////////////////////////////////////

class CmdChangeColor : public ICommand
{
public:
	CmdChangeColor(const SkyEditorCmdType type, const ColorRGB& color)          
		: ICommand(type), color_(color) {}

	virtual ColorRGB GetColorRGB() const { return color_; }

	ColorRGB color_;
};

///////////////////////////////////////////////////////////

class CmdChangeVec3 : public ICommand
{
public:
	CmdChangeVec3(const SkyEditorCmdType type, const Vec3& vec3)
		: ICommand(type), vec3_(vec3) {}

	virtual Vec3 GetVec3() const { return vec3_; }

	Vec3 vec3_;
};

///////////////////////////////////////////////////////////