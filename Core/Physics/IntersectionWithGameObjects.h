////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     IntersectionWithModels.h
//
// Description:  contains functional for computing intersection between a direction vector
//               and a bounding box;
//               
//               Example of using: in most 3D applications the user will need to click
//               on the screen with the mouse to select or interact with one of the 3D
//               objects in the scene. This process is usually referred to as selection
//               or picking. 
//               The process of picking involved translating a 2D mouse coordinate position
//               into a vector that is in world space. That vector is then used for 
//               intersection checks with all the visible 3D objects. Once the 3D object
//               is determinated the test can be further refined to determine exactly
//               which polygon was selected on that 3D object
//
// Created:      01.10.23
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////
// INCLUDES
//////////////////////////////////
#include "../Model/GameObject.h"


//////////////////////////////////
// DEFINITIONS
//////////////////////////////////

// values which define line segments intersection
#define PARAM_LINE_NO_INTERSECT 0
#define PARAM_LINE_INTERSECT_IN_SEGMENT 1
#define PARAM_LINE_INTERSECT_OUT_SEGMENT 2
#define PARAM_LINE_INTERSECT_EVERYWHERE 3


#define EPSILON_E5 (float)(1E-5)     // anything that avoids division overflow




//////////////////////////////////
// Class: IntersectionWithModels
//////////////////////////////////
class IntersectionWithGameObjects
{

#if 0
public:
	IntersectionWithGameObjects();
	~IntersectionWithGameObjects();

	// functions for the picking: 
	GameObject*  TestIntersectionWithGameObject(const int mouseX, const int mouseY,
		const POINT & windowDimensions,
		const std::map<std::string, GameObject*> & modelsList,
		const DirectX::XMMATRIX & worldMatrix,             // global matrix of the world not of a model
		const DirectX::XMVECTOR & cameraPosVec,
		const DirectX::XMMATRIX & cameraViewMatrix,
		const DirectX::XMMATRIX & cameraProjMatrix);

	bool RaySphereIntersect(const DirectX::XMVECTOR & rayOrigin,
		const DirectX::XMVECTOR & rayDirection,
		const float radius);

	int Intersect3D_RayTriangle(const DirectX::XMVECTOR & ray,
		const DirectX::XMVECTOR & rayDirection,
		const DirectX::XMVECTOR & v0,   // vertex 0 of triangle
		const DirectX::XMVECTOR & v1,   // vertex 1 of triangle
		const DirectX::XMVECTOR & v2,   // vertex 2 of triangle
		DirectX::XMFLOAT3 & iPoint);    // intersection point

	bool RayTriangleIntersect(const DirectX::XMVECTOR & rayOrigin,
		const DirectX::XMVECTOR & rayDirection,
		const DirectX::XMVECTOR & vertice0,
		const DirectX::XMVECTOR & vertice1,
		const DirectX::XMVECTOR & vertice2);

	const DirectX::XMFLOAT3 & GetIntersectionPoint() const;

private:
	bool isVectorZero(const DirectX::XMVECTOR & vector) const;

private:
	DirectX::XMFLOAT3 intersectionPoint_;
#endif
};
