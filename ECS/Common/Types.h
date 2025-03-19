// *********************************************************************************
// Filename:     Types.h
// Description:  contains types for Entity-Component-System module of the engine;
// 
// Created:
// *********************************************************************************
#pragma once


#include <string>
#include <DirectXMath.h>

// Common typedefs
using UINT = unsigned int;
using XMFLOAT2 = DirectX::XMFLOAT2;
using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMVECTOR = DirectX::XMVECTOR;
using XMMATRIX = DirectX::XMMATRIX;

using u32            = uint32_t;
using size           = ptrdiff_t;  // used for indexing, or for storing the result from std::ssize()
using index          = ptrdiff_t;

using ComponentsHash = uint32_t;
using ModelID        = uint32_t;

// textures related typedefs
using TexID          = uint32_t;
using TexPath        = std::string;

using EntityID       = uint32_t;
using EntityName     = std::string;
using ComponentID    = std::string;
using SystemID       = std::string;

const EntityID   INVALID_ENTITY_ID{ 0 };
const EntityName INVALID_ENTITY_NAME{ "invalid" };

const TexID      INVALID_TEXTURE_ID{ 0 };
const TexPath    INVALID_TEXTURE_PATH{ "invalid" };

namespace ECS
{

enum ComponentType
{
    NameComponent,                 // REQUIRED: attach some name for the entity
    TransformComponent,            // REQUIRED: set that entity has properties: position (x,y,z), direction (quaternion), and scale (uniform)
    MoveComponent,                 // set that this entity must be transformed over the time using some transformation matrix (for instance rotate around itself and go into some particular direction)
    RenderedComponent,             // set that this entity is renderable (preferably it is a model), set that this entity must be rendered with particular kind of shader, maybe with specific primitive topology
    ModelComponent,                // attach to entity a 2D/3D model by ID

    CameraComponent,               // attach to entity a camera
    TexturedComponent,             // entity has its own set of textures for its particular mesh (if it is a complex model) so we don't use textures by default (which goes together with mesh) but use own textures (for instance: default cube has some wood texture but we use different other)
    TextureTransformComponent,     // set that texture has some kind of transformation (maybe it is a translation over some atlas texture so we create an animation, or rotation around texture center -- creates a rotating fireball)
    LightComponent,                // attach to entity some type of light source (directed, point, spotlight, etc.)
    RenderStatesComponent,         // for using different render states: blending, alpha clipping, fill mode, cull mode, etc.
    BoundingComponent,             // for using AABB, OBB, bounding spheres


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

    LAST_COMPONENT_TYPE
};

enum RenderShaderType
{
    COLOR_SHADER,
    TEXTURE_SHADER,
    LIGHT_SHADER,
    SKYDOME_SHADER
};


}
