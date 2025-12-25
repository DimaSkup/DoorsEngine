// *********************************************************************************
// Filename:     Types.h
// Description:  contains types for Entity-Component-System module of the engine;
// 
// Created:
// *********************************************************************************
#pragma once

#include <DirectXMath.h>

namespace ECS
{

// Common typedefs
using UINT          = unsigned int;
using XMFLOAT2      = DirectX::XMFLOAT2;
using XMFLOAT3      = DirectX::XMFLOAT3;
using XMFLOAT4      = DirectX::XMFLOAT4;
using XMVECTOR      = DirectX::XMVECTOR;
using XMMATRIX      = DirectX::XMMATRIX;


// for detailed (I hope) description of each component
// you need to look for responsible component's header file
enum eComponentType
{
    NameComponent,                 // REQUIRED: attach some name for the entity
    TransformComponent,            // REQUIRED: set that entity has properties: position (x,y,z), direction (quaternion), and scale (uniform)
    MoveComponent,                 // set that this entity must be transformed over the time using some transformation matrix (for instance rotate around itself and go into some particular direction)
    RenderedComponent,             // set that this entity is renderable (preferably it is a model), set that this entity must be rendered with particular kind of shader, maybe with specific primitive topology
    ModelComponent,                // attach to entity a 2D/3D model by ID

    CameraComponent,               // attach to entity a camera
    MaterialComponent,
    TextureTransformComponent,     // set that texture has some kind of transformation (maybe it is a translation over some atlas texture so we create an animation, or rotation around texture center -- creates a rotating fireball)
    LightComponent,                // attach to entity some type of light source (directed, point, spotlight, etc.)
    RenderStatesComponent,         // for using different render states: blending, alpha clipping, fill mode, cull mode, etc.
    BoundingComponent,             // for using AABB, OBB, bounding spheres

    PlayerComponent,               // to hold First-Person-Shooter (FPS) player's data
    ParticlesComponent,
    InventoryComponent,            // inventory of entity
    AnimationComponent,            // for model skinning

    // NOT IMPLEMENTED YET
    AIComponent,
    HealthComponent,
    DamageComponent,
    EnemyComponent,
    ColliderComponent,

    PhysicsTypeComponent,
    VelocityComponent,
    GroundedComponent,
    CollisionComponent,

    NUM_COMPONENTS
};

#if 0
enum RenderShaderType
{
    COLOR_SHADER,
    TEXTURE_SHADER,
    LIGHT_SHADER,
    SKYDOME_SHADER
};
#endif

} // namespace ECS
