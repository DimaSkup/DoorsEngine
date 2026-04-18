// =================================================================================
// Filename:   Trigger.h
// Desc:       an ECS component for triggers
//             (if something intersects a trigger's volume then
//             we will do some event by name)
// 
// Created:    14.04.26   by DimaSkup
// =================================================================================
#pragma once
#include <types.h>
#include <cvector.h>
#include <math/vec3.h>

namespace ECS
{

// =================================================================================
// HELPER DATA TYPES
// =================================================================================

enum eTriggerShape : uint8
{
    TRIGGER_SHAPE_AABB,
    TRIGGER_SHAPE_SPHERE,

    NUM_TRIGGER_SHAPE_TYPES
};

//---------------------------------------------------------
// common-purpose trigger: can be called ONLY once
//---------------------------------------------------------
struct TriggerOnce
{
    EventID         onEnterTrigger;  // exec only once when enter into trigger
    eTriggerShape   shapeType;
    Vec3            pos;             // in world space
    Vec3            ext;             // if shapeType == SPHERE then x,y,z stores radius
};

struct TriggersOnce
{
    cvector<EntityID>    ids;
    cvector<TriggerOnce> triggers;
};

//---------------------------------------------------------
// common-purpose trigger: can be called multiple times
//---------------------------------------------------------
struct TriggerMultiple
{
    EventID         onEnterTrigger;  // exec only once when enter into trigger
    EventID         onCollide;       // exec each frame when in trigger (can be empty)
    EventID         onLeaveTrigger;  // exec only once when leave trigger
    eTriggerShape   shapeType;
    Vec3            pos;             // in world space
    Vec3            ext;             // if shapeType == SPHERE then x,y,z stores radius
};

struct TriggersMultiple
{
    cvector<EntityID>        ids;
    cvector<TriggerMultiple> triggers;
};

#if 0
//---------------------------------------------------------
// gives damage
//---------------------------------------------------------
struct TriggerHurt
{
    EventID onEnterTrigger;  // exec only once when enter into trigger
    EventID onCollide;       // exec each frame when in trigger
    EventID onLeaveTrigger;  // exec only once when leave trigger
};

#endif

// =================================================================================
// ECS COMPONENT
// =================================================================================
struct Trigger
{
    TriggersOnce     triggersOnce;
    TriggersMultiple triggersMultiple;
};

} // namespace
