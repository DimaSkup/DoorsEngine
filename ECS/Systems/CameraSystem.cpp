// =================================================================================
// Filename:    CameraSystem.cpp
// 
// Created:     14.01.25  by DimaSkup
// =================================================================================
#include "CameraSystem.h"
#include "../Common/Assert.h"

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

	comp.views_[idx] = view;
	comp.projs_[idx] = proj;
	comp.invViews_[idx] = DirectX::XMMatrixInverse(nullptr, view);
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

	// if there is no such record yet we create a new one
	if (!Utils::BinarySearch(comp.ids_, id))
	{
		const index insertAt = Utils::GetPosForID(comp.ids_, id);

		Utils::InsertAtPos(comp.ids_, insertAt, id);
		Utils::InsertAtPos(comp.views_, insertAt, view);
		Utils::InsertAtPos(comp.projs_, insertAt, proj);
		Utils::InsertAtPos(comp.invViews_, insertAt, DirectX::XMMatrixInverse(nullptr, view));
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
	return pCameraComponent_->views_[GetIdxByID(id)];
}

const DirectX::XMMATRIX& CameraSystem::GetProjMatrixByID(const EntityID id)
{
	return pCameraComponent_->projs_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

void CameraSystem::GetViewAndProjByID(
	const EntityID id,
	DirectX::XMMATRIX& view,
	DirectX::XMMATRIX& proj)
{
	const index idx = GetIdxByID(id);
	view = pCameraComponent_->views_[idx];
	proj = pCameraComponent_->projs_[idx];
}


} // namespace ECS



