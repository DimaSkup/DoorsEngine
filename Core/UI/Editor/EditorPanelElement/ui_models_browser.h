// =================================================================================
// Filename:    ui_models_browser.h
// Description: a common element which is used several times in the editor and
//              represents a selectable (!) list of models
//              (it can be a list of names or pictures of these models/assets)
//
// Created:     22.03.2025 by DimaSkup
// =================================================================================
#pragma once

#include <cvector.h>
#include <string>


namespace UI
{

// forward declaration
class IFacadeEngineToUI;


class UIModelsBrowser
{
public:
    void LoadModelsNamesList(IFacadeEngineToUI* pFacade);
    void PrintModelsNamesList();

private:
    ModelID                     selectedModelID_ = 0;
    cvector<std::string>        modelsNames_;           // a list of models (assets) names
    std::string                 selectedModelName_;
};

}; // namespace UI
