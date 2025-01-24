// *********************************************************************************
// Filename:     Types.h
// Description:  contains some common typedefs for the engine
// 
// Created:      10.07.24
// *********************************************************************************
#pragma once

#include <cstdint>
#include <string>

using u32 = uint32_t;
using size = ptrdiff_t;
using index = ptrdiff_t;
using UINT = unsigned int;

//
// MESH/MODEL TYPEDEFS
//
using MeshName  = std::string;
using ModelName = std::string;
using ModelID   = uint32_t;

//
// TEXTURES TYPEDEFS
//
using TexID = uint32_t;                                // texture ID
using TexPath = std::string;                           // texture path
using TexName = std::string;                           // texture name


//
// TEXT TYPEDEFS (for UI)
//
using SentenceID = uint32_t;