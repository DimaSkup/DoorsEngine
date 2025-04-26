// =================================================================================
// Filename:     Player.h
// Description:  an ECS component to hold First-Person-Shooter (FPS) player's data
//
// Created:      22.04.24
// =================================================================================
#pragma once

#include "../Common/Types.h"

namespace ECS
{

// ECS component
struct PlayerData
{
    eComponentType type = eComponentType::PlayerComponent;

    float currSpeed = 10.0f;
    float walkSpeed = 10.0f;
    float runSpeed  = 20.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    bool isFreeFlyMode = true;
    bool isRunning = false;
    bool isTurnedOnFlashlight = false;

    XMVECTOR rightVec = { 1,0,0 };
};


} // namespace
