// =================================================================================
// Filename:      model.h
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


namespace Core
{

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

// is used when we derive axis-aligned bounding box (AABB) from bounding sphere,
// or vise versa to setup how exactly we want to calculate the bounding shape
enum eBoundingDeriveType
{
    AABB_INSIDE,        // inside of sphere
    AABB_OUTSIDE,       // outside of sphere
    SPHERE_INSIDE,      // inside of AABB
    SPHERE_OUTSIDE,     // outside of AABB
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
class Model
{
public:
    Model();
    Model(const ModelID id);
    ~Model();

    // move constructor/assignment
    Model(Model&& rhs) noexcept;
    Model& operator=(Model&& rhs) noexcept;

    // restrict shallow copying
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    //----------------------------
    // deep copy
    //----------------------------
    void Copy(const Model& rhs);

    void InitBuffers();
    void Shutdown();
    void ClearMemory();

    void CopyVertices(const Vertex3D* vertices, const int numVertices);
    void CopyIndices(const UINT* indices, const int numIndices);

    //----------------------------
    // memory allocation
    //----------------------------
    bool AllocMem(const int numVertices, const int numIndices, const int numSubsets);

    void AllocVertices(const int numVertices);
    void AllocIndices (const int numIndices);


    //----------------------------
    // query API
    //----------------------------
    ModelID         GetId()          const;
    const char*     GetName()        const;

    Subset*         GetSubsets();
    const Subset*   GetSubsets()     const;

    Vertex3D*       GetVertices();
    UINT*           GetIndices();

    const Vertex3D* GetVertices()    const;
    const UINT*     GetIndices()     const;

    int             GetNumVertices() const;
    int             GetNumIndices()  const;
    int             GetNumSubsets()  const;

    D3D11_PRIMITIVE_TOPOLOGY GetPrimTopology() const;

    MeshGeometry& GetMeshes();
    const MeshGeometry& GetMeshes() const;

    const DirectX::BoundingSphere& GetModelBoundSphere() const;
    const DirectX::BoundingBox&    GetModelAABB()        const;
    const DirectX::BoundingBox*    GetSubsetsAABB()      const;

    //----------------------------
    // update API
    //----------------------------
    void SetName(const char* name);

    void SetPrimTopology(const D3D11_PRIMITIVE_TOPOLOGY topology);

    void SetMaterialForSubset(const SubsetID subsetID, const MaterialID materialId);
    void SetMaterialsForSubsets(
        const SubsetID* subsetIds,
        const MaterialID* materialIds,
        const size count);

    void SetModelBoundSphere(const DirectX::BoundingSphere& sphere);
    void SetModelAABB       (const DirectX::BoundingBox& box);

    void SetSubsetAABB(const SubsetID subsetId,
                       const DirectX::BoundingBox& box);

    void SetSubsetAABBs(const SubsetID* subsetsIds,
                        const DirectX::BoundingBox* boxes,
                        const int count);

    void ComputeBoundings();
    void ComputeSubsetsAABB();

    //----------------------------
    // LOD related methods
    //----------------------------
    bool  HasLods()    const;
    uint8 GetNumLods() const;

    void SetLod           (const eModelLodLevel lod, const ModelID modelId);
    void SetLodDistance   (const eModelLodLevel lod, uint16 distance);

    ModelID GetLod        (const eModelLodLevel lod) const;
    uint16  GetLodDistance(const eModelLodLevel lod) const;


    // IF YOU DO SOMETHING WITH FIELDS DON'T FORGET TO CHECK WARNING BEFORE THE CLASS!!!
private:
    char                     name_[MAX_LEN_MODEL_NAME] = "inv";
    ModelID                  id_ = 0;
    uint32                   numVertices_ = 0;
    uint32                   numIndices_  = 0;
    uint16                   numSubsets_ = 0;

    D3D11_PRIMITIVE_TOPOLOGY primTopology_ = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    MeshGeometry             meshes_;                     // contains all the meshes data
    DirectX::BoundingSphere  modelBoundSphere_;           // sphere around the whole model
    DirectX::BoundingBox     modelAABB_;                  // AABB of the whole model
    DirectX::BoundingBox*    subsetsAABB_ = nullptr;      // AABB of each subset (mesh)

    // keep CPU copies of the meshes data to read from
    Vertex3D*                vertices_ = nullptr;
    UINT*                    indices_ = nullptr;

    // lods of this model:
    // (LOD_0 - this model by itself, for other lods we bound other models by IDs)
    ModelID                  lods_[NUM_LOD_LEVELS];
    uint16                   lodDistance_[NUM_LOD_LEVELS];
    uint8                    numLods_ = 0;
};

//---------------------------------------------------------
// inline functions
//---------------------------------------------------------
inline ModelID          Model::GetId()             const { return id_; }
inline const char*      Model::GetName()           const { return name_; }

inline Vertex3D*        Model::GetVertices()             { return vertices_; }
inline const Vertex3D*  Model::GetVertices()       const { return vertices_; }
inline UINT*            Model::GetIndices()              { return indices_; }
inline const UINT*      Model::GetIndices()        const { return indices_; }
inline int              Model::GetNumVertices()    const { return numVertices_; }
inline int              Model::GetNumIndices()     const { return numIndices_; }

inline int              Model::GetNumSubsets()     const { return numSubsets_; }
inline Subset*          Model::GetSubsets()              { return meshes_.subsets_; }
inline const Subset*    Model::GetSubsets()        const { return meshes_.subsets_; }

inline bool             Model::HasLods()           const { return numLods_ != 0; }
inline uint8            Model::GetNumLods()        const { return numLods_; }


inline MeshGeometry& Model::GetMeshes()
{
    return meshes_;
}

inline const MeshGeometry& Model::GetMeshes() const
{
    return meshes_;
}

inline const DirectX::BoundingSphere& Model::GetModelBoundSphere() const
{
    return modelBoundSphere_;
}

inline const DirectX::BoundingBox& Model::GetModelAABB() const
{
    return modelAABB_;
}

inline const DirectX::BoundingBox* Model::GetSubsetsAABB() const
{
    return subsetsAABB_;
}

inline void Model::SetModelBoundSphere(const DirectX::BoundingSphere& sphere)
{
    modelBoundSphere_ = sphere;
}

inline void Model::SetModelAABB(const DirectX::BoundingBox& aabb)
{
    modelAABB_ = aabb;
}

inline void Model::SetSubsetAABB(const SubsetID id, const DirectX::BoundingBox& aabb)
{
    assert(id < GetNumSubsets() && "invalid id of subset (mesh)");
    SetSubsetAABBs(&id, &aabb, 1);
}

inline void Model::SetMaterialForSubset(const SubsetID meshId, const MaterialID matId)
{
    assert(meshId < GetNumSubsets() && "invalid id of subset (mesh)");
    SetMaterialsForSubsets(&meshId, &matId, 1);
}

//---------------------------------------------------------
// Desc:  get LOD's model ID
//---------------------------------------------------------
inline ModelID Model::GetLod(const eModelLodLevel lod) const
{
    assert(lod < NUM_LOD_LEVELS);
    return lods_[lod];
}

//---------------------------------------------------------
// Desc:  get a distance where we switch to lod by input type
//---------------------------------------------------------
inline uint16 Model::GetLodDistance(const eModelLodLevel lod) const
{
    assert(lod < NUM_LOD_LEVELS);
    return lodDistance_[lod];
}

//---------------------------------------------------------
// set/get primitive topology for this model
//---------------------------------------------------------
inline void Model::SetPrimTopology(const D3D11_PRIMITIVE_TOPOLOGY topology)
{
    primTopology_ = topology;
}

inline D3D11_PRIMITIVE_TOPOLOGY Model::GetPrimTopology() const
{
    return primTopology_;
}

} // namespace Core
