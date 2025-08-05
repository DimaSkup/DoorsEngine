// =================================================================================
// Filename:    EnttParticlesData.h
// Description: entity particles data containers for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     04.08.2025  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/Vectors.h>
#include <UICommon/Color.h>


namespace UI
{

// container of particles data (MVC model)
struct EnttParticlesModel
{
    ColorRGB color          = { 0,1,0 };
    Vec3     externForce    = {0,0,0};
    int      spawnNumPerSec = 1;
    int      lifespanMs     = 1000;
    float    mass           = 1.0f;
    float    size           = 1.0f;
    float    friction       = 0.01f;
};

} // namespace 
