// *********************************************************************************
// Filename:     Types.h
// Description:  contains some common typedefs and constants for the UI
// 
// Created:      28.03.2025 by DimaSkup
// *********************************************************************************
#pragma once

#include <cstdint>
#include <string>


namespace UI
{

using u32 = uint32_t;
using size = ptrdiff_t;
using index = ptrdiff_t;
using UINT = unsigned int;

//
// MESH/MODEL TYPEDEFS
//
using MeshName = std::string;
using ModelName = std::string;
using ModelID = uint32_t;
using SubsetID = uint16_t;                    // mesh ID
constexpr ModelID INVALID_MODEL_ID = 0;

//
// MATERIAL TYPEDEFS
//
using MaterialName = std::string;
using MaterialID = uint32_t;
constexpr MaterialID INVALID_MATERIAL_ID = 0;

//
// TEXTURES TYPEDEFS
//
using TexID = uint32_t;                       // texture ID
using TexPath = std::string;                    // texture path
using TexName = std::string;                    // texture name
constexpr TexID INVALID_TEXTURE_ID = 0;
constexpr size  NUM_TEXTURE_TYPES = 21;

//
// TEXT TYPEDEFS
//
using SentenceID = uint32_t;

} // namespace UI

