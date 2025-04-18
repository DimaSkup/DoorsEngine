// =================================================================================
// Filename:    CameraSystem.cpp
// 
// Created:     14.01.25  by DimaSkup
// =================================================================================
#include "../Common/pch.h"
#include "CameraSystem.h"

#pragma warning (disable : 4996)


namespace ECS
{

CameraSystem::CameraSystem(Camera* pCameraComponent) 
	: pCameraComponent_(pCameraComponent)
{
	Assert::NotNullptr(pCameraComponent, "ptr to the camera component == nullptr");
}

CameraSystem::~CameraSystem() 
{
}

///////////////////////////////////////////////////////////

void CameraSystem::Update(
	const EntityID id,
	const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& proj)
{
	Camera& comp = *pCameraComponent_;

	const index idx = GetIdxByID(id);

    if (idx == -1)
    {
        sprintf(g_String, "can't update camera by id: %ud", id);
        LogErr(g_String);
        return;
    }

	comp.views[idx] = view;
	comp.projs[idx] = proj;
	comp.invViews[idx] = DirectX::XMMatrixInverse(nullptr, view);
}


// =================================================================================
//                        public API: add / remove
// =================================================================================

void CameraSystem::AddRecord(
	const EntityID id,
	const DirectX::XMMATRIX& view,   // camera view matrix
	const DirectX::XMMATRIX& proj)   // camera projection matrix
{
	Camera& comp = *pCameraComponent_;

    const index idx = comp.ids.get_insert_idx(id);
    const bool exist = (comp.ids[idx] == id);

	// if there is no such record yet we create a new one
	if (!exist)
	{
        comp.ids.insert_before(idx, id);
        comp.views.insert_before(idx, view);
        comp.projs.insert_before(idx, proj);
        comp.invViews.insert_before(idx, DirectX::XMMatrixInverse(nullptr, view));
	}
    else
    {
        sprintf(g_String, "can't add new record there is already such by ID: %ud", id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void CameraSystem::AddRecords()
{
	assert(0 && "TODO");
}

void CameraSystem::RemoveRecord()
{
	assert(0 && "TODO");
}

void CameraSystem::RemoveRecords()
{
	assert(0 && "TODO");
}


// =================================================================================
//                           public API: getters
// =================================================================================

const DirectX::XMMATRIX& CameraSystem::GetViewMatrixByID(const EntityID id)
{
	return pCameraComponent_->views[GetIdxByID(id)];
}

const DirectX::XMMATRIX& CameraSystem::GetProjMatrixByID(const EntityID id)
{
	return pCameraComponent_->projs[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

void CameraSystem::GetViewAndProjByID(
	const EntityID id,
	DirectX::XMMATRIX& view,
	DirectX::XMMATRIX& proj)
{
	const index idx = GetIdxByID(id);

    if (idx == -1)
    {
        sprintf(g_String, "can't get data by id: %ud", id);
        LogErr(g_String);
        return;
    }

	view = pCameraComponent_->views[idx];
	proj = pCameraComponent_->projs[idx];
}


} // namespace ECS



