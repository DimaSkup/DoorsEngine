/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: material_reader.h
    Desc:     read materials from file

    Created:  19.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once

namespace Core
{

class MaterialReader
{
public:
    MaterialReader() {}

    bool Read(const char* filePath);
};

} // namespace
