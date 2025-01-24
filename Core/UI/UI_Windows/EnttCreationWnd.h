// ********************************************************************************
// Filename:     EnttCreationWnd.h
// Description:  a UI window for creation and setup an ECS Entity
// 
// Created:      04.06.24
// ********************************************************************************
#pragma once




#include "../Mesh/MeshHelperTypes.h"
#include "../Texture/TextureHelperTypes.h"

#include "../../Model/ModelStorage.h"

 // from the ECS module
#include "Common/Types.h"              
#include "Entity/EntityMgr.h"

#include <string>
#include <set>
#include <DirectXMath.h>



class EnttCreationWnd
{
public:
	using EntityMgr = ECS::EntityMgr;

public:

	// data structure to hold the state of the window between frames
	struct WndStates
	{
		WndStates() {}

		// common
		std::string entityName;
		std::string errorMsg;
		std::set<ECS::ComponentType> addedComponents;   // is used in the selectable menu for adding components

		ECS::TransformRawData transform;
		ECS::MovementRawData movement;

		ModelName modelName;
		TexName chosenTexName;

		void Reset()
		{
		}
	};

public:
	EnttCreationWnd(ID3D11DeviceContext* pContext) 
	{
		Assert::NotNullptr(pContext, "ptr to device context == nullptr");
		pContext->GetDevice(&pDevice_);
	}

	void Shutdown() { SafeDelete(pWndStates_); }

	void ShowWndToCreateEntt(bool* pOpen, EntityMgr& mgr);
	void ShowSelectableMenuToAddComponents(EntityMgr& mgr);
	void ShowSetupFieldsOfAddedComponents();

	// components setup elements
	void ShowFieldsToSetupTransformParams(ECS::TransformRawData& data);
	void ShowFieldsToSetupMovementParams(ECS::MovementRawData& data);
	void ShowSelectableMenuOfMeshes(MeshName& chosenMesh);
	void ShowTexturesMenuForMesh(TexName& chosenTex);
	
	// "Create/Close/Reset/etc." buttons
	void ShowButtonsOfWnd(EntityMgr& mgr, bool* pOpen);

	void AddChosenComponentsToEntity(EntityMgr& mgr, const EntityID id);
	
private:
	WndStates* pWndStates_ = nullptr;
	ID3D11Device* pDevice_ = nullptr;
};