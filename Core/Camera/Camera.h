// =================================================================================
// Filename:     Camera.h
// Description:  camera basic class
// 
// Created:      04.02.25   by DimaSkup
// =================================================================================
#pragma once

#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	~Camera();

	// get/set world camera position
	DirectX::XMFLOAT3 GetPosition()                                      const;
	inline DirectX::XMVECTOR GetPositionVec()                            const { return pos_; };
	inline void SetPosition(const float x, const float y, const float z)       { pos_ = { x,y,z }; }
	inline void SetPosition(const DirectX::XMFLOAT3& pos)                      { pos_ = DirectX::XMLoadFloat3(&pos); };

	// get camera basis vectors
	inline DirectX::XMVECTOR GetRightVec()                               const { return right_; }
	inline DirectX::XMVECTOR GetUpVec()                                  const { return up_; }
	inline DirectX::XMVECTOR GetLookVec()                                const { return look_; }
	DirectX::XMFLOAT3 GetRight()                                         const;
	DirectX::XMFLOAT3 GetUp()                                            const;
	DirectX::XMFLOAT3 GetLook()                                          const;
	
	// switch btw free camera (for editor) and game mode
	inline void SetFreeCamera(const bool state)                                { isFreeCamera_ = state;}
	inline bool IsFreeCamera()                                           const { return isFreeCamera_; }

	// get/set fixed camera properties
	inline bool IsFixedLook()                                            const { return isFixedLook_; }
	inline void SetFixedLookState(const bool isFixed)                          { isFixedLook_ = isFixed; }
	inline void SetFixedLookAtPoint(const DirectX::XMVECTOR& lookAt)           { lookAtPoint_ = lookAt;}

	void PitchAroundFixedLook(const float angle);
	void RotateYAroundFixedLook(const float angle);
	



	// get frustum properties
	inline float GetNearZ()                                              const { return nearZ_; }
	inline float GetFarZ()                                               const { return farZ_; }
	inline float GetAspect()                                             const { return aspect_; }
	inline float GetFovY()                                               const { return fovY_; }
	inline float GetFovX()                                               const { return 2.0f * (float)atan(0.5f * GetNearWndWidth() / nearZ_); }

	// get near and far plane dimensions in view space coordinates
	float GetNearWndWidth()                                              const { return aspect_* nearWndHeight_; }
	float GetNearWndHeight()                                             const { return nearWndHeight_; }
	float GetFarWndWidth()                                               const { return aspect_ * farWndHeight_; }
	float GetFarWndHeight()                                              const { return farWndHeight_; }

	// set frustum
	void SetProjection(const float fovY, const float aspect, const float zn, const float zf);

	// define camera space using LookAt parameters
	void LookAt(
		const DirectX::XMVECTOR& pos,
		const DirectX::XMVECTOR& target,
		const DirectX::XMVECTOR& worldUp);

	void LookAt(
		const DirectX::XMFLOAT3& pos,
		const DirectX::XMFLOAT3& target,
		const DirectX::XMFLOAT3& up);

	// get view/proj matrices
	const DirectX::XMMATRIX& View()                                      const { return view_; }
	const DirectX::XMMATRIX& InverseView()                               const { return invView_; }
	const DirectX::XMMATRIX& Proj()                                      const { return proj_; }
	DirectX::XMMATRIX ViewProj()                                         const { return DirectX::XMMatrixMultiply(view_, proj_); }

	// strafe/walk the camera by distance d
	void Strafe(const float d);
	void Walk(const float d);
	void MoveUp(const float d);

	// set/get movement speed
	void SetIsRunning(const bool state)                                        { isRunning_ = state;}
	void SetWalkSpeed(const float speed)                                       { if (speed > 0) walkSpeed_ = speed; }
	void SetRunSpeed(const float speed)                                        { if (speed > 0) runSpeed_  = speed; }
	float GetSpeed()                                                     const { return (isRunning_) ? runSpeed_ : walkSpeed_; }
	float GetWalkSpeed()                                                 const { return walkSpeed_; }
	float GetRunSpeed()                                                  const { return runSpeed_; }

	// rotate the camera
	void Pitch(const float angle);
	void RotateY(const float angle);

	// after modifying camera position/orientation, call
	// to rebuild the view matrix once per frame
	void UpdateViewMatrix();


private:
	// cache view/inverse_view/proj matrices
	DirectX::XMMATRIX view_    = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX invView_ = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX proj_    = DirectX::XMMatrixIdentity();

	// camera coordinate system with coordinates relative to world space
	DirectX::XMVECTOR pos_  { 0,0,0 };        // view space origin
	DirectX::XMVECTOR right_{ 1,0,0 };        // view space x-axis
	DirectX::XMVECTOR up_   { 0,1,0 };        // view space y-axis
	DirectX::XMVECTOR look_ { 0,0,1 };        // view space z-axis
	DirectX::XMVECTOR lookAtPoint_{ 0,1,0 };  // world space look_at point (is used when we concentrate camera on some point)

	// cache frustum properties
	float nearZ_         = 0.0f;
	float farZ_          = 0.0f;
	float aspect_        = 0.0f;
	float fovY_          = 0.0f;
	float nearWndHeight_ = 0.0f;
	float farWndHeight_  = 0.0f;

	float walkSpeed_     = 10.0f;
	float runSpeed_      = 20.0f;

	// defines if camera is fixed at some particular look_at point
	bool isFixedLook_    = false;

	bool isFreeCamera_   = true;
	bool isRunning_      = false;
};
