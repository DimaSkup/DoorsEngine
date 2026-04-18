#include "../Common/pch.h"
#include "TriggerSystem.h"
#include <geometry/intersection_tests.h>

namespace ECS
{

//---------------------------------------------------------
// constructor
//---------------------------------------------------------
TriggerSystem::TriggerSystem(Trigger* pTriggerComp) : pTriggerComp_(pTriggerComp)
{
    if (!pTriggerComp)
        LogFatal(LOG, "input ptr to a TRIGGER component == NULL");
}

//---------------------------------------------------------
// add a trigger which can be activated only once
//---------------------------------------------------------
bool TriggerSystem::AddTriggerOnce(
    const EntityID enttId,
    const EventID eventId,
    const eTriggerShape shape,
    const Vec3& pos,
    const Vec3& ext)
{
    // check input args
    if (enttId == INVALID_ENTT_ID)
    {
        LogErr(LOG, "input entt id is invalid");
        return false;
    }
    if (eventId == INVALID_EVENT_ID)
    {
        LogErr(LOG, "input event id is invalid");
        return false;
    }

    cvector<EntityID>&    ids      = pTriggerComp_->triggersOnce.ids;
    cvector<TriggerOnce>& triggers = pTriggerComp_->triggersOnce.triggers;

    bool bUniqueTrigger = !ids.binary_search(enttId);
    if (!bUniqueTrigger)
    {
        LogErr(LOG, "there is already a trigger by id: %d (event id: %d)", (int)enttId, (int)eventId);
        return false;
    }

    // sorted insertion
    const index idx = ids.get_insert_idx(enttId);

    ids.insert_before(idx, enttId);
    triggers.insert_before(idx, TriggerOnce());

    // setup the trigger
    TriggerOnce& trigger = triggers[idx];
    trigger.onEnterTrigger = eventId;
    trigger.shapeType = shape;
    trigger.pos = pos;
    trigger.ext = ext;

    return true;
}

//---------------------------------------------------------
// add a trigger which can be activated multiple times
//---------------------------------------------------------
bool TriggerSystem::AddTriggerMultiple(
    const EntityID enttId,
    const EventID onEnter,
    const EventID onCollide,
    const EventID onLeave,
    const eTriggerShape shape,
    const Vec3& pos,
    const Vec3& ext)
{
    // check input args
    if (enttId == INVALID_ENTT_ID)
    {
        LogErr(LOG, "input entt id is invalid");
        return false;
    }


    if (onEnter == INVALID_EVENT_ID ||
        onCollide == INVALID_EVENT_ID ||
        onLeave == INVALID_EVENT_ID)
    {
        LogErr(LOG, "some input event id is invalid (onEnter: %d, onCollide: %d, onLeave: %d)", (int)onEnter, (int)onCollide, (int)onEnter);
        return false;
    }

    cvector<EntityID>&        ids      = pTriggerComp_->triggersMultiple.ids;
    cvector<TriggerMultiple>& triggers = pTriggerComp_->triggersMultiple.triggers;

    bool bUniqueTrigger = !ids.binary_search(enttId);
    if (!bUniqueTrigger)
    {
        LogErr(LOG, "there is already a trigger by id: %d (events onEnter: %d, onCollide: %d, onLeave: %d)", (int)onEnter, (int)onCollide, (int)onEnter);
        return false;
    }

    // sorted insertion
    const index idx = ids.get_insert_idx(enttId);

    ids.insert_before(idx, enttId);
    triggers.insert_before(idx, TriggerMultiple());

    // setup the trigger
    TriggerMultiple& trigger = triggers[idx];

    trigger.onEnterTrigger = onEnter;
    trigger.onCollide      = onCollide;
    trigger.onLeaveTrigger = onLeave;

    trigger.shapeType = shape;
    trigger.pos = pos;
    trigger.ext = ext;

    return true;
}

//---------------------------------------------------------
// Desc:  get triggers which can be activated only once
// Args:  inSphere  - test collision of triggers agains this sphere
// Out:   triggered - arr of indices to triggered triggers
//---------------------------------------------------------
void TriggerSystem::GetTriggeredOnce(const Sphere& inSphere, cvector<index>& triggered)
{
    triggered.reserve(8);

    sizeof(Vec3);
    sizeof(TriggerOnce);
    sizeof(TriggerMultiple);

    const cvector<TriggerOnce>& triggers = pTriggerComp_->triggersOnce.triggers;

    for (index i = 0; i < triggers.size(); ++i)
    {
        const TriggerOnce& trigger = triggers[i];
        bool bIntersect = false;

        // test sphere/AABB
        if (trigger.shapeType == TRIGGER_SHAPE_AABB)
        {
            const Rect3d rect = { trigger.pos, trigger.ext };
            bIntersect = IntersectRectSphere(rect, inSphere);
        }

        // test sphere/sphere
        else if (trigger.shapeType == TRIGGER_SHAPE_SPHERE)
        {
            const float radius = trigger.ext.x;
            const Sphere sphere = { trigger.pos, radius };
            bIntersect = IntersectSphereSphere(sphere, inSphere);
        }

        // ERROR
        else
        {
            EntityID id = pTriggerComp_->triggersOnce.ids[i];
            LogErr(LOG, "unknown shape type (%d) for trigger (%d)", (int)trigger.shapeType, (int)id);
        }

        if (bIntersect)
        {
            // store an index of triggered trigger
            triggered.push_back(i);
        }
    }
}

} // namespace
