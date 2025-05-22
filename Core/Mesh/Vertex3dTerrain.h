// =================================================================================
// Filename:      Vertex3dTerrain.h 
// Description:   a structure type for 3D vertices of terrain
//
// Created:       20.05.2025 by DimaSkup
// =================================================================================
#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

namespace Core
{

class Vertex3dTerrain
{
public:
    // default constructor
    Vertex3dTerrain();

    Vertex3dTerrain(Vertex3dTerrain&& rhs) noexcept;
    Vertex3dTerrain& operator=(Vertex3dTerrain&& rhs) noexcept;

    Vertex3dTerrain(const Vertex3dTerrain& rhs);
    Vertex3dTerrain& operator=(const Vertex3dTerrain& rhs);


    // a constructor with raw input params
    Vertex3dTerrain(
        const float posX,
        const float posY,
        const float posZ,
        const float texX = 0,
        const float texY = 0,
        const float normalX = 0,
        const float normalY = 0,
        const float normalZ = 0,
        const float tangentX = 0,
        const float tangentY = 0,
        const float tangentZ = 0,
        const float red = 1,
        const float green = 1,
        const float blue = 1,
        const float alpha = 1);


    // a constructor with XM-type input params
    Vertex3dTerrain(
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT2& tex = { 0,0 },
        const DirectX::XMFLOAT3& nor = { 0,0,0 },
        const DirectX::XMFLOAT3& tang = { 0,0,0 },
        const DirectX::PackedVector::XMCOLOR& col = { 1,0,0,1 });    // ARGB

    Vertex3dTerrain(
        DirectX::XMFLOAT3&& pos,
        DirectX::XMFLOAT2&& tex,
        DirectX::XMFLOAT3&& nor,
        DirectX::XMFLOAT3&& tang = { 0,0,0 },
        DirectX::PackedVector::XMCOLOR&& col = { 1,0,0,1 });    // ARGB

    // ---------------------------------------------------- //

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texture;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT3 tangent;
    DirectX::PackedVector::XMCOLOR color;   // 32-bit ARGB packed color
};

} // namespace
