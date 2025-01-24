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
	STATIC,                     // no movement over the texture, just static transformation of the texture (for instance: scaling of the terrain texture)
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

	ComponentType type_ = ComponentType::TextureTransformComponent;

	std::vector<EntityID> ids_;
	std::vector<TexTransformType> transformTypes_;
	std::vector<XMMATRIX> texTransforms_;           // current textures transformations

	TexStaticTransformations texStaticTrans_;
	TexAtlasAnimations texAtlasAnim_;
	TexRotationsAroundCoords texRotations_;
};


};
