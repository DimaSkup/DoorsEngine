#include "../Common/pch.h"
#include "PlayerSystem.h"


using namespace DirectX;

namespace ECS
{

// static array for internal purposes
static cvector<EntityID> s_Ids;


//---------------------------------------------------------
// just constructor
//---------------------------------------------------------
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
    const uint64 states   = data_.playerStates_;
    const int speedMul    = 1 + 9 * IsFreeFlyMode();  // move faster if we are in free fly
    const float speed     = GetSpeed() * deltaTime * speedMul;
    float horizontalSpeed = 0;

    XMVECTOR offset = { 0,0,0 };

    static float distInJump = 0;
    static bool  inJump   = false;
    static float jumpTime = 0;
    static float h0       = 0;

    const EntityID playerID = playerID_;


    if (states & MOVE_FORWARD)
    {
        offset += pTransformSys_->GetDirectionVec(playerID);
        horizontalSpeed = speed;
    }
    if (states & MOVE_BACK)
    {
        offset -= pTransformSys_->GetDirectionVec(playerID);
        horizontalSpeed = speed;
    }
    if (states & MOVE_RIGHT)
    {
        offset += data_.rightVec;
        horizontalSpeed = speed;
    }
    if (states & MOVE_LEFT)
    {
        offset -= data_.rightVec;
        horizontalSpeed = speed;
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

    if (states & JUMP)
    {
        // if we weren't in jump before
        if (!inJump)
        {
            inJump     = true;
            h0         = GetPosition().y;
            jumpTime   = 0;
            distInJump = 0;
        }
    }
    // we interrupted the jump or jump cycle is over
    else
    {
        data_.jumpOffset = 0.0f;
        inJump           = false;
        jumpTime         = 0.0f;
        distInJump       = 0;
    }


    // compute jump
    if (inJump)
    {
        constexpr float th     = 0.3f;        // time to reach the peak height
        constexpr float invTH  = 1.0f / th;

        // derive gravity and init. velocity in terms of peak height and duration to peak
        const float g = -2 * data_.jumpMaxHeight * (invTH * invTH);
        const float v0 = 2 * data_.jumpMaxHeight * invTH;

        // increate the time in jump
        float t = jumpTime;
        jumpTime += deltaTime;

        // compute current jump offset
        data_.jumpOffset = 0.5f * g * (t * t) + v0 * t + h0;

        // if jump cycle is over
        if ((jumpTime >= 2 * th) && (data_.jumpOffset <= data_.minVerticalOffset))
        {
            // turn off the player's jump state
            StopJump();
            inJump = false;
            jumpTime = 0;
            distInJump = 0;
        }
    }

    // normalize the movement direction vector and scale it according to player's speed
    offset = XMVector3Normalize(offset);
    offset = XMVectorMultiply(XMVectorReplicate(speed), offset);

    // compute player's new position
    const XMVECTOR playerPosVec = GetPosVec() + offset;
    XMFLOAT3 playerPos;
    XMStoreFloat3(&playerPos, playerPosVec);


    if (inJump)
    {
        // prevent falling through the terrain when we jump from lower to higher part
        if (data_.minVerticalOffset >= data_.jumpOffset)
        {
            playerPos.y = data_.minVerticalOffset;
            data_.jumpOffset = data_.minVerticalOffset;
        }
        else
        {
            playerPos.y = data_.jumpOffset;
        }
    }

    if (!inJump && !IsFreeFlyMode())
        playerPos.y = data_.minVerticalOffset;
        

    // update player's postion
    pTransformSys_->SetPosition(playerID, playerPos);

    // compute and set new position for each child
    pHierarchySys_->GetChildrenArr(playerID, s_Ids);

    for (const EntityID childID : s_Ids)
    {
        const XMFLOAT3 childRelPos = pHierarchySys_->GetRelativePos(childID);
        const float posX = playerPos.x + childRelPos.x;
        const float posY = playerPos.y + childRelPos.y;
        const float posZ = playerPos.z + childRelPos.z;

        pTransformSys_->SetPosition(childID, { posX, posY, posZ });
    }

    // reset all the movement states
    data_.playerStates_ &= ~(GetFlagsMove());
}

//---------------------------------------------------------
// Desc:   set a movement state for the player
// Args:   - state: expected to be some of the movement states
//---------------------------------------------------------
void PlayerSystem::Move(ePlayerState state)
{
    data_.playerStates_ |= state;
}

//---------------------------------------------------------
// Desc:   turn on/off the player's free fly mode
//---------------------------------------------------------
void PlayerSystem::SetFreeFlyMode(const bool mode)
{
    // turn on
    if (mode)
        data_.playerStates_ |= FREE_FLY;

    // turn off
    else
        data_.playerStates_ &= ~(FREE_FLY);
}

//---------------------------------------------------------
// Desc:   turn on/off the player's flashlight
//---------------------------------------------------------
void PlayerSystem::SwitchFlashLight(const bool state)
{
    // turn on
    if (state)
        data_.playerStates_ |= TURNED_FLASHLIGHT;

    // turn off
    else
        data_.playerStates_ &= ~(TURNED_FLASHLIGHT);
}

//---------------------------------------------------------
// Desc:   turn on/off the player's running state
//---------------------------------------------------------
void PlayerSystem::SetIsRunning(const bool running)
{
    if (running)
    {
        data_.playerStates_ |= RUN;
        data_.playerStates_ &= ~(WALK | CRAWL | IDLE);
        SetCurrentSpeed(data_.speedRun);
    }
    else
    {
        data_.playerStates_ |= WALK;
        data_.playerStates_ &= ~(RUN | CRAWL | IDLE);
        SetCurrentSpeed(data_.speedWalk);
    }
}

//---------------------------------------------------------

void PlayerSystem::SetIsWalking(void)
{
    data_.playerStates_ |= WALK;
    data_.playerStates_ &= ~(RUN | CRAWL | IDLE);
    SetCurrentSpeed(data_.speedWalk);
}

//---------------------------------------------------------

void PlayerSystem::SetIsReloading(const bool reloading)
{
    if (reloading)
        data_.playerStates_ |= RELOADING;
    else
        data_.playerStates_ &= ~(RELOADING);
}

//---------------------------------------------------------

void PlayerSystem::SetIsShooting(const bool shooting)
{
    if (shooting && !IsReloading())
        data_.playerStates_ |= SHOOT;
    else
        data_.playerStates_ &= ~(SHOOT);
}

//---------------------------------------------------------

void PlayerSystem::SetIsIdle(void)
{
    data_.playerStates_ |= IDLE;
}

// =================================================================================
// Player rotation
// =================================================================================

//---------------------------------------------------------
// Desc:  rotate input vector using quaternion(axis, angle)
// 
//        NOTE!!!: input quat is supposed to be unit length
//---------------------------------------------------------
inline XMVECTOR RotateVecByQuat(const XMVECTOR& vec, const XMVECTOR& rotationQuat)
{
    // to help you to understand wtf is going on here :)
    // const XMVECTOR invQuat = XMQuaternionConjugate(rotationQuat);
    // const XMVECTOR tmpVec  = XMQuaternionMultiply(invQuat, vec);
    // const XMVECTOR resVec  = XMQuaternionMultiply(tmpVec, rotationQuat);
    // return resVec;

    // rotated_vec = inv_quat * orig_vec * quat; 
    return XMQuaternionMultiply(XMQuaternionMultiply(XMQuaternionConjugate(rotationQuat), vec), rotationQuat);
}

//---------------------------------------------------------

inline XMVECTOR RotateVecByQuat(const XMVECTOR& vec, const XMVECTOR& rotQuat, const XMVECTOR& invQuat)
{
    return XMQuaternionMultiply(XMQuaternionMultiply(invQuat, vec), rotQuat);
}

//---------------------------------------------------------
// Desc:  rotate the look vector about the view space right vector
// Args:  - angle:   pitch angle in RADIANS
//---------------------------------------------------------
void PlayerSystem::Pitch(float angle)
{
    data_.pitch += angle;
    ClampPitch(data_.pitch);


    const EntityID playerId = playerID_;

    // get arr of player's children entities
    pHierarchySys_->GetChildrenArr(playerId, s_Ids);

    // adjust position of each child relatively to the player
    const XMVECTOR playerPosW   = pTransformSys_->GetPositionVec(playerID_);
    const XMVECTOR rotationQuat = DirectX::XMQuaternionRotationAxis(data_.rightVec, angle);
    const XMVECTOR invQuat      = XMQuaternionConjugate(rotationQuat);


    for (int i = 0; const EntityID childID : s_Ids)
    {
        const XMFLOAT3 oldRelPos = pHierarchySys_->GetRelativePos(childID);
        const XMVECTOR newRelPos = RotateVecByQuat({ oldRelPos.x, oldRelPos.y, oldRelPos.z, 0 }, rotationQuat, invQuat);

        pTransformSys_->SetPositionVec(childID, playerPosW + newRelPos);
        pHierarchySys_->SetRelativePos(childID, newRelPos);
    }

    // adjust rotation of the player and its each child
    s_Ids.push_back(playerId);
    pTransformSys_->RotateLocalSpacesByQuat(s_Ids.data(), s_Ids.size(), rotationQuat);
}

//---------------------------------------------------------
// Desc:  rotate the basis vectors about the world's y-axis
//---------------------------------------------------------
void PlayerSystem::RotateY(float angle)
{
    data_.yaw += angle;
    ClampYaw(data_.yaw);

    const EntityID playerId     = playerID_;
    const XMVECTOR rotationQuat = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, angle);
    data_.rightVec              = RotateVecByQuat(data_.rightVec, rotationQuat);


    // get arr of player's children entities
    pHierarchySys_->GetChildrenArr(playerId, s_Ids);

    // adjust position of each child relatively to the player
    const XMVECTOR playerPosW   = pTransformSys_->GetPositionVec(playerId);
    const XMVECTOR invQuat      = XMQuaternionConjugate(rotationQuat);

    for (int i = 0; const EntityID childID : s_Ids)
    {
        const XMFLOAT3 oldRelPos    = pHierarchySys_->GetRelativePos(childID);
        const XMVECTOR newRelPos    = RotateVecByQuat({ oldRelPos.x, oldRelPos.y, oldRelPos.z, 0 }, rotationQuat, invQuat);

        pTransformSys_->SetPositionVec(childID, playerPosW + newRelPos);
        pHierarchySys_->SetRelativePos(childID, newRelPos);
    }

    // adjust rotation of the player and its each child
    s_Ids.push_back(playerId);
    pTransformSys_->RotateLocalSpacesByQuat(s_Ids.data(), s_Ids.size(), rotationQuat);
}

}; // namespace ECS
