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


namespace ECS
{
   
class PlayerSystem
{
public:
    PlayerSystem(
        TransformSystem* pTransformSys,
        CameraSystem* pCameraSys);


    inline XMFLOAT3 GetPosition()  const { return pTransformSys_->GetPosition(playerID_); }
    inline XMFLOAT3 GetDirection() const { return pTransformSys_->GetDirection(playerID_); }
    inline XMVECTOR GetPosVec()    const { return pTransformSys_->GetPositionVec(playerID_); }
    inline XMVECTOR GetDirVec()    const { return pTransformSys_->GetDirectionVec(playerID_); }
    inline float    GetSpeed()     const { return data_.currSpeed; }
    inline float    GetWalkSpeed() const { return data_.walkSpeed; }
    inline float    GetRunSpeed()  const { return data_.runSpeed; }

    // strafe/walk the player by distance d
    void Strafe(const float d);
    void Walk  (const float d);
    void MoveUp(const float d);

    // rotate the player
    void Pitch  (const float angle);
    void RotateY(const float angle);

    ///////////////////////////////////////////////////////////



    // set/get movement speed
    void SetIsRunning(const bool state)
    {
        data_.isRunning = state;
        data_.currSpeed = (state) ? data_.runSpeed : data_.walkSpeed;
    }

    void SetWalkSpeed(const float speed) { data_.walkSpeed = (speed > 0) ? speed : 10.0f; }
    void SetRunSpeed (const float speed) { data_.runSpeed  = (speed > 0) ? speed : 20.0f; }

    inline bool IsFreeFlyMode()                    const { return data_.isFreeFlyMode; }
    inline void SetFreeFlyMode(const bool mode)          { data_.isFreeFlyMode = mode; }

    // get/set flashlight state
    inline void SwitchFlashLight(const bool state)       { data_.isTurnedOnFlashlight = state; }
    inline bool IsTurnedOnFlashLight()             const { return data_.isTurnedOnFlashlight; }

private:
    TransformSystem* pTransformSys_ = nullptr;
    CameraSystem* pCameraSys_ = nullptr;

    EntityID   playerID_ = INVALID_ENTITY_ID;
    PlayerData data_;
};

} // namespace ECS
