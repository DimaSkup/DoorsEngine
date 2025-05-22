// =================================================================================
// Filename:    CameraSystem.cpp
// 
// Created:     14.01.25  by DimaSkup
// =================================================================================
#include "../Common/pch.h"
#include "CameraSystem.h"

#pragma warning (disable : 4996)
using namespace DirectX;


namespace ECS
{

CameraSystem::CameraSystem(Camera* pCameraComponent, TransformSystem* pTransformSys) :
    pCameraComponent_(pCameraComponent),
    pTransformSys_(pTransformSys)
{
    Assert::NotNullptr(pCameraComponent, "ptr to the camera component == nullptr");
    Assert::NotNullptr(pTransformSys,    "ptr to the transform system == nullptr");
}

CameraSystem::~CameraSystem() 
{
}


// =================================================================================
//                        public API: add / remove
// =================================================================================
void CameraSystem::AddRecord(const EntityID id, const CameraData& data)
{
    Camera& comp = *pCameraComponent_;

    if (comp.data.contains(id))
    {
        sprintf(g_String, "can't add a new record; there is already such by ID: %ud", id);
        LogErr(g_String);
        return;
    }

    // make a record and setup proj matrix
    comp.data.emplace(id, data);
    SetupProjection(id, data.fovY, data.aspectRatio, data.nearZ, data.farZ);
}

///////////////////////////////////////////////////////////

void CameraSystem::RemoveRecord(const EntityID id)
{
    if (pCameraComponent_->data.erase(id) == 0)
    {
        sprintf(g_String, "can't remove (there is no camera by ID: %ld)", id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

inline bool CameraSystem::HasEntity(const EntityID id) const
{
    // return true if there is a camera with ID or false in another case
    if (pCameraComponent_->data.contains(id))
    {
        return true;
    }
    else
    {
        sprintf(g_String, "there is no camera with ID: %ld", id);
        LogErr(g_String);
        return false;
    }
}

///////////////////////////////////////////////////////////

void CameraSystem::Strafe(const EntityID id, const float d)
{
    // move left/right the camera by distance d
    const XMVECTOR  s     = XMVectorReplicate(d);
    const XMVECTOR& right = GetRightVec(id);
    const XMVECTOR& pos   = pTransformSys_->GetPositionVec(id);

    // pos += d * right_vec
    pTransformSys_->SetPositionVec(id, XMVectorMultiplyAdd(s, right, pos));
}

///////////////////////////////////////////////////////////

void CameraSystem::Walk(const EntityID id, const float d)
{
    // move forward/backward the camera by distance d
    const XMVECTOR s = XMVectorReplicate(d);
    XMVECTOR pos;
    XMVECTOR look;

    // pos += d * look_vec
    pTransformSys_->GetPosAndDir(id, pos, look);
    pTransformSys_->SetPositionVec(id, XMVectorMultiplyAdd(s, look, pos));
}

///////////////////////////////////////////////////////////

void CameraSystem::MoveUp(const EntityID id, const float d)
{
    // move the camera up by distance d
    XMVECTOR s         = XMVectorReplicate(d);
    const XMVECTOR pos = pTransformSys_->GetPositionVec(id);

    // pos_ += d * world_up_vec
    pTransformSys_->SetPositionVec(id, XMVectorMultiplyAdd(s, { 0,1,0 }, pos));
}

///////////////////////////////////////////////////////////

void CameraSystem::Pitch(const EntityID id, const float angle)
{
    // rotate the look vector about the view space right vector
    const XMVECTOR Q = DirectX::XMQuaternionRotationAxis(GetRightVec(id), angle);
    pTransformSys_->RotateLocalSpaceByQuat(id, Q);
}

///////////////////////////////////////////////////////////

void CameraSystem::RotateY(const EntityID id, const float angle)
{
    // rotate the basis vectors (right/look) about the world's y-axis
    const XMVECTOR quat = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, angle);
    const XMVECTOR invQuat = DirectX::XMQuaternionConjugate(quat);

    // update right vector of the camera: rotated_vec = inv_quat * vec * quat
    const XMVECTOR tmpVec = XMQuaternionMultiply(invQuat, GetRightVec(id));
    SetRightVec(id, XMQuaternionMultiply(tmpVec, quat));

    // update look vector (direction)
    pTransformSys_->RotateLocalSpaceByQuat(id, quat);
}

///////////////////////////////////////////////////////////

void CameraSystem::GetAllCamerasIds(cvector<EntityID>& outIds) const
{
    Camera& comp = *pCameraComponent_;
    outIds.resize(comp.data.size() - 1);   // -1 because we don't want to get id == 0 (INVALID_ENTITY_ID)

    for (int i = 0; const auto & it : comp.data)
    {
        if (it.first != INVALID_ENTITY_ID)
            outIds[i++] = it.first;
    }
}

///////////////////////////////////////////////////////////

void CameraSystem::SetAspectRatio(const EntityID id, const float aspect)
{
    // update aspect ration and perspective projection of camera by ID
    if (HasEntity(id))
    {
        CameraData& data = pCameraComponent_->data.at(id);

        data.aspectRatio = aspect;
        data.proj = XMMatrixPerspectiveFovLH(data.fovY, aspect, data.nearZ, data.farZ);
    }
}

///////////////////////////////////////////////////////////

void CameraSystem::SetBaseViewMatrix(const EntityID id, const XMMATRIX& baseView)
{
    if (HasEntity(id))
        pCameraComponent_->data[id].baseView = baseView;
}

///////////////////////////////////////////////////////////

void CameraSystem::SetupProjection(
    const EntityID id,
    const float fovY,
    const float aspectRatio,
    const float nearZ,
    const float farZ)
{
    // setup the projection matrix and frustum properties

    if (HasEntity(id))
    {
        CameraData& data = pCameraComponent_->data.at(id);

        data.nearZ = nearZ;
        data.farZ = farZ;
        data.aspectRatio = aspectRatio;
        data.fovY = fovY;

        data.nearWndHeight = 2.0f * nearZ * tanf(0.5f * fovY);
        data.farWndHeight  = 2.0f * farZ  * tanf(0.5f * fovY);

        data.proj = XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ);
    }
}

///////////////////////////////////////////////////////////

void CameraSystem::SetupOrthographicMatrix(
    const EntityID id,
    const float viewWidth,
    const float viewHeight,
    const float nearZ,
    const float farZ)
{
    // Initialize the orthographic matrix for 2D rendering

    if (HasEntity(id))
    {
        CameraData& data = pCameraComponent_->data.at(id);

        data.ortho = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);
    }
}

///////////////////////////////////////////////////////////

void CameraSystem::RotateYAroundFixedLook(const EntityID id, const float angle)
{
    // move horizontally around fixed look_at point by angle in radians

    assert(0 && "FIXME");
#if 0

    XMMATRIX R = XMMatrixTransformation(
        lookAtPoint_,
        { 0,0,0 },
        { 1,1,1 },
        lookAtPoint_,
        XMQuaternionRotationRollPitchYaw(0, angle, 0),
        { 0,0,0 });

    XMVECTOR newPos = XMVector3Transform(GetPositionVec(), R);
    LookAt(newPos, lookAtPoint_, { 0,1,0 });
#endif
}

///////////////////////////////////////////////////////////

void CameraSystem::PitchAroundFixedLook(const EntityID id, const float angle)
{
    // move vertically around fixed look_at point by angle in radians

    assert(0 && "FIXME");

#if 0
    using namespace DirectX;

    // distance from the current position to the fixed look_at point
    float distToLookAt = XMVectorGetX(XMVector3Length(lookAtPoint_ - GetPositionVec()));

    XMVECTOR horizontalAxis = XMVector3Normalize(XMVector3Cross({ 0, 1, 0 }, look_));
    XMMATRIX R              = XMMatrixRotationNormal(horizontalAxis, angle);
    XMVECTOR newLookVec     = XMVector3Normalize(XMVector3TransformCoord(look_, R));

    // p = p0 + v*t
    XMVECTOR newPos = lookAtPoint_ - (newLookVec * distToLookAt);

    // update position
    LookAt(newPos, lookAtPoint_, { 0,1,0 });
#endif
}

///////////////////////////////////////////////////////////

#if 0
void CameraSystem::Update()
{
    // update view matrix of each existing camera
    for (auto& [cameraID, data] : pCameraComponent_->data)
    {
        UpdateView(cameraID);
    }
}
#endif

///////////////////////////////////////////////////////////

const XMMATRIX& CameraSystem::UpdateView(const EntityID id)
{
    if (!HasEntity(id))
        return pCameraComponent_->data[0].view;

    CameraData& camData = pCameraComponent_->data[id];

    XMVECTOR P;   // camera's position
    XMVECTOR L;   // camera's direction
    
    pTransformSys_->GetPosAndDir(id, P, L);

    // compute camera's right vector (cross: look X world_up)
    XMVECTOR R = XMVector3Normalize(XMVector3Cross({ 0,1,0,0 }, L));

    // camera's up vector: L, R already ortho-normal, so no need to normalize cross product
    XMVECTOR U = XMVector3Cross(L, R);


    // fill in the view matrix entries
    float x = -XMVectorGetX(XMVector3Dot(P, R));
    float y = -XMVectorGetX(XMVector3Dot(P, U));
    float z = -XMVectorGetY(XMVector3Dot(P, L));

    XMFLOAT3 right;
    XMFLOAT3 up;
    XMFLOAT3 look;
    XMStoreFloat3(&right, R);
    XMStoreFloat3(&up, U);
    XMStoreFloat3(&look, L);

    camData.right = R;

    camData.view =
    {
        right.x,   up.x,   look.x,    0,
        right.y,   up.y,   look.y,    0,
        right.z,   up.z,   look.z,    0,
              x,      y,        z,    1
    };

    // compute inverse view matrix
    camData.invView = XMMatrixInverse(nullptr, camData.view);

    return camData.view;
}


// =================================================================================
// Get camera's position/direction/vectors data
// =================================================================================
XMFLOAT3 CameraSystem::GetLook(const EntityID id) const
{
    XMFLOAT3 dir;
    XMStoreFloat3(&dir, pTransformSys_->GetDirectionVec(id));
    return dir;
}

///////////////////////////////////////////////////////////

bool CameraSystem::SetRightVec(const EntityID id, const XMVECTOR& right)
{
    bool exist = false;

    if (exist = HasEntity(id))
        pCameraComponent_->data[id].right = DirectX::XMVector3Normalize(right);

    return exist;
}


} // namespace ECS



