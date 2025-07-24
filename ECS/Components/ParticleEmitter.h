// =================================================================================
// Filename:   ParticleEmitter.h
// Desc:       data structures and ECS compute for particle emission
//
// Created:    23.07.2025  by DimaSkup
// =================================================================================
#pragma once

#include <DirectXMath.h>
#include <cvector.h>

namespace ECS
{

struct Particle
{
    DirectX::XMVECTOR pos   = { 0,0,0 };  // current position: this is the particle's current position in 2D/3D space
    DirectX::XMVECTOR vel   = { 0,0,0 };  // velocity: this is the particle's direction and speed
    DirectX::XMFLOAT3 color = { 0,0,0 };  // the current color of the particle (RGB triplet)

    float translucency = 0;               // current alpha value (transparency) of the particle
    float ageMs          = 0;             // Life span in milliseconds(!!!). This is how long the particle will live
    float mass         = 0;               // is used to accurately model particle motion
    float size         = 0;               // particle's visual size
    float friction     = 0;               // air resistance: this is the particle's susceptibility to friction in the air
};

struct ParticleInitData
{
    DirectX::XMFLOAT3 vel;
    DirectX::XMFLOAT3 color;
};

struct ParticleRenderInstance
{
    DirectX::XMFLOAT3 pos;
    float translucency = 0.0f;
    DirectX::XMFLOAT3 color;
    DirectX::XMFLOAT2 size;
};

struct ParticleEmitter
{
    cvector<Particle> particles_;
};

} // namespace
