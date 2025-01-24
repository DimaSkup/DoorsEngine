////////////////////////////////////////////////////////////////////
// Filename:     frustumclass.h
// Description:  this class incapsulates calculations for the six 
//               planes_ of the view frustum; and functions for 
//               checking if either a point, cube, spehere, or rectangle
//               are inside the viewing frustum or not.
//
// Revising:     13.08.22
////////////////////////////////////////////////////////////////////
#pragma once


#include <DirectXMath.h>
#include <DirectXCollision.h>

class FrustumClass
{
public:
	FrustumClass(void);
	~FrustumClass(void);

	void Initialize(const float screenDepth);

	void ConstructFrustum(
		const DirectX::XMMATRIX & projectionMatrix, 
		const DirectX::XMMATRIX & viewMatrix);

	bool CheckPoint(const DirectX::XMVECTOR & point);
	bool CheckCube(const DirectX::XMFLOAT3 & centerPos, const float radius);
	bool CheckSphere(const DirectX::XMFLOAT3 & centerPos, const float radius = 1.0f);
	bool CheckRectangle(const float, const float, const float, const float, const float, const float);
	bool CheckRectangle2(const float, const float, const float, const float, const float, const float);
	bool CheckRectangle22(const DirectX::XMVECTOR & minDimensions, const DirectX::XMVECTOR & maxDimensions);

	bool CheckRectangle3(const DirectX::XMFLOAT3 & minDimensions, const DirectX::XMFLOAT3 & maxDimensions);
	bool IsPointInsideFrustum(const DirectX::XMVECTOR & vector);

	// memory allocation (is necessary because of the XM-structures in this class)
	void* operator new(size_t i);
	void operator delete(void* ptr);

private:  // restrict a copying of this class instance
	FrustumClass(const FrustumClass & obj);
	FrustumClass & operator=(const FrustumClass & obj);

private:
	float planeDotCoord(const DirectX::XMVECTOR & plane, const DirectX::XMVECTOR & vector);

private:
	float screenDepth_ = 0.0f;
	DirectX::XMVECTOR planes_[6];
};
