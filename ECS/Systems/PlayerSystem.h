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

    inline EntityID GetPlayerID()  const { return playerID_; }
    inline XMFLOAT3 GetPosition()  const { return pTransformSys_->GetPosition(playerID_); }
    inline XMFLOAT3 GetDirection() const { return pTransformSys_->GetDirection(playerID_); }
    inline XMVECTOR GetPosVec()    const { return pTransformSys_->GetPositionVec(playerID_); }
    inline XMVECTOR GetDirVec()    const { return pTransformSys_->GetDirectionVec(playerID_); }

    inline float    GetSpeed()     const { return data_.currSpeed; }
    inline float    GetWalkSpeed() const { return data_.walkSpeed; }
    inline float    GetRunSpeed()  const { return data_.runSpeed; }
    inline float    GetJumpOffset() const { return data_.jumpOffset; }

    constexpr inline uint64 GetFlagsMove() const { return (MOVE_FORWARD | MOVE_BACK | MOVE_RIGHT | MOVE_LEFT | MOVE_UP | MOVE_DOWN); }

    inline bool IsMoving()           const { return data_.playerStates_ & GetFlagsMove(); }
    inline bool IsFreeFlyMode()      const { return data_.playerStates_ & FREE_FLY; }
    inline bool IsFlashLightActive() const { return data_.playerStates_ & TURNED_FLASHLIGHT; }
    inline bool IsJump()             const { return data_.playerStates_ & JUMP; }
    

    // set movement state
    void Move(ePlayerState movementState);

    // rotate the player
    void Pitch  (float angle);
    void RotateY(float angle);

    inline void SetCurrentSpeed(const float speed) { data_.currSpeed = (speed > 0) ? speed : 5; }
    inline void SetWalkSpeed   (const float speed) { data_.walkSpeed = (speed > 0) ? speed : 5; }
    inline void SetRunSpeed    (const float speed) { data_.runSpeed  = (speed > 0) ? speed : 20; }
    inline void SetMinVerticalOffset(const float minOffset) { data_.minVerticalOffset = minOffset; }

    inline void StopJump()
    {
        data_.playerStates_ &= ~(JUMP);
        data_.jumpOffset = 0;
    }
    
    void SetIsRunning    (const bool isRun);
    void SetFreeFlyMode  (const bool mode);
    void SwitchFlashLight(const bool state);

private:
    TransformSystem* pTransformSys_ = nullptr;
    CameraSystem*    pCameraSys_    = nullptr;
    HierarchySystem* pHierarchySys_ = nullptr;

    EntityID   playerID_ = INVALID_ENTITY_ID;
    PlayerData data_;

};

} // namespace ECS
