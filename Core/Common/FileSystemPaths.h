// *********************************************************************************
// Filename:     FileSystemPaths.h
// Description:  constants for different paths
// 
// Created:      21.11.24
// *********************************************************************************
#pragma once

#include <string>

// Rel - relative
static const std::string g_RelModelsDirPath = "data/models/";
static const std::string g_RelImportedModelsDirPath = "data/models/imported/";
static const std::string g_RelTexDirPath = "data/textures";


// full paths from the sys root
static const std::string g_DataDir(DATA_DIR);
static const std::string g_TexDirPath(g_DataDir + "textures/");
static const std::string g_ModelsDirPath(MODELS_DIR);
static const std::string g_ImportedModelsDirPath(g_ModelsDirPath + "imported/");
static const std::string g_DefaultModelsDirPath(g_ModelsDirPath + "default/");
