////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     IntersectionWithGameObjects.cpp
//
// Description:  contains an implementation of functional for intersection with models
//               a.k.a picking;
//
// Created:      01.10.23
////////////////////////////////////////////////////////////////////////////////////////////
#include "IntersectionWithGameObjects.h"

#include <iomanip>       // we print into the console some debug data about intersection so we need to see it in a convenient view

using namespace DirectX;



#if 0
IntersectionWithGameObjects::IntersectionWithGameObjects()
{
}

IntersectionWithGameObjects::~IntersectionWithGameObjects()
{
}




////////////////////////////////////////////////////////////////////////////////////////////
//                                PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////

GameObject* IntersectionWithGameObjects::TestIntersectionWithGameObject(const int mouseX,
	const int mouseY,
	const POINT & windowDimensions,
	const std::map<std::string, GameObject*> & gameObjectsList,
	const DirectX::XMMATRIX & worldMatrix,             // global matrix of the world not of a model
	const DirectX::XMVECTOR & cameraPosVec,
	const DirectX::XMMATRIX & cameraViewMatrix,
	const DirectX::XMMATRIX & cameraProjMatrix)
{
	// there is general intersection check that
	// forms the vector for checking the intersection and then calls the specific type
	// of intersection check required. 
	// If an intersection occured with some model we return a pointer to this model;

	// DETAILED DESCRIPTION:
	/*

	This function takes as input the 2D mouse coordinates and then forms a vector in 3D
	space which it uses to then check for an intersection with the sphere (or some another
	shape). That vector is called the picking ray. The picking ray has the origin and
	a direction. With the 3D coordinate (origin) and 3D vector/normal (direction) we can
	create a line in 3D space and find out what it collides with.

	Usually we are very used to a vertex shader that takes a 3D point (vertice) and moves
	it from 3D space onto the 2D screen so it can be rendered as the pixel. Well now we are
	doing the exact opposite and moving a 2D point from the screen into 3D space. So what
	we need to do it just reverse our usual process. So where we would usually take a 3D point
	from world to view to projection to make a 2D point, we will now instead take 
	a 2D point and go from projection to view to world and turn it into a 3D point.

	To do the reverse process we first start by taking the mouse coordinates and moving
	them into the -1 to +1 range on both axis. When we have that we then divide by the
	screen aspect using the projection matrix. With that value we can then multiply it by
	the inverse view matrix (inverse because we are going in reverse direction) to get the
	direction vector in view space. We can set the origin of the vector in view space
	to just be the location of the camera.

	With the direction vector and origin in view space we can now complete the final process
	of moving it into 3D world space. To do so we first need to get the world matrix and
	translate it by the position of the sphere. With the updated world matrix we once again
	need to invert it (since the process is going in the opposite direction) and then we
	can multiply the origin and direction by the inverted world matrix. We also normalize
	the direction after the multiplication. This gives us the origin and direction of the
	vector in 3D world space so that we can do tests with other objects that are also
	in 3D world space.

	Now that we have the origin of the vector and the direction of the vector we can
	perform an interstection test. For instance it can be a ray-sphere intersection test,
	but you could perform any kind of intersection test that you have the vector in 3D
	world space

	*/
	

	DirectX::XMFLOAT3 gameObjPos{ 0.0f, 0.0f, 0.0f };  // a position of some game object on the scene
	DirectX::XMMATRIX inverseViewMatrix;
	DirectX::XMMATRIX inverseWorldMatrix;
	DirectX::XMMATRIX translateMatrix;
	DirectX::XMMATRIX tempWorldMatrix;                 // we translate the world matrix for each model separately

	DirectX::XMVECTOR direction;
	DirectX::XMVECTOR rayOrigin;
	DirectX::XMVECTOR rayDirection;
	DirectX::XMFLOAT4X4 fInvViewMatrix;

	//DirectX::XMMATRIX projMatrix{ pCamera->GetProjectionMatrix() };

	float pointX = 0.0f;
	float pointY = 0.0f;
	bool intersect = false;

	//////////////////////////////////////////////////////////

	// move the mouse cursor coordinates into the -1 to +1 range
	pointX = (2.0f * static_cast<float>(windowDimensions.x / 2.0f) / static_cast<float>(windowDimensions.x)) - 1.0f;
	pointY = ((2.0f * static_cast<float>(windowDimensions.y / 2.0f) / static_cast<float>(windowDimensions.y)) - 1.0f) * -1.0f;

	// adjust the points using the projection matrix to account for the aspect ration of the viewport;
	pointX = pointX / (DirectX::XMVectorGetX(cameraProjMatrix.r[0]));
	pointY = pointY / (DirectX::XMVectorGetY(cameraProjMatrix.r[1]));

	// get the inverse of the view matrix
	inverseViewMatrix = DirectX::XMMatrixInverse(nullptr, cameraViewMatrix);

	// convert the inverse of the view matrix into a 4x4 float type
	DirectX::XMStoreFloat4x4(&fInvViewMatrix, inverseViewMatrix);

	// calculate the direction of the picking ray in view space
	direction.m128_f32[0] = (pointX * fInvViewMatrix._11) + (pointY * fInvViewMatrix._21) + fInvViewMatrix._31;
	direction.m128_f32[1] = (pointX * fInvViewMatrix._12) + (pointY * fInvViewMatrix._22) + fInvViewMatrix._32;
	direction.m128_f32[2] = (pointX * fInvViewMatrix._13) + (pointY * fInvViewMatrix._23) + fInvViewMatrix._33;
	direction.m128_f32[3] = 1.0f;


	//////////////////////////////////////////////////////////

	// check intersection with each model on the scene (custom models, cubes, spheres, etc.)
	for (const auto & elem : gameObjectsList)
	{
	
		if (elem.first == "triangle(1)")
		{
			continue;
		}

		// translate the world matrix to the location of the model
		gameObjPos = elem.second->GetPosition();
		translateMatrix = DirectX::XMMatrixTranslation(gameObjPos.x, gameObjPos.y, gameObjPos.z);
		tempWorldMatrix = worldMatrix * translateMatrix;

		// now get the inverse of the translated world matrix
		inverseWorldMatrix = DirectX::XMMatrixInverse(nullptr, tempWorldMatrix);

		// now transform the ray origin and the ray direction from view space to world space
		rayOrigin = DirectX::XMVector3TransformCoord(cameraPosVec, inverseWorldMatrix);
		rayDirection = DirectX::XMVector3TransformNormal(direction, inverseWorldMatrix);
		
		// normalize the ray direction
		//rayDirection = DirectX::XMVector3Normalize(rayDirection);

		// now perform the ray-sphere intersection test;
		// if we have an intersection with some model we return a pointer to it
		if (RaySphereIntersect(rayOrigin, rayDirection, 1.0f))
		{
			return elem.second;
		}

	} // end for


	/////////////////////////////////////////////////

#if 0

	// get a triangle game obj
	GameObject* pTriangleGameObj = gameObjectsList.at("triangle(1)");
	assert(pTriangleGameObj != nullptr);

	// set rotation for the triangle
	pTriangleGameObj->GetData()->SetRotationInDegrees(0, 90, 0);

	// define an origin ray and a direction ray
	rayOrigin = DirectX::XMVector3TransformCoord(cameraPosVec, DirectX::XMMatrixInverse(nullptr, worldMatrix));
	rayDirection = DirectX::XMVector3TransformNormal(direction, DirectX::XMMatrixInverse(nullptr, worldMatrix));

	// translate each triangle's vertex to its position in the world
	const std::vector<Vertex3D> & verticesArr = pTriangleGameObj->GetData()->GetVertices();
	XMVECTOR vecPosV0 = XMLoadFloat3(&verticesArr[0].position);
	XMVECTOR vecPosV1 = XMLoadFloat3(&verticesArr[1].position);
	XMVECTOR vecPosV2 = XMLoadFloat3(&verticesArr[2].position);

	vecPosV0 = XMVector3TransformCoord(vecPosV0, pTriangleModel->GetModelDataObj()->GetWorldMatrix());
	vecPosV1 = XMVector3TransformCoord(vecPosV1, pTriangleModel->GetModelDataObj()->GetWorldMatrix());
	vecPosV2 = XMVector3TransformCoord(vecPosV2, pTriangleModel->GetModelDataObj()->GetWorldMatrix());



	// now perform the ray-triangle intersection test;
	bool isIntersectTriangle = RayTriangleIntersect(rayOrigin, rayDirection,
		vecPosV0,
		vecPosV1,
		vecPosV2);

	if (isIntersectTriangle)
	{
		Log::Print("triangle intersection");
		return pTriangleModel;
	}
	else
	{
		Log::Error("MISS");
	}

#endif

	return nullptr;
	
} // end TestIntersectionWithGameObjects

  ///////////////////////////////////////////////////////////

bool IntersectionWithGameObjects::RaySphereIntersect(const DirectX::XMVECTOR & rayOrigin,
	const DirectX::XMVECTOR & rayDirection,
	const float radius)
{
	// this function executes the ray-sphere intersection check; 
	// For other intersection tests such as ray-triangle,
	// ray-rectangle, and so forth you would add them here


	// this function performs the math of a basic ray-sphere intersection test

	float a = 0.0f;
	float b = 0.0f;
	float c = 0.0f;
	float discriminant = 0.0f;

	DirectX::XMFLOAT3 fRayOrigin;
	DirectX::XMFLOAT3 fRayDirection;

	DirectX::XMStoreFloat3(&fRayOrigin, rayOrigin);
	DirectX::XMStoreFloat3(&fRayDirection, rayDirection);


	// calculate the a, b and c coefficients
	a = (fRayDirection.x * fRayDirection.x) + (fRayDirection.y * fRayDirection.y) + (fRayDirection.z * fRayDirection.z);
	b = ((fRayDirection.x * fRayOrigin.x) + (fRayDirection.y * fRayOrigin.y) + (fRayDirection.z * fRayOrigin.z)) * 2.0f;
	c = (fRayOrigin.x * fRayOrigin.x) + (fRayOrigin.y * fRayOrigin.y) + (fRayOrigin.z * fRayOrigin.z) - (radius * radius);

	// find the discriminant
	discriminant = (b * b) - (4 * a * c);

	// if discriminant is negative the picking ray missed the sphere, otherwise it intersected the sphere
	return (discriminant > 0.0f);

} // end RaySphereIntersect

  ///////////////////////////////////////////////////////////

int IntersectionWithGameObjects::Intersect3D_RayTriangle(const XMVECTOR & rayOrigin,
	const XMVECTOR & rayDirection,  
	const XMVECTOR & v0,   // vertex 0 of triangle
	const XMVECTOR & v1,   // vertex 1 of triangle
	const XMVECTOR & v2,   // vertex 2 of triangle
	XMFLOAT3 & iPoint)     // intersection point
{
	// this function finds the 3D intersection of a ray with a triangle (v0, v1, v2);
	//    Input:   a XMVECTOR ray, and a triangle (v0, v1, v2)
	//    Output:  iPoint = intersection point (when it exists)
	//    Return:  -1 = triangle is degenerate (a segment or point)
	//              0 = disjoint (no intersect)
	//              1 = intersect in unique point iPoint
	//              3 = are in the same plane (ray coincide with plane)


	XMVECTOR u, v, n;                              // triangle vectors
	XMVECTOR w0, w;                           // ray vectors
	float r = 0, a = 0, b = 0;                     // params to calc ray-plane intersect


	// get triangle edge vectors and plane normal
	u = v1 - v0;  
	v = v2 - v0;
	n = XMVector3Normalize(XMVector3Cross(u, v));             // cross product

	if (isVectorZero(n))                           // triangle is degenerate
		return -1;                                 // do not deal with this case

	//w0 = rayOrigin - v0;
	w0 = v0 - rayOrigin;
	float distance = XMVectorGetX(XMVector3Dot(n, v0));
	a = -(XMVectorGetX(XMVector3Dot(n, w0)) - distance);
	b = XMVectorGetX(XMVector3Dot(n, rayDirection));

	if (fabs(b) <= EPSILON_E5)                     // ray is parallel to triangle plane
	{
		return (a == 0) ?
			PARAM_LINE_INTERSECT_EVERYWHERE :      // ray lies in triangle plane
			PARAM_LINE_NO_INTERSECT;               // ray disjoint from plane
	}

	// get intersect point of ray with triangle plane
	r = a / b;
	if (r < 0.0)                                   // ray goes away from triangle
	{
		int i = 0;
		i++;
		return PARAM_LINE_NO_INTERSECT;            // => no intersect
	}
	                                               // for a segment, also test if (t > 1.0) => no intersect

	// ---------------------------------------------------- //

	// is iPoint inside the triangle?
	float uu, uv, vv, wu, wv;
	float D_inv;                                   // inverted denominator (1.0 / denominator)
	uu = XMVectorGetX(XMVector3Dot(u, u));
	uv = XMVectorGetX(XMVector3Dot(u, v));
	vv = XMVectorGetX(XMVector3Dot(v, v));
	w = XMLoadFloat3(&iPoint) - v0;                // a vector from v0 to iPoint (intersection point)
	wu = XMVectorGetX(XMVector3Dot(w, u));
	wv = XMVectorGetX(XMVector3Dot(w, v));
	D_inv = 1.0f / (uv * uv - uu * vv);

	// get and test parametric coords
	float s = (uv * wv - vv * wu) * D_inv;
	if ((s < 0.0) || (s > 1.0))                        // iPoint is outside triangle
		return PARAM_LINE_NO_INTERSECT;
	float t = (uv * wu - uu * wv) * D_inv;
	if ((t < 0.0) || ((s + t) > 1.0))                  // iPoint is outside triangle
		return PARAM_LINE_NO_INTERSECT;

	// intersect point of ray and plane (equation: p = p0 + v*t)
	XMVECTOR Q = rayOrigin + (rayDirection * r);
	XMStoreFloat3(&iPoint, Q);

	return PARAM_LINE_INTERSECT_IN_SEGMENT;        // iPoint is inside triangle

} // end RayTriangleIntersect

///////////////////////////////////////////////////////////

bool IntersectionWithGameObjects::RayTriangleIntersect(const DirectX::XMVECTOR & rayOrigin,
	const DirectX::XMVECTOR & rayDirection,
	const DirectX::XMVECTOR & v0,
	const DirectX::XMVECTOR & v1,
	const DirectX::XMVECTOR & v2)
{
	DirectX::XMFLOAT3 inter_point;
	DirectX::XMVECTOR inter_vector;

	////////////////////////////////////////////////////////////////////////////

	// these vectors coincide with the plane
	DirectX::XMVECTOR vecV0V1 = v1 - v0;
	DirectX::XMVECTOR vecV0V2 = v2 - v0;
	DirectX::XMVECTOR vecV0_RayOrigin = rayOrigin - v0;

	DirectX::XMVECTOR planeNormal = DirectX::XMVector3Cross(vecV0V1, vecV0V2);

	// check if the ray origin is located in positive half-space relative to the plane;
	DirectX::XMVECTOR vecPlaneDotRayOrigin = DirectX::XMVector3Dot(vecV0_RayOrigin, planeNormal);

	// if we are in negative half space we can't see the triangle
	// so we can't intersect it as well;
	if (vecPlaneDotRayOrigin.m128_f32[0] < 0)
		return false;

	// we are in positive half-space so we check if the ray intersect the plane anywhere
	DirectX::XMVECTOR vecPlaneDotRayDir = DirectX::XMVector3Dot(rayDirection, planeNormal);

	// if the direction ray doesn't intersect the plane at all
	if (vecPlaneDotRayDir.m128_f32[0] > 0)
		return false;

	////////////////////////////////////////////////////////////////////////////

	// CUSTOM METHOD FOR FINDING AN INTERSECTION POINT

	// find an intersection point; first of all we need to find a t parameter:
	float t = -(planeNormal.m128_f32[0] * rayOrigin.m128_f32[0] +
		planeNormal.m128_f32[1] * rayOrigin.m128_f32[1] +
		planeNormal.m128_f32[2] * rayOrigin.m128_f32[2] -
		planeNormal.m128_f32[0] * v0.m128_f32[0] -
		planeNormal.m128_f32[1] * v0.m128_f32[1] -
		planeNormal.m128_f32[2] * v0.m128_f32[2]) / (vecPlaneDotRayDir.m128_f32[0]);

	// put the t parameter into an equation of the straight line and
	// find coordinates of an intersection point (x, y, z)
	inter_point.x = rayOrigin.m128_f32[0] + rayDirection.m128_f32[0] * t;
	inter_point.y = rayOrigin.m128_f32[1] + rayDirection.m128_f32[1] * t;
	inter_point.z = rayOrigin.m128_f32[2] + rayDirection.m128_f32[2] * t;

	intersectionPoint_ = inter_point;

	////////////////////////////////////////////////////////////////////////////

	// USING DIRECTX FUNCTIONS TO FIND A POINT OF INTERSECTION

	// check if the ray is parallel to the plane (which is made by three input vertices)
	DirectX::XMVECTOR plane = DirectX::XMPlaneFromPoints(v0, v1, v2);
	DirectX::XMVECTOR raySecondPoint{ rayOrigin.m128_f32[0] + rayDirection.m128_f32[0], rayOrigin.m128_f32[1] + rayDirection.m128_f32[1], rayOrigin.m128_f32[2] + rayDirection.m128_f32[2], 1.0f };
	inter_vector = DirectX::XMPlaneIntersectLine(plane, rayOrigin, raySecondPoint);

	
	
	
	////////////////////////////////////////////////////////////////////////////

	// DEBUG DATA

	std::stringstream debugMsg;
	
	debugMsg << std::setprecision(2) << std::fixed << "inter1: "
		<< inter_vector.m128_f32[0] << " "
		<< inter_vector.m128_f32[1] << " "
		<< inter_vector.m128_f32[2];
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());

	debugMsg << std::setprecision(2) << std::fixed << "v0: "
		<< v0.m128_f32[0] << " "
		<< v0.m128_f32[1] << " "
		<< v0.m128_f32[2];
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());

	debugMsg << std::setprecision(2) << std::fixed << "v1: "
		<< v1.m128_f32[0] << " "
		<< v1.m128_f32[1] << " "
		<< v1.m128_f32[2];
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());

	debugMsg << std::setprecision(2) << std::fixed << "v2: "
		<< v2.m128_f32[0] << " "
		<< v2.m128_f32[1] << " "
		<< v2.m128_f32[2];
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());

	debugMsg << std::setprecision(2) << std::fixed << "orig: "
		<< rayOrigin.m128_f32[0] << " "
		<< rayOrigin.m128_f32[1] << " "
		<< rayOrigin.m128_f32[2];
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());

	debugMsg << std::setprecision(2) << std::fixed << "dir: "
		<< rayDirection.m128_f32[0] << " "
		<< rayDirection.m128_f32[1] << " "
		<< rayDirection.m128_f32[2];
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());

	debugMsg << std::setprecision(2) << std::fixed << "p norm: "
		<< planeNormal.m128_f32[0] << " "
		<< planeNormal.m128_f32[1] << " "
		<< planeNormal.m128_f32[2];
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());
	
	debugMsg << std::setprecision(2) << "inter2: "
		<< inter_point.x << " "
		<< inter_point.y << " "
		<< inter_point.z;
	Log::Debug(debugMsg.str().c_str());
	debugMsg.str(std::string());

	return true;

} // RayTriangleIntersect

///////////////////////////////////////////////////////////

const DirectX::XMFLOAT3 & IntersectionWithGameObjects::GetIntersectionPoint() const
{
	return intersectionPoint_;

} // end GetIntersectionPoint








////////////////////////////////////////////////////////////////////////////////////////////
//
//                           PRIVATE FUNCTIONS (HELPERS)
// 
////////////////////////////////////////////////////////////////////////////////////////////


bool IntersectionWithGameObjects::isVectorZero(const DirectX::XMVECTOR & vector) const
{
	// defines if the input vector is a zero vector

	return ((vector.m128_f32[0] < EPSILON_E5) &&    // x < 0
		    (vector.m128_f32[1] < EPSILON_E5) &&    // y < 0
		    (vector.m128_f32[2] < EPSILON_E5));     // z < 0

} // end isVectorZero


#endif