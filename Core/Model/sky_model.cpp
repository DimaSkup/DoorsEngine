// =================================================================================
// Filename:      SkyModel.cpp
// Description:   a class for the sky model (because it is a specific model)
// 
// Created:       23.12.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "sky_model.h"
#pragma warning (disable : 4996)


namespace Core
{

SkyModel::SkyModel()
{
}

SkyModel::~SkyModel()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:   release memory
//---------------------------------------------------------
void SkyModel::Shutdown()
{
    materialId_ = INVALID_MATERIAL_ID;

    vb_.Shutdown();
    ib_.Shutdown();
}

//---------------------------------------------------------
// Desc:   initialize vertex and index buffers with input data
//---------------------------------------------------------
bool SkyModel::InitializeBuffers(
    ID3D11Device* pDevice,
    const Vertex3DPos* vertices,
    const USHORT* indices,
    const int numVertices,
    const int numIndices)
{
    try
    {
        CAssert::True(vertices,        "input ptr to arr of vertices == nullptr");
        CAssert::True(indices,         "input ptr to arr of indices == nullptr");
        CAssert::True(numVertices > 0, "input number of vertices must be > 0");
        CAssert::True(numIndices > 0,  "input number of indices must be > 0");

        constexpr bool isDynamic = false;
        vb_.Initialize(pDevice, vertices, numVertices, isDynamic);
        ib_.Initialize(pDevice, indices, numIndices, isDynamic);

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        return false;
    }
}

//---------------------------------------------------------
// Desc:   set new name if it is valid or don't change in another case
//---------------------------------------------------------
void SkyModel::SetName(const char* newName)
{
    if (StrHelper::IsEmpty(newName))
    {
        LogErr(LOG, "can't set a new name for the sky model: input name is empty");
        return;
    }

    size_t sz = strlen(newName);

    // trim length if necessary
    if (sz > MAX_LEN_SKY_MODEL_NAME-1)
        sz = MAX_LEN_SKY_MODEL_NAME-1;    

    strncpy(name_, newName, sz);
    name_[MAX_LEN_SKY_MODEL_NAME - 1] = '\0';
}

//---------------------------------------------------------
// Desc:   setup a material for the sky
//---------------------------------------------------------
void SkyModel::SetMaterialId(const MaterialID id)
{
    if (id == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "input material id is invalid");
        return;
    }

    materialId_ = id;
}

} // namespace
