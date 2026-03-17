/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: terrain_initializer.h
    Desc:     a functional for creation and setup of a terrain

    Created:  11.07.2025  by DimaSkup
\**********************************************************************************/
#pragma once


namespace Core
{

class TerrainInitializer
{
public:
    TerrainInitializer() {}

    bool Init(const char* configFilename);
};

} // namespace
