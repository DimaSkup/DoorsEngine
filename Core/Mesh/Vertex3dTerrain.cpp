#include "Vertex3dTerrain.h"
#include <xutility>

namespace Core
{

Vertex3dTerrain::Vertex3dTerrain() :
	position(0.0f, 0.0f, 0.0f),
	texture(0.0f, 0.0f),
	normal(0.0f, 0.0f, 0.0f),
	tangent(0.0f, 0.0f, 0.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f)  // a default color of a vertex is pink
{
	// default constructor
}

///////////////////////////////////////////////////////////

Vertex3dTerrain::Vertex3dTerrain(const Vertex3dTerrain& rhs)	:
	position(rhs.position),
	texture(rhs.texture),
	normal(rhs.normal),
	tangent(rhs.tangent),
	color(rhs.color)
{
	// copy constructor
}

///////////////////////////////////////////////////////////

Vertex3dTerrain& Vertex3dTerrain::operator=(const Vertex3dTerrain& rhs)
{
	// copy assignment
	if (this != &rhs)
		std::construct_at(this, rhs);

	return *this;
}

///////////////////////////////////////////////////////////

Vertex3dTerrain::Vertex3dTerrain(Vertex3dTerrain&& rhs) noexcept :
	position(rhs.position),
	texture(rhs.texture),
	normal(rhs.normal),
	tangent(rhs.tangent),
	color(rhs.color)
{
	// move constructor
}

///////////////////////////////////////////////////////////

Vertex3dTerrain& Vertex3dTerrain::operator=(Vertex3dTerrain&& rhs) noexcept
{
	// move assignment
	if (this != &rhs)
		std::construct_at(this, std::move(rhs));

	return *this;
}

///////////////////////////////////////////////////////////

Vertex3dTerrain::Vertex3dTerrain(
	const float posX, const float posY, const float posZ,
	const float texX, const float texY,
	const float nx, const float ny, const float nz,              // normal vec
	const float tx, const float ty, const float tz,              // tangent
	const float r, const float g, const float b, const float a)  // default color { 1, 1, 1, 1 }
	:
	position { posX, posY, posZ },
	texture { texX, texY },
	normal { nx, ny, nz},
	tangent { tx, ty, tz },
	color { r, g, b, a }
{
	// a constructor with raw input params
}

///////////////////////////////////////////////////////////

Vertex3dTerrain::Vertex3dTerrain(
	const DirectX::XMFLOAT3& pos,
	const DirectX::XMFLOAT2& tex,
	const DirectX::XMFLOAT3& nor,
	const DirectX::XMFLOAT3& tang,
	const DirectX::PackedVector::XMCOLOR& col)
	:
	position(pos),
	texture(tex),
	normal(nor),
	tangent(tang),
	color(col)
{
	// a constructor with XM-type input params
}

Vertex3dTerrain::Vertex3dTerrain(
	DirectX::XMFLOAT3&& pos,
	DirectX::XMFLOAT2&& tex,
	DirectX::XMFLOAT3&& nor,
	DirectX::XMFLOAT3&& tang,
	DirectX::PackedVector::XMCOLOR&& col)    // ARGB
	:
	position(pos),
	texture(tex),
	normal(nor),
	tangent(tang),
	color(col)
{
}


} // namespace Core
