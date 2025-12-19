/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: model_exporter.h
    Desc:     exports models which were imported or manually generated into
              the .de3d format

    Created:  11.11.2024 by DimaSkup
\**********************************************************************************/
#pragma once

#include <d3d11.h>


namespace Core
{

class BasicModel;
struct Subset;

//---------------------------------------------------------

class ModelExporter
{
public:
    ModelExporter();

    bool ExportIntoDE3D(
        ID3D11Device* pDevice,
        const BasicModel* pModel,
        const char* targetDir,
        const char* targetName);
};

} // namespace
