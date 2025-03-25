// =================================================================================
// Filename:    EntityCreatorWnd.h
// Description: renders and handles a window for creation and setup of new entity;
//
// Created:     18.03.2025  by DimaSkup
// =================================================================================
#pragma once

#include <CoreCommon/cvector.h>
#include <UICommon/IFacadeEngineToUI.h>
#include <UICommon/Vectors.h>
#include "../../EditorPanelElement/ModelsAssetsList.h"



namespace UI
{

struct eAddedComponents
{
    bool isAddedMovement = false;
    bool isAddedRendered = false;

    bool isAddedModel = false;
    bool isAddedCamera = false;
    bool isAddedTexture = false;
    bool isAddedTexTransform = false;
};

///////////////////////////////////////////////////////////

struct TransformComponentData
{
    Vec3 position;
    Vec3 direction;
    float uniformScale = 1.0f;
};

struct ModelComponentData
{
    EntityID modelID = 0;
    std::string selectedModelName = "invalid_model";
};

struct NameComponentData
{
    const int maxNameLength = 64;
    char enttName[64]{ '\0' };
};

///////////////////////////////////////////////////////////

class EntityCreatorWnd
{
public:
    EntityCreatorWnd();

    void RenderCreationWindow(bool* pOpen, IFacadeEngineToUI* pFacade);

private:
    void ShowAddComponentCheckboxes();

    void ShowTransformComponentFields();
    void ShowModelComponentFields(IFacadeEngineToUI* pFacade);
    void ShowRenderedComponentFields();

private:
    // window elements
    ModelsAssetsList        modelsList_;

    // components data
    eAddedComponents        addedComponents_;
    TransformComponentData  transformData_;
    ModelComponentData      modelData_;
    NameComponentData       nameData_;
    
};

};
