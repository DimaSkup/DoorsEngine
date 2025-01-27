/////////////////////////////////////////////////////////////////////
// Filename:     BasicCamera.h
// Description:  BasicCamera tells the DirectX from where we are 
//               looking at the scene. Initializes the view matrix
//               which we use to render image
// Created:      07.04.22
// Revising:     20.12.22
/////////////////////////////////////////////////////////////////////
#pragma once

#include <DirectXMath.h>


//////////////////////////////////
// Class name: BasicCamera
//////////////////////////////////
class BasicCamera
{
public:
	struct CameraInitParams
	{
		CameraInitParams(
			const int wndWidth,
			const int wndHeight,
			const float nearZ,
			const float farZ,
			const float fovDegrees,
			const float speed,
			const float sensitivity) 
			:
			wndWidth_(wndWidth),
			wndHeight_(wndHeight),
			nearZ_(nearZ),
			farZ_(farZ),
			fovDegrees_(fovDegrees),
			speed_(speed),
			sensitivity_(sensitivity),
			aspectRatio_((float)wndWidth / (float)wndHeight) 
		{
		}

		int wndWidth_ = 800;
		int wndHeight_ = 600;
		float nearZ_ = 1;                       // near Z-coordinate of the frustum
		float farZ_  = 100;                     // far Z-coordinate of the frustum
		float fovDegrees_ = 90;                 // field of view
		float speed_ = 1;                       // camera movement speed
		float sensitivity_ = 1;                 // camera rotation speed
		float aspectRatio_ = 1.7777f;
	};

private:

	struct DEFAULT_VECTORS
	{
		const DirectX::XMVECTOR DEFAULT_FORWARD_VECTOR_  = { 0, 0, 1, 0 };
		const DirectX::XMVECTOR DEFAULT_UP_VECTOR_       = { 0, 1, 0, 0 };
		const DirectX::XMVECTOR DEFAULT_BACKWARD_VECTOR_ = { 0, 0, -1, 0 };
		const DirectX::XMVECTOR DEFAULT_LEFT_VECTOR_     = { -1, 0, 0, 0 };
		const DirectX::XMVECTOR DEFAULT_RIGHT_VECTOR_    = { 1, 0, 0, 0 };
	};

public:
	BasicCamera();
	~BasicCamera();

	void Initialize(const CameraInitParams& params);

	void UpdateViewMatrix();

	void SetProjectionValues(
		const float fov,        // field of view (in radians)
		const float aspectRatio,
		const float nearZ, 
		const float farZ);

	DirectX::XMFLOAT3 GetRotationFloat3InDegrees();

	void SetRotationInDeg(const DirectX::XMVECTOR& newAngle);
	void AdjustRotationInDeg(const DirectX::XMVECTOR& angle);

	// functions for handling planar reflections
	void UpdateReflectionViewMatrix(
		const DirectX::XMFLOAT3& reflectionPlanePos, 
		const DirectX::XMFLOAT3& relfectionPlaneRotation);

	// memory allocation
	void* operator new(size_t i);
	void operator delete(void* p);


public:  // INLINE SETTERS METHODS

	inline void SetPosition(const float x, const float y, const float z)  { posVector_ = { x, y, z, 1.0f }; }
	inline void SetPosition(const DirectX::XMFLOAT3& pos)                 { posVector_ = { pos.x, pos.y, pos.z, 1.0f }; }
	inline void SetRotationInRad   (const DirectX::XMVECTOR& newAngle)    { rotVector_ = newAngle; }
	inline void AdjustPosition     (const DirectX::XMVECTOR& translation) { posVector_ = DirectX::XMVectorAdd(posVector_, translation); }
	inline void AdjustRotationInRad(const DirectX::XMVECTOR& angle)       { rotVector_ = DirectX::XMVectorAdd(rotVector_, angle); }
	inline void SetStrideByY       (const float newStrideByY)             { strideByY_ = newStrideByY; }

public:  // INLINE GETTERS METHODS

	inline const DirectX::XMMATRIX& GetViewMatrix()           const { return view_; }
	inline const DirectX::XMMATRIX& GetInverseViewMatrix()    const { return invView_; }
	inline const DirectX::XMMATRIX& GetProjectionMatrix()     const { return projection_; }
	inline const float*             GetViewMatrixRawData()    const { return view_.r->m128_f32; }
	inline const float*             GetProjMatrixRawData()    const { return projection_.r->m128_f32;}

	inline void     GetPositionFloat3(DirectX::XMFLOAT3& pos) const { XMStoreFloat3(&pos, posVector_); }
	inline DirectX::XMFLOAT3 GetPositionFloat3()              const { DirectX::XMFLOAT3 pos; XMStoreFloat3(&pos, posVector_); return pos; }

	inline const DirectX::XMVECTOR& GetPosition()             const { return posVector_; }// DirectX::XMVectorAdd(posVector_, { 0.0f, strideByY_, 0.0f }); }
	inline const DirectX::XMVECTOR& GetRotation()             const { return rotVector_; }
	inline const DirectX::XMVECTOR& GetLookAt()               const { return vecLookAt_; }
	//inline const float GetCameraNear()                        const { return cameraNear_; }
	//inline const float GetCameraDepth()                       const { return cameraDepth_; }
	inline const float GetFovInRad()                          const { return fovInRad_; }

	inline void SetIsRunning(bool state) { isRunning_ = state; }

	// get directions vectors
	inline const DirectX::XMVECTOR& GetDirectionVector()      const { return vecCameraDir_; }
	inline const DirectX::XMVECTOR& GetForwardVector()        const { return vecForward_; }
	inline const DirectX::XMVECTOR& GetRightVector()          const { return vecRight_; }
	inline const DirectX::XMVECTOR& GetBackwardVector()       const { return vecBackward_; }
	inline const DirectX::XMVECTOR& GetLeftVector()           const { return vecLeft_; }

	inline const DirectX::XMMATRIX& GetReflectionViewMatrix() const { return reflectionView_; }
	inline void GetReflectionViewMatrix(DirectX::XMMATRIX& reflectionViewMatrix) { reflectionViewMatrix = reflectionView_; }
	

protected:
	//DirectX::XMFLOAT3 lookAtPoint_;       // the camera's look at point
	//DirectX::XMFLOAT3 up_;                // the camera's up direction
	DirectX::XMMATRIX view_;                // the current view matrix
	DirectX::XMMATRIX invView_;             // current inverse view matrix
	DirectX::XMMATRIX projection_;          // the current projection matrix
	DirectX::XMMATRIX reflectionView_;

	DirectX::XMVECTOR posVector_;           // the camera position (VECTOR)
	DirectX::XMVECTOR rotVector_;           // the camera rotation (VECTOR)

	DirectX::XMVECTOR vecForward_;          // the current forward direction of the game obj
	DirectX::XMVECTOR vecBackward_;
	DirectX::XMVECTOR vecLeft_;
	DirectX::XMVECTOR vecRight_;
	DirectX::XMVECTOR vecLookAt_;
	DirectX::XMVECTOR vecCameraDir_;        // normalize(lookAt - pos)
	

	const DEFAULT_VECTORS defaultVectors_;

	//float pitch_ = 0.0f;         // the current value of camera's pitch
	//float yaw_ = 0.0f;           // the current value of camera's yaw
	//float roll_ = 0.0f;	         // the current value of camera's roll

	// a camera movement speed in different states
	bool isRunning_      = false;
	float walkSpeed_     = 0.02f;
	float runSpeed_      = 0.04f;       // when we press SHIFT (or something else for running)
	
	float rotationSpeed_ = 0.01f;
	float strideByY_     = 0.0f; 

	//float cameraNear_    = 0.0f;        // camera near plane (nearZ / screenNear)
	//float cameraDepth_   = 0.0f;        // camera far plane (farZ / screenDepth)
	float fovInRad_      = 0.0f;        // field of view in radians
};