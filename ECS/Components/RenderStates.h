// *********************************************************************************
// Filename:     RenderStates.h
// Description:  an ECS component which is responsible for storing 
//               render states of entities
//
// Created:      28.08.24
// *********************************************************************************
#pragma once

#include "../Common/Types.h"
#include "../Common/cvector.h"

namespace ECS
{

// render states types
enum RSTypes
{
    // rasterizer params
    FILL_SOLID,
    FILL_WIREFRAME,
    CULL_BACK,
    CULL_FRONT,
    CULL_NONE,
    FRONT_COUNTER_CLOCKWISE,  // CCW
    FRONT_CLOCKWISE,

    // blending states
    NO_RENDER_TARGET_WRITES,
    NO_BLENDING,
    ALPHA_ENABLE,
    ADDING,
    SUBTRACTING,
    MULTIPLYING,
    TRANSPARENCY,

    // is used to render textures which is either completely opaque or completely 
    // transparent (for instance: wire fence, foliage, tree leaves); so if pixels
    // have alpha values close to 0 we can reject a src pixel from being futher processed
    NO_ALPHA_CLIPPING,
    ALPHA_CLIPPING,  
    
    // defines if the current entity reflects the other entities or not
    REFLECTION_PLANE,
    NOT_REFLECTION_PLANE,

    // to make possible iteration over the enum
    LAST_RS_TYPE   
};


struct EnttToStates
{
    EntityID id_;
    cvector<RSTypes> states_;
};

static const cvector<RSTypes> g_DefaultStates =
{
    FILL_SOLID,
    CULL_BACK,
    FRONT_COUNTER_CLOCKWISE,
    NO_BLENDING,
    NO_ALPHA_CLIPPING,
    NOT_REFLECTION_PLANE
};

static const cvector<RSTypes> g_AlphaClipCullNoneStates =
{
    FILL_SOLID,
    CULL_NONE,
    FRONT_COUNTER_CLOCKWISE,
    NO_BLENDING,
    ALPHA_CLIPPING,
    NOT_REFLECTION_PLANE
};

///////////////////////////////////////////////////////////

struct RenderStates
{
    ComponentType type_ = ComponentType::RenderStatesComponent;

    cvector<EntityID> ids_;
    cvector<u32> statesHashes_;    // hash where each bit responds for a specific render state
};


// --------------------------------------------------------

struct EnttsDefaultState
{
    cvector<EntityID> ids_;                              // default: fill solid, cull back, no blending, no alpha clipping

    void Clear() { ids_.clear(); }
};

// ------------------------------------------------

struct EnttsAlphaClipping
{
    cvector<EntityID> ids_;                              // for instance: fooliage, bushes, tree leaves (but no blending)

    void Clear() { ids_.clear(); }
};

// ------------------------------------------------

struct EnttsBlended
{
    cvector<EntityID>   ids_;
    cvector<size>       instanceCountPerBS_;
    cvector<RSTypes>    states_;                   // each instances set has its own blending state

    void Clear()
    {
        ids_.clear();
        instanceCountPerBS_.clear();
        states_.clear();
    }
};

// ------------------------------------------------

struct EnttsReflection
{
    cvector<EntityID> ids_;                              // reflection planes
    RSTypes     states_[6] =
    {
        FILL_SOLID,
        CULL_NONE,
        FRONT_COUNTER_CLOCKWISE,
        TRANSPARENCY,                // we need to see through reflection planes
        ALPHA_CLIPPING,
        REFLECTION_PLANE
    };  
    u32 hash_ = 0;

    void Clear() { ids_.clear(); }
};

// ------------------------------------------------

struct EnttsFarThanFog
{
    // container for IDs of entities that are farther than the fog range
    // so we render them as simply colored model without any additional computations
    cvector<EntityID> ids_;

    void Clear() { ids_.clear(); }
};


};  // namespace ECS
