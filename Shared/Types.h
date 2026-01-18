// *********************************************************************************
// Filename:     Types.h
// Description:  contains some common typedefs and constants for the engine
// 
// Created:      10.07.24
// *********************************************************************************
#pragma once

#include <stdint.h>


using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using uint   = unsigned int;
using ushort = unsigned short;

//using u32    = uint32_t;
using size   = ptrdiff_t;
using index  = ptrdiff_t;
using UINT   = unsigned int;

using EntityID          = uint32_t;             // ECS entity ID
using ComponentBitfield = uint32_t;             // ECS component bitfield


using SentenceID        = uint32_t;             // UI sentence ID
using SubmeshID         = uint16_t;             // entity/model submesh (subset) ID
using SubsetID          = uint16_t;             // entity/model submesh (subset) ID (wtf?)

// each resource instance (like model, shader, skeleton, etc.) has its own unique ID
using ModelID           = uint32_t;
using MaterialID        = uint32_t;         
using TexID             = uint32_t;
using SkeletonID        = uint32_t;         // for model skinning
using AnimationID       = uint32_t;         // for model skinning
using ShaderID          = uint32_t;
using SoundID           = uint32_t;

// render states related stuff
using RsID              = uint16_t;         // rasterizer state id
using BsID              = uint16_t;         // blending state id
using DssID             = uint16_t;         // depth-stencil state id

// common constants
constexpr EntityID   INVALID_ENTITY_ID      = 0;
constexpr ModelID    INVALID_MODEL_ID       = 0;
constexpr MaterialID INVALID_MATERIAL_ID    = 0;
constexpr TexID      INVALID_TEX_ID         = 0;        // invalid texture id
constexpr ShaderID   INVALID_SHADER_ID      = 0;
constexpr SoundID    INVALID_SOUND_ID       = 0;
constexpr size       NUM_TEXTURE_TYPES      = 21;       // limit of textures per mesh

constexpr int        MAX_LEN_MESH_NAME      = 32;       // max length for a model subset's (mesh) name
constexpr int        MAX_LEN_ENTT_NAME      = 32;
constexpr int        MAX_LEN_MODEL_NAME     = 32;       // limit for the model's name length
constexpr int        MAX_LEN_TEX_NAME       = 32;       // limit for the texture's name length
constexpr int        MAX_LEN_MAT_NAME       = 32;       // limit for the material's name length
constexpr int        MAX_LEN_SHADER_NAME    = 32;
constexpr int        MAX_LEN_SKY_MODEL_NAME = 16;
constexpr int        MAX_LEN_SOUND_NAME     = 32;
constexpr int        MAX_LEN_RND_STATE_NAME = 16;

constexpr int        MAX_NUM_POST_EFFECTS = 8;          // maximal number of post effects that can be applied at the same time

constexpr int        MAX_NUM_DIRECTED_LIGHTS = 3;       // maximal number of directed lights on the scene

constexpr int        MAX_NUM_BONES_PER_CHARACTER = 192;

struct ModelName
{
    char name[MAX_LEN_MODEL_NAME]{ '\0' };
};

// texture name
struct TexName
{
    char name[MAX_LEN_TEX_NAME]{ '\0' };
};

struct ShaderName
{
    char name[MAX_LEN_SHADER_NAME]{ '\0' };
};

struct SoundName
{
    char name[MAX_LEN_SOUND_NAME]{ '\0' };
};

struct RenderStateName
{
    char name[MAX_LEN_RND_STATE_NAME]{ '\0' };
};


//---------------------------------------------------------
// helper defines/structures/enums for skinned data (vertex skinning, animations)
//---------------------------------------------------------
#define MAX_NUM_BONES_PER_VERTEX 4
#define MAX_LEN_ANIMATION_NAME   48
#define MAX_LEN_BONE_NAME        32
#define MAX_LEN_SKELETON_NAME    32

struct AnimationName
{
    char name[MAX_LEN_ANIMATION_NAME]{ '\0' };
};

struct BoneName
{
    char name[MAX_LEN_BONE_NAME]{ '\0' };
};

struct SkeletonName
{
    char name[MAX_LEN_SKELETON_NAME]{ '\0' };
};

//---------------------------------------------------------

struct EntityModelMesh
{
    EntityModelMesh() {}

    EntityModelMesh(
        const EntityID inEnttId,
        const MaterialID inMatId,
        const ModelID inModelId,
        const SubmeshID inMeshId)
        :
        enttId(inEnttId),
        matId(inMatId),
        modelId(inModelId),
        subsetId(inMeshId) {}


    EntityID   enttId   = INVALID_ENTITY_ID;
    MaterialID matId    = INVALID_MATERIAL_ID;
    ModelID    modelId  = INVALID_MODEL_ID;
    SubmeshID  subsetId = 0;
};

//---------------------------------------------------------
// data about intersection of a ray and some geometry
//---------------------------------------------------------
struct IntersectionData
{
    EntityID enttId = 0;                    // intersected entity
    ModelID modelId = 0;                    // intersected model
    uint triangleIdx = 0;                   // intersected triangle (its index)

    float rayOrigX, rayOrigY, rayOrigZ;     // ray origin
    
    float vx0, vy0, vz0;                    // intersected triangle endpoints positions (in world space)
    float vx1, vy1, vz1;
    float vx2, vy2, vz2;

    float px, py, pz;                       // intersection point
    float nx, ny, nz;                       // normal vector of intersected triangle
};
