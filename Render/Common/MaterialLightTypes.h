// =================================================================================
// Filename:      MaterialLightTypes.h
// Description:   contains declarations of Material and different light src types
// 
// Created:       30.08.24
// =================================================================================
#pragma once

#include <DirectXMath.h>

namespace Render
{

struct MaterialColors
{
    DirectX::XMFLOAT4 ambient  = { 1,1,1,1 };
    DirectX::XMFLOAT4 diffuse  = { 1,1,1,1 };
    DirectX::XMFLOAT4 specular = { 0,0,0,1 };             // w = specPower (specular power)
    DirectX::XMFLOAT4 reflect  = { 0,0,0,0 };

    MaterialColors() {}

    MaterialColors(
        const DirectX::XMFLOAT4& inAmbient,
        const DirectX::XMFLOAT4& inDiffuse,
        const DirectX::XMFLOAT4& inSpecular,
        const DirectX::XMFLOAT4& inReflect) :
        ambient(inAmbient),
        diffuse(inDiffuse),
        specular(inSpecular),
        reflect(inReflect) {}
};

///////////////////////////////////////////////////////////

struct DirLight
{
    DirLight() {}

    DirLight(
        DirectX::XMFLOAT4& ambient,
        DirectX::XMFLOAT4& diffuse,
        DirectX::XMFLOAT4& specular,
        DirectX::XMFLOAT3& direction)
        :
        ambient(ambient),
        diffuse(diffuse),
        specular(specular),
        direction(direction) {}


    DirectX::XMFLOAT4 ambient   = { 0,0,0,1 };
    DirectX::XMFLOAT4 diffuse   = { 0,0,0,1 };
    DirectX::XMFLOAT4 specular  = { 0,0,0,1 };
    DirectX::XMFLOAT3 direction = { 0,0,1 };
    float pad = 0;                             // pad the last float so we can array of light if we wanted
};

///////////////////////////////////////////////////////////

struct PointLight
{
    PointLight() {}

    DirectX::XMFLOAT4 ambient   = { 0,0,0,1 };
    DirectX::XMFLOAT4 diffuse   = { 0,0,0,1 };
    DirectX::XMFLOAT4 specular  = { 0,0,0,1 };

    // packed into 4D vector: (position, range)
    DirectX::XMFLOAT3 position = { 0,0,0 };
    float range = 1.0f;

    // packed into 4D vector: (1/att(A0,A1,A2), pad)
    DirectX::XMFLOAT3 att = { 1,1,1 };
    float pad = 0;                    // pad the last float so we can array of light if we wanted
};

///////////////////////////////////////////////////////////

struct SpotLight
{
    SpotLight() {}

    DirectX::XMFLOAT4 ambient   = { 0,0,0,1 };
    DirectX::XMFLOAT4 diffuse   = { 0,0,0,1 };
    DirectX::XMFLOAT4 specular  = { 0,0,0,1 };

    // packed into 4D vector: (position, range)
    DirectX::XMFLOAT3 position  = { 0,0,0 };
    float range = 1.0f;

    // packed into 4D vector: (direction, spot exponent)
    DirectX::XMFLOAT3 direction = { 0,0,1 };
    float spot = 1.0f;

    // packed into 4D vector: (1/att(A0,A1,A2), pad)
    DirectX::XMFLOAT3 att = { 1,1,1 };
    float pad = 0;                    // pad the last float so we can array of light if we wanted
};


} // namespace
