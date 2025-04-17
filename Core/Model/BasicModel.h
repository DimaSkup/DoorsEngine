// =================================================================================
// Filename:      BasicModel.h
// Description:   a middle-level model class; this contains a MeshGeometry instance
//                specifying the geometry of the model as well as the materials
//                and textures needed to draw the mesh.
//                Additionally, it keeps a system memory 
//                copy of the mesh. System memory copies are needed for when we
//                need to read the mesh data such as for computing bounding volumes,
//                picking, or collision detection
// 
// Created:       30.10.24
// =================================================================================
#pragma once



#include "../Mesh/Vertex.h"
#include "../Mesh/Material.h"
#include "../Mesh/MeshGeometry.h"

#include "../Texture/TextureTypes.h"

#include <d3d11.h>
#include <DirectXCollision.h>


enum eModelType : uint8_t
{
    Invalid,
    Cube,
    Cylinder,
    Plane,
    Pyramid,
    Skull,
    Sphere,
    GeoSphere,
    Imported,    // if we load a model from the file
    Terrain,
    LineBox,     // is used to visualise bounding boxes (AABB)
    Sky,         // this model is used to render the sky
};

///////////////////////////////////////////////////////////

namespace Core
{

class BasicModel
{
public:
    BasicModel();
    ~BasicModel();

    // move constructor/assignment
    BasicModel(BasicModel&& rhs) noexcept;
    BasicModel& operator=(BasicModel&& rhs) noexcept;

    // restrict shallow copying
    BasicModel(const BasicModel&) = delete;
    BasicModel& operator=(const BasicModel&) = delete;

    // deep copy
    void Copy(ID3D11Device* pDevice, const BasicModel& rhs);

    void InitializeBuffers(ID3D11Device* pDevice);
    void Shutdown();
    void ClearMemory();

    void CopyVertices(const Vertex3D* vertices, const int numVertices);
    void CopyIndices(const UINT* indices, const int numIndices);

    // memory allocation
    void AllocateMemory();
    void AllocateMemory(const int numVertices, const int numIndices, const int numSubsets);

    void AllocateVertices(const int numVertices);
    void AllocateIndices(const int numIndices);


    // query API
    inline ModelID GetID()                              const { return id_; }
    inline const char* GetName()                        const { return name_; }
    inline const DirectX::BoundingBox& GetModelAABB()   const { return modelAABB_; }
    inline const DirectX::BoundingBox* GetSubsetsAABB() const { return subsetsAABB_; }
    inline MeshGeometry::Subset* GetSubsets()                 { return meshes_.subsets_; }

    inline int GetNumVertices()                         const { return numVertices_; }
    inline int GetNumIndices()                          const { return numIndices_; }
    inline int GetNumSubsets()                          const { return numSubsets_; }


    // update API
    void SetName(const char* newName);
   
    void SetMaterialForSubset(const SubsetID subsetID, const MaterialID materialID);
    void SetMaterialsForSubsets(
        const SubsetID* subsetIDs,
        const MaterialID* materialIDs,
        const size count);

    void SetModelAABB(const DirectX::BoundingBox& aabb);

    void SetSubsetAABB(const SubsetID subsetID, const DirectX::BoundingBox& aabb);
    void SetSubsetAABBs(
        const SubsetID* subsetsIDs,
        const DirectX::BoundingBox* AABBs,
        const int count);

    void ComputeModelAABB();
    void ComputeSubsetsAABB();

public:
    char                  name_[32] = "inv";
    ModelID               id_ = 0;
    uint32_t              numVertices_ = 0;
    uint32_t              numIndices_  = 0;
    uint16_t              numSubsets_ = 0;
    uint16_t              numBones_ = 0;
    uint16_t              numAnimClips_ = 0;                             // animation clips
    eModelType            type_ = eModelType::Invalid;
    
    MeshGeometry          meshes_;                     // contains all the meshes data
    DirectX::BoundingBox  modelAABB_;                  // AABB of the whole model
    DirectX::BoundingBox* subsetsAABB_ = nullptr;      // AABB of each subset (mesh)

    // keep CPU copies of the meshes data to read from
    Vertex3D*             vertices_ = nullptr;
    UINT*                 indices_ = nullptr;
};

} // namespace Core
