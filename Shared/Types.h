// *********************************************************************************
// Filename:     Types.h
// Description:  contains some common typedefs and constants for the engine
// 
// Created:      10.07.24
// *********************************************************************************
#pragma once

#include <stdint.h>


struct Float4
{
    Float4(const float _x, const float _y, const float _z, const float _w) :
        x(_x), y(_y), z(_z), w(_w) {}

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

// --------------------------

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using uint   = unsigned int;
using ushort = unsigned short;

using u32   = uint32_t;
using size  = ptrdiff_t;
using index = ptrdiff_t;
using UINT  = unsigned int;

using EntityID          = uint32_t;             // ECS entity ID
using ComponentBitfield = uint32_t;             // ECS component bitfield


using SentenceID        = uint32_t;             // UI sentence ID
using SubmeshID         = uint16_t;             // entity/model submesh (subset) ID
using SubsetID          = uint16_t;             // entity/model submesh (subset) ID (wtf?)

using ModelID           = uint32_t;
using MaterialID        = uint32_t;
using TexID             = uint32_t;             // texture ID

// common constants
constexpr EntityID   INVALID_ENTITY_ID = 0;
constexpr ModelID    INVALID_MODEL_ID = 0;
constexpr MaterialID INVALID_MATERIAL_ID = 0;
constexpr TexID      INVALID_TEXTURE_ID = 0;
constexpr size       NUM_TEXTURE_TYPES = 21;           // limit of textures per mesh

constexpr int        SUBSET_NAME_LENGTH_LIMIT = 32;    // max length for a model subset's (mesh) name
constexpr int        MAX_LENGTH_MODEL_NAME = 32;       // limit for the model's name length
constexpr int        MAX_LENGTH_TEXTURE_NAME = 32;     // limit for the texture's name length
constexpr int        MAX_LENGTH_MATERIAL_NAME = 32;    // limit for the material's name length

constexpr int        MAX_NUM_DIRECTED_LIGHTS = 3;      // maximal number of directed lights on the scene

struct ModelName
{
    char name[MAX_LENGTH_MODEL_NAME]{ '\0' };
};

// texture name
struct TexName
{
    char name[MAX_LENGTH_TEXTURE_NAME]{ '\0' };
};

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


