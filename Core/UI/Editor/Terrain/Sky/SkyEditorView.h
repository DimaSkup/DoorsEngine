// ====================================================================================
// Filename:      SkyEditorView.h
// Description:   the View part of the SkyEditor MVC;
//                visualises data of the Model::Sky;
//                contains fields to change the Sky params;
// ====================================================================================
#pragma once

#include "SkyEditorModel.h"


namespace UI
{
// forward declaration
class IEditorController;


class ViewSky
{
private:
	IEditorController* pController_ = nullptr;

public:
    ViewSky(IEditorController* pController);
   
    void Render(const ModelSky& data);
};

}

