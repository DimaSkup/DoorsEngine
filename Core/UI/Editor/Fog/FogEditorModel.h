// =================================================================================
// Filename:    FogEditorModel.h
// Description: holds everything that is fog data related 
//              (and used for the editor)
// Created:     
// =================================================================================
#pragma once

#include "../../UICommon/Color.h"


namespace Model
{

class Fog
{
private:
	ColorRGB fogColor_{ 0.5f, 0.5f, 0.5f };
	float fogStart_ = 0.0f;
	float fogRange_ = 0.0f;

public:

	void Update(const ColorRGB& fogColor, const float fogStart, const float fogRange)
	{
		// update the Model with actual data so the View will show
		// fields with current fog values
		fogColor_ = fogColor;
		fogStart_ = fogStart;
		fogRange_ = fogRange;
	}

	inline void SetColor(const ColorRGB& color) { fogColor_ = color; }
	inline void SetStart(const float start)     { fogStart_ = start; }
	inline void SetRange(const float range)     { fogRange_ = range; }

	void GetData(ColorRGB& fogColor, float& fogStart, float& fogRange) const
	{
		fogColor = fogColor_;
		fogStart = fogStart_;
		fogRange = fogRange_;
	}

	inline const ColorRGB& GetColor() const { return fogColor_; }
	inline float GetStart()           const { return fogStart_; }
	inline float GetRange()           const { return fogRange_; }

};

}