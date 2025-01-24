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
	TransformComponent,
	MoveComponent,
	RenderedComponent,
	ModelComponent,

	WorldMatrixComponent,
	NameComponent,
	AIComponent,
	HealthComponent,

	DamageComponent,
	EnemyComponent,
	EditorTransformComponent,
	ColliderComponent,

	PhysicsTypeComponent,
	VelocityComponent,
	GroundedComponent,
	CollisionComponent,

	CameraComponent,
	TexturedComponent,
	TextureTransformComponent,
	LightComponent,
	RenderStatesComponent,         // for using different render states: blending, alpha clipping, fill mode, cull mode, etc.

	BoundingComponent,             // for using AABB, OBB, bounding spheres

	LAST_COMPONENT_TYPE
};


struct TransformRawData
{
	XMFLOAT3 pos;     // position
	XMFLOAT3 dir;     // direction (roll pitch yaw)
	float uniScale;   // uniform scale
};

struct MovementRawData
{
	XMFLOAT3 trans;   // translation
	XMFLOAT3 rot;     // rotation (roll pitch yaw)
	float uniScale;   // uniform scale factor
};

enum RenderShaderType
{
	COLOR_SHADER,
	TEXTURE_SHADER,
	LIGHT_SHADER,
	SKYDOME_SHADER
};


}
