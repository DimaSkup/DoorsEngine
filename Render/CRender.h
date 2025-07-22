// =================================================================================
// Filename:     CRender.h
// Description:  this class is responsible for rendering all the 
//               graphics onto the screen;
// 
// Created:      02.12.22 (moved into the Render module at 29.08.24)
// =================================================================================
#pragma once

#include "Common/ConstBufferTypes.h"
#include "Shaders/ShadersContainer.h"
#include "Common/RenderTypes.h"

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
    float             fogRange = 200;                // max distance after which all the objects are fogged
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

    // setup logger file ptr if we want to write logs into the file
    void SetupLogger(FILE* pFile);

    // initialize the rendering subsystem
    bool Initialize(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const InitParams& params);

    bool ShadersHotReload(ID3D11Device* pDevice);


    // ================================================================================
    //                                   Updating 
    // ================================================================================
    void UpdatePerFrame(ID3D11DeviceContext* pContext, const PerFrameData& data);

    void UpdateInstancedBuffer(
        ID3D11DeviceContext* pContext,
        const InstBuffData& data);

    void UpdateInstancedBuffer(
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX* worlds,
        const DirectX::XMMATRIX* texTransforms,
        const Material* materials,
        const int count);

    void UpdateInstancedBufferWorlds(
        ID3D11DeviceContext* pContext, 
        cvector<DirectX::XMMATRIX>& worlds);

    void UpdateInstancedBufferMaterials(
        ID3D11DeviceContext* pContext,
        cvector<Material>& materials);


    // ================================================================================
    //                                  Rendering
    // ================================================================================

    void RenderBoundingLineBoxes(
        ID3D11DeviceContext* pContext,
        const Instance* instances,
        const int numModels);

    void RenderInstances(
        ID3D11DeviceContext* pContext,
        const ShaderTypes type,
        const Instance* instances,
        const int numModels);

    void RenderSkyDome(
        ID3D11DeviceContext* pContext,
        const SkyInstance& sky,
        const DirectX::XMMATRIX& worldViewProj);




    // ================================================================================
    //                                   Getters
    // ================================================================================
    inline ShadersContainer& GetShadersContainer() { return shadersContainer_; }
    inline LightShader&      GetLightShader()      { return shadersContainer_.lightShader_; }

    inline void GetFogData(DirectX::XMFLOAT3& color, float& start, float& range, bool& enabled)
    {
        // cbps - const buffer for pixel shader
        color   = cbpsRareChanged_.data.fogColor;
        start   = cbpsRareChanged_.data.fogStart;
        range   = cbpsRareChanged_.data.fogRange;
        enabled = cbpsRareChanged_.data.fogEnabled;
    }


    // ================================================================================
    //                                   Setters
    // ================================================================================
    void SetFogEnabled      (ID3D11DeviceContext* pContext, const bool state);
    void SetFogStart        (ID3D11DeviceContext* pContext, const float start);
    void SetFogRange        (ID3D11DeviceContext* pContext, const float range);
    void SetFogColor        (ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3 color);
    void SetWorldViewOrtho  (ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& WVO);

    void SwitchFlashLight   (ID3D11DeviceContext* pContext, const bool state);

    void SwitchAlphaClipping(ID3D11DeviceContext* pContext, const bool state);
    void SwitchDebugState   (ID3D11DeviceContext* pContext, const eDebugState state);
    void SetDirLightsCount  (ID3D11DeviceContext* pContext, int numOfLights);

    void InitFogParams(
        ID3D11DeviceContext* pContext,
        const DirectX::XMFLOAT3& fogColor,
        const float fogStart,
        const float fogRange);

    void SetSkyGradient(
        ID3D11DeviceContext* pContext,
        const DirectX::XMFLOAT3& colorCenter,
        const DirectX::XMFLOAT3& colorApex);

    void SetSkyColorCenter(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);
    void SetSkyColorApex  (ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);

    void SetViewProj      (ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj);

private:
    void UpdateLights(
        const DirLight* dirLights,
        const PointLight* pointLights,
        const SpotLight* spotLights,
        const int numDirLights,
        const int numPointLights,
        const int numSpotLights);

public:
    ID3D11DeviceContext*                          pContext_ = nullptr;

    ID3D11Buffer*                                 pInstancedBuffer_ = nullptr;
    cvector<ConstBufType::InstancedData>          instancedData_;    // instances common buffer

    // const buffers for vertex shaders
    ConstantBuffer<ConstBufType::cbvsPerFrame>    cbvsPerFrame_;     
    ConstantBuffer<ConstBufType::cbpsPerFrame>    cbpsPerFrame_;    

    // const buffers for pixel shaders
    ConstantBuffer<ConstBufType::cbpsRareChanged> cbpsRareChanged_; 

    // const buffers for geometry shaders
    //ConstantBuffer<ConstBufType::GeomertyShaderConstBuf_PerObject> cbgsPerObject_;
    //ConstantBuffer<ConstBufType::GeometryShaderConstBuf_Fixed>     cbgsFixed_;
    ConstantBuffer<ConstBufType::GeometryShaderConstBuf_PerFrame>  cbgsPerFrame_;



    RenderDataStorage dataStorage_;
    PerFrameData      perFrameData_;                              // we need to keep this data because we use it multiple times during the frame
    ShadersContainer  shadersContainer_;                          // a struct with shader classes objects

    bool              isDebugMode_ = false;                       // do we use the debug shader?
};

}; // namespace Render
