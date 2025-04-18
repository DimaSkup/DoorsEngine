// =================================================================================
// Filename:     EditorCommands.h
// Description:  defines commands according to which we execute changes
// 
// Created:      01.01.25
// =================================================================================
#pragma once

#include "ICommand.h"


namespace UI
{

enum eEditorCmdType
{
	INVALID_CMD,

	// model
	CHANGE_ENTITY_POSITION,
	CHANGE_ENTITY_DIRECTION,
	CHANGE_ENTITY_SCALE,

	// sky
	CHANGE_SKY_COLOR_CENTER,    // change the horizon color of the sky gradient
	CHANGE_SKY_COLOR_APEX,      // change the top color of the sky gradient
	CHANGE_SKY_OFFSET,          // change offset of the sky mesh

	// directed light
	CHANGE_DIR_LIGHT_AMBIENT,
	CHANGE_DIR_LIGHT_DIFFUSE,
	CHANGE_DIR_LIGHT_SPECULAR,

	// point light
	CHANGE_POINT_LIGHT_AMBIENT,
	CHANGE_POINT_LIGHT_DIFFUSE,
	CHANGE_POINT_LIGHT_SPECULAR,
	CHANGE_POINT_LIGHT_ATTENUATION,
	CHANGE_POINT_LIGHT_RANGE,

	// spotlight
	CHANGE_SPOT_LIGHT_AMBIENT,
	CHANGE_SPOT_LIGHT_DIFFUSE,
	CHANGE_SPOT_LIGHT_SPECULAR,
	CHANGE_SPOT_LIGHT_RANGE,           // how far spotlight can lit
	CHANGE_SPOT_LIGHT_ATTENUATION,
	CHANGE_SPOT_LIGHT_SPOT_EXPONENT,   // light intensity fallof (for control the spotlight cone)

    // fog
    CHANGE_FOG_ENABLED,   // enable/disabled the fog on the scene
	CHANGE_FOG_COLOR,
	CHANGE_FOG_START,     // distance where for starts
	CHANGE_FOG_RANGE,     // distance after which the objects are fully fogged
};

///////////////////////////////////////////////////////////

class CmdChangeVec3 : public ICommand
{
public:
	CmdChangeVec3(const eEditorCmdType type, const Vec3& vec3)
		: ICommand(type, vec3) {}
};

///////////////////////////////////////////////////////////

class CmdChangeVec4 : public ICommand
{
public:
	CmdChangeVec4(const eEditorCmdType type, const Vec4& vec4)
		: ICommand(type, vec4) {}
};

///////////////////////////////////////////////////////////

class CmdChangeFloat : public ICommand
{
public:
	CmdChangeFloat(const eEditorCmdType type, const float value)
		: ICommand(type, value) {}
};

///////////////////////////////////////////////////////////

class CmdChangeColor : public ICommand
{
public:
	CmdChangeColor(const eEditorCmdType type, const ColorRGB& color)
		: ICommand(type, color) {}

	CmdChangeColor(const eEditorCmdType type, const ColorRGBA& color)
		: ICommand(type, color) {}
};

} // namespace UI
