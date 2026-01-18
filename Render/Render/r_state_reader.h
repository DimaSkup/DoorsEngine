/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: r_state_reader.h
    Desc:     functional for reading render states declarations from file
              and creating responsible D3D descriptions;

              also can convert word representation of description in D3D desc

              is used by the RenderStates class

    Created:  17.01.2026 by DimaSkup
\**********************************************************************************/
#pragma once

#include <d3d11.h>

namespace Render
{

class RenderStateReader
{
public:
    //
    // rs  - rasterizer state
    // bs  - blending state
    // dss - depth-stencil state
    //
    void LoadRsDesc (FILE* pFile, D3D11_RASTERIZER_DESC& outDesc, const bool msaaEnabled);
    void LoadBsDesc (FILE* pFile, D3D11_BLEND_DESC& ouDesc);
    void LoadDssDesc(FILE* pFile, D3D11_DEPTH_STENCIL_DESC& desc);

    // rasterizer
    D3D11_FILL_MODE GetFillMode(const char* fill);
    D3D11_CULL_MODE GetCullMode(const char* cull);

    // blending
    D3D11_BLEND_OP           GetBlendOperation(const char* op);
    D3D11_BLEND              GetBlendFactor(const char* f);
    D3D11_COLOR_WRITE_ENABLE GetRenderTargetWriteMask(const char* mask);

    // depth-stencil
    D3D11_STENCIL_OP         GetStencilOp(const char* op);
    D3D11_COMPARISON_FUNC    GetCmpFunc(const char* func);
    D3D11_DEPTH_WRITE_MASK   GetDepthWriteMask(const char* mask);

private:
    void GetRasterStateDescParam(
        const char* propName,
        const char* propValue,
        const bool msaaEnable,
        D3D11_RASTERIZER_DESC& desc);

    void ReadDssBackFaceParam (const char* propName, const char* propValue, D3D11_DEPTH_STENCIL_DESC& desc);
    void ReadDssDepthParam    (const char* propName, const char* propValue, D3D11_DEPTH_STENCIL_DESC& desc);
    void ReadDssFrontFaceParam(const char* propName, const char* propValue, D3D11_DEPTH_STENCIL_DESC& desc);
    void ReadDssStencilParam  (const char* propName, const char* propValue, D3D11_DEPTH_STENCIL_DESC& desc);
};

} // namespace
