// *********************************************************************************
// Filename:    Vertex.cpp
// Description: contains constructors for the Vertex3D type;
//              implements some functional for the InputLayouts class;
// 
// Created:     13.04.24
// *********************************************************************************
#include "Vertex.h"
#include <xutility>


namespace Core
{

Vertex3D::Vertex3D() :
	position(0.0f, 0.0f, 0.0f),
	texture(0.0f, 0.0f),
	normal(0.0f, 0.0f, 0.0f),
	tangent(0.0f, 0.0f, 0.0f)
{
	// default constructor
}

///////////////////////////////////////////////////////////

Vertex3D::Vertex3D(const Vertex3D& rhs)	:
	position(rhs.position),
	texture(rhs.texture),
	normal(rhs.normal),
	tangent(rhs.tangent)
{
	// copy constructor
}

///////////////////////////////////////////////////////////

Vertex3D& Vertex3D::operator=(const Vertex3D& rhs)
{
	// copy assignment
	if (this != &rhs)
		std::construct_at(this, rhs);

	return *this;
}

///////////////////////////////////////////////////////////

Vertex3D::Vertex3D(Vertex3D&& rhs) noexcept :
	position(rhs.position),
	texture(rhs.texture),
	normal(rhs.normal),
	tangent(rhs.tangent)
{
	// move constructor
}

///////////////////////////////////////////////////////////

Vertex3D& Vertex3D::operator=(Vertex3D&& rhs) noexcept
{
	// move assignment
	if (this != &rhs)
		std::construct_at(this, std::move(rhs));

	return *this;
}

///////////////////////////////////////////////////////////

Vertex3D::Vertex3D(
	const float posX, const float posY, const float posZ,
	const float texX, const float texY,
	const float nx, const float ny, const float nz,              // normal vec
	const float tx, const float ty, const float tz)              // tangent
	:
	position { posX, posY, posZ },
	texture { texX, texY },
	normal { nx, ny, nz},
	tangent { tx, ty, tz }
{
	// a constructor with raw input params
}

///////////////////////////////////////////////////////////

Vertex3D::Vertex3D(
	const DirectX::XMFLOAT3& pos,
	const DirectX::XMFLOAT2& tex,
	const DirectX::XMFLOAT3& nor,
	const DirectX::XMFLOAT3& tang)
	:
	position(pos),
	texture(tex),
	normal(nor),
	tangent(tang)
{
	// a constructor with XM-type input params
}

Vertex3D::Vertex3D(
	DirectX::XMFLOAT3&& pos,
	DirectX::XMFLOAT2&& tex,
	DirectX::XMFLOAT3&& nor,
	DirectX::XMFLOAT3&& tang)
	:
	position(pos),
	texture(tex),
	normal(nor),
	tangent(tang)
{
}


} // namespace Core
