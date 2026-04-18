/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: AnimationSystem.h
    Desc:     ECS system to handle animations (model skinning) of entities

    Created:  22.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include "../Components/Trigger.h"
#include <geometry/sphere.h>

namespace ECS
{

class TriggerSystem
{
public:
    TriggerSystem(Trigger* pTriggerComp);

    bool AddTriggerOnce(
        const EntityID enttId,
        const EventID eventId,
        const eTriggerShape shape,
        const Vec3& pos,
        const Vec3& ext);

    bool AddTriggerMultiple(
        const EntityID enttId,
        const EventID onEnter,
        const EventID onCollide,
        const EventID onLeave,
        const eTriggerShape shape,
        const Vec3& pos,
        const Vec3& ext);

    void GetTriggeredOnce    (const Sphere& sphere, cvector<index>& triggered);
    void GetTriggeredMultiple(const Sphere& sphere, cvector<index>& triggered);

private:
    // ptr to component
    Trigger* pTriggerComp_;
};

} // namespace
