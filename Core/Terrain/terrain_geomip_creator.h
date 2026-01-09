/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: terrain_geomip_creator.h
    Desc:     a functional for creation and setup of a geomipmapped terrain

    Created:  11.07.2025  by DimaSkup
\**********************************************************************************/
#pragma once


namespace Core
{

class TerrainGeomipCreator
{
public:
    TerrainGeomipCreator() {}

    bool Create(const char* configFilename);
};

} // namespace Core
