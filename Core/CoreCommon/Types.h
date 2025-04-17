// *********************************************************************************
// Filename:     Types.h
// Description:  contains some common typedefs and constants for the engine
// 
// Created:      10.07.24
// *********************************************************************************
#pragma once

#include <stdint.h>

namespace Core
{

struct Float4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

// --------------------------

using u32   = uint32_t;
using size  = ptrdiff_t;
using index = ptrdiff_t;
using UINT  = unsigned int;

using EntityID   = uint32_t;
//using EntityName = std::string;

//
// MESH/MODEL TYPEDEFS
//
//using MeshName   = std::string;
//using ModelName  = std::string;
using ModelID    = uint32_t;
using SubsetID   = uint16_t;                    // mesh ID
constexpr ModelID INVALID_MODEL_ID = 0;
constexpr int SUBSET_NAME_LENGTH_LIMIT = 32;    // max length for a model subset's name

constexpr int MAX_LENGTH_MODEL_NAME   = 32;
constexpr int MAX_LENGTH_TEXTURE_NAME = 32;

struct ModelName
{
    char name[MAX_LENGTH_MODEL_NAME]{ '\0' };
};

struct TexName
{
    char name[MAX_LENGTH_TEXTURE_NAME]{ '\0' };
};

//
// MATERIAL TYPEDEFS
//
//using MaterialName = std::string;
using MaterialID   = uint32_t;
constexpr MaterialID INVALID_MATERIAL_ID = 0;

//
// TEXTURES TYPEDEFS
//
using TexID   = uint32_t;                       // texture ID
//using TexPath = std::string;                    // texture path
//using TexName = std::string;                    // texture name
constexpr TexID INVALID_TEXTURE_ID = 0;
constexpr size  NUM_TEXTURE_TYPES  = 21;

} // namespace Core
