// =================================================================================
// Filename:     EntityEditorCommands.h
// 
// Description:  defines commands according to which we execute 
//               updating of the entity using the editor
// 
// Created:      01.01.25
// =================================================================================
#pragma once

#include "../../UICommon/ICommand.h"

enum EntityEditorCmdType
{
	CHANGE_POSITION,
	CHANGE_ROTATION,
	CHANGE_SCALE,
	CHANGE_SKY_COLOR_CENTER,    // change the horizon color of the sky gradient
	CHANGE_SKY_COLOR_APEX,      // change the top color of the sky gradient
	CHANGE_SKY_OFFSET,          // change offset of the sky mesh
};

///////////////////////////////////////////////////////////

class CmdEntityChangeVec3 : public ICommand
{
public:
	CmdEntityChangeVec3(const EntityEditorCmdType type, const Vec3& vec3)
		: ICommand(type), vec3_(vec3) {}

	virtual Vec3 GetVec3() const { return vec3_; }

	Vec3 vec3_;
};

///////////////////////////////////////////////////////////

class CmdEntityChangeVec4 : public ICommand
{
public:
	CmdEntityChangeVec4(const EntityEditorCmdType type, const Vec4& vec4)
		: ICommand(type), vec4_(vec4) {}

	virtual Vec4 GetVec4() const { return vec4_; }

	Vec4 vec4_;
};

///////////////////////////////////////////////////////////

class CmdEntityChangeFloat : public ICommand
{
public:
	CmdEntityChangeFloat(const EntityEditorCmdType type, const float value)
		: ICommand(type), value_(value) {}

	virtual float GetFloat() const { return value_; }

	float value_;
};

///////////////////////////////////////////////////////////

class CmdChangeColor : public ICommand
{
public:
	CmdChangeColor(const EntityEditorCmdType type, const ColorRGB& color)
		: ICommand(type), color_(color) {}

	virtual ColorRGB GetColorRGB() const { return color_; }

	ColorRGB color_;
};