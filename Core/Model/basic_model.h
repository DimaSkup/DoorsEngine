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

#include "../Texture/enum_texture_types.h"

#include "../Mesh/vertex.h"
#include "../Mesh/material.h"
#include "../Mesh/mesh_geometry.h"

#include <d3d11.h>
#include <DirectXCollision.h>



enum eModelLodLevel : uint8
{
    // LOD_0,  // is the model by itself
    LOD_1,
    LOD_2,
    
    NUM_LOD_LEVELS = 2,
};

//---------------------------------------------------------

enum eModelType : uint8_t
{
    MODEL_TYPE_Invalid,
    MODEL_TYPE_Cube,
    MODEL_TYPE_Cylinder,
    MODEL_TYPE_Plane,
    MODEL_TYPE_Pyramid,
    MODEL_TYPE_Skull,
    MODEL_TYPE_Sphere,
    MODEL_TYPE_GeoSphere,
    MODEL_TYPE_Imported,    // if we load a model from the file
    MODEL_TYPE_Terrain,
    MODEL_TYPE_LineBox,     // is used to visualise bounding boxes (AABB)
    MODEL_TYPE_Sky,         // this model is used to render the sky
    MODEL_TYPE_Lod,
};

//---------------------------------------------------------
//
//       ATTENTION, WARNING, ACHTUNG:
//
//       if you add a new field into this class (or remove), don't forget to
//       modify the move constructor/assignment and Copy method
//       to avoid a fucking fuck-up when you lost your models data :)
// 
//---------------------------------------------------------
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

    //----------------------------
    // deep copy
    //----------------------------
    void Copy(ID3D11Device* pDevice, const BasicModel& rhs);

    void InitializeBuffers(ID3D11Device* pDevice);
    void Shutdown();
    void ClearMemory();

    void CopyVertices(const Vertex3D* vertices, const int numVertices);
    void CopyIndices(const UINT* indices, const int numIndices);

    //----------------------------
    // memory allocation
    //----------------------------
    bool AllocateMemory();
    bool AllocateMemory(const int numVertices, const int numIndices, const int numSubsets);

    void AllocateVertices(const int numVertices);
    void AllocateIndices(const int numIndices);


    //----------------------------
    // query API
    //----------------------------
    inline ModelID GetID()                              const { return id_; }
    inline const char* GetName()                        const { return name_; }
    inline const DirectX::BoundingBox& GetModelAABB()   const { return modelAABB_; }
    inline const DirectX::BoundingBox* GetSubsetsAABB() const { return subsetsAABB_; }
    inline Subset* GetSubsets()                               { return meshes_.subsets_; }

    inline int GetNumVertices()                         const { return numVertices_; }
    inline int GetNumIndices()                          const { return numIndices_; }
    inline int GetNumSubsets()                          const { return numSubsets_; }

    //----------------------------
    // update API
    //----------------------------
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


    //----------------------------
    // LOD related methods
    //----------------------------
    inline bool  HasLods()    const { return numLods_ != 0; }
    inline uint8 GetNumLods() const { return numLods_; }

    void SetLod           (const eModelLodLevel lod, const ModelID modelId);
    void SetLodDistance   (const eModelLodLevel lod, uint16 distance);

    ModelID GetLod        (const eModelLodLevel lod) const;
    uint16  GetLodDistance(const eModelLodLevel lod) const;
    


    // IF YOU DO SOMETHING WITH FIELDS DON'T FORGET TO CHECK WARNING BEFORE THE CLASS!!!
public:
    char                  name_[MAX_LEN_MODEL_NAME] = "inv";
    ModelID               id_ = 0;
    uint32_t              numVertices_ = 0;
    uint32_t              numIndices_  = 0;
    uint16_t              numSubsets_ = 0;
    uint16_t              numBones_ = 0;
    uint16_t              numAnimClips_ = 0;                             // animation clips
    eModelType            type_ = eModelType::MODEL_TYPE_Invalid;
    
    MeshGeometry          meshes_;                     // contains all the meshes data
    DirectX::BoundingBox  modelAABB_;                  // AABB of the whole model
    DirectX::BoundingBox* subsetsAABB_ = nullptr;      // AABB of each subset (mesh)

    // keep CPU copies of the meshes data to read from
    Vertex3D*             vertices_ = nullptr;
    UINT*                 indices_ = nullptr;

    // lods of this model:
    // (LOD_0 - this model by itself, for other lods we bound other models by IDs)
    ModelID               lods_[NUM_LOD_LEVELS];
    uint16                lodDistance_[NUM_LOD_LEVELS];
    uint8                 numLods_;
};

} // namespace Core
