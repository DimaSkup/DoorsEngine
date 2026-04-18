/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: WeaponSystem.cpp
    Desc:     ECS system to handle weapons

    Created:  11.04.2026 by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "WeaponSystem.h"

namespace ECS
{

//---------------------------------------------------------
// Desc:  constructor
// Args:  pWeaponComponent - a ptr to weapons ECS component
//---------------------------------------------------------
WeaponSystem::WeaponSystem(WeaponComp* pWpnComp) :
    pWpnComp_(pWpnComp)
{
    if (!pWpnComp)
    {
        LogFatal(LOG, "input ptr to weapon component == NULL");
    }
}

//---------------------------------------------------------
// Desc:  bind a weapon component to entity by id
// Args:  id  - entity identifier
//        wpn - weapon parameters
//---------------------------------------------------------
bool WeaponSystem::AddRecord(const EntityID id, const ECS::Weapon& wpn)
{
    WeaponComp& comp = *pWpnComp_;

    if (comp.ids.binary_search(id))
    {
        LogErr(LOG, "there is already a record by id: %" PRIu32, id);
        return false;
    }

    // add a new record
    const index idx = comp.ids.get_insert_idx(id);
    comp.ids.insert_before(idx, id);
    comp.weapons.insert_before(idx, wpn);

    return true;
}

//---------------------------------------------------------
// Desc:  return weapon's data by input id
//---------------------------------------------------------
const Weapon& WeaponSystem::GetWeaponById(const EntityID id) const
{
    const index idx = pWpnComp_->ids.get_idx(id);

    if (!pWpnComp_->ids.is_valid_index(idx))
    {
        LogErr(LOG, "no weapon by id: %" PRIu32, id);
        return pWpnComp_->weapons[0];
    }

    return pWpnComp_->weapons[idx];
}

} // namespace
