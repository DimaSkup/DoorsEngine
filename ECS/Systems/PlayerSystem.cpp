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
#if 0

void PlayerSystem::Update(const float deltaTime)
{
    const uint64 states   = data_.playerStates_;
    const int speedMul    = 1 + 9 * IsFreeFlyMode();  // move faster if we are in free fly
    const float speed     = GetSpeed() * deltaTime * speedMul;
    float horizontalSpeed = 0;

    XMVECTOR offset = { 0,0,0 };
    
    static bool  inJump   = false;
    static float jumpTime = 0;
    static float h0       = 0;

    const EntityID playerID = playerID_;

    if (!(states & JUMP))
    {
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

        // normalize the movement direction vector and scale it according to player's speed
        offset = XMVector3Normalize(offset);
        offset = XMVectorMultiply(XMVectorReplicate(speed), offset);

        // compute player's new position
        const XMVECTOR playerPosVec = GetPosVec() + offset;
        XMFLOAT3 playerPos;
        XMStoreFloat3(&playerPos, playerPosVec);
        pTransformSys_->SetPosition(playerID, playerPos);


        // compute and set new position for each child
        pHierarchySys_->GetChildrenArr(playerID, s_Ids);

        for (const EntityID childID : s_Ids)
        {
            const XMFLOAT3 childRelPos = pHierarchySys_->GetRelativePos(childID);
            const XMFLOAT3 childNewPos = playerPos + childRelPos;

            pTransformSys_->SetPosition(childID, childNewPos);
        }

        // move player and its all children
        //pTransformSys_->AdjustPositions(s_Ids.data(), s_Ids.size(), offset);
    }
    if (states & JUMP)
    {
        // if we weren't in jump before
        if (!inJump)
        {
            inJump = true;
            h0 = GetPosition().y;
        }
        // compute jump
        else
        {
            constexpr float height = 5.0f;        // peak height
            constexpr float th     = 0.5f;        // time to reach the peak height
            constexpr float invTH  = 1.0f / th;

            // derive gravity and init. velocity in terms of peak height and duration to peak
            constexpr float g = -2 * height * (invTH * invTH);
            constexpr float v0 = 2 * height * invTH;

            // increate the time in jump
            float t = jumpTime;
            jumpTime += deltaTime;

            // compute current jump offset
            data_.jumpOffset = 0.5f * g * (t * t) + v0 * t + h0;

            // if jump cycle is over
            if (jumpTime >= 2 * th)
            {
                data_.jumpOffset = 0.0f;
                inJump = false;
                jumpTime = 0.0f;

                // turn off the player's jump stat
                data_.playerStates_ &= ~(JUMP);
            }
            else
            {
                // normalize the movement direction vector and scale it according to player's speed
                offset = XMVector3Normalize(offset);
                offset = XMVectorMultiply(XMVectorReplicate(speed), offset);
                
                // compute and set player's new position
                const XMVECTOR playerPosVec = GetPosVec() + offset;
                XMFLOAT3 playerPos;
                XMStoreFloat3(&playerPos, playerPosVec);
                pTransformSys_->SetPosition(playerID, playerPos);

                if (states & JUMP)
                    playerPos.y = data_.jumpOffset;

                pTransformSys_->SetPosition(playerID, playerPos);


                // adjust position of the player and its children
                pHierarchySys_->GetChildrenArr(playerID, s_Ids);

                for (EntityID childID : s_Ids)
                {
                    const XMFLOAT3 childRelPos = pHierarchySys_->GetRelativePos(childID);
                    const XMFLOAT3 childNewPos = playerPos + childRelPos;

                    pTransformSys_->SetPosition(childID, childNewPos);
                }
            }
        }
    }

    // reset all the movement states
    data_.playerStates_ &= ~(GetFlagsMove());
}

#else

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
            inJump = true;
            h0 = GetPosition().y;
            jumpTime = 0;
            distInJump = 0;
        }
    }
    // we interrupted the jump or jump cycle is over
    else
    {
        data_.jumpOffset = 0.0f;
        inJump = false;
        jumpTime = 0.0f;
        distInJump = 0;
    }


    // compute jump
    if (inJump)
    {
#if 0
        constexpr float height = 5.0f;        // peak height
        const float vx = GetSpeed();
        const float xh = 5;

        // derive gravity and init. velocity in terms of foot speed and lateral distance to peak of jump
        const float th = xh / vx;
        const float g = -2 * height / (th*th);
        const float v0 = 2 * height / th;

        // increate the time in jump
        float t = jumpTime;
        jumpTime += deltaTime;

        distInJump = (vx * t);

        // if jump cycle is over
        if (distInJump >= 2 * vx * th)
        {
            // turn off the player's jump state
            StopJump();
            inJump = false;
            jumpTime = 0;
            distInJump = 0;
        }
        else
        {
            // compute current jump offset
            data_.jumpOffset = 0.5f * g * (t * t) + v0 * t + h0;
        }
#else
        constexpr float height = 5.0f;        // peak height
        constexpr float th     = 0.5f;        // time to reach the peak height
        constexpr float invTH  = 1.0f / th;

        // derive gravity and init. velocity in terms of peak height and duration to peak
        constexpr float g = -2 * height * (invTH * invTH);
        constexpr float v0 = 2 * height * invTH;

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
#endif
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
        if (data_.minVerticalOffset >= data_.jumpOffset)
        {
            playerPos.y = data_.minVerticalOffset;
            data_.jumpOffset = data_.minVerticalOffset;
        }

        else
            playerPos.y = data_.jumpOffset;
    }

    if (!inJump && !IsFreeFlyMode())
        playerPos.y = data_.minVerticalOffset;
        

    pTransformSys_->SetPosition(playerID, playerPos);


    // compute and set new position for each child
    pHierarchySys_->GetChildrenArr(playerID, s_Ids);

    for (const EntityID childID : s_Ids)
    {
        const XMFLOAT3 childRelPos = pHierarchySys_->GetRelativePos(childID);
        const XMFLOAT3 childNewPos = playerPos + childRelPos;

        pTransformSys_->SetPosition(childID, childNewPos);
    }

    // reset all the movement states
    data_.playerStates_ &= ~(GetFlagsMove());
}
#endif








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
        SetCurrentSpeed(data_.runSpeed);
    }
    else
    {
        data_.playerStates_ &= ~(RUN | CRAWL);
        data_.playerStates_ |= WALK;
        SetCurrentSpeed(data_.walkSpeed);
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
    // const XMVECTOR tmpVec  = XMQuaternionMultiply(invQuat, vec);
    // const XMVECTOR resVec  = XMQuaternionMultiply(tmpVec, rotationQuat);
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

    const EntityID playerId = playerID_;

    // get arr of player's children entities
    pHierarchySys_->GetChildrenArr(playerId, s_Ids);

    // adjust position of each child relatively to the player
    const XMVECTOR playerPosW = pTransformSys_->GetPositionVec(playerID_);
    const XMVECTOR rotationQuat = DirectX::XMQuaternionRotationAxis(data_.rightVec, angle);
    const XMMATRIX R = XMMatrixRotationQuaternion(rotationQuat);

    for (int i = 0; const EntityID childID : s_Ids)
    {
        XMVECTOR childPosW    = pTransformSys_->GetPositionVec(childID);
        XMVECTOR relPos       = DirectX::XMVectorSubtract(childPosW, playerPosW);
        XMVECTOR newRelPos    = XMVector3Transform(relPos, R);
        XMVECTOR childNewPosW = XMVectorAdd(playerPosW, newRelPos);

        pTransformSys_->SetPositionVec(childID, childNewPosW);

        // recompute relative position of child
        pHierarchySys_->UpdateRelativePos(childID);
    }

    // adjust rotation of the player and its each child
    pTransformSys_->RotateLocalSpaceByQuat(playerID_, rotationQuat);
    pTransformSys_->RotateLocalSpacesByQuat(s_Ids.data(), s_Ids.size(), rotationQuat);
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
    pHierarchySys_->GetChildrenArr(id, s_Ids);

    // adjust position of each child relatively to the player
    const XMMATRIX R = DirectX::XMMatrixRotationY(angle);

    for (int i = 0; const EntityID childID : s_Ids)
    {
        const XMVECTOR childPosW    = pTransformSys_->GetPositionVec(childID);           // get current position of child
        const XMVECTOR relPos       = DirectX::XMVectorSubtract(childPosW, playerPosW);  // compute child position relatively to the player

        //XMVECTOR newRelPos = RotateVecByQuat(relPos, rotationQuat);
        XMVECTOR newRelPos    = XMVector3Transform(relPos, R);                     // compute new relative position of child
        XMVECTOR childNewPosW = XMVectorAdd(playerPosW, newRelPos);                // compute new world position of child

        pTransformSys_->SetPositionVec(childID, childNewPosW);

        // recompute relative position of child
        pHierarchySys_->UpdateRelativePos(childID);
    }

    // adjust rotation of the player and its each child
    pTransformSys_->RotateLocalSpaceByQuat(id, rotationQuat);
    pTransformSys_->RotateLocalSpacesByQuat(s_Ids.data(), s_Ids.size(), rotationQuat);

  
}

}; // namespace ECS
