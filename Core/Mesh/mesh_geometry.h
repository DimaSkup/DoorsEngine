// ********************************************************************************
// Filename:      MeshGeometry.h
// Description:   a low-level class that encapsulates the vertex and index buffers,
//                as well as subsets (meshes) data for our models
// 
// Created:       24.10.24
// ********************************************************************************
#pragma once

#include <types.h>
#include "vertex.h"
#include "vertex_buffer.h"
#include "index_buffer.h"


namespace Core
{

// single mesh data
struct Subset
{
    Subset() {}

    uint32     vertexStart = 0;                         // start pos of vertex in the common buffer
    uint32     vertexCount = 0;                         // how many vertices this subset has
    uint32     indexStart = 0;                          // start pos of index in the common buffer
    uint32     indexCount = 0;                          // how many indices this subset has
    char       name[MAX_LEN_MESH_NAME]{ '\0' };         // each subset must have its own name
    MaterialID materialId = INVALID_MATERIAL_ID;        // an ID to the related material (multiple meshes/subset of the model can have the same material so we just can use the same ID)
    uint16     id = -1;                                 // subset ID
};

//---------------------------------------------------------

class MeshGeometry
{
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
        const Vertex3D* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices,
        const MeshGeometry& mesh);

    bool AllocateSubsets(const int numSubsets);
    void SetSubsets     (const Subset* subsets, const int numSubsets);

    void InitVertexBuffer(const Vertex3D* vertices, const int count);
    void InitIndexBuffer (const UINT* indices, const int count);

    void SetSubsetName         (const SubsetID meshId, const char* name);
    void SetSubsetLSpaceMatrix (const SubsetID meshId, const DirectX::XMMATRIX& m);

    void SetMaterialForSubset  (const SubsetID meshId, const MaterialID matID);
    void SetMaterialsForSubsets(const SubsetID* meshesIds, const MaterialID* materialsIds, const size count);

private:
    bool CheckInputParamsForMaterialsSetting(
        const SubsetID* subsetsIds,
        const MaterialID* materialsIds,
        const size count);

public:
    VertexBuffer<Vertex3D> vb_;
    IndexBuffer<UINT>      ib_;
    Subset*                subsets_ = nullptr;       // data about each mesh of model
    DirectX::XMMATRIX*     localSpaceMatrices_ = nullptr;
    uint16                 vertexStride_ = 0;
    uint16                 numSubsets_ = 0;
};

}
