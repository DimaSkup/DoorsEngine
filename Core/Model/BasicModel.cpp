// =================================================================================
// Filename:      BasicModel.cpp
// 
// Created:       30.10.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "BasicModel.h"


namespace Core
{

BasicModel::BasicModel()
{
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

///////////////////////////////////////////////////////////

BasicModel::BasicModel(BasicModel&& rhs) noexcept :
    id_         (rhs.id_),
    type_       (std::exchange(rhs.type_, eModelType::Invalid)),
    meshes_     (std::move(rhs.meshes_)),
    modelAABB_  (std::exchange(rhs.modelAABB_, {})),
    
    subsetsAABB_(std::exchange(rhs.subsetsAABB_, nullptr)),
    vertices_   (std::exchange(rhs.vertices_, nullptr)),
    indices_    (std::exchange(rhs.indices_, nullptr)),

    numVertices_(rhs.numVertices_),
    numIndices_ (rhs.numIndices_),
    numSubsets_ (rhs.numSubsets_),
    numBones_   (rhs.numBones_),
    numAnimClips_(rhs.numAnimClips_)
{
    // move constructor
    memmove(name_, rhs.name_, strlen(rhs.name_));
}

///////////////////////////////////////////////////////////

BasicModel& BasicModel::operator=(BasicModel&& rhs) noexcept
{
    // move assignment
    if (this != &rhs)
    {
        Shutdown();                    // lifetime of *this ends
        std::construct_at(this, std::move(rhs));
    }

    return *this;
}

///////////////////////////////////////////////////////////

void BasicModel::Copy(ID3D11Device* pDevice, const BasicModel& rhs)
{
    // deep copy of data

    assert(0 && "TODO: FIX IT");
#if 0
    Clear();
    AllocateMemory(rhs.numVertices_, rhs.numIndices_, rhs.numSubsets_);

    type_ = rhs.type_;
    name_ = "copy_of_" + rhs.name_;

    modelAABB_ = rhs.modelAABB_;

    // copy textures / materials / AABB of each subset
    std::copy(rhs.texIDs_, rhs.texIDs_ + numTextures_, texIDs_);
    std::copy(rhs.materials_, rhs.materials_ + numMats_, materials_);
    std::copy(rhs.subsetsAABB_, rhs.subsetsAABB_ + numSubsets_, subsetsAABB_);

    CopyVertices(rhs.vertices_, rhs.numVertices_);
    CopyIndices(rhs.indices_, rhs.numIndices_);

    // copy VB/IB, and subsets data
    meshes_.Copy(pDevice, vertices_, indices_, numVertices_, numIndices_, rhs.meshes_);

    numVertices_ = rhs.numVertices_;
    numIndices_  = rhs.numIndices_;
    numMats_     = rhs.numMats_;
    numSubsets_  = rhs.numSubsets_;
    numTextures_ = rhs.numTextures_;
#endif
}

///////////////////////////////////////////////////////////

void BasicModel::InitializeBuffers(ID3D11Device* pDevice)
{
    meshes_.InitVertexBuffer(pDevice, vertices_, numVertices_);
    meshes_.InitIndexBuffer(pDevice, indices_, numIndices_);
}

///////////////////////////////////////////////////////////

void BasicModel::Shutdown()
{
    id_ = 0;
    name_[0] = '\0';
    type_ = eModelType::Invalid;

    meshes_.~MeshGeometry();

    SafeDeleteArr(subsetsAABB_);
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    numVertices_  = 0;
    numIndices_   = 0;
    numSubsets_   = 0;
    numBones_     = 0;
    numAnimClips_ = 0;
}

///////////////////////////////////////////////////////////

void BasicModel::ClearMemory()
{
    meshes_.~MeshGeometry();

    SafeDeleteArr(subsetsAABB_);
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);
}



// =================================================================================
//                       PUBLIC MEMORY ALLOCATION API
// =================================================================================
void BasicModel::AllocateMemory()
{
    try
    {
        assert(numVertices_ > 0);
        assert(numIndices_ > 0);
        assert(numSubsets_ > 0);

        //
        // prepare memory
        //
        ClearMemory();
        meshes_.AllocateSubsets(numSubsets_);

        vertices_    = new Vertex3D[numVertices_]{};
        indices_     = new UINT[numIndices_]{ 0 };
        subsetsAABB_ = new DirectX::BoundingBox[numSubsets_];

        // if we want to create just only one subset (mesh) 
        // we setup this subset's data right here
        if (numSubsets_ == 1)
        {
            MeshGeometry::Subset& subset = meshes_.subsets_[0];

            subset.vertexCount = numVertices_;
            subset.indexCount = numIndices_;
        }
    }
    catch (const std::bad_alloc& e)
    {
        Shutdown();

        LogErr(e.what());
        LogErr("can't allocate memory for some data of the model");
        return;
    }
}

///////////////////////////////////////////////////////////

void BasicModel::AllocateMemory(
    const int numVertices,
    const int numIndices,
    const int numSubsets)
{
    try
    {
        assert(numVertices > 0);
        assert(numIndices > 0);
        assert(numSubsets > 0);

        // prepare memory
        ClearMemory();
        meshes_.AllocateSubsets(numSubsets);

        numVertices_ = numVertices;
        numIndices_  = numIndices;
        numSubsets_  = numSubsets;

        vertices_    = new Vertex3D[numVertices_]{};
        indices_     = new UINT[numIndices_]{ 0 };
        subsetsAABB_ = new DirectX::BoundingBox[numSubsets];

        // if we want to create just only one subset (mesh) 
        // we setup this subset's data right here
        if (numSubsets == 1)
        {
            MeshGeometry::Subset& subset = meshes_.subsets_[0];

            subset.vertexCount = numVertices;
            subset.indexCount = numIndices;
        }
    }
    catch (const std::bad_alloc& e)
    {
        Shutdown();

        LogErr(e.what());
        LogErr("can't allocate memory for some data of the model");
        return;
    }
}

///////////////////////////////////////////////////////////

void BasicModel::AllocateVertices(const int numVertices)
{
    CAssert::True(numVertices > 0, "new number of vertices must be > 0");

    SafeDeleteArr(vertices_);
    vertices_ = new Vertex3D[numVertices]{};
    numVertices_ = numVertices;
}

///////////////////////////////////////////////////////////

void BasicModel::AllocateIndices(const int numIndices)
{
    CAssert::True(numIndices > 0, "new number of indices must be > 0");

    SafeDeleteArr(indices_);
    indices_ = new UINT[numIndices]{ 0 };
    numIndices_ = numIndices;
}

///////////////////////////////////////////////////////////

void BasicModel::CopyVertices(const Vertex3D* vertices, const int numVertices)
{
    // check input data and check if we have enough allocated memory
    CAssert::True(vertices != nullptr, "input ptr to vertices == nullptr");
    CAssert::True(numVertices > 0,     "input number of vertices must be > 0");

    std::copy(vertices, vertices + numVertices, vertices_);
}

///////////////////////////////////////////////////////////

void BasicModel::CopyIndices(const UINT* indices, const int numIndices)
{
    // check input data and check if we have enough allocated memory
    CAssert::True(indices != nullptr, "input ptr to indices == nullptr");
    CAssert::True(numIndices > 0,     "input number of indices must be > 0");

    std::copy(indices, indices + numIndices, indices_);
}


// =================================================================================
//                           PUBLIC UPDATING API
// =================================================================================

void BasicModel::SetName(const char* name)
{
    // set new name if it is valid or don't change in another case
    if ((name == nullptr) || (name[0] == '\0'))
    {
        LogErr("can't set a new name for the sky model: input name is empty");
        return;
    }

    size_t len = strlen(name);

    // if input name is too long we limit its length
    if (len > MAX_LENGTH_MODEL_NAME - 1)
        len = MAX_LENGTH_MODEL_NAME - 1;

    strncpy(name_, name, len);
    name_[len] = '\0';
}

///////////////////////////////////////////////////////////

void BasicModel::SetMaterialForSubset(const SubsetID subsetID, const MaterialID materialID)
{
    CAssert::True((subsetID >= 0) && (subsetID < GetNumSubsets()), "invalid ID of subset (wrong value range)");

    SetMaterialsForSubsets(&subsetID, &materialID, 1);
}

///////////////////////////////////////////////////////////

void BasicModel::SetMaterialsForSubsets(
    const SubsetID* subsetsIDs,
    const MaterialID* materialsIDs,
    const size count)
{
    CAssert::True(subsetsIDs != nullptr, "arr of subset ids == nullptr");
    CAssert::True(materialsIDs != nullptr, "arr of materials ids == nullptr");
    CAssert::True(count > 0, "wrong num of input data elements");

    // set new material (new ID) for each input subset by its ID
    for (int i = 0; i < count; ++i)
    {
        const SubsetID subsetID     = subsetsIDs[i];
        const MaterialID materialID = materialsIDs[i];

        meshes_.subsets_[subsetID].materialId = materialID;
    }
}

///////////////////////////////////////////////////////////

void BasicModel::SetModelAABB(const DirectX::BoundingBox& aabb)
{
    // set AABB for the whole model
    modelAABB_ = aabb;
}

///////////////////////////////////////////////////////////

void BasicModel::SetSubsetAABB(const SubsetID subsetID, const DirectX::BoundingBox& aabb)
{
    CAssert::True((subsetID >= 0) && (subsetID < GetNumSubsets()), "invalid ID of subset (wrong range)");
    SetSubsetAABBs(&subsetID, &aabb, 1);
}

///////////////////////////////////////////////////////////

void BasicModel::SetSubsetAABBs(
    const SubsetID* subsetsIDs,
    const DirectX::BoundingBox* AABBs,
    const int count)
{
    CAssert::True(subsetsIDs != nullptr, "ptr to subsets IDs == nullptr");
    CAssert::True(AABBs != nullptr,      "ptr to AABBs == nullptr");
    CAssert::True(count > 0,             "wrong num of input elems (must be > 0)");

    // subset (mesh) ID is the same as its idx in the BasicModel class;
    // so we can access AABB by the same idx as well
    for (int i = 0; i < count; ++i)
    {
        const int idx = subsetsIDs[i];
        subsetsAABB_[idx] = AABBs[i];
    }
}

///////////////////////////////////////////////////////////

void BasicModel::ComputeModelAABB()
{
    // compute a bounding box of the whole model by AABB of each subset (mesh)
    // NOTE: subsets AABBs must be already computed before

    using namespace DirectX;

    XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
    XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

    // go through each subset (mesh)
    for (int i = 0; i < numSubsets_; ++i)
    {
        const DirectX::BoundingBox& subsetAABB = subsetsAABB_[i];

        // define min/max point of this mesh
        const XMVECTOR meshVecCenter  = XMLoadFloat3(&subsetAABB.Center);
        const XMVECTOR meshVecExtents = XMLoadFloat3(&subsetAABB.Extents);
        const XMVECTOR meshVecMax = meshVecCenter + meshVecExtents;
        const XMVECTOR meshVecMin = meshVecCenter - meshVecExtents;

        vMin = XMVectorMin(vMin, meshVecMin);
        vMax = XMVectorMax(vMax, meshVecMax);
    }

    // compute a model's AABB
    XMStoreFloat3(&modelAABB_.Center,  0.5f * (vMin + vMax));
    XMStoreFloat3(&modelAABB_.Extents, 0.5f * (vMax - vMin));
}

///////////////////////////////////////////////////////////

void BasicModel::ComputeSubsetsAABB()
{
    // compute a bounding box of each subset (mesh) of the model by its vertices
    // NOTE: there must be already data of vertices and subsets

    using namespace DirectX;
    
    // go through each subset (mesh)
    for (int i = 0; i < numSubsets_; ++i)
    {
        MeshGeometry::Subset& subset = meshes_.subsets_[i];
        Vertex3D* vertices = vertices_ + subset.vertexStart;

        XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
        XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };
        
        // go through each vertex of this subset (mesh)
        for (u32 vIdx = 0; vIdx < subset.vertexCount; ++vIdx)
        {
            XMVECTOR P = XMLoadFloat3(&vertices[vIdx].position);
            vMin = DirectX::XMVectorMin(vMin, P);
            vMax = DirectX::XMVectorMax(vMax, P);
        }

        // convert min/max representation to center and extents representation
        DirectX::XMStoreFloat3(&subsetsAABB_[i].Center,  0.5f * (vMin + vMax));
        DirectX::XMStoreFloat3(&subsetsAABB_[i].Extents, 0.5f * (vMax - vMin));
    }
}

} // namespace Core
