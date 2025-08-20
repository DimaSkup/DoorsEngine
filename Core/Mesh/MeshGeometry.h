// ********************************************************************************
// Filename:      MeshGeometry.h
// Description:   a low-level class that encapsulates the vertex and index buffers,
//                as well as subsets (meshes) data
// 
// Created:       24.10.24
// ********************************************************************************
#pragma once

#include <Types.h>
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"


namespace Core
{

class MeshGeometry
{
public:
    struct Subset
    {
        Subset() {}

        uint32_t   vertexStart = 0;                         // start pos of vertex in the common buffer
        uint32_t   vertexCount = 0;                         // how many vertices this subset has
        uint32_t   indexStart = 0;                          // start pos of index in the common buffer
        uint32_t   indexCount = 0;                          // how many indices this subset has
        char       name[SUBSET_NAME_LENGTH_LIMIT]{ '\0' };  // each subset must have its own name
        MaterialID materialId = INVALID_MATERIAL_ID;        // an ID to the related material (multiple meshes/subset of the model can have the same material so we just can use the same ID)
        uint16_t   id = -1;                                 // subset ID
    };


public:
    MeshGeometry();
    ~MeshGeometry();

    MeshGeometry(const MeshGeometry& rhs) = delete;
    MeshGeometry& operator=(const MeshGeometry& rhs) = delete;

    // move constructor/assignment
    MeshGeometry(MeshGeometry&& rhs) noexcept;
    MeshGeometry& operator=(MeshGeometry&& rhs) noexcept;

    void Shutdown();

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

    void InitVertexBuffer(ID3D11Device* pDevice, const Vertex3D* vertices, const int count);
    void InitIndexBuffer (ID3D11Device* pDevice, const UINT* indices, const int count);

    void SetSubsetName(const SubsetID subsetID, const char* name);

    void SetMaterialForSubset(const SubsetID subsetID, const MaterialID matID);
    void SetMaterialsForSubsets(const SubsetID* subsetsIDs, const MaterialID* materialsIDs, const size count);


private:
    bool CheckInputParamsForMaterialsSetting(
        const SubsetID* subsetsIDs,
        const MaterialID* materialsIDs,
        const size count);

public:
    VertexBuffer<Vertex3D> vb_;
    IndexBuffer<UINT>      ib_;
    MeshGeometry::Subset*  subsets_ = nullptr;       // data about each mesh of model
    uint16_t               vertexStride_ = 0;
    uint16_t               numSubsets_ = 0;
};

}
