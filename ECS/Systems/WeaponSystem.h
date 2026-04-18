/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: WeaponSystem.h
    Desc:     ECS system to handle weapons

    Created:  11.04.2026 by DimaSkup
\**********************************************************************************/
#pragma once
#include "../Components/Weapon.h"

namespace ECS
{

class WeaponSystem
{
public:
    WeaponSystem(WeaponComp* pWeaponComp);

    bool AddRecord(const EntityID id, const Weapon& wpnData);

    const Weapon& GetWeaponById(const EntityID id) const;

private:
    WeaponComp* pWpnComp_ = nullptr;
};

} // namespace
