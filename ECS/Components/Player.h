// =================================================================================
// Filename:     Player.h
// Description:  an ECS component to hold First-Person-Shooter (FPS) player's data
//
// Created:      22.04.24
// =================================================================================
#pragma once

namespace ECS
{

enum ePlayerState
{
    JUMP                = (1 << 0),
    RUN                 = (1 << 1),
    WALK                = (1 << 2),
    CRAWL               = (1 << 3),
    FREE_FLY            = (1 << 4),
    SHOOT               = (1 << 5),
    MOVE_FORWARD        = (1 << 6),
    MOVE_BACK           = (1 << 7),
    MOVE_RIGHT          = (1 << 8),
    MOVE_LEFT           = (1 << 9),
    MOVE_UP             = (1 << 10),
    MOVE_DOWN           = (1 << 11),
    TURNED_FLASHLIGHT   = (1 << 12),
};

// ECS component
struct PlayerData
{
    float currSpeed = 5.0f;
    float walkSpeed = 5.0f;
    float runSpeed  = 20.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float jumpOffset = 0.0f;
    float minVerticalOffset = 0;

    uint64 playerStates_ = 0;

    DirectX::XMVECTOR rightVec = { 1,0,0 };
};


} // namespace
