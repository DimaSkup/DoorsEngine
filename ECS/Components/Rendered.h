// *********************************************************************************
// Filename:     Rendered.h
// Description:  an ECS component which adds entities for rendering
// 
// Created:      21.05.24
// *********************************************************************************
#pragma once

#include <Types.h>
#include <cvector.h>

namespace ECS
{

struct Rendered
{
    cvector<EntityID>                   ids;                    // renderable entities (can be visible)
    cvector<EntityID>                   visibleEnttsIDs;        // currently visible entts (models) for this frame
    cvector<EntityID>                   visiblePointLightsIDs;  // currently visible point light sources
};

}
