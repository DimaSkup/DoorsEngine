#pragma once

#include "InputLayouts.h"
#include "../Common/RenderTypes.h"

#include <types.h>
#include <cvector.h>
#include <d3d11.h>
#include <map>


namespace Render
{

struct ShaderInitParams;
class Shader;

//-----------------------------------------------

class ShaderMgr
{
public:
    ShaderMgr();
    ~ShaderMgr();

    bool Init(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX& WVO,
        const bool forceRuntimeCompilation = false);

    bool HotReload(ID3D11Device* pDevice);

  
    void BindVB(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* pVB,
        const UINT stride,
        const UINT offset);

    void BindBuffers(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* const* pVBs,
        ID3D11Buffer* pIB,
        const UINT* pStrides,
        const UINT* pOffsets,
        int numVBs);

    // for main color-light rendering pass
    void BindShader (ID3D11DeviceContext* pContext, Shader* pShader);

    // for depth pre-pass (we need only vertex shader)
    void DepthPrePassBindShaderById  (ID3D11DeviceContext* pContext, const ShaderID id);
    void DepthPrePassBindShaderByName(ID3D11DeviceContext* pContext, const char* shaderName);
    void DepthPrePassBindShader      (ID3D11DeviceContext* pContext, Shader* pShader);

    // rendering methods
    void Render(ID3D11DeviceContext* pContext, const UINT indexCount);

    void Render(
        ID3D11DeviceContext* pContext,
        const InstanceBatch& instances,
        const UINT startInstanceLocation);


    // getters
    Shader*     GetShaderById    (const ShaderID id) const;
    Shader*     GetShaderByName  (const char* name) const;
    ShaderID    GetShaderIdByName(const char* name) const;
    const char* GetShaderNameById(const ShaderID id);

    void GetArrShadersIds  (cvector<ShaderID>& outIds) const;
    void GetArrShadersNames(cvector<ShaderName>& outNames) const;

    const VertexInputLayoutMgr& GetInputLayoutsMgr() const;

private:
    bool AddShader(ID3D11Device* pDevice, const ShaderInitParams* initParams);


private:
    VertexInputLayoutMgr         inputLayoutsMgr_;
    std::map<ShaderID, Shader*>  idToShader_;
    ShaderID                     lastShaderId_ = 0;
};

//---------------------------------------------------------
// Desc:  return a ref to the manager of vertex input layouts
//---------------------------------------------------------
inline const VertexInputLayoutMgr& ShaderMgr::GetInputLayoutsMgr() const
{
    return inputLayoutsMgr_;
}

} // namespace

