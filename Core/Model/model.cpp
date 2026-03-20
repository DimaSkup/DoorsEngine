// =================================================================================
// Filename:      model.cpp
// 
// Created:       30.10.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "model.h"


namespace Core
{

//---------------------------------------------------------
// constructor
//---------------------------------------------------------
Model::Model() : numLods_(0)
{
    // setup each LOD: model id
    for (int i = 0; i < NUM_LOD_LEVELS; ++i)
        lods_[i] = 0;

    // setup each LOD: distance
    for (int i = 0; i < NUM_LOD_LEVELS; ++i)
        lodDistance_[i] = UINT16_MAX;
}

//---------------------------------------------------------
// constructor with initial model ID
//---------------------------------------------------------
Model::Model(const ModelID id) : Model()
{
    id_ = id;
}

//---------------------------------------------------------
// destructor
//---------------------------------------------------------
Model::~Model()
{
    Shutdown();
}

//---------------------------------------------------------
// move constructor
//---------------------------------------------------------
Model::Model(Model&& rhs) noexcept :
    id_         (rhs.id_),
    meshes_     (std::move(rhs.meshes_)),
    modelAABB_  (std::exchange(rhs.modelAABB_, {})),
    
    subsetsAABB_(std::exchange(rhs.subsetsAABB_, nullptr)),
    vertices_   (std::exchange(rhs.vertices_, nullptr)),
    indices_    (std::exchange(rhs.indices_, nullptr)),

    numVertices_ (rhs.numVertices_),
    numIndices_  (rhs.numIndices_),
    numSubsets_  (rhs.numSubsets_),
    primTopology_(rhs.primTopology_),
    numLods_     (rhs.numLods_)
{
    memmove(name_, rhs.name_, strlen(rhs.name_));

    // copy LODs data
    memcpy(lods_, rhs.lods_, sizeof(lods_));
    memcpy(lodDistance_, rhs.lodDistance_, sizeof(lodDistance_));
}

//---------------------------------------------------------
// move assignment
//---------------------------------------------------------
Model& Model::operator=(Model&& rhs) noexcept
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
void Model::Copy(const Model& rhs)
{
    Shutdown();
    AllocMem(rhs.numVertices_, rhs.numIndices_, rhs.numSubsets_);

    snprintf(name_, MAX_LEN_MODEL_NAME, "copy_of_%s", rhs.name_);

    // copy bounding shapes
    modelBoundSphere_ = rhs.modelBoundSphere_;
    modelAABB_        = rhs.modelAABB_;
    std::copy(rhs.subsetsAABB_, rhs.subsetsAABB_ + numSubsets_, subsetsAABB_);

    // copy geometry
    CopyVertices(rhs.vertices_, rhs.numVertices_);
    CopyIndices(rhs.indices_, rhs.numIndices_);

    // initialize vertex and index buffers, and copy subsets (meshes) metadata
    meshes_.Copy(vertices_, indices_, numVertices_, numIndices_, rhs.meshes_);

    numVertices_  = rhs.numVertices_;
    numIndices_   = rhs.numIndices_;
    numSubsets_   = rhs.numSubsets_;
    primTopology_ = rhs.primTopology_;

    // copy LODs data
    memcpy(lods_, rhs.lods_, sizeof(lods_));
    memcpy(lodDistance_, rhs.lodDistance_, sizeof(lods_));
    numLods_ = rhs.numLods_;
}

//---------------------------------------------------------
// Desc:   initialize VB/IB with model's geometry
//---------------------------------------------------------
void Model::InitBuffers(void)
{
    meshes_.InitVertexBuffer(vertices_, numVertices_);
    meshes_.InitIndexBuffer (indices_, numIndices_);
}

//---------------------------------------------------------
// Desc:   kill this dude
//---------------------------------------------------------
void Model::Shutdown(void)
{
    id_ = 0;
    name_[0] = '\0';

    meshes_.Shutdown();

    SafeDeleteArr(subsetsAABB_);
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    numVertices_  = 0;
    numIndices_   = 0;
    numSubsets_   = 0;
}

//---------------------------------------------------------
// Desc:   release memory from this stuff
//---------------------------------------------------------
void Model::ClearMemory()
{
    meshes_.Shutdown();

    SafeDeleteArr(subsetsAABB_);
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);
}

//---------------------------------------------------------
// Desc:   allocate memory for vertices, indices, and subsets (meshes)
//---------------------------------------------------------
bool Model::AllocMem(const int numVerts, const int numIdxs, const int numMeshes)
{
    if (numVerts <= 0 || numIdxs <= 0 || numMeshes <= 0)
    {
        LogErr(LOG, "some of input args is <= 0 (num vertex: %d, num idxs: %d, num meshes: %s)", numVerts, numIdxs, numMeshes);
        return false;
    }

    ClearMemory();

    if (!meshes_.AllocateSubsets(numMeshes))
    {
        LogErr(LOG, "can't alloc mem for subsets (meshes) of model: %s", name_);
        ClearMemory();
        return false;
    }

    numVertices_ = numVerts;
    numIndices_  = numIdxs;
    numSubsets_  = numMeshes;

    vertices_    = NEW Vertex3D[numVerts];
    indices_     = NEW UINT[numIdxs];
    subsetsAABB_ = NEW DirectX::BoundingBox[numMeshes];

    if (!vertices_ || !indices_ || !subsetsAABB_)
    {
        LogErr(LOG, "can't alloc mem for geometry data of model: %s", name_);
        ClearMemory();
        return false;
    }

    // init all indices with zeros
    memset(indices_, 0, sizeof(UINT) * numIdxs);


    // if we want to create just only one subset (mesh) 
    // we setup this subset's data right here
    if (numMeshes == 1)
    {
        Subset& subset     = meshes_.subsets_[0];
        subset.vertexCount = numVerts;
        subset.indexCount  = numIdxs;
    }

    return true;
}

//---------------------------------------------------------

void Model::AllocVertices(const int numVertices)
{
    assert(numVertices > 0);

    SafeDeleteArr(vertices_);
    vertices_ = new Vertex3D[numVertices]{};
    numVertices_ = numVertices;
}

//---------------------------------------------------------
// Desc:   allocate memory for indices
//---------------------------------------------------------
void Model::AllocIndices(const int numIndices)
{
    assert(numIndices > 0);

    SafeDeleteArr(indices_);
    indices_ = new UINT[numIndices]{ 0 };
    numIndices_ = numIndices;
}

//---------------------------------------------------------
// Desc:   setup vertices of the current model with input vertices
//---------------------------------------------------------
void Model::CopyVertices(const Vertex3D* vertices, const int numVertices)
{
    assert(vertices);
    assert(numVertices);

    std::copy(vertices, vertices + numVertices, vertices_);
}

//---------------------------------------------------------
// Desc:   setup indices of the current model with input indices
//---------------------------------------------------------
void Model::CopyIndices(const UINT* indices, const int numIndices)
{
    assert(indices);
    assert(numIndices);

    std::copy(indices, indices + numIndices, indices_);
}

//---------------------------------------------------------
// Desc:   set new material (new ID) for each input subset by its ID
//---------------------------------------------------------
void Model::SetMaterialsForSubsets(
    const SubsetID* subsetsIds,
    const MaterialID* materialsIds,
    const size count)
{
    assert(subsetsIds);
    assert(materialsIds);
    assert(count > 0);

    for (int i = 0; i < count; ++i)
    {
        const SubsetID subsetID     = subsetsIds[i];
        assert(subsetID < numSubsets_);

        meshes_.subsets_[subsetID].materialId = materialsIds[i];
    }
}

//---------------------------------------------------------
// Desc:   setup a bounding box for subsets (meshs) by its IDs (indices)
//---------------------------------------------------------
void Model::SetSubsetAABBs(
    const SubsetID* subsetsIds,
    const DirectX::BoundingBox* boundBoxes,
    const int count)
{
    assert(subsetsIds);
    assert(boundBoxes);
    assert(count > 0);

    for (int i = 0; i < count; ++i)
    {
        const SubsetID meshId = subsetsIds[i];
        assert(meshId < numSubsets_);

        subsetsAABB_[meshId] = boundBoxes[i];
    }
}

//---------------------------------------------------------
// Desc:  compute model's:
//        1. LOCAL axis-aligned bounding box (AABB) for each subset (mesh),
//        2. LOCAL AABB for the whole model
//        3. LOCAL bounding sphere for model
//---------------------------------------------------------
void Model::ComputeBoundings()
{
    using namespace DirectX;

    // 1. local box for each subset
    ComputeSubsetsAABB();


    // 2. merge bounding box of each subset to make AABB for model
    XMVECTOR modelMin{ +FLT_MAX, +FLT_MAX, +FLT_MAX };
    XMVECTOR modelMax{ -FLT_MIN, -FLT_MIN, -FLT_MIN };

    for (int i = 0; i < numSubsets_; ++i)
    {
        const XMFLOAT3& c = subsetsAABB_[i].Center;
        const XMFLOAT3& e = subsetsAABB_[i].Extents;

        const XMVECTOR meshMin = { c.x-e.x, c.y-e.y, c.z-e.z };
        const XMVECTOR meshMax = { c.x+e.x, c.y+e.y, c.z+e.z };

        modelMin = XMVectorMin(modelMin, meshMin);
        modelMax = XMVectorMax(modelMax, meshMax);
    }

    // convert min/max representation to center and extents representation
    XMStoreFloat3(&modelAABB_.Center,  0.5f * (modelMax + modelMin));
    XMStoreFloat3(&modelAABB_.Extents, 0.5f * (modelMax - modelMin));


    // 3. compute bounding sphere of the model
    const XMFLOAT3&        e = modelAABB_.Extents;
    modelBoundSphere_.Center = modelAABB_.Center;
    modelBoundSphere_.Radius = sqrtf(SQR(e.x) + SQR(e.y) + SQR(e.z));
}

//---------------------------------------------------------
// Desc:   compute a bounding box of each subset (mesh) of the model by its vertices
// NOTE:   there must be already data of vertices and subsets
//---------------------------------------------------------
void Model::ComputeSubsetsAABB()
{
    assert(meshes_.subsets_);
    assert(numSubsets_ > 0);

    using namespace DirectX;
    
    // go through each subset (mesh)
    for (int i = 0; i < numSubsets_; ++i)
    {
        const Subset& subset     = meshes_.subsets_[i];
        const Vertex3D* vertices = vertices_ + subset.vertexStart;

        XMVECTOR vMin{ +FLT_MAX, +FLT_MAX, +FLT_MAX };
        XMVECTOR vMax{ -FLT_MIN, -FLT_MIN, -FLT_MIN };
        
        // go through each vertex of this subset (mesh)
        for (uint32 v = 0; v < subset.vertexCount; ++v)
        {
            XMVECTOR P = XMLoadFloat3(&vertices[v].pos);
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
void Model::SetLod(const eModelLodLevel lod, const ModelID modelId)
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

    for (int i = 0; i < NUM_LOD_LEVELS; ++i)
        numLods += (lods_[i] != INVALID_MODEL_ID);

    numLods_ = numLods;
}

//---------------------------------------------------------
// Desc:  setup a distance where we switch to lod by input type
//---------------------------------------------------------
void Model::SetLodDistance(const eModelLodLevel lod, uint16 distance)
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
// Desc:  setup a name for this model
//---------------------------------------------------------
void Model::SetName(const char* name)
{
    assert(name && name[0] != '\0');

    size_t len = strlen(name);

    if (len > MAX_LEN_MODEL_NAME)
        len = MAX_LEN_MODEL_NAME - 1;

    strncpy(name_, name, len);
    name_[len] = '\0';
};

} // namespace Core
