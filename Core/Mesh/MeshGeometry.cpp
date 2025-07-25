// =================================================================================
// Filename:    MeshGeometry.cpp
// 
// Created:     29.10.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "MeshGeometry.h"

#pragma warning (disable : 4996)


namespace Core
{

MeshGeometry::MeshGeometry() 
{
}

MeshGeometry::~MeshGeometry() 
{
    Shutdown();
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
        this->Shutdown();
        std::construct_at(this, std::move(rhs));
    }

    return *this;
}


// =================================================================================
//                              PUBLIC METHODS
// =================================================================================
void MeshGeometry::Shutdown()
{
    // release memory
    SafeDeleteArr(subsets_);
    numSubsets_ = 0;

    vb_.Shutdown();
    ib_.Shutdown();
}

///////////////////////////////////////////////////////////

void MeshGeometry::Copy(
    ID3D11Device* pDevice,
    const Vertex3D* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices,
    const MeshGeometry& mesh)
{
    // deep copy of data from the input mesh geometry obj

    CAssert::True(vertices != nullptr, "input ptr to vertices == nullptr");
    CAssert::True(indices != nullptr,  "input ptr to indices == nullptr");
    CAssert::True(numVertices > 0,     "input number of vertices must be > 0");
    CAssert::True(numIndices > 0,      "input number of indices must be > 0");

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
        CAssert::True(numSubsets > 0, "num of subsets must be > 0");

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
        LogErr(e);
        LogErr("can't allocate memory for subsets");
        throw EngineException("can't allocate memory for subsets");
    }
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetSubsets(const Subset* subsets, const int numSubsets)
{
    // set new subsets (meshes) data

    CAssert::True(subsets != nullptr, "input ptr to subset arr == nullptr");
    CAssert::True(numSubsets > 0, "input number of subsets must be > 0");
    
    AllocateSubsets(numSubsets);
    std::copy(subsets, subsets + numSubsets, this->subsets_);
}

///////////////////////////////////////////////////////////

void MeshGeometry::InitVertexBuffer(
    ID3D11Device* pDevice,
    const Vertex3D* vertices,
    const int numVertices)
{
    CAssert::True(vertices != nullptr, "input ptr to vertices arr == nullptr");
    CAssert::True(numVertices > 0, "input number of vertices must be > 0");

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
    CAssert::True(indices != nullptr, "input ptr to indices arr == nullptr");
    CAssert::True(numIndices > 0, "input number of indices must be > 0");

    constexpr bool isDynamic = false;
    ib_.Initialize(pDevice, indices, numIndices, isDynamic);
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetSubsetName(const SubsetID subsetID, const char* name)
{
    // setup a name for subset by ID
    try
    {
        size_t length = strlen(name);

        CAssert::True(subsetID >= 0,   "subset id is invalid");
        CAssert::True(name != nullptr, "input ptr to name string == nullptr");
        CAssert::True(length > 0,      "length of input name string must be > 0");

        if (length > SUBSET_NAME_LENGTH_LIMIT)
            length = SUBSET_NAME_LENGTH_LIMIT;

        strncpy(subsets_[subsetID].name, name, length);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't set a name for subset (id: %d): invalid input args", subsetID);
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetMaterialForSubset(const SubsetID subsetID, const MaterialID matID)
{
    try
    {
        CAssert::True((subsetID >= 0) && (subsetID < numSubsets_), "input ID to subset is invalid");
        CAssert::True(matID > 0,                                   "input ID to material is invalid");

        subsets_[subsetID].materialID = matID;
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't setup a material (ID: %ud) for subset (ID: %ud)", matID, subsetID);
        LogErr(e);
        LogErr(g_String);
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
        LogErr("can't set materials for subsets: invalid input args");
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
        CAssert::True(subsetsIDs != nullptr,   "input ptr to subsets IDs arr == nullptr");
        CAssert::True(materialsIDs != nullptr, "input ptr to materials IDs arr == nullptr");
        CAssert::True(count > 0,               "input number of elements must be > 0");

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
        LogErr(e);
        return false;
    }
}

} // namespace Core
