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

using uint8    = uint8_t;
//using uint8  = unsigned char;
using uint   = unsigned int;
using ushort = unsigned short;

using u32   = uint32_t;
using size  = ptrdiff_t;
using index = ptrdiff_t;
using UINT  = unsigned int;

using EntityID   = uint32_t;

//
// MESH/MODEL TYPEDEFS/CONSTS
//
using ModelID    = uint32_t;
using SubsetID   = uint16_t;                    // mesh ID
constexpr ModelID INVALID_MODEL_ID = 0;

constexpr int SUBSET_NAME_LENGTH_LIMIT = 32;    // max length for a model subset's (mesh) name
constexpr int MAX_LENGTH_MODEL_NAME    = 32;
constexpr int MAX_LENGTH_TEXTURE_NAME  = 32;
constexpr int MAX_LENGTH_MATERIAL_NAME = 32;

struct ModelName
{
    char name[MAX_LENGTH_MODEL_NAME]{ '\0' };
};

struct TexName
{
    char name[MAX_LENGTH_TEXTURE_NAME]{ '\0' };
};

//
// MATERIAL TYPEDEFS/CONSTS
//
using MaterialID   = uint32_t;
constexpr MaterialID INVALID_MATERIAL_ID = 0;

//
// TEXTURES TYPEDEFS/CONSTS
//
using     TexID   = uint32_t;                       // texture ID
constexpr TexID INVALID_TEXTURE_ID = 0;
constexpr size  NUM_TEXTURE_TYPES  = 21;
