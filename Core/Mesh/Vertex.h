// =================================================================================
// Filename:    Vertex.h
// Description: contains vertex structures are used in the engine;
//              also defines the input layout descriptions, and creates a single 
//              instance of each input layout and expose it globally by storing it 
//              as public static variable in an InputLayouts class;
// 
// =================================================================================
#pragma once

#include <DirectXMath.h>


namespace Core
{

struct VertexFont
{
    //
    // a structure type for rendering of 2D font
    // (the VertexFont structure must match both in the FontClass and TextStore)
    //

    VertexFont() : position(0, 0), texture(0, 0) {}

    VertexFont(const DirectX::XMFLOAT2& pos, DirectX::XMFLOAT2& tex)
        : position(pos), texture(tex) {}

    VertexFont(DirectX::XMFLOAT2&& pos, DirectX::XMFLOAT2&& tex)
        : position(pos), texture(tex) {}

    DirectX::XMFLOAT2 position;
    DirectX::XMFLOAT2 texture;
};


// =================================================================================

struct BillboardSprite
{
    DirectX::XMFLOAT4 color = { 1,0,0,1 };   // w-component contains translucency value
    DirectX::XMFLOAT3 pos   = { 0,0,0 };
    DirectX::XMFLOAT2 uv0   = { 0,0 };
    DirectX::XMFLOAT2 uv1   = { 1,1 };
    DirectX::XMFLOAT2 size  = { 1,1 };
};

// =================================================================================

struct VertexGrass
{
    DirectX::XMFLOAT3 pos       = {0,0,0};
    DirectX::XMFLOAT2 size;
};

// =================================================================================

struct Vertex3DPos
{
    // a 3D vertex which contains only position data

    Vertex3DPos() : position({ 0,0,0 }) {}
    Vertex3DPos(const DirectX::XMFLOAT3& pos) : position(pos) {}

    DirectX::XMFLOAT3 position;
};

// =================================================================================

struct VertexPosColor
{
    VertexPosColor() {}

    DirectX::XMFLOAT3 position;
    uint32_t          color;
};

// =================================================================================

struct VertexPosTex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 tex;
};

// =================================================================================

class Vertex3D
{
    //
    // a Vertex3D structure type for 3D vertices
    //

public:
    // default constructor
    Vertex3D();

    Vertex3D(Vertex3D&& rhs) noexcept;
    Vertex3D& operator=(Vertex3D&& rhs) noexcept;

    Vertex3D(const Vertex3D& rhs);
    Vertex3D& operator=(const Vertex3D& rhs);


    // a constructor with raw input params
    Vertex3D(
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
        const float tangHandedness = 1);


    // a constructor with XM-type input params
    Vertex3D(
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT2& tex = {0,0},
        const DirectX::XMFLOAT3& nor = {0,0,0},
        const DirectX::XMFLOAT4& tang = {0,0,0,1});

    Vertex3D(
        DirectX::XMFLOAT3&& pos,
        DirectX::XMFLOAT2&& tex,
        DirectX::XMFLOAT3&& nor,
        DirectX::XMFLOAT4&& tang = { 0,0,0,1 });

public:
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texture;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT4 tangent;
};

} // namespace Core
