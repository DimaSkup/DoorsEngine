// =================================================================================
// Filename:    EnttLightData.h
// Description: entity light data containers for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     30.01.25
// =================================================================================
#pragma once

#include <UICommon/Color.h>
#include <UICommon/Vectors.h>


namespace UI
{

// =================================================================================
// Directed light data (for instance: sun)
// =================================================================================
struct EnttDirLightData 
{
public:
    ColorRGBA ambient;
    ColorRGBA diffuse;
    ColorRGBA specular;

public:
    inline void SetData(
        const ColorRGBA& inAmbient,
        const ColorRGBA& inDiffuse,
        const ColorRGBA& inSpecular)
    {
        ambient   = inAmbient;
        diffuse   = inDiffuse;
        specular  = inSpecular;
    }

    inline EnttDirLightData GetData() const { return *this; }
};


// =================================================================================
// Point light data (for instance: light bulb, candle)
// =================================================================================
class EnttPointLightData
{
public:
    ColorRGBA ambient;
    ColorRGBA diffuse;
    ColorRGBA specular;
    Vec3      position;
    float     range = 0;
    Vec3      attenuation;

public:
    inline void SetData(
        const ColorRGBA& inAmbient,
        const ColorRGBA& inDiffuse,
        const ColorRGBA& inSpecular,
        const float inRange,
        const Vec3& inAttenuation)
    {
        ambient     = inAmbient;
        diffuse     = inDiffuse;
        specular    = inSpecular;
        range       = inRange;
        attenuation = inAttenuation;
    }

    inline EnttPointLightData GetData() const { return *this; }
};


// =================================================================================
// Spotlight data (for instance: flashlight)
// =================================================================================
class EnttSpotLightData 
{
public:
    ColorRGBA ambient;
    ColorRGBA diffuse;
    ColorRGBA specular;

    Vec3 attenuation;
    float range   = 0.0f;
    float spotExp = 0.0f;    // spot exponent: light intensity fallof (for control the spotlight cone)

public:
    inline void SetData(
        const ColorRGBA& inAmbient,
        const ColorRGBA& inDiffuse,
        const ColorRGBA& inSpecular,
        const Vec3& inAttenuation,
        const float inRange,
        const float inSpotExp)
    {
        ambient     = inAmbient;
        diffuse     = inDiffuse;
        specular    = inSpecular;
        attenuation = inAttenuation;
        range       = inRange;
        spotExp     = inSpotExp;
    }

    inline EnttSpotLightData GetData() const { return *this; }
};

} // namespace UI
