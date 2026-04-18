// *********************************************************************************
// Filename:     Types.h
// Description:  contains types for Entity-Component-System module of the engine;
// 
// Created:
// *********************************************************************************
#pragma once

namespace ECS
{


// for detailed (I hope) description of each component
// you need to look for responsible component's header file
enum eComponentType
{
    NameComponent,              // REQUIRED: attach some name for the entity
    TransformComponent,         // REQUIRED: set that entity has properties: position (x,y,z), direction (quaternion), and scale (uniform)
    MoveComponent,              // set that this entity must be transformed over the time using some transformation matrix (for instance rotate around itself and go into some particular direction)
    RenderedComponent,          // set that this entity is renderable (preferably it is a model), set that this entity must be rendered with particular kind of shader, maybe with specific primitive topology
    ModelComponent,             // attach to entity a 2D/3D model by ID

    CameraComponent,            // attach to entity a camera
    MaterialComponent,          // set each mesh of entity (model) has some particular material
    TextureTransformComponent,  // set that texture has some kind of transformation (maybe it is a translation over some atlas texture so we create an animation, or rotation around texture center -- creates a rotating fireball)
    LightComponent,             // attach to entity some type of light source (directed, point, spotlight, etc.)
    BoundingComponent,          // for using AABB, OBB, bounding spheres

    PlayerComponent,            // to hold First-Person-Shooter (FPS) player's data
    ParticlesComponent,    
    InventoryComponent,         // inventory of entity
    AnimationComponent,         // for model skinning
    SpriteComponent,            // 2D sprite

    WeaponComponent,
    TriggerComponent,

    // NOT IMPLEMENTED YET
    AIComponent,
    HealthComponent,
    DamageComponent,
    EnemyComponent,
    ColliderComponent,

    PhysicsTypeComponent,
    CollisionComponent,

    NUM_COMPONENTS
};

} // namespace ECS
