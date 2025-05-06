// *********************************************************************************
// Filename:     Types.h
// Description:  contains some common typedefs and constants for the UI
// 
// Created:      28.03.2025 by DimaSkup
// *********************************************************************************
#pragma once

#include <stdint.h>

namespace UI
{

using u32   = uint32_t;
using size  = ptrdiff_t;
using index = ptrdiff_t;
using UINT  = unsigned int;

using EntityID  = uint32_t;
using ModelID   = uint32_t;
//using SRV       = ID3D11ShaderResourceView;


//
// MESH/MODEL TYPEDEFS
//
using ModelID   = uint32_t;
using SubsetID  = uint16_t;                    // mesh ID
constexpr ModelID INVALID_MODEL_ID = 0;

//
// MATERIAL TYPEDEFS
//
using MaterialID = uint32_t;
constexpr MaterialID INVALID_MATERIAL_ID = 0;

constexpr int SUBSET_NAME_LENGTH_LIMIT = 32;    // max length for a model subset's (mesh) name
constexpr int MAX_LENGTH_MODEL_NAME    = 32;
constexpr int MAX_LENGTH_TEXTURE_NAME  = 32;
constexpr int MAX_LENGTH_MATERIAL_NAME = 32;

//
// TEXTURES TYPEDEFS
//
using TexID     = uint32_t;                       // texture ID
constexpr TexID INVALID_TEXTURE_ID = 0;
constexpr size  NUM_TEXTURE_TYPES = 21;

//
// TEXT TYPEDEFS
//
using SentenceID = uint32_t;

} // namespace UI

