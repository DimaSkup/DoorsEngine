// =================================================================================
// Filename:    MeshGeometry.cpp
// 
// Created:     29.10.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "mesh_geometry.h"

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

//---------------------------------------------------------
// move constructor
//---------------------------------------------------------
MeshGeometry::MeshGeometry(MeshGeometry&& rhs) noexcept :
    vb_(std::move(rhs.vb_)),
    ib_(std::move(rhs.ib_)),
    vertexStride_(std::exchange(rhs.vertexStride_, 0)),
    subsets_(std::exchange(rhs.subsets_, nullptr)),
    localSpaceMatrices_(std::exchange(rhs.localSpaceMatrices_, nullptr)),
    numSubsets_(std::exchange(rhs.numSubsets_, 0))
{
}

//---------------------------------------------------------
// move assignment
//---------------------------------------------------------
MeshGeometry& MeshGeometry::operator=(MeshGeometry&& rhs) noexcept
{
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
    SafeDeleteArr(localSpaceMatrices_);
    numSubsets_ = 0;

    vb_.Shutdown();
    ib_.Shutdown();
}

//---------------------------------------------------------
// Desc:   deep copy of data from the input mesh geometry obj
//---------------------------------------------------------
void MeshGeometry::Copy(
    ID3D11Device* pDevice,
    const Vertex3D* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices,
    const MeshGeometry& mesh)
{
    // I just want to be sure that it won't be a pain in my ass someday :)
    CAssert::True(vertices != nullptr, "input ptr to vertices == nullptr");
    CAssert::True(indices != nullptr,  "input ptr to indices == nullptr");
    CAssert::True(numVertices > 0,     "input number of vertices must be > 0");
    CAssert::True(numIndices > 0,      "input number of indices must be > 0");

    InitVertexBuffer(pDevice, vertices, numVertices);
    InitIndexBuffer(pDevice, indices, numIndices);
    SetSubsets(mesh.subsets_, mesh.numSubsets_);
}

//---------------------------------------------------------
// Desc:   allocate memory for subsets data
// Args:   - numSubsets:  how many subsets/sumeshes/meshes we have
//---------------------------------------------------------
bool MeshGeometry::AllocateSubsets(const int numSubsets)
{
    if (numSubsets <= 0)
    {
        LogErr(LOG, "num of subsets must be > 0");
        return false;
    }

    // we already have enough memory; just go out
    if (this->numSubsets_ == numSubsets)
        return true;

    // release memory before allocation
    Shutdown();


    subsets_ = NEW Subset[numSubsets];
    if (!subsets_)
    {
        Shutdown();
        LogErr(LOG, "can't alloc mem for subsets of model");
        return false;
    }

    localSpaceMatrices_ = NEW DirectX::XMMATRIX[numSubsets];
    if (!localSpaceMatrices_)
    {
        Shutdown();
        LogErr(LOG, "can't alloc mem for local space matrices of model");
        return false;
    }

    numSubsets_ = numSubsets;

    // setup ID for each subset
    for (int i = 0; i < numSubsets_; ++i)
        subsets_[i].id = i;

    return true;
}

//---------------------------------------------------------
// Desc:   set new subsets (meshes) data
//---------------------------------------------------------
void MeshGeometry::SetSubsets(const Subset* subsets, const int numSubsets)
{
    CAssert::True(subsets != nullptr, "input ptr to subset arr == nullptr");
    CAssert::True(numSubsets > 0,     "input number of subsets must be > 0");
    
    AllocateSubsets(numSubsets);
    std::copy(subsets, subsets + numSubsets, subsets_);
}

//---------------------------------------------------------
// Desc:   initialize a vertex buffer with input data
//---------------------------------------------------------
void MeshGeometry::InitVertexBuffer(
    ID3D11Device* pDevice,
    const Vertex3D* vertices,
    const int numVertices)
{
    CAssert::True(vertices != nullptr, "input ptr to vertices arr == nullptr");
    CAssert::True(numVertices > 0,     "input number of vertices must be > 0");

    constexpr bool isDynamic = false;
    vb_.Shutdown();
    vb_.Initialize(pDevice, vertices, numVertices, isDynamic);
    vertexStride_ = sizeof(Vertex3D);
}

//---------------------------------------------------------
// Desc:   initialize an index buffer with input data
//---------------------------------------------------------
void MeshGeometry::InitIndexBuffer(
    ID3D11Device* pDevice,
    const UINT* indices,
    int numIndices)
{
    CAssert::True(indices != nullptr, "input ptr to indices arr == nullptr");
    CAssert::True(numIndices > 0,     "input number of indices must be > 0");

    constexpr bool isDynamic = false;
    ib_.Shutdown();
    ib_.Initialize(pDevice, indices, numIndices, isDynamic);
}

//---------------------------------------------------------
// Desc:   set a name for subset by Id (identifier is the same as subset's idx)
//---------------------------------------------------------
void MeshGeometry::SetSubsetName(const SubsetID subsetId, const char* inName)
{
    if (subsetId >= numSubsets_)
    {
        LogErr(LOG, "subset id is invalid: %" PRIu16, subsetId);
        return;
    }

    if (StrHelper::IsEmpty(inName))
    {
        LogErr(LOG, "input name is empty");
        return;
    }

    size_t len = strlen(inName);

    // if input name is too long we limit its length
    if (len > MAX_LEN_MESH_NAME-1)
        len = MAX_LEN_MESH_NAME-1;

    // set new name
    char* name = subsets_[subsetId].name;
    strncpy(name, inName, len);
    name[len] = '\0';
}


//---------------------------------------------------------
// Desc:   setup local space matrix for subset (submesh) by id (its idx)
//---------------------------------------------------------
void MeshGeometry::SetSubsetLSpaceMatrix(const SubsetID subsetId, const DirectX::XMMATRIX& m)
{
    if (subsetId >= numSubsets_)
    {
        LogErr(LOG, "subset id is invalid: %" PRIu16, subsetId);
        return;
    }

    localSpaceMatrices_[subsetId] = m;
}

//---------------------------------------------------------
// Desc:   setup a material for subset by ID (index)
//---------------------------------------------------------
void MeshGeometry::SetMaterialForSubset(const SubsetID subsetId, const MaterialID matID)
{
    // check input args
    if (subsetId < 0 || subsetId > numSubsets_)
    {
        LogErr(LOG, "input ID to subset is invalid (its value: %" PRIu16 ")", subsetId);
        return;
    }

    if (matID == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "input ID to material is invalid (is equal 0)");
        return;
    }

    subsets_[subsetId].materialId = matID;
}

//---------------------------------------------------------
// Desc:   setup materials for subsets by ID
//---------------------------------------------------------
void MeshGeometry::SetMaterialsForSubsets(
    const SubsetID* subsetsIDs,
    const MaterialID* materialsIDs,
    const size count)
{

    if (!CheckInputParamsForMaterialsSetting(subsetsIDs, materialsIDs, count))
    {
        LogErr(LOG, "can't set materials for subsets: invalid input args");
        return;
    }

    for (index i = 0; i < count; ++i)
        subsets_[subsetsIDs[i]].materialId = materialsIDs[i];
}

//---------------------------------------------------------
// Desc:   check if input subsets IDs and materials IDs are valid
//---------------------------------------------------------
bool MeshGeometry::CheckInputParamsForMaterialsSetting(
    const SubsetID* subsetsIDs,
    const MaterialID* materialsIDs,
    const size count)
{
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
            isValid &= (materialsIDs[i] != INVALID_MATERIAL_ID);

        return isValid;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        return false;
    }
}

} // namespace Core
