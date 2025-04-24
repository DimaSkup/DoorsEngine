#include "../Common/pch.h"
#include "PlayerSystem.h"

using namespace DirectX;

namespace ECS
{

PlayerSystem::PlayerSystem(
    TransformSystem* pTransformSys,
    CameraSystem* pCameraSys)
    :
    pTransformSys_(pTransformSys),
    pCameraSys_(pCameraSys)
{
    Assert::True(pTransformSys != nullptr, "input ptr to transform system == nullptr");
    Assert::True(pCameraSys != nullptr,    "input ptr to camera system == nullptr");
}


// =================================================================================
// Player movement
// =================================================================================
void PlayerSystem::Strafe(const float d)
{
    // move left/right the player by distance d
    const XMVECTOR  s     = XMVectorReplicate(d);
    const XMVECTOR& right = pCameraSys_->GetRightVec(playerID_);
    const XMVECTOR& pos   = pTransformSys_->GetPositionVec(playerID_);

    // pos += d * right_vec
    pTransformSys_->SetPositionVec(playerID_, XMVectorMultiplyAdd(s, right, pos));
}

///////////////////////////////////////////////////////////

void PlayerSystem::Walk(const float d)
{
    // move forward/backward the player by distance d
    const XMVECTOR  s = XMVectorReplicate(d);
    XMVECTOR pos;
    XMVECTOR look;

    // pos += d * look_vec
    pTransformSys_->GetPosAndDir(playerID_, pos, look);
    pTransformSys_->SetPositionVec(playerID_, XMVectorMultiplyAdd(s, look, pos));   
}

///////////////////////////////////////////////////////////

void PlayerSystem::MoveUp(const float d)
{
    // if the player in free fly mode:
    
    if (data_.isFreeFlyMode)
    {
        XMVECTOR s = XMVectorReplicate(d);
        const XMVECTOR pos = pTransformSys_->GetPositionVec(playerID_);

        // pos_ += d * world_up_vec
        pTransformSys_->SetPositionVec(playerID_, XMVectorMultiplyAdd(s, {0,1,0}, pos));
    }
}


// =================================================================================
// Player rotation
// =================================================================================
void PlayerSystem::Pitch(const float angle)
{
    // rotate the look vector about the view space right vector

    LogDbg("pitch player camera");
    const XMVECTOR& right = pCameraSys_->GetRightVec(playerID_);
    const XMMATRIX R = XMMatrixRotationAxis(right, angle);
    const XMVECTOR look = pTransformSys_->GetDirectionVec(playerID_);

    pTransformSys_->SetDirection(playerID_, XMVector3TransformNormal(look, R));
}

///////////////////////////////////////////////////////////

void PlayerSystem::RotateY(const float angle)
{
    // rotate the basis vectors about the world's y-axis

    const XMMATRIX R = DirectX::XMMatrixRotationY(angle);
    const EntityID id = playerID_;
    
    // update right vector of the camera
    const XMVECTOR right = pCameraSys_->GetRightVec(id);
    pCameraSys_->SetRightVec(id, XMVector3TransformNormal(right, R));

    // update look vector (direction) of the player
    const XMVECTOR look = pTransformSys_->GetDirectionVec(id);
    pTransformSys_->SetDirection(id, XMVector3TransformNormal(look, R));
}

}; // namespace ECS
