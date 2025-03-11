// ********************************************************************************
// Filename:      MeshGeometry.h
// Description:   a low-level class that encapsulates the vertex and index buffers,
//                as well as subsets (meshes) data
// 
// Created:       24.10.24
// ********************************************************************************
#pragma once

//#include <CoreCommon/Types.h>

#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "MeshMaterial.h"

#include <cstdint>
#include <d3d11.h>
#include <DirectXCollision.h>

#define SUBSET_NAME_LENGTH_LIMIT 32

namespace Core
{

class MeshGeometry
{
public:
    struct Subset
    {
        Subset() {}

        uint32_t vertexStart_ = 0;     // start pos of vertex in the common buffer
        uint32_t vertexCount_ = 0;     // how many vertices this subset has
        uint32_t indexStart_ = 0;      // start pos of index in the common buffer
        uint32_t indexCount_ = 0;      // how many indices this subset has
        char name_[SUBSET_NAME_LENGTH_LIMIT]{ '\0' };      // each subset must have its own name
        uint16_t id_ = -1;
        bool alphaClip_ = false;  // apply alpha clipping to this subset or not
    };


public:
    MeshGeometry();
    ~MeshGeometry();

    MeshGeometry(const MeshGeometry& rhs) = delete;
    MeshGeometry& operator=(const MeshGeometry& rhs) = delete;

    // move constructor/assignment
    MeshGeometry(MeshGeometry&& rhs) noexcept;
    MeshGeometry& operator=(MeshGeometry&& rhs) noexcept;


    // deep copy
    void Copy(
        ID3D11Device* pDevice,
        const Vertex3D* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices,
        const MeshGeometry& mesh);

    void AllocateSubsets(const int numSubsets);
    void SetSubsets(const Subset* subsets, const int numSubsets);

    void InitVB(ID3D11Device* pDevice, const Vertex3D* pVertices, int count);
    void InitIB(ID3D11Device* pDevice, const UINT* pIndices, int count);

    void SetSubsetName(const uint8_t subsetID, const char* name);

public:
    VertexBuffer<Vertex3D> vb_;
    IndexBuffer<UINT>      ib_;
    MeshGeometry::Subset*  subsets_ = nullptr;       // data about each mesh of model
    uint16_t               vertexStride_ = 0;
    uint16_t               numSubsets_ = 0;
};

}
