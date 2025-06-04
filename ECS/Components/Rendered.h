// *********************************************************************************
// Filename:     Rendered.h
// Description:  an ECS component which adds entities for rendering
// 
// Created:      21.05.24
// *********************************************************************************
#pragma once

#include "../Common/ECSTypes.h"
#include <Types.h>
#include <cvector.h>
#include <d3d11.h>

namespace ECS
{

struct RenderInitParams
{
    RenderShaderType         shaderType;
    D3D11_PRIMITIVE_TOPOLOGY topologyType;
};

///////////////////////////////////////////////////////////

struct Rendered
{
    cvector<EntityID>                   ids;
    cvector<RenderShaderType>           shaderTypes;
    cvector<D3D11_PRIMITIVE_TOPOLOGY>   primTopologies;

    cvector<EntityID>                   visibleEnttsIDs;        // currently visible entts (models) for this frame
    cvector<EntityID>                   visiblePointLightsIDs;  // currently visible point light sources
};

}
