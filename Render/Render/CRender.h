// =================================================================================
// Filename:     CRender.h
// Description:  this class is responsible for rendering all the 
//               graphics onto the screen;
// 
// Created:      02.12.22 (moved into the Render module at 29.08.24)
// =================================================================================
#pragma once

#include <enum_rnd_debug_type.h>
#include <enum_weather_params.h>

#include "../Common/ConstBufferTypes.h"
#include "../Common/RenderTypes.h"

#include "../Shaders/SamplerState.h"        // for using the ID3D11SamplerState 
#include "../Shaders/ConstantBuffer.h"

#include "d3dclass.h"

#include <math/vec4.h>
#include <d3d11.h>
#include <DirectXMath.h>


namespace Render
{
// forward declaration
class ShaderMgr;


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

    bool              vsyncEnabled    = false;
    bool              fullscreen      = true;
    bool              enable4xMSAA    = false;
    float             nearZ           = 0.01f;
    float             farZ            = 100.0f;
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
    bool Init(HWND hwnd, const InitParams& params);
    void Shutdown();

    bool ShadersHotReload();

    inline D3DClass&            GetD3D()          { return d3d_; }
    inline RenderStates&        GetRenderStates() { return d3d_.renderStates_; }
    inline ID3D11Device*        GetDevice()       { return Render::g_pDevice; }
    inline ID3D11DeviceContext* GetContext()      { return Render::g_pContext; }


    // ================================================================================
    //                                   Updating 
    // ================================================================================
    void Update();

    void UpdatePerFrame       (const PerFrameData& data);
    void UpdateInstancedBuffer(const InstancesBuf& buf);

    void UpdateInstancedBuffer(
        const DirectX::XMMATRIX* worlds,
        const MaterialColors* matColors,
        const int count);

    // ================================================================================
    //                                  Binders
    // ================================================================================
    inline void BindVB(ID3D11Buffer* const* ppVB, const UINT stride, const UINT offset)
    {
        GetContext()->IASetVertexBuffers(0, 1, ppVB, &stride, &offset);
    }

    inline void BindIB(ID3D11Buffer* pIB, const DXGI_FORMAT format)
    {
        GetContext()->IASetIndexBuffer(pIB, format, 0);
    }

    void BindShaderById(const ShaderID id);
    void BindShaderByName(const char* shaderName);
   
    inline void SetPrimTopology(const D3D_PRIMITIVE_TOPOLOGY type)
    {
        if (currPrimTopology_ == type)
            return;

        currPrimTopology_ = type;
        GetContext()->IASetPrimitiveTopology(type);
    }


    // ================================================================================
    //                                  Rendering
    // ================================================================================

    void PushSpriteToRender(const Render::Sprite2D& sprite);
    void Render2dSprites();

    void RenderInstances(
        const InstanceBatch& instances,
        const UINT startInstanceLocation);

    void DepthPrepassInstances(
        const InstanceBatch& instances,
        const UINT startInstanceLocation);

    void RenderSkyDome(
        const SkyInstance& sky,
        const DirectX::XMMATRIX& worldViewProj);

    void RenderFont(
        ID3D11Buffer* const* vertexBuffers,
        ID3D11Buffer* pIndexBuffer,
        const uint32* indexCounts,
        const size numVertexBuffers,
        const UINT fontVertexStride);


    // ================================================================================
    //                                   Getters
    // ================================================================================
    void  GetFogData(DirectX::XMFLOAT3& color, float& start, float& range, bool& enabled);

    const DirectX::XMFLOAT3& GetSkyCenterColor() const;
    const DirectX::XMFLOAT3& GetSkyApexColor()   const;

    bool ShaderExist       (const ShaderID id) const;
    void GetArrShadersIds  (cvector<ShaderID>& outIds) const;
    void GetArrShadersNames(cvector<ShaderName>& outNames) const;


    // ================================================================================
    //                                   Setters
    // ================================================================================
    void SetFogEnabled      (const bool state);
    void SetFogStart        (const float start);
    void SetFogRange        (const float range);
    void SetFogColor        (const DirectX::XMFLOAT3 color);

    void  UpdateCbWeatherParam(const eWeatherParam param, const float val);
    float GetWeatherParam     (const eWeatherParam param) const;

    void SwitchFlashLight   (const bool state);

    void SwitchAlphaClipping   (const bool state);
    void UpdateCbDirLightsCount(const uint numOfLights);

    void SetDebugFontColor(const DirectX::XMFLOAT3& color);

    void SetSkyGradient(
        const DirectX::XMFLOAT3& colorCenter,
        const DirectX::XMFLOAT3& colorApex);

    void SetSkyColorCenter(const DirectX::XMFLOAT3& color);
    void SetSkyColorApex  (const DirectX::XMFLOAT3& color);

    void UpdateCbBoneTransforms(const cvector<DirectX::XMMATRIX>& boneTransforms);

    void UpdateCbDebug(const uint currBoneId);

    void UpdateCbMaterialColors(
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,
        const Vec4& reflect);

    void UpdateCbTerrainMaterial(
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,
        const Vec4& reflect);

    //-----------------------------------------------------
   
    void UpdateCbWorldAndViewProj(
        const DirectX::XMMATRIX& world,
        const DirectX::XMMATRIX& viewProj);

    void UpdateCbWorld            (const DirectX::XMMATRIX& world);
    void UpdateCbViewProj         (const DirectX::XMMATRIX& viewProj);
    void UpdateCbWorldViewProj    (const DirectX::XMMATRIX& wvp);
    void UpdateCbWorldViewOrtho   (const DirectX::XMMATRIX& WVO);
    void UpdateCbWorldInvTranspose(const DirectX::XMMATRIX& worldInvTranspose);

    void UpdateCbDebugType(const eRndDbgType type);

    void UpdateCbTime(const float deltaTime, const float gameTime);

    void UpdateCbSky(
        const float cloud1TranslationX,
        const float cloud1TranslationZ,
        const float cloud2TranslationX,
        const float cloud2TranslationZ,
        const float cloudBrightness);

    void UpdateCbCamera(
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& proj,
        const DirectX::XMMATRIX& invView,
        const DirectX::XMMATRIX& invProj,
        const DirectX::XMFLOAT3& cameraPos,
        const float nearZ,
        const float farZ);

    void UpdateCbCameraPos(const DirectX::XMFLOAT3& pos);
    DirectX::XMFLOAT3 GetCameraPos() const;

    void  SetPostFxParam(const uint16 postFxParamIdx, const float value);
    float GetPostFxParam(const uint16 postFxParamIdx);

    // GRASS
    inline float GetGrassDistFullSize() const { return cbgsGrassParams_.data.distGrassFullSize; }
    inline float GetGrassDistVisible()  const { return cbgsGrassParams_.data.distGrassVisible; }

    bool SetGrassDistFullSize(const float dist);
    bool SetGrassDistVisible(const float dist);

private:
    bool InitConstBuffers(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const InitParams& params);

    bool InitInstancesBuffer(ID3D11Device* pDevice);
    bool InitSamplers       (ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

    void UpdateLights(
        const DirLight* dirLights,
        const PointLight* pointLights,
        const SpotLight* spotLights,
        uint numDirLights,
        uint numPointLights,
        uint numSpotLights);

public:
    Render::D3DClass                                d3d_;

    ID3D11DeviceContext*                            pContext_         = nullptr;
    ID3D11Buffer*                                   pInstancedBuffer_ = nullptr;
    cvector<ConstBufType::InstancedData>            instancedData_;    // instances common buffer

    // common const buffers
    ConstantBuffer<ConstBufType::ViewProj>          cbViewProj_;
    ConstantBuffer<ConstBufType::Time>              cbTime_;
    ConstantBuffer<ConstBufType::CameraParams>      cbCamera_;
    ConstantBuffer<ConstBufType::Weather>           cbWeather_;
    ConstantBuffer<ConstBufType::Debug>             cbDebug_;

    // const buffers for vertex shaders
    ConstantBuffer<ConstBufType::WorldAndViewProj>  cbvsWorldAndViewProj_;
    ConstantBuffer<ConstBufType::WorldViewProj>     cbvsWorldViewProj_;
    ConstantBuffer<ConstBufType::WorldViewOrtho>    cbvsWorldViewOrtho_;
    ConstantBuffer<ConstBufType::WorldInvTranspose> cbvsWorldInvTranspose_;
    ConstantBuffer<ConstBufType::Skinned>           cbvsSkinned_;
    ConstantBuffer<ConstBufType::Sprite>            cbvsSprite_;

    // const buffers for geometry shaders
    ConstantBuffer<ConstBufType::GrassParams>       cbgsGrassParams_;

    // const buffers for pixel shaders
    ConstantBuffer<ConstBufType::cbpsPerFrame>            cbpsPerFrame_;
    ConstantBuffer<ConstBufType::cbpsRareChanged>         cbpsRareChanged_;
    ConstantBuffer<ConstBufType::MaterialData>            cbpsTerrainMaterial_;
    ConstantBuffer<ConstBufType::ConstBuf_FontPixelColor> cbpsFontPixelColor_;
    ConstantBuffer<ConstBufType::MaterialData>            cbpsMaterial_;
    ConstantBuffer<ConstBufType::Sky>                     cbpsSky_;
    ConstantBuffer<ConstBufType::PostFx>                  cbpsPostFx_;

    RenderDataStorage   dataStorage_;
    PerFrameData        perFrameData_;                                 // we need to keep this data because we use it multiple times during the frame

    // samplers for texturing
    SamplerState        samplerAnisotrop_;
    SamplerState        samplerSky_;
    SamplerState        samplerLinearClamp_;
    SamplerState        samplerLinearWrap_;

    bool                isDebugMode_      = false;                     // do we use the debug shader?
    bool                isChangedPostFxs_ = false;                     // did we change any post effect parameter?
    
    ShaderID            currShaderId_ = INVALID_SHADER_ID;             // id of the currently bounded shader
    char                currShaderName_[MAX_LEN_SHADER_NAME]{'\0'};    // name of the currently bounded shader

    D3D11_PRIMITIVE_TOPOLOGY currPrimTopology_ = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

private:
    ShaderMgr* pShaderMgr_ = nullptr;

    Sprite2D spritesRenderList_[32];
    int      numSpritesToRender_ = 0;
};

}; // namespace Render
