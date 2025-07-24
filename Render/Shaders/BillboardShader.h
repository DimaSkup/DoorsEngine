#pragma once

#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "SamplerState.h"
//#include "../Common/ConstBufferTypes.h"
//#include "../Common/RenderTypes.h"
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

    void Render(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* pVB,
        SRV* const* ppTextureArrSRV,
        const UINT stride,
        const UINT numVertices);

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

    char className_[32]{"BillboardShader"};
};

}  // namespace Render
