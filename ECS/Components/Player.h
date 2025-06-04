// =================================================================================
// Filename:     Player.h
// Description:  an ECS component to hold First-Person-Shooter (FPS) player's data
//
// Created:      22.04.24
// =================================================================================
#pragma once

namespace ECS
{

// ECS component
struct PlayerData
{
    float currSpeed = 10.0f;
    float walkSpeed = 10.0f;
    float runSpeed  = 20.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    bool isFreeFlyMode = true;
    bool isRunning = false;
    bool isTurnedOnFlashlight = false;

    DirectX::XMVECTOR rightVec = { 1,0,0 };
};


} // namespace
