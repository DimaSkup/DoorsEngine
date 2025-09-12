// =================================================================================
// Filename:     CRender.h
// Description:  this class is responsible for rendering all the 
//               graphics onto the screen;
// 
// Created:      02.12.22 (moved into the Render module at 29.08.24)
// =================================================================================
#pragma once

#include "../Common/ConstBufferTypes.h"
#include "../Common/RenderTypes.h"

#include "../Shaders/ShaderMgr.h"
#include "../Shaders/SamplerState.h"        // for using the ID3D11SamplerState 
#include "../Shaders/ConstantBuffer.h"

#include <d3d11.h>
#include <DirectXMath.h>


namespace Render
{

//---------------------------------------------------------
// Desc:  this struct contains init params for the rendering
//---------------------------------------------------------
struct InitParams
{
    
    // for 2D rendering
    DirectX::XMMATRIX worldViewOrtho = DirectX::XMMatrixIdentity();

    // fog params
    DirectX::XMFLOAT3 fogColor{ 0.5f, 0.5f, 0.5f };
    float             fogStart = 50;                 // a distance where the fog starts
    float             fogRange = 200;                // start+range = after this distance all the objects are fully fogged

    MaterialColors    terrainMatColors;
};

//---------------------------------------------------------
// Class
//---------------------------------------------------------
class CRender
{
public:
    CRender();
    ~CRender();

    // restrict a copying of this class instance
    CRender(const CRender& obj) = delete;
    CRender& operator=(const CRender& obj) = delete;


    // initialize the rendering subsystem
    bool Initialize(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const InitParams& params);

    bool InitConstBuffers(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const InitParams& params);

    bool InitInstancesBuffer(ID3D11Device* pDevice);
    bool InitSamplers       (ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

    bool ShadersHotReload(ID3D11Device* pDevice);


    // ================================================================================
    //                                   Updating 
    // ================================================================================
    void UpdatePerFrame       (ID3D11DeviceContext* pContext, const PerFrameData& data);
    void UpdateInstancedBuffer(ID3D11DeviceContext* pContext, const InstancesBuf& buf);

    void UpdateInstancedBuffer(
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX* worlds,
        const MaterialColors* matColors,
        const int count);


    // ================================================================================
    //                                  Rendering
    // ================================================================================

    void RenderInstances(
        ID3D11DeviceContext* pContext,
        const InstanceBatch& instances,
        const UINT startInstanceLocation);

    void RenderSkyDome(
        ID3D11DeviceContext* pContext,
        const SkyInstance& sky,
        const DirectX::XMMATRIX& worldViewProj);

    void RenderFont(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* const* vertexBuffers,
        ID3D11Buffer* pIndexBuffer,
        const uint32* indexCounts,
        const size numVertexBuffers,
        const UINT fontVertexStride,
        SRV* const* ppFontTexSRV);


    // ================================================================================
    //                                   Getters
    // ================================================================================

    inline void GetFogData(DirectX::XMFLOAT3& color, float& start, float& range, bool& enabled)
    {
        // cbps - const buffer for pixel shader
        color   = cbpsRareChanged_.data.fogColor;
        start   = cbpsRareChanged_.data.fogStart;
        range   = cbpsRareChanged_.data.fogRange;
        enabled = cbpsRareChanged_.data.fogEnabled;
    }

    inline const DirectX::XMFLOAT3& GetSkyCenterColor() const
    {
        return cbpsRareChanged_.data.colorCenter_;
    }

    inline const DirectX::XMFLOAT3& GetSkyApexColor() const
    {
        return cbpsRareChanged_.data.colorApex_;
    }


    // ================================================================================
    //                                   Setters
    // ================================================================================
    void SetFogEnabled      (ID3D11DeviceContext* pContext, const bool state);
    void SetFogStart        (ID3D11DeviceContext* pContext, const float start);
    void SetFogRange        (ID3D11DeviceContext* pContext, const float range);
    void SetFogColor        (ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3 color);

    void SwitchFlashLight   (ID3D11DeviceContext* pContext, const bool state);

    void SwitchAlphaClipping(ID3D11DeviceContext* pContext, const bool state);
    void SwitchDebugState   (ID3D11DeviceContext* pContext, const eDebugState state);
    void SetDirLightsCount  (ID3D11DeviceContext* pContext, int numOfLights);

    void SetDebugFontColor(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);

    void SetSkyGradient(
        ID3D11DeviceContext* pContext,
        const DirectX::XMFLOAT3& colorCenter,
        const DirectX::XMFLOAT3& colorApex);

    void SetSkyColorCenter(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);
    void SetSkyColorApex  (ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);


    //-----------------------------------------------------
    void SetViewProj(
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX& viewProj);

    void SetWorldAndViewProj(
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX& world,
        const DirectX::XMMATRIX& viewProj);

    void SetWorldViewProj(
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX& wvp);

    void SetWorldViewOrtho(
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX& WVO);

    // GRASS
    float GetGrassDistFullSize() const { return cbgsGrassParams_.data.distGrassFullSize; }
    float GetGrassDistVisible()  const { return cbgsGrassParams_.data.distGrassVisible; }

    bool SetGrassDistFullSize(const float dist);
    bool SetGrassDistVisible(const float dist);

private:
    void UpdateLights(
        const DirLight* dirLights,
        const PointLight* pointLights,
        const SpotLight* spotLights,
        const int numDirLights,
        const int numPointLights,
        const int numSpotLights);

public:
    ShaderMgr                                       shaderMgr_;

    ID3D11DeviceContext*                            pContext_         = nullptr;
    ID3D11Buffer*                                   pInstancedBuffer_ = nullptr;
    cvector<ConstBufType::InstancedData>            instancedData_;    // instances common buffer


    // const buffers for vertex shaders
    ConstantBuffer<ConstBufType::ViewProj>          cbvsViewProj_;
    ConstantBuffer<ConstBufType::WorldAndViewProj>  cbvsWorldAndViewProj_;
    ConstantBuffer<ConstBufType::WorldViewProj>     cbvsWorldViewProj_;
    ConstantBuffer<ConstBufType::WorldViewOrtho>    cbvsWorldViewOrtho_;

    // const buffers for geometry shaders
    ConstantBuffer<ConstBufType::GS_PerFrame>       cbgsPerFrame_;
    ConstantBuffer<ConstBufType::GrassParams>       cbgsGrassParams_;

    // const buffers for pixel shaders
    ConstantBuffer<ConstBufType::cbpsPerFrame>            cbpsPerFrame_;
    ConstantBuffer<ConstBufType::cbpsRareChanged>         cbpsRareChanged_;
    ConstantBuffer<ConstBufType::MaterialData>            cbpsTerrainMaterial_;
    ConstantBuffer<ConstBufType::ConstBuf_FontPixelColor> cbpsFontPixelColor_;
    ConstantBuffer<ConstBufType::DebugMode>               cbpsDebugMode_;
    ConstantBuffer<ConstBufType::MaterialData>            cbpsMaterial_;

    RenderDataStorage dataStorage_;
    PerFrameData      perFrameData_;                              // we need to keep this data because we use it multiple times during the frame

    // samplers for texturing
    SamplerState basicSampler_;          
    SamplerState skySampler_;          


    bool isDebugMode_ = false;                       // do we use the debug shader?
};

}; // namespace Render
