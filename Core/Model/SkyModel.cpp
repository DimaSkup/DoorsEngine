// =================================================================================
// Filename:      SkyModel.cpp
// Description:   a class for the sky model (because it is a specific model)
// 
// Created:       23.12.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "SkyModel.h"
#pragma warning (disable : 4996)


namespace Core
{

SkyModel::SkyModel()  {}
SkyModel::~SkyModel() {}

///////////////////////////////////////////////////////////

bool SkyModel::InitializeBuffers(
	ID3D11Device* pDevice,
	const Vertex3DPos* vertices,
	const USHORT* indices,
	const int numVertices,
	const int numIndices)
{
	// initialize vertex and index buffers with input data
    try
    {
        CAssert::True(vertices, "input ptr to arr of vertices == nullptr");
        CAssert::True(indices, "input ptr to arr of indices == nullptr");
        CAssert::True(numVertices > 0, "input number of vertices must be > 0");
        CAssert::True(numIndices > 0, "input number of indices must be > 0");

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

///////////////////////////////////////////////////////////

void SkyModel::SetName(const char* newName)
{
	// set new name if it is valid or don't change in another case
    if ((newName == nullptr) || (newName[0] == '\0'))
    {
        LogErr("can't set a new name for the sky model: input name is empty");
        return;
    }

    const size_t sz = strlen(newName);
    const size_t size = (sz > 32) ? 32 : sz;    // trim length if necessary to 32

    strncpy(name_, newName, size);
}

///////////////////////////////////////////////////////////

void SkyModel::SetTexture(const int idx, const TexID texID)
{
	CAssert::True((idx > -1) && (idx < maxTexNum_), "wrong data to set texture");
	texIDs_[idx] = texID;
}

} // namespace Core
