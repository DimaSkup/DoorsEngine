/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: model_loader.h
    Desc:     load model from internal format file

    Created:  19.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once


namespace Core
{

class BasicModel;

class ModelLoader
{
public:
    ModelLoader() {}

    bool Load(const char* filePath, BasicModel* pModel);
};

} // namespace
