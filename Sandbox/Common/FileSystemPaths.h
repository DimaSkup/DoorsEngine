// *********************************************************************************
// Filename:     FileSystemPaths.h
// Description:  constants for different paths
// 
// Created:      21.11.24
// *********************************************************************************
#pragma once

namespace Game
{

    // Rel - relative
    static const char* g_RelPathDataDir         = "data/";
    static const char* g_RelPathExtModelsDir    = "data/models/ext/";      // external models (.obj, .blend, .fbx, etc.)
    static const char* g_RelPathAssetsDir       = "data/models/assets/";   // assets (models in the internal format: .de3d)
    static const char* g_RelPathTexDir          = "data/textures/";
    static const char* g_RelPathUIDataDir       = "data/ui/";
    static const char* g_RelPathAudioDir        = "data/audio/";
}

// full paths from the sys root
//static const std::string g_BuildDir(BUILD_DIR);
