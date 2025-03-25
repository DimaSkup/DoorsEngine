// =================================================================================
// Filename:    CameraSystem.h
// Description: an ECS system is used for handling cameras
// 
// Created:     14.01.25  by DimaSkup
// =================================================================================
#pragma once

#include "../Components/Camera.h"


namespace ECS
{

class CameraSystem
{
public:
    CameraSystem(Camera* pCameraComponent);
    ~CameraSystem();

    void Update(
        const EntityID id,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& proj);

    void AddRecord(
        const EntityID id,
        const DirectX::XMMATRIX& view,    // camera view matrix
        const DirectX::XMMATRIX& proj);   // camera projection matrix

    void AddRecords();

    void RemoveRecord();
    void RemoveRecords();

    //
    // getters
    //
    const DirectX::XMMATRIX& GetViewMatrixByID(const EntityID id);
    const DirectX::XMMATRIX& GetProjMatrixByID(const EntityID id);

    void GetViewAndProjByID(
        const EntityID id,
        DirectX::XMMATRIX& view,
        DirectX::XMMATRIX& proj);

private:
    inline index GetIdxByID(const EntityID id)
    {
        // return valid idx if there is an entity by such ID;
        // or return -1 if there is no such entity;
        const Camera& comp  = *pCameraComponent_;
        const index idx     = comp.ids.get_idx(id);
        const bool exist    = (comp.ids[idx] == id);

        return (exist) ? idx : -1;
    }

private:
    Camera* pCameraComponent_ = nullptr;
};

}
