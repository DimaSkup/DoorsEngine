#include "../Common/pch.h"
#include "PlayerSystem.h"

using namespace DirectX;

namespace ECS
{

PlayerSystem::PlayerSystem(
    TransformSystem* pTransformSys,
    CameraSystem*    pCameraSys,
    HierarchySystem* pHierarchySys)
    :
    pTransformSys_(pTransformSys),
    pCameraSys_(pCameraSys),
    pHierarchySys_(pHierarchySys)
{
    CAssert::True(pTransformSys != nullptr, "input ptr to transform system == nullptr");
    CAssert::True(pCameraSys    != nullptr, "input ptr to camera system == nullptr");
    CAssert::True(pHierarchySys != nullptr, "input ptr to camera system == nullptr");
}

//---------------------------------------------------------
// Desc:    update the player's states
// Args:    - deltaTime: the time since the prev frame
//---------------------------------------------------------
void PlayerSystem::Update(const float deltaTime)
{
    uint64 states = data_.playerStates_;
    const float speed = GetSpeed() * deltaTime;

    XMVECTOR offset = { 0,0,0 };

    const EntityID id = playerID_;

    if (states & MOVE_FORWARD)
    {
        offset += pTransformSys_->GetDirectionVec(id);
    }
    if (states & MOVE_BACK)
    {
        offset -= pTransformSys_->GetDirectionVec(id);
    }
    if (states & MOVE_RIGHT)
    {
        offset += data_.rightVec;
    }
    if (states & MOVE_LEFT)
    {
        offset -= data_.rightVec;
    }
    if (states & JUMP)
    {
        // DO JUMP STUFF
    }
    if (states & MOVE_UP)
    {
        // if we in free fly mode we just move up/down (freely move by Y-axis)
        offset += { 0, 1, 0 };
    }
    if (states & MOVE_DOWN)
    {
        offset -= { 0, 1, 0 };
    }

    // normalize the movement direction vector and scale it according to player's speed
    offset = XMVector3Normalize(offset);
    offset = XMVectorMultiply(XMVectorReplicate(speed), offset);

    // adjust position of the player and its children
    cvector<EntityID> ids;
    pHierarchySys_->GetChildrenArr(id, ids);
    ids.push_back(id);

    // move player and all its children by offset
    pTransformSys_->AdjustPositions(ids.data(), ids.size(), offset);

    // reset all the movement states
    data_.playerStates_ &= ~(GetFlagsMove());
}

//---------------------------------------------------------
// Desc:   set a movement state for the player
// Args:   - state: expected to be some of the movement states
//---------------------------------------------------------
void PlayerSystem::Move(ePlayerState state)
{
    // if movement
    if (GetFlagsMove() & state)
        data_.playerStates_ |= state;
}

//---------------------------------------------------------
// Desc:   turn on/off the player's free fly mode
//---------------------------------------------------------
void PlayerSystem::SetFreeFlyMode(const bool mode)
{
    if (mode)
        data_.playerStates_ |= FREE_FLY;
    else
        data_.playerStates_ &= ~(FREE_FLY);
}

//---------------------------------------------------------
// Desc:   turn on/off the player's flashlight
//---------------------------------------------------------
void PlayerSystem::SwitchFlashLight(const bool state)
{
    // turn on the flashlight
    if (state)
        data_.playerStates_ |= TURNED_FLASHLIGHT;

    // turn off the flashlight
    else
        data_.playerStates_ &= ~(TURNED_FLASHLIGHT);
}

//---------------------------------------------------------
// Desc:   turn on/off the player's running state
//---------------------------------------------------------
void PlayerSystem::SetIsRunning(const bool isRun)
{
    if (isRun)
    {
        data_.playerStates_ |= RUN;
        data_.playerStates_ &= ~(WALK | CRAWL);
        data_.currSpeed = data_.runSpeed;
    }
    else
    {
        data_.playerStates_ &= ~(RUN | CRAWL);
        data_.playerStates_ |= WALK;
        data_.currSpeed = data_.walkSpeed;
    }
}

// =================================================================================
// Player rotation
// =================================================================================
bool ClampPitch(const float angle, float& pitch)
{
    // limit the pitch value in range(-PIDIV2+0.1 < yaw < PIDIV2-0.1)
    // return true if pitch was clamped

    pitch += angle;

    if (pitch > DirectX::XM_PIDIV2 - 0.1f)
    {
        pitch = DirectX::XM_PIDIV2 - 0.1f;
        return true;                         // we had to clamp the pitch so we can't rotate the player further
    }

    if (pitch < -DirectX::XM_PIDIV2 + 0.1f)
    {
        pitch = -DirectX::XM_PIDIV2 + 0.1f;
        return true;                         // we had to clamp the pitch so we can't rotate the player further
    }

    // we didn't have to clamp the pitch
    return false;
}

///////////////////////////////////////////////////////////

inline void ClampYaw(const float angle, float& yaw)
{
    // limit the yaw value in range (-2PI < yaw < 2PI)
    yaw += angle;
    yaw = (yaw > +DirectX::XM_2PI) ? -DirectX::XM_2PI : yaw;
    yaw = (yaw < -DirectX::XM_2PI) ? +DirectX::XM_2PI : yaw;
}

///////////////////////////////////////////////////////////

inline XMVECTOR RotateVecByQuat(const XMVECTOR& vec, const XMVECTOR& rotationQuat)
{
    // rotate input vector using quaternion(axis, angle)
    // 
    // NOTE!!!: input quat is supposed to be unit length

    // to help you to understand wtf is going on here :)
    // const XMVECTOR invQuat = XMQuaternionConjugate(rotationQuat);
    // const XMVECTOR tmpVec = XMQuaternionMultiply(invQuat, vec);
    // const XMVECTOR resVec = XMQuaternionMultiply(tmpVec, rotationQuat);
    // return resVec;

    // rotated_vec = inv_quat * orig_vec * quat; 
    return XMQuaternionMultiply(XMQuaternionMultiply(XMQuaternionConjugate(rotationQuat), vec), rotationQuat);
}

///////////////////////////////////////////////////////////

void PlayerSystem::Pitch(float angle)
{
    // rotate the look vector about the view space right vector
 
    // if we has to clamp pitch values we just do nothing and go out
    if (ClampPitch(angle, data_.pitch))
        return;

    // get arr of player's children entities
    cvector<EntityID> playerChildren;
    pHierarchySys_->GetChildrenArr(playerID_, playerChildren);

    // adjust position of each child relatively to the player
    const XMVECTOR playerPosW = pTransformSys_->GetPositionVec(playerID_);
    const XMVECTOR rotationQuat = DirectX::XMQuaternionRotationAxis(data_.rightVec, angle);
    const XMMATRIX R = XMMatrixRotationQuaternion(rotationQuat);

    for (int i = 0; const EntityID childID : playerChildren)
    {
        XMVECTOR childPosW    = pTransformSys_->GetPositionVec(childID);
        XMVECTOR relPos       = DirectX::XMVectorSubtract(childPosW, playerPosW);
        XMVECTOR newRelPos    = XMVector3Transform(relPos, R);
        XMVECTOR childNewPosW = XMVectorAdd(playerPosW, newRelPos);

        pTransformSys_->SetPositionVec(childID, childNewPosW);
    }

    // adjust rotation of the player and its each child
    pTransformSys_->RotateLocalSpaceByQuat(playerID_, rotationQuat);
    pTransformSys_->RotateLocalSpacesByQuat(playerChildren.data(), playerChildren.size(), rotationQuat);
}

///////////////////////////////////////////////////////////

void PlayerSystem::RotateY(float angle)
{
    // rotate the basis vectors about the world's y-axis

    ClampYaw(angle, data_.yaw);

    const EntityID id = playerID_;
    const XMVECTOR rotationQuat = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, angle);
    data_.rightVec = RotateVecByQuat(data_.rightVec, rotationQuat);

    // update look vector (direction) of the player
    const XMVECTOR playerPosW   = pTransformSys_->GetPositionVec(id);

    // get arr of player's children entities
    cvector<EntityID> playerChildren;
    pHierarchySys_->GetChildrenArr(id, playerChildren);

    // adjust position of each child relatively to the player
    const XMMATRIX R = DirectX::XMMatrixRotationY(angle);

    for (int i = 0; const EntityID childID : playerChildren)
    {
        const XMVECTOR childPosW    = pTransformSys_->GetPositionVec(childID);           // get current position of child
        const XMVECTOR relPos       = DirectX::XMVectorSubtract(childPosW, playerPosW);  // compute child position relatively to the player

        //XMVECTOR newRelPos = RotateVecByQuat(relPos, rotationQuat);
        XMVECTOR newRelPos    = XMVector3Transform(relPos, R);                     // compute new relative position of child
        XMVECTOR childNewPosW = XMVectorAdd(playerPosW, newRelPos);                // compute new world position of child

        pTransformSys_->SetPositionVec(childID, childNewPosW);
    }

    // adjust rotation of the player and its each child
    pTransformSys_->RotateLocalSpaceByQuat(id, rotationQuat);
    pTransformSys_->RotateLocalSpacesByQuat(playerChildren.data(), playerChildren.size(), rotationQuat);
}

}; // namespace ECS
