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

struct TransformComponentData
{
    Vec3 position;
    Vec3 direction;
    float uniformScale = 1.0f;
};

struct ModelComponentData
{
    uint32_t modelID = 0;
    std::string selectedModelName = "invalid_model";

    Core::cvector<std::string> modelsNames;
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
    eAddedComponents addedComponents_;
    const int maxNameLength_ = 64;
    char enttName_[64]{ '\0' };

    // components data
    TransformComponentData transformData_;
    ModelComponentData modelData_;
};

};
