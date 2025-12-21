////////////////////////////////////////////////////////////////////
// Filename:     model_math.h
// Description:  constains functions for calculation model math
//               (normal, tangent, binormal, etc.)
//
// Created:      05.02.23
////////////////////////////////////////////////////////////////////
#pragma once

#include "../Mesh/Vertex.h"

namespace Core
{

class ModelMath
{
public:
    void CalcNormals(
        Vertex3D* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

    void CalcTangents(
        Vertex3D* vertices,
        const UINT* indices,
        const uint numVertices,
        const uint numIndices);
};

} // namespace Core

