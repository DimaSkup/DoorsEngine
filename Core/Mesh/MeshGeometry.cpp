// =================================================================================
// Filename:    MeshGeometry.cpp
// 
// Created:     29.10.24
// =================================================================================
#include "MeshGeometry.h"
#include <CoreCommon/MemHelpers.h>
#include <format>

#pragma warning (disable : 4996)

namespace Core
{

MeshGeometry::MeshGeometry() 
{
}

MeshGeometry::~MeshGeometry() 
{
    // release memory
    SafeDeleteArr(subsets_);
    numSubsets_ = 0;

    vb_.~VertexBuffer();
    ib_.~IndexBuffer();
}

///////////////////////////////////////////////////////////

MeshGeometry::MeshGeometry(MeshGeometry&& rhs) noexcept :
    vb_(std::move(rhs.vb_)),
    ib_(std::move(rhs.ib_)),
    vertexStride_(std::exchange(rhs.vertexStride_, 0)),
    subsets_(std::exchange(rhs.subsets_, nullptr)),
    numSubsets_(std::exchange(rhs.numSubsets_, 0))
{
    // move constructor
}

MeshGeometry& MeshGeometry::operator=(MeshGeometry&& rhs) noexcept
{
    // move assignment
    if (this != &rhs)
    {
        this->~MeshGeometry();
        std::construct_at(this, std::move(rhs));
    }

    return *this;
}


// =================================================================================
//                              PUBLIC METHODS
// =================================================================================

void MeshGeometry::Copy(
    ID3D11Device* pDevice,
    const Vertex3D* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices,
    const MeshGeometry& mesh)
{
    // deep copy of data from the input mesh geometry obj

    Assert::True(vertices != nullptr, "input ptr to vertices == nullptr");
    Assert::True(indices != nullptr,  "input ptr to indices == nullptr");
    Assert::True(numVertices > 0,     "input number of vertices must be > 0");
    Assert::True(numIndices > 0,      "input number of indices must be > 0");

    InitVertexBuffer(pDevice, vertices, numVertices);
    InitIndexBuffer(pDevice, indices, numIndices);
    SetSubsets(mesh.subsets_, mesh.numSubsets_);
}

///////////////////////////////////////////////////////////

void MeshGeometry::AllocateSubsets(const int numSubsets)
{
    // allocate memory for this number of subsets
    try
    {
        Assert::True(numSubsets > 0, "num of subsets must be > 0");

        // we already have enough memory; just go out
        if (this->numSubsets_ == numSubsets)
            return;

        this->~MeshGeometry();
        subsets_ = new Subset[numSubsets];
        numSubsets_ = numSubsets;

        // setup ID for each subset
        for (int i = 0; i < numSubsets_; ++i)
            subsets_[i].id = i;
    }
    catch (EngineException& e)
    {
        const std::string errMsg = "can't allocate memory for subsets";
        Log::Error(e);
        Log::Error(errMsg);
        throw EngineException(errMsg);
    }
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetSubsets(const Subset* subsets, const int numSubsets)
{
    // set new subsets (meshes) data

    Assert::True(subsets != nullptr, "input ptr to subset arr == nullptr");
    Assert::True(numSubsets > 0, "input number of subsets must be > 0");
    
    AllocateSubsets(numSubsets);
    std::copy(subsets, subsets + numSubsets, this->subsets_);
}

///////////////////////////////////////////////////////////

void MeshGeometry::InitVertexBuffer(
    ID3D11Device* pDevice,
    const Vertex3D* vertices,
    const int numVertices)
{
    Assert::True(vertices != nullptr, "input ptr to vertices arr == nullptr");
    Assert::True(numVertices > 0, "input number of vertices must be > 0");

    constexpr bool isBufferDynamic = false;

    vb_.Initialize(pDevice, vertices, numVertices, isBufferDynamic);
    vertexStride_ = sizeof(Vertex3D);
}

///////////////////////////////////////////////////////////

void MeshGeometry::InitIndexBuffer(
    ID3D11Device* pDevice,
    const UINT* indices,
    int numIndices)
{
    Assert::True(indices != nullptr, "input ptr to indices arr == nullptr");
    Assert::True(numIndices > 0, "input number of indices must be > 0");

    ib_.Initialize(pDevice, indices, numIndices);
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetSubsetName(const SubsetID subsetID, const char* name)
{
    // setup a name for subset by ID
    try
    {
        size_t length = strlen(name);

        Assert::True(subsetID >= 0,   "subset id is invalid");
        Assert::True(name != nullptr, "input ptr to name string == nullptr");
        Assert::True(length > 0,      "length of input name string must be > 0");

        if (length > SUBSET_NAME_LENGTH_LIMIT)
            length = SUBSET_NAME_LENGTH_LIMIT;

        strncpy(subsets_[subsetID].name, name, length);
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error(std::format("can't set a name for subset {}: invalid input args", subsetID));
    }
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetMaterialForSubset(const SubsetID subsetID, const MaterialID matID)
{
    try
    {
        Assert::True((subsetID >= 0) && (subsetID < numSubsets_), "input ID to subset is invalid");
        Assert::True(matID > 0,                                   "input ID to material is invalid");

        subsets_[subsetID].materialID = matID;
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error(std::format("can't setup a material (its ID: {}) for subset by ID: {}", matID, subsetID));
    }
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetMaterialsForSubsets(
    const SubsetID* subsetsIDs,
    const MaterialID* materialsIDs,
    const size count)
{
    // setup materials for subsets by ID

    if (!CheckInputParamsForMaterialsSetting(subsetsIDs, materialsIDs, count))
    {
        Log::Error("can't set materials for subsets: invalid input args");
        return;
    }

    for (index i = 0; i < count; ++i)
        subsets_[subsetsIDs[i]].materialID = materialsIDs[i];
}

// =================================================================================
// Private helpers
// =================================================================================
bool MeshGeometry::CheckInputParamsForMaterialsSetting(
    const SubsetID* subsetsIDs,
    const MaterialID* materialsIDs,
    const size count)
{
    // check if input subsets IDs and materials IDs are valid
    try
    {
        Assert::True(subsetsIDs != nullptr,   "input ptr to subsets IDs arr == nullptr");
        Assert::True(materialsIDs != nullptr, "input ptr to materials IDs arr == nullptr");
        Assert::True(count > 0,               "input number of elements must be > 0");

        bool isValid = true;
        const uint16_t numSubsets = numSubsets_;

        // check input subsets IDs
        for (index i = 0; i < count; ++i)
            isValid &= (bool)((subsetsIDs[i] >= 0) & (subsetsIDs[i] < numSubsets));

        // check input materials IDs
        for (index i = 0; i < count; ++i)
            isValid &= (bool)materialsIDs[i];

        return isValid;
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        return false;
    }
}

} // namespace Core
