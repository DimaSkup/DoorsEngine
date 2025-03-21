// *********************************************************************************
// Filename:     FileSystemPaths.h
// Description:  constants for different paths
// 
// Created:      21.11.24
// *********************************************************************************
#pragma once

#include <string>

// Rel - relative
static const std::string g_RelPathExtModelsDir = "data/models/ext/";     // external models (.obj, .blend, .fbx, etc.)
static const std::string g_RelPathAssetsDir = "data/models/assets/";      // assets (models in the internal format: .de3d)
//static const std::string g_RelImportedModelsDirPath = "data/models/imported/";
static const std::string g_RelPathTexDir = "data/textures/";
static const std::string g_RelPathUIDataDir = "data/ui/";

const std::string g_RelPathAudioDir = "data/audio/";


// full paths from the sys root
static const std::string g_BuildDir(BUILD_DIR);
//static const std::string g_DataDir(g_BuildDir + "data/");
//static const std::string g_TexDirPath(g_DataDir + "textures/");
//static const std::string g_ModelsDirPath(g_DataDir + "models/");
//static const std::string g_ImportedModelsDirPath(g_DataDir + "models/imported/");
//static const std::string g_UIDataDirPath(g_DataDir + "ui/");
