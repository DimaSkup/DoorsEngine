//==================================================================================
// Filename:    MaterialLoader.h
// Desc:        for loading materials from a file
//
// Created:     08.09.2025 by DimaSkup
//==================================================================================
#pragma once

namespace Core
{

class MaterialLoader
{
public:
    MaterialLoader() {}

    bool LoadFromFile(const char* path);
};

} // namespace
