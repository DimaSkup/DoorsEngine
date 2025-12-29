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

namespace Core
{

class BasicModel;

//---------------------------------------------------------

class ModelExporter
{
public:
    ModelExporter();

    bool ExportIntoDE3D(
        const BasicModel* pModel,
        const char* targetDir,
        const char* targetName);
};

} // namespace
