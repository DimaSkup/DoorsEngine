// =================================================================================
// Filename:     PlayerSystem.h
// Description:  an ECS system to handle First-Person-Shooter (FPS) player's data
//
// Created:      22.04.24
// =================================================================================
#pragma once

#include "../Components/Player.h"
#include "../Systems/TransformSystem.h"
#include "../Systems/CameraSystem.h"
#include "../Systems/HierarchySystem.h"
#include "../Systems/WeaponSystem.h"


namespace ECS
{
   
class PlayerSystem
{
private:
    static const uint MAX_NUM_PLAYER_WEAPONS = 8;

public:
    PlayerSystem(
        TransformSystem* pTransformSys,
        CameraSystem*    pCameraSys,
        HierarchySystem* pHierarchySys,
        WeaponSystem*    pWeaponSys);

    void Update(const float deltaTime);

    inline PlayerData& GetData() { return data_; }
    inline void SetPlayer(const EntityID id) { playerID_ = id; }

    inline void          SetActiveWeaponId(const EntityID id)       { data_.activeWeaponId = id; }
    inline EntityID      GetActiveWeaponId()                  const { return data_.activeWeaponId; }
    inline const Weapon& GetActiveWeapon()                    const { return pWeaponSys_->GetWeaponById(GetActiveWeaponId()); }

    inline EntityID          GetPlayerID()  const { return playerID_; }
    inline DirectX::XMFLOAT3 GetPosition()  const { return pTransformSys_->GetPosition(playerID_); }
    inline DirectX::XMFLOAT3 GetDirection() const { return pTransformSys_->GetDirection(playerID_); }
    inline DirectX::XMVECTOR GetPosVec()    const { return pTransformSys_->GetPositionVec(playerID_); }
    inline DirectX::XMVECTOR GetDirVec()    const { return pTransformSys_->GetDirectionVec(playerID_); }

    // setup movement speed
    inline float    GetSpeed()              const { return data_.currSpeed; }
    inline float    GetSpeedWalk()          const { return data_.speedWalk; }
    inline float    GetSpeedRun()           const { return data_.speedRun; }
    inline float    GetSpeedCrawl()         const { return data_.speedCrawl; }
    inline float    GetSpeedFreeFly()       const { return data_.speedFreeFly; }

    // setup jump
    inline float    GetJumpOffset()         const { return data_.jumpOffset; }
    inline float    GetOffsetOverTerrain()  const { return data_.offsetOverTerrain; }
    inline float    GetJumpMaxHeight()      const { return data_.jumpMaxHeight; }

    constexpr inline uint64 GetFlagsMove() const { return (MOVE_FORWARD | MOVE_BACK | MOVE_RIGHT | MOVE_LEFT | MOVE_UP | MOVE_DOWN); }

    inline float    GetCurrActTime() const { return data_.currActTime; }
    inline float    GetEndActTime() const { return data_.endActTime; }

    void            SetCurrActTime(const float time);
    void            SetEndActTime(const float time);

    AnimationID     GetCurrAnimId(void) const;
    void            SetCurrAnimId(const AnimationID animId);

    // player's weapons
    void     BindWeapon(const uint slot, const EntityID wpnId);
    EntityID GetWeapon (const uint slot);

    // set movement state
    void Move(ePlayerState movementState);

    // rotate the player
    void Pitch  (float angle);
    void RotateY(float angle);

    // setup movement speed
    inline void SetCurrentSpeed     (const float speed)     { if (speed > 0) data_.currSpeed    = speed; }
    inline void SetSpeedWalk        (const float speed)     { if (speed > 0) data_.speedWalk    = speed; }
    inline void SetSpeedRun         (const float speed)     { if (speed > 0) data_.speedRun     = speed; }
    inline void SetSpeedCrawl       (const float speed)     { if (speed > 0) data_.speedCrawl   = speed; }
    inline void SetSpeedFreeFly     (const float speed)     { if (speed > 0) data_.speedFreeFly = speed; }

    // setup height/jump
    inline void SetMinVerticalOffset(const float minOffset) { data_.minVerticalOffset = minOffset; }
    inline void SetOffsetOverTerrain(const float offset)    { data_.offsetOverTerrain = offset; }
    inline void SetJumpMaxHeight    (const float maxH)      { data_.jumpMaxHeight = maxH; }

    inline uint64 GetPlayerStates() const { return data_.playerStates; }

    inline void StopJump()
    {
        data_.playerStates &= ~(JUMP);
        data_.jumpOffset = 0;
    }

    bool IsSoundShotPlaying(void) const { return data_.soundShotPlaying; }
    void SetSoundShotPlaying(const bool onOff) { data_.soundShotPlaying = onOff; }


    void SetIsReloading(const bool reloading);
    void SetIsShooting (const bool shoot);
    void SetIsWalking  (void);
    void SetIsRunning  (const bool running);
    void SetIsMoving   (const bool moving) { isMoving = moving; }
    void SetIsIdle     (void);
    void SetIsDrawWeapon(const bool drawing) { isDrawWeapon = drawing; }


    inline bool IsReloading()        const { return data_.playerStates & RELOADING; }
    inline bool IsShooting()         const { return data_.playerStates & SHOOT; }
    inline bool IsMoving()           const { return isMoving; }
    inline bool IsWalking()          const { return data_.playerStates & WALK; }
    inline bool IsRunning()          const { return data_.playerStates & RUN; }
    inline bool IsIdle()             const { return data_.playerStates & IDLE; }
    inline bool IsDrawWeapon()       const { return isDrawWeapon; }

    inline bool IsFreeFlyMode()      const { return data_.playerStates & FREE_FLY; }
    inline bool IsFlashLightActive() const { return data_.playerStates & TURNED_FLASHLIGHT; }
    inline bool IsJump()             const { return data_.playerStates & JUMP; }

    
    void SetFreeFlyMode  (const bool mode);
    void SwitchFlashLight(const bool state);

public:
    PlayerData       data_;

private:
    TransformSystem* pTransformSys_ = nullptr;
    CameraSystem*    pCameraSys_    = nullptr;
    HierarchySystem* pHierarchySys_ = nullptr;
    WeaponSystem*    pWeaponSys_    = nullptr;

    EntityID         playerID_ = INVALID_ENTT_ID;
   

    EntityID         weaponsIds_[MAX_NUM_PLAYER_WEAPONS];
    size             numWeapons_;

    bool             isMoving = false;
    bool             isDrawWeapon = false;
};


//==================================================================================
// INLINE FUNCTIONS
//==================================================================================
inline void PlayerSystem::SetCurrActTime(const float time)
{
    if (time < 0) return;
    data_.currActTime = time;
}

inline void PlayerSystem::SetEndActTime(const float time)
{
    if (time < 0) return;
    data_.endActTime = time;
}

//---------------------------------------------------------
// get/set current animation for the player
//---------------------------------------------------------
inline AnimationID PlayerSystem::GetCurrAnimId() const
{
    return data_.currAnimId;
}

inline void PlayerSystem::SetCurrAnimId(const AnimationID animId)
{
    data_.currAnimId = animId;
}

} // namespace ECS
