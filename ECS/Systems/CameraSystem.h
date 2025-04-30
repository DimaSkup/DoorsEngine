// =================================================================================
// Filename:    CameraSystem.h
// Description: an ECS system is used for handling cameras
// 
// Created:     14.01.25  by DimaSkup
// =================================================================================
#pragma once

#include "../Systems/TransformSystem.h"
#include "../Components/Camera.h"


namespace ECS
{

class CameraSystem
{
public:
    CameraSystem(Camera* pCameraComponent, TransformSystem* pTransformSys);
    ~CameraSystem();

    //void Update();
    const XMMATRIX& UpdateView(const EntityID id);

    void AddRecord   (const EntityID id, const CameraData& data);
    void RemoveRecord(const EntityID id);
    bool HasEntity   (const EntityID id) const;

    void Strafe (const EntityID id, const float d);
    void Walk   (const EntityID id, const float d);
    void MoveUp (const EntityID id, const float d);

    void Pitch  (const EntityID id, const float angle);
    void RotateY(const EntityID id, const float angle);

    inline const CameraData& GetCameraData(const EntityID id) const { return pCameraComponent_->data[id]; }

    inline const XMMATRIX& GetBaseView   (const EntityID id) const { return (HasEntity(id)) ? pCameraComponent_->data[id].baseView : pCameraComponent_->data[0].baseView; }
    inline const XMMATRIX& GetView       (const EntityID id) const { return (HasEntity(id)) ? pCameraComponent_->data[id].view     : pCameraComponent_->data[0].view; }
    inline const XMMATRIX& GetInverseView(const EntityID id) const { return (HasEntity(id)) ? pCameraComponent_->data[id].invView  : pCameraComponent_->data[0].invView; }
    inline const XMMATRIX& GetProj       (const EntityID id) const { return (HasEntity(id)) ? pCameraComponent_->data[id].proj     : pCameraComponent_->data[0].proj; }
    inline const XMMATRIX& GetOrtho      (const EntityID id) const { return (HasEntity(id)) ? pCameraComponent_->data[id].ortho    : pCameraComponent_->data[0].ortho; }
    
    inline const XMMATRIX  GetViewProj(const EntityID id) const
    {
        if (HasEntity(id))
        {
            const XMMATRIX& view = GetView(id);
            const XMMATRIX& proj = GetProj(id);
            return view * proj;
        }

        return DirectX::XMMatrixIdentity();
    }
    

    void SetBaseViewMatrix(const EntityID id, const XMMATRIX& baseView);

    void SetupProjection(
        const EntityID id,
        const float fovY,
        const float aspectRatio,
        const float nearZ,
        const float farZ);

    void SetupOrthographicMatrix(
        const EntityID id,
        const float viewWidth,
        const float viewHeight,
        const float nearZ,
        const float farZ);
 
    // get camera basis vectors
    inline const XMVECTOR& GetPosVec  (const EntityID id) const { return pTransformSys_->GetPositionVec(id); }
    inline const XMVECTOR& GetLookVec (const EntityID id) const { return pTransformSys_->GetDirectionVec(id); }
    inline const XMVECTOR& GetRightVec(const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).right : GetCameraData(0).right; }

    inline XMFLOAT3 GetPos (const EntityID id) const { return pTransformSys_->GetPosition(id); }
    XMFLOAT3        GetLook(const EntityID id) const;

    bool SetRightVec(const EntityID id, const XMVECTOR& right);

    // get frustum properties
    inline float GetNearZ (const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).nearZ : -1.0f; }
    inline float GetFarZ  (const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).farZ  : -1.0f; }
    inline float GetAspect(const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).aspectRatio : -1.0f; }
    inline float GetFovY  (const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).fovY  : -1.0f; }
    inline float GetFovX  (const EntityID id) const { return 2.0f * atanf(0.5f * GetNearWndWidth(id) / GetNearZ(id)); }

    // get near and far plane dimensions in view space coordinates
    float GetNearWndWidth (const EntityID id) const { return GetAspect(id) * GetNearWndHeight(id); }
    float GetFarWndWidth  (const EntityID id) const { return GetAspect(id) * GetFarWndHeight(id); }
    float GetNearWndHeight(const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).nearWndHeight : -1.0f; }
    float GetFarWndHeight (const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).farWndHeight  : -1.0f; }



    void PitchAroundFixedLook(const EntityID id, const float angle);
    void RotateYAroundFixedLook(const EntityID id, const float angle);

    // get/set fixed camera properties
    inline bool IsFixedLook(const EntityID id) const { return (HasEntity(id)) ? GetCameraData(id).isFixedLook_ : false; }
    //inline void SetFixedLookState(const bool isFixed) { isFixedLook_ = isFixed; }
    //inline void SetFixedLookAtPoint(const DirectX::XMVECTOR& lookAt) { lookAtPoint_ = lookAt; }

private:
    Camera*          pCameraComponent_ = nullptr;
    TransformSystem* pTransformSys_    = nullptr;
};

}
