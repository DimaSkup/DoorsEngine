/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: material_writer.h
    Desc:     write materials into file

    Created:  19.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once

namespace Core
{

struct Material;

class MaterialWriter
{
public:
    MaterialWriter() {}

    bool Write(
        const Material* materials,
        const int numMaterials,
        const char* filePath);
};


} // namespace
