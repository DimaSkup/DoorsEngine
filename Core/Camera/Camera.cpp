// =================================================================================
// Filename:     Camera.cpp
// Description:  implementation of the Camera class functional
// 
// Created:      04.02.25  by DimaSkup
// =================================================================================
#include "Camera.h"

using namespace DirectX;


Camera::Camera()
{
	SetProjection(DirectX::XM_PIDIV4, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera()
{
}

// =================================================================================
// Public methods
// =================================================================================

DirectX::XMFLOAT3 Camera::GetPosition() const 
{
	DirectX::XMFLOAT3 pos; 
	DirectX::XMStoreFloat3(&pos, pos_);
	return pos; 
}

///////////////////////////////////////////////////////////

DirectX::XMFLOAT3 Camera::GetRight() const
{ 
	DirectX::XMFLOAT3 right; 
	DirectX::XMStoreFloat3(&right, right_); 
	return right; 
}

///////////////////////////////////////////////////////////

DirectX::XMFLOAT3 Camera::GetUp() const
{ 
	DirectX::XMFLOAT3 right; 
	DirectX::XMStoreFloat3(&right, right_); 
	return right; 
}

///////////////////////////////////////////////////////////

DirectX::XMFLOAT3 Camera::GetLook() const
{ 
	DirectX::XMFLOAT3 look;
	DirectX::XMStoreFloat3(&look, look_);
	return look;
}

///////////////////////////////////////////////////////////

void Camera::SetProjection(
	const float fovY,
	const float aspect,
	const float zn,           
	const float zf)           
{
	// setup the projection matrix and frustum properties
	nearZ_  = zn;
	farZ_   = zf;
	aspect_ = aspect;
	fovY_   = fovY;

	nearWndHeight_ = 2.0f * nearZ_ * tanf(0.5f * fovY_);
	farWndHeight_  = 2.0f * farZ_ * tanf(0.5f * fovY_);

	proj_ = XMMatrixPerspectiveFovLH(fovY, aspect, zn, zf);
}

///////////////////////////////////////////////////////////

void Camera::LookAt(
	const XMVECTOR& pos,
	const XMVECTOR& target,     // look at position
	const XMVECTOR& worldUp)
{
	// define camera space using LookAt parameters

	XMVECTOR L = XMVector3Normalize(target - pos);
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Normalize(XMVector3Cross(L, R));

	pos_ = pos;
	right_ = R;
	up_    = U;
	look_  = L;
}

///////////////////////////////////////////////////////////

void Camera::LookAt(
	const XMFLOAT3& pos,
	const XMFLOAT3& target,    // look at position
	const XMFLOAT3& up)
{
	// define camera space using LookAt parameters

	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

///////////////////////////////////////////////////////////

void Camera::Strafe(const float d)
{
	// pos_ += d*right_
	XMVECTOR s = XMVectorReplicate(d);
	pos_ = XMVectorMultiplyAdd(s, right_, pos_);
}

///////////////////////////////////////////////////////////

void Camera::Walk(const float d)
{
	// pos_ += d*look_
	XMVECTOR s = XMVectorReplicate(d);
	pos_ = XMVectorMultiplyAdd(s, look_, pos_);
}

///////////////////////////////////////////////////////////

void Camera::MoveUp(const float d)
{
	// if free camera mode:
	// pos_ += d*world_up
	if (isFreeCamera_)
	{
		XMVECTOR s = XMVectorReplicate(d);
		pos_ = XMVectorMultiplyAdd(s, { 0,1,0 }, pos_);
	}
}

///////////////////////////////////////////////////////////

void Camera::Pitch(const float angle)
{
	// rotate up and look vector about the view space right vector

	XMMATRIX R = XMMatrixRotationAxis(right_, angle);

	up_   = XMVector3TransformNormal(up_, R);
	look_ = XMVector3TransformNormal(look_, R);
}

///////////////////////////////////////////////////////////

void Camera::RotateY(const float angle)
{
	// rotate the basis vectors about the world y-axis

	XMMATRIX R = XMMatrixRotationY(angle);

	right_ = XMVector3TransformNormal(right_, R);
	up_    = XMVector3TransformNormal(up_, R);
	look_  = XMVector3TransformNormal(look_, R);
}

///////////////////////////////////////////////////////////

void Camera::RotateYAroundFixedLook(const float angle)
{
	// move horizontally around fixed look_at point by angle in radians

	XMMATRIX R = XMMatrixTransformation(
		lookAtPoint_,
		{ 0,0,0 },
		{ 1,1,1 },
		lookAtPoint_,
		XMQuaternionRotationRollPitchYaw(0, angle, 0),
		{ 0,0,0 });

	XMVECTOR newPos = XMVector3Transform(GetPositionVec(), R);
	LookAt(newPos, lookAtPoint_, { 0,1,0 });
}

///////////////////////////////////////////////////////////

void Camera::PitchAroundFixedLook(const float angle)
{
	// move vertically around fixed look_at point by angle in radians

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
}

///////////////////////////////////////////////////////////

void Camera::UpdateViewMatrix()
{
	XMVECTOR& R = right_;
	XMVECTOR& U = up_;
	XMVECTOR& L = look_;
	XMVECTOR& P = pos_;

	// keep camera's axes orthogonal to each other and of unit length
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// U, L already othor-normal, so no need to normalize cross product
	R = XMVector3Cross(U, L);

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

	view_ =
	{
		right.x,   up.x,   look.x,    0,
		right.y,   up.y,   look.y,    0,
		right.z,   up.z,   look.z,    0,
			  x,      y,        z,    1
	};

	// compute inverse view matrix
	invView_ = XMMatrixInverse(nullptr, view_);
}