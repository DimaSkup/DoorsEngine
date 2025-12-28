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



namespace ECS
{
   
class PlayerSystem
{
public:
    PlayerSystem(
        TransformSystem* pTransformSys,
        CameraSystem*    pCameraSys,
        HierarchySystem* pHierarchySys);

    void Update(const float deltaTime);

    inline void SetPlayer(const EntityID id) { playerID_ = id; }

    inline void SetActiveWeapon(const EntityID id)       { data_.activeWeaponId = id; }
    inline EntityID GetActiveWeapon()              const { return data_.activeWeaponId; }

    inline EntityID GetPlayerID()  const { return playerID_; }
    inline XMFLOAT3 GetPosition()  const { return pTransformSys_->GetPosition(playerID_); }
    inline XMFLOAT3 GetDirection() const { return pTransformSys_->GetDirection(playerID_); }
    inline XMVECTOR GetPosVec()    const { return pTransformSys_->GetPositionVec(playerID_); }
    inline XMVECTOR GetDirVec()    const { return pTransformSys_->GetDirectionVec(playerID_); }

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

    inline uint64 GetPlayerStates() const { return data_.playerStates_; }

    inline void StopJump()
    {
        data_.playerStates_ &= ~(JUMP);
        data_.jumpOffset = 0;
    }

#if 0
    void ResetStates()
    {
        data_.playerStates_ &= ~(RUN | WALK | CRAWL | SHOOT | RELOADING);
    }
#endif

    void SetIsReloading(const bool reloading);
    void SetIsShooting (const bool shoot);
    void SetIsWalking  (void);
    void SetIsRunning  (const bool running);
    void SetIsMoving   (const bool moving) { isMoving = moving; }
    void SetIsIdle     (void);
    void SetIsDrawWeapon(const bool drawing) { isDrawWeapon = drawing; }


    inline bool IsReloading()        const { return data_.playerStates_ & RELOADING; }
    inline bool IsShooting()         const { return data_.playerStates_ & SHOOT; }
    inline bool IsMoving()           const { return isMoving; }
    inline bool IsWalking()          const { return data_.playerStates_ & WALK; }
    inline bool IsRunning()          const { return data_.playerStates_ & RUN; }
    inline bool IsIdle()             const { return data_.playerStates_ & IDLE; }
    inline bool IsDrawWeapon()       const { return isDrawWeapon; }

    inline bool IsFreeFlyMode()      const { return data_.playerStates_ & FREE_FLY; }
    inline bool IsFlashLightActive() const { return data_.playerStates_ & TURNED_FLASHLIGHT; }
    inline bool IsJump()             const { return data_.playerStates_ & JUMP; }

    
    void SetFreeFlyMode  (const bool mode);
    void SwitchFlashLight(const bool state);

 

private:
    TransformSystem* pTransformSys_ = nullptr;
    CameraSystem*    pCameraSys_    = nullptr;
    HierarchySystem* pHierarchySys_ = nullptr;

    EntityID   playerID_ = INVALID_ENTITY_ID;
    PlayerData data_;

    bool isMoving = false;
    bool isDrawWeapon = false;
};

} // namespace ECS
