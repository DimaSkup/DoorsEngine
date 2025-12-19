// ====================================================================================
// Filename:      FogEditorView.h
// Description:   the View part of the FogEditor MVC;
//                visualises data of the Model::Fog;
//                contains fields to change the Fog params;
// 
// Created:       31.12.24
// ====================================================================================
#pragma once



#include "FogEditorModel.h"



namespace UI
{
// forward declaration
class IEditorController;


class ViewFog
{
private:
    IEditorController* pController_ = nullptr;

public:
    ViewFog(IEditorController* pController);

    void Draw(const ModelFog* pData);
};

} // namespace UI
