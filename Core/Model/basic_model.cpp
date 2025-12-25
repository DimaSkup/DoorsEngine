// =================================================================================
// Filename:      BasicModel.cpp
// 
// Created:       30.10.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "basic_model.h"


namespace Core
{

BasicModel::BasicModel() : numLods_(0)
{
    // setup each LOD: model id
    memset(lods_, 0, sizeof(lods_));

    // setup each LOD: distance
    for (int i = 0; i < (int)NUM_LOD_LEVELS; ++i)
        lodDistance_[i] = UINT16_MAX;


#if 0
    // to get knowledge about size of a single BasicModel instance
    LogMsg("sizeof(BasicModel):            " + std::to_string(sizeof(BasicModel)));
    LogMsg("sizeof(MeshGeometry::Subset):  " + std::to_string(sizeof(MeshGeometry::Subset)));
    LogMsg("sizeof(MeshGeometry):          " + std::to_string(sizeof(MeshGeometry)));
    LogMsg("sizeof(VertexBuffer<Vertex3D>):" + std::to_string(sizeof(VertexBuffer<Vertex3D>)));
    LogMsg("sizeof(IndexBuffer<UINT>):     " + std::to_string(sizeof(IndexBuffer<UINT>)));
    LogMsg("sizeof(ID3D11Buffer):          " + std::to_string(sizeof(ID3D11Buffer)));
#endif
}

///////////////////////////////////////////////////////////

BasicModel::~BasicModel()
{
    Shutdown();
}

//---------------------------------------------------------
// move constructor
//---------------------------------------------------------
BasicModel::BasicModel(BasicModel&& rhs) noexcept :
    id_         (rhs.id_),
    type_       (std::exchange(rhs.type_, MODEL_TYPE_Invalid)),
    meshes_     (std::move(rhs.meshes_)),
    modelAABB_  (std::exchange(rhs.modelAABB_, {})),
    
    subsetsAABB_(std::exchange(rhs.subsetsAABB_, nullptr)),
    vertices_   (std::exchange(rhs.vertices_, nullptr)),
    indices_    (std::exchange(rhs.indices_, nullptr)),

    numVertices_ (rhs.numVertices_),
    numIndices_  (rhs.numIndices_),
    numSubsets_  (rhs.numSubsets_),
    numBones_    (rhs.numBones_),
    numAnimClips_(rhs.numAnimClips_),
    numLods_     (rhs.numLods_)
{
    memmove(name_, rhs.name_, strlen(rhs.name_));

    // copy LODs data
    memcpy(lods_, rhs.lods_, sizeof(lods_));
    memcpy(lodDistance_, rhs.lodDistance_, sizeof(lods_));
}

//---------------------------------------------------------
// move assignment
//---------------------------------------------------------
BasicModel& BasicModel::operator=(BasicModel&& rhs) noexcept
{
    if (this != &rhs)
    {
        Shutdown();                    // lifetime of *this ends
        std::construct_at(this, std::move(rhs));
    }

    return *this;
}

//---------------------------------------------------------
// deep copy of data
//---------------------------------------------------------
void BasicModel::Copy(ID3D11Device* pDevice, const BasicModel& rhs)
{

    Shutdown();
    AllocateMemory(rhs.numVertices_, rhs.numIndices_, rhs.numSubsets_);

    type_ = rhs.type_;
    snprintf(name_, MAX_LEN_MODEL_NAME, "copy_of_%s", rhs.name_);

    // copy bounding shapes
    modelAABB_ = rhs.modelAABB_;
    std::copy(rhs.subsetsAABB_, rhs.subsetsAABB_ + numSubsets_, subsetsAABB_);

    // copy geometry
    CopyVertices(rhs.vertices_, rhs.numVertices_);
    CopyIndices(rhs.indices_, rhs.numIndices_);

    // initialize vertex and index buffers, and copy subsets (meshes) metadata
    meshes_.Copy(pDevice, vertices_, indices_, numVertices_, numIndices_, rhs.meshes_);

    numVertices_  = rhs.numVertices_;
    numIndices_   = rhs.numIndices_;
    numSubsets_   = rhs.numSubsets_;
    numBones_     = rhs.numBones_;
    numAnimClips_ = rhs.numAnimClips_;

    // copy LODs data
    memcpy(lods_, rhs.lods_, sizeof(lods_));
    memcpy(lodDistance_, rhs.lodDistance_, sizeof(lods_));
    numLods_ = rhs.numLods_;
}

//---------------------------------------------------------
// Desc:   initialize VB/IB with model's geometry
//---------------------------------------------------------
void BasicModel::InitializeBuffers(ID3D11Device* pDevice)
{
    meshes_.InitVertexBuffer(pDevice, vertices_, numVertices_);
    meshes_.InitIndexBuffer (pDevice, indices_, numIndices_);
}

//---------------------------------------------------------
// Desc:   kill this dude
//---------------------------------------------------------
void BasicModel::Shutdown()
{
    id_ = 0;
    name_[0] = '\0';
    type_ = MODEL_TYPE_Invalid;

    meshes_.Shutdown();

    SafeDeleteArr(subsetsAABB_);
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    numVertices_  = 0;
    numIndices_   = 0;
    numSubsets_   = 0;
    numBones_     = 0;
    numAnimClips_ = 0;
}

//---------------------------------------------------------
// Desc:   release memory from this stuff
//---------------------------------------------------------
void BasicModel::ClearMemory()
{
    meshes_.Shutdown();

    SafeDeleteArr(subsetsAABB_);
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);
}

//---------------------------------------------------------
// Desc:   allocate memory for vertices, indices, and subsets (meshes)
//---------------------------------------------------------
bool BasicModel::AllocateMemory(
    const int numVertices,
    const int numIndices,
    const int numSubsets)
{
    if (numVertices <= 0 || numIndices <= 0 || numSubsets <= 0)
    {
        LogErr(LOG, "some of input args is <= 0 (num vertex: %d, num idxs: %d, num meshes: %s)", numVertices, numIndices, numSubsets);
        return false;
    }

    ClearMemory();

    if (!meshes_.AllocateSubsets(numSubsets))
    {
        LogErr(LOG, "can't alloc memory for subsets (meshes) of model: %s", name_);
        ClearMemory();
        return false;
    }

    numVertices_ = numVertices;
    numIndices_  = numIndices;
    numSubsets_  = numSubsets;

    vertices_    = NEW Vertex3D[numVertices];
    indices_     = NEW UINT[numIndices];
    subsetsAABB_ = NEW DirectX::BoundingBox[numSubsets];

    if (!vertices_ || !indices_ || !subsetsAABB_)
    {
        LogErr(LOG, "can't allocate memory for geometry data of model: %s", name_);
        ClearMemory();
        return false;
    }

    memset(indices_, 0, sizeof(UINT) * numIndices);


    // if we want to create just only one subset (mesh) 
    // we setup this subset's data right here
    if (numSubsets == 1)
    {
        Subset& subset     = meshes_.subsets_[0];
        subset.vertexCount = numVertices;
        subset.indexCount  = numIndices;
    }

    return true;
}

///////////////////////////////////////////////////////////

void BasicModel::AllocateVertices(const int numVertices)
{
    CAssert::True(numVertices > 0, "new number of vertices must be > 0");

    SafeDeleteArr(vertices_);
    vertices_ = new Vertex3D[numVertices]{};
    numVertices_ = numVertices;
}

//---------------------------------------------------------
// Desc:   allocate memory for indices
//---------------------------------------------------------
void BasicModel::AllocateIndices(const int numIndices)
{
    CAssert::True(numIndices > 0, "new number of indices must be > 0");

    SafeDeleteArr(indices_);
    indices_ = new UINT[numIndices]{ 0 };
    numIndices_ = numIndices;
}

//---------------------------------------------------------
// Desc:   setup vertices of the current model with input vertices
//---------------------------------------------------------
void BasicModel::CopyVertices(const Vertex3D* vertices, const int numVertices)
{
    CAssert::True(vertices != nullptr, "input ptr to vertices == nullptr");
    CAssert::True(numVertices > 0,     "input number of vertices must be > 0");

    std::copy(vertices, vertices + numVertices, vertices_);
}

//---------------------------------------------------------
// Desc:   setup indices of the current model with input indices
//---------------------------------------------------------
void BasicModel::CopyIndices(const UINT* indices, const int numIndices)
{
    CAssert::True(indices != nullptr, "input ptr to indices == nullptr");
    CAssert::True(numIndices > 0,     "input number of indices must be > 0");

    std::copy(indices, indices + numIndices, indices_);
}


// =================================================================================
//                           PUBLIC UPDATING API
// =================================================================================

//---------------------------------------------------------
//---------------------------------------------------------
void BasicModel::SetMaterialForSubset(const SubsetID subsetID, const MaterialID materialID)
{
    CAssert::True((subsetID >= 0) && (subsetID < GetNumSubsets()), "invalid ID of subset (wrong value range)");

    SetMaterialsForSubsets(&subsetID, &materialID, 1);
}

//---------------------------------------------------------
// Desc:   set new material (new ID) for each input subset by its ID
//---------------------------------------------------------
void BasicModel::SetMaterialsForSubsets(
    const SubsetID* subsetsIDs,
    const MaterialID* materialsIDs,
    const size count)
{
    CAssert::True(subsetsIDs != nullptr, "arr of subset ids == nullptr");
    CAssert::True(materialsIDs != nullptr, "arr of materials ids == nullptr");
    CAssert::True(count > 0, "wrong num of input data elements");

    for (int i = 0; i < count; ++i)
    {
        const SubsetID subsetID     = subsetsIDs[i];
        const MaterialID materialID = materialsIDs[i];

        meshes_.subsets_[subsetID].materialId = materialID;
    }
}

//---------------------------------------------------------
// Desc:   set AABB for the whole model
//---------------------------------------------------------
void BasicModel::SetModelAABB(const DirectX::BoundingBox& aabb)
{
    modelAABB_ = aabb;
}

//---------------------------------------------------------
// Desc:   setup a bounding box for subset (mesh) by ID (its index)
//---------------------------------------------------------
void BasicModel::SetSubsetAABB(const SubsetID subsetID, const DirectX::BoundingBox& aabb)
{
    CAssert::True((subsetID >= 0) && (subsetID < GetNumSubsets()), "invalid ID of subset (wrong range)");
    SetSubsetAABBs(&subsetID, &aabb, 1);
}

//---------------------------------------------------------
// Desc:   setup a bounding box for subsets (meshs) by its IDs (indices)
//---------------------------------------------------------
void BasicModel::SetSubsetAABBs(
    const SubsetID* subsetsIDs,
    const DirectX::BoundingBox* AABBs,
    const int count)
{
    CAssert::True(subsetsIDs != nullptr, "ptr to subsets IDs == nullptr");
    CAssert::True(AABBs != nullptr,      "ptr to AABBs == nullptr");
    CAssert::True(count > 0,             "wrong num of input elems (must be > 0)");

    for (int i = 0; i < count; ++i)
    {
        const int idx = subsetsIDs[i];
        subsetsAABB_[idx] = AABBs[i];
    }
}

//---------------------------------------------------------
// Desc:   compute a bounding box of the whole model by AABB of each subset (mesh)
// NOTE:   subsets AABBs must be already computed before
//---------------------------------------------------------
void BasicModel::ComputeModelAABB()
{
    using namespace DirectX;

    XMVECTOR vMin{ +FLT_MAX, +FLT_MAX, +FLT_MAX };
    XMVECTOR vMax{ -FLT_MIN, -FLT_MIN, -FLT_MIN };

    // go through each subset (mesh)
    for (int i = 0; i < numSubsets_; ++i)
    {
        const BoundingBox& subsetAABB = subsetsAABB_[i];

        // define min/max point of this mesh
        const XMVECTOR center  = XMLoadFloat3(&subsetAABB.Center);
        const XMVECTOR extents = XMLoadFloat3(&subsetAABB.Extents);
        const XMVECTOR meshMaxPoint = center + extents;
        const XMVECTOR meshMinPoint = center - extents;

        vMin = XMVectorMin(vMin, meshMinPoint);
        vMax = XMVectorMax(vMax, meshMaxPoint);
    }

    // compute a model's AABB
    XMStoreFloat3(&modelAABB_.Center,  0.5f * (vMin + vMax));
    XMStoreFloat3(&modelAABB_.Extents, 0.5f * (vMax - vMin));
}

//---------------------------------------------------------
// Desc:   compute a bounding box of each subset (mesh) of the model by its vertices
// NOTE:   there must be already data of vertices and subsets
//---------------------------------------------------------
void BasicModel::ComputeSubsetsAABB()
{
    assert(meshes_.subsets_ != nullptr);

    using namespace DirectX;
    
    // go through each subset (mesh)
    for (int i = 0; i < numSubsets_; ++i)
    {
        const Subset& subset     = meshes_.subsets_[i];
        const Vertex3D* vertices = vertices_ + subset.vertexStart;

        XMVECTOR vMin{ +FLT_MAX, +FLT_MAX, +FLT_MAX };
        XMVECTOR vMax{ -FLT_MIN, -FLT_MIN, -FLT_MIN };
        
        // go through each vertex of this subset (mesh)
        for (uint32 vIdx = 0; vIdx < subset.vertexCount; ++vIdx)
        {
            XMVECTOR P = XMLoadFloat3(&vertices[vIdx].position);
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // convert min/max representation to center and extents representation
        XMStoreFloat3(&subsetsAABB_[i].Center,  0.5f * (vMin + vMax));
        XMStoreFloat3(&subsetsAABB_[i].Extents, 0.5f * (vMax - vMin));
    }
}


//==================================================================================
// LOD related methods
//==================================================================================

//---------------------------------------------------------
// Desc:  setup a LOD by input type;
//        we set another model as a LOD of particular level for this model
//---------------------------------------------------------
void BasicModel::SetLod(const eModelLodLevel lod, const ModelID modelId)
{
    if (lod >= NUM_LOD_LEVELS)
        return;

    // we must set LODs from closest to farthes
    if (lod > numLods_)
        return;

    if (modelId == INVALID_MODEL_ID)
        return;


    lods_[lod] = modelId;

    // calc the current number of LODs
    int numLods = 0;

    for (int i = 0; i < (int)NUM_LOD_LEVELS; ++i)
        numLods += (lods_[i] != INVALID_MODEL_ID);

    numLods_ = numLods;
}

//---------------------------------------------------------
// Desc:  setup a distance where we switch to lod by input type
//---------------------------------------------------------
void BasicModel::SetLodDistance(const eModelLodLevel lod, uint16 distance)
{
    if (lod >= NUM_LOD_LEVELS)
        return;

    // the distance to LOD can't be lower than the previous LOD's distance
    if (lod == LOD_1)
    {
        Clamp(distance, (uint16)1, lodDistance_[LOD_2]);
        lodDistance_[lod] = distance;
    }
    else if (lod == LOD_2)
    {
        Clamp(distance, lodDistance_[LOD_1], UINT16_MAX);
        lodDistance_[lod] = distance;
    }
    else
    {
        LogErr(LOG, "LOD number is invalid (%d)", (int)lod);
        return;
    }
}

//---------------------------------------------------------
// Desc:  get LOD's model ID
//---------------------------------------------------------
ModelID BasicModel::GetLod(const eModelLodLevel lod) const
{
    if (lod >= NUM_LOD_LEVELS)
        return INVALID_MODEL_ID;

    return lods_[lod];
}

//---------------------------------------------------------
// Desc:  get a distance where we switch to lod by input type
//---------------------------------------------------------
uint16 BasicModel::GetLodDistance(const eModelLodLevel lod) const
{
    if (lod >= NUM_LOD_LEVELS)
        return UINT16_MAX;

    return lodDistance_[lod];
}

} // namespace Core
