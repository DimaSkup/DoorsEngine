// =================================================================================
// Filename:    FogEditorModel.h
// Description: holds everything that is fog data related 
//              (and used for the editor)
// Created:     
// =================================================================================
#pragma once

#include <UICommon/Color.h>

namespace UI
{

struct FogData
{
    ColorRGB fogColor{ 0.5f, 0.5f, 0.5f };
    float    fogStart = 0.0f;
    float    fogRange = 0.0f;
    bool     fogEnabled = true;
};

///////////////////////////////////////////////////////////

class ModelFog
{
private:
    FogData data_;

public:

    void Update(
        const ColorRGB& fogColor,
        const float fogStart,
        const float fogRange,
        const bool fogEnabled)
    {
        // update the Model with actual data so the View will show
        // fields with current fog values
        data_.fogColor = fogColor;
        data_.fogStart = fogStart;
        data_.fogRange = fogRange;
        data_.fogEnabled = fogEnabled;
    }

    inline void SetColor(const ColorRGB& color) { data_.fogColor = color; }
    inline void SetStart(const float start)     { data_.fogStart = start; }
    inline void SetRange(const float range)     { data_.fogRange = range; }
    inline void SetEnabled(const bool enabled)  { data_.fogEnabled = enabled; }

    inline void GetData(FogData& outData) const { outData = data_; }

    inline const ColorRGB& GetColor()     const { return data_.fogColor; }
    inline float GetStart()               const { return data_.fogStart; }
    inline float GetRange()               const { return data_.fogRange; }
    inline float GetEnabled()             const { return data_.fogEnabled; }
};

} // namespace UI
