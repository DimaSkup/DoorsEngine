#pragma once

#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "../Common/ConstBufferTypes.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>

namespace Render
{

class BillboardShader
{
    using SRV = ID3D11ShaderResourceView;

public:
    BillboardShader();
    ~BillboardShader();

    // restrict a copying of this class instance
    BillboardShader(const BillboardShader& obj) = delete;
    BillboardShader& operator=(const BillboardShader& obj) = delete;

    // Public modification API
    bool Initialize(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath,
        const char* gsFilePath);

    void UpdateInstancedBuffer(
        ID3D11DeviceContext* pContext,
        const Material* materials,
        const DirectX::XMFLOAT3* positions,   // positions in world space
        const DirectX::XMFLOAT2* sizes,       // sizes of billboards (width, height)
        const int numBillboards);

    void Render(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* pBillboardVB,
        ID3D11Buffer* pBillboardIB,
        SRV* const* ppTextureArrSRV,
        const UINT stride,            // vertex stride
        const DirectX::XMFLOAT3 position);

    // Public rendering API
    void Render(ID3D11DeviceContext* pContext, const Instance& instance);

    void Shutdown();

    // Public query API
    inline const char* GetShaderName() const { return className_; }

private:
    void InitializeShaders(
        ID3D11Device* pDevice,
        const char* vsFilename,
        const char* gsFilename,
        const char* psFilename);

private:
    VertexShader        vs_;
    GeometryShader      gs_;
    PixelShader         ps_;
    SamplerState        samplerState_;

    ID3D11Buffer*                               pInstancedBuffer_ = nullptr;
    cvector<BuffTypes::InstancedDataBillboards> instancedData_;

    char className_[32]{"BillboardShader"};
    const int numMaxInstances_ = 500;                     // limit of instances
    int numCurrentInstances_ = 0;                         // how many instances will we render for this frame?
};

}  // namespace Render
