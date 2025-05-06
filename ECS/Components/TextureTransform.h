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
//                                 COMPONENT
// ************************************************************************************
struct TextureTransform
{
    // NOTE: we get responsible texture transformation by data idx of ID

    TextureTransform()
    {
        // setup transfromation for "invalid" entity so if we will query for not
        // existent entity we will receive data of entity by ID == 0 
        ids.push_back(0);
        transformTypes.push_back(STATIC);

        const DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
        texTransforms.push_back(I);

        StaticTexTransInitParams params(1, I, I);
        texStaticTrans.ids.push_back(0);
        texStaticTrans.transformations.push_back(I);

    }

    cvector<EntityID>           ids;
    cvector<TexTransformType>   transformTypes;
    cvector<XMMATRIX>           texTransforms;           // current textures transformations

    TexStaticTransformations    texStaticTrans;
    TexAtlasAnimations          texAtlasAnim;
    TexRotationsAroundCoords    texRotations;

    eComponentType              type = eComponentType::TextureTransformComponent;
};


};
