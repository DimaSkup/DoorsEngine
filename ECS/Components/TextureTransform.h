// *********************************************************************************
// Filename:     TextureTransform.h
// Description:  an ECS component which contains 
//               textures transformations for entities
// 
// Created:      29.06.24
// *********************************************************************************
#pragma once

#include "Helpers/TextureTransformHelpers.h"


namespace ECS
{

enum TexTransformType
{
    STATIC,                     // just static transformation of the texture (for instance: scaling of the terrain texture, or translation to make water flowing)
    ATLAS_ANIMATION,            // move over an atlas texture to make an animation
    ROTATION_AROUND_TEX_COORD,  // rotation around particular texture coords during the time
};


// ************************************************************************************
//
//                                 COMPONENT
// 
// ************************************************************************************
struct TextureTransform
{
    // NOTE: we get responsible texture transformation by data idx of ID

    cvector<EntityID>           ids;
    cvector<TexTransformType>   transformTypes;
    cvector<XMMATRIX>           texTransforms;           // current textures transformations

    TexStaticTransformations    texStaticTrans;
    TexAtlasAnimations          texAtlasAnim;
    TexRotationsAroundCoords    texRotations;

    ComponentType               type = ComponentType::TextureTransformComponent;
};


};
