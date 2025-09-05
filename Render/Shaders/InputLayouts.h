// =================================================================================
// Filename:      InputLayouts.h
// Description:   a manager/container for vertex input layouts
//
// Created:       11.05.2025
// =================================================================================
#pragma once

#include <d3d11.h>
#include <cvector.h>


namespace Render
{

struct VertexInputLayout
{
    char name[32]{'\0'};
    cvector<D3D11_INPUT_ELEMENT_DESC> desc;
};

//---------------------------------------------------------

class VertexInputLayoutMgr
{
public:
    VertexInputLayoutMgr();

    const VertexInputLayout& GetInputLayoutByName(const char* name);

private:
    void LoadAndCreateInputLayouts();

public:
    cvector<VertexInputLayout> inputLayouts;
};

} // namespace Render
