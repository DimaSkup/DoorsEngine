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

#include <d3d11.h>


namespace Core
{

class TerrainGeomipCreator
{
public:
    TerrainGeomipCreator() {}

    bool Create(ID3D11Device* pDevice, const char* configFilename);
};

} // namespace Core
