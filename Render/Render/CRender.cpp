// =================================================================================
// Filename:     CRender.cpp
// Description:  there are functions for rendering graphics;
// Created:      01.01.23
// =================================================================================
#include "../Common/pch.h"
#include "CRender.h"
#include "d3dclass.h"
#include "../Shaders/Shader.h"
#include <math/dx_math_helpers.h>
#include <post_fx_enum.h>
#include "../Shaders/ShaderMgr.h"

using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMMATRIX = DirectX::XMMATRIX;


namespace Render
{

//---------------------------------------------------------
// global instance of the Render
//---------------------------------------------------------
CRender g_Render;

//---------------------------------------------------------
// constructor/destructor
//---------------------------------------------------------
CRender::CRender()
{
}

CRender::~CRender()
{
    Shutdown();
}

// =================================================================================
//                               public methods
// =================================================================================

bool CRender::Init(HWND hwnd, const InitParams& params) 
{
    try
    {
        bool result = true;
        HRESULT hr = S_OK;

        result = d3d_.Initialize(
            hwnd,
            params.vsyncEnabled,
            params.fullscreen,
            params.enable4xMSAA,
            params.nearZ,
            params.farZ);
        CAssert::True(result, "can't initialize the D3DClass");

        ID3D11Device*        pDevice  = GetDevice();
        ID3D11DeviceContext* pContext = GetContext();

        result = InitConstBuffers(pDevice, pContext, params);
        CAssert::True(result, "can't init const buffers");

        result = InitInstancesBuffer(pDevice);
        CAssert::True(result, "can't init instances buffer");

        result = InitSamplers(pDevice, pContext);
        CAssert::True(result, "can't init samplers");

        pShaderMgr_ = NEW ShaderMgr();
        CAssert::NotNullptr(pShaderMgr_, "can't alloc memory from a shader manager");

        result = pShaderMgr_->Init(pDevice, pContext, params.worldViewOrtho);
        CAssert::True(result, "can't init shaders manager");
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the CRender module");
        Shutdown();
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  clean memory from stuff
//---------------------------------------------------------
void CRender::Shutdown()
{
    d3d_.Shutdown();
    SafeDelete(pShaderMgr_);
}

//---------------------------------------------------------
// Desc:   initialize and bind constant buffers
//---------------------------------------------------------
bool CRender::InitConstBuffers(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    const InitParams& params)
{
    try
    {
        // NOTATION:
        // cbvs - const buffer for vertex shader
        // cbgs - const buffer for geometry shader
        // cbps - const buffer for pixel shader

        // check some input init params
        CAssert::True(params.fogStart > 0.0f,            "fog start distance can't be <= 0");
        CAssert::True(params.fogRange > params.fogStart, "fog range can't be < fog start");

        HRESULT hr = S_FALSE;


        // INIT COMMON CONST BUFFERS --------------------

        hr = cbViewProj_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (viewProj)");

        hr = cbTime_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (time)");

        hr = cbCamera_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (camera params)");

        hr = cbWeather_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (weather params)");

        hr = cbDebug_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (debug)");


        // INIT CONST BUFFERS FOR VERTEX SHADERS --------------------

        hr = cbvsWorldAndViewProj_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (world and viewProj; for VS)");

        hr = cbvsWorldViewProj_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (worldViewProj; for VS)");

        hr = cbvsWorldViewOrtho_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (worldViewOrtho; for VS)");

        hr = cbvsWorldInvTranspose_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (world inverse transpose; for VS)");

        hr = cbvsSkinned_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (model skinning/animation)");

        hr = cbvsSprite_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (2D sprites)");


        // INIT CONST BUFFERS FOR GEOMETRY SHADERS ------------------

        hr = cbgsGrassParams_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (grass params; for GS)");

      

        // INIT CONST BUFFERS FOR PIXEL SHADERS ---------------------

        hr = cbpsPerFrame_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (per frame; for PS)");

        hr = cbpsRareChanged_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (rare changed; for PS)");

        hr = cbpsTerrainMaterial_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (terrain material; for PS)");

        hr = cbpsFontPixelColor_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (font pixel color; for PS)");

        hr = cbpsMaterial_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (material: for PS)");

        hr = cbpsSky_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (sky: for PS)");

        hr = cbpsPostFx_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (postFX: for PS)");


        // SETUP SOME CONST BUFFERS ---------------------------------

        cbvsWorldViewOrtho_.data.worldViewOrtho = params.worldViewOrtho;

        cbvsWorldInvTranspose_.data.worldInvTranspose = DirectX::XMMatrixIdentity();

        // since fog props is changed very rarely we setup rare changed cbps (const buffer for pixel shader)
        cbWeather_.data.fogColor = params.fogColor;
        cbWeather_.data.fogStart = params.fogStart;
        cbWeather_.data.fogRange = params.fogRange;

        // setup the material colors for terrain
        cbpsTerrainMaterial_.data.ambient  = params.terrainMatColors.ambient;
        cbpsTerrainMaterial_.data.diffuse  = params.terrainMatColors.diffuse;
        cbpsTerrainMaterial_.data.specular = params.terrainMatColors.specular;
        cbpsTerrainMaterial_.data.reflect  = params.terrainMatColors.reflect;

        // setup 2D font color
        cbpsFontPixelColor_.data.pixelColor = { 1,1,1 };

        // setup time params
        cbTime_.data.deltaTime = 0;
        cbTime_.data.gameTime = 0;

        // setup camera's params
        cbCamera_.data.nearPlane = params.nearZ;
        cbCamera_.data.farPlane  = params.farZ;

        // setup post effects with initial params
        cbpsPostFx_.data.data[POST_FX_PARAM_SCREEN_WIDTH]  = 1600;
        cbpsPostFx_.data.data[POST_FX_PARAM_SCREEN_HEIGHT] = 900;

        cbpsPostFx_.data.data[POST_FX_PARAM_TEXEL_SIZE_X] = 1.0f / 1600.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_TEXEL_SIZE_Y] = 1.0f / 900.0f;

        // shockwave distortion
        cbpsPostFx_.data.data[POST_FX_PARAM_SHOCKWAVE_DISTORT_CX]        = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SHOCKWAVE_DISTORT_CY]        = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SHOCKWAVE_DISTORT_SPEED]     = 3.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SHOCKWAVE_DISTORT_THICKNESS] = 0.3f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SHOCKWAVE_DISTORT_AMPLITUDE] = 0.06f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SHOCKWAVE_DISTORT_SHARPNESS] = 6.0f;

        // glitch
        cbpsPostFx_.data.data[POST_FX_PARAM_GLITCH_INTENSITY]     = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_GLITCH_COLOR_SPLIT]   = 2.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_GLITCH_BLOCK_SIZE]    = 0.05f;
        cbpsPostFx_.data.data[POST_FX_PARAM_GLITCH_SPEED]         = 2.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_GLITCH_SCANLINE]      = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_GLITCH_NOISE_AMOUNT]  = 0.5f;

        cbpsPostFx_.data.data[POST_FX_PARAM_POSTERIZATION_LEVELS] = 6;
        cbpsPostFx_.data.data[POST_FX_PARAM_BLOOM_THRESHOLD]      = 0.5f;

        // brightness/contrast adjust
        cbpsPostFx_.data.data[POST_FX_PARAM_BRIGHTNESS] = 0.2f;
        cbpsPostFx_.data.data[POST_FX_PARAM_CONTRAST] = 1.5f;

        cbpsPostFx_.data.data[POST_FX_PARAM_CHROMATIC_ABERRATION_STRENGTH] = 0.1f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SHIFT_HUE]               = 1.3f;

        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_CX]           = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_CY]           = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_INTENSITY]    = 0.1f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_RADIAL_POWER] = 1.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_R] = 1.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_G] = 1.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_B] = 1.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_SPLIT_SAMPLES]      = 5.0f;

        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_TINT_R] = 1.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_TINT_G] = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_TINT_B] = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_COLOR_TINT_INTENSITY] = 1.0f;

        cbpsPostFx_.data.data[POST_FX_PARAM_CRT_CURVATURE]             = 0.2f;
        cbpsPostFx_.data.data[POST_FX_PARAM_FILM_GRAIN_STRENGTH]       = 0.2f;
        cbpsPostFx_.data.data[POST_FX_PARAM_FROST_GLASS_BLUR_STRENGTH] = 5.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_HEAT_DISTORT_STRENGTH]     = 0.01f;
        cbpsPostFx_.data.data[POST_FX_PARAM_NEGATIVE_GLOW_STRENGTH]    = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_NEGATIVE_GLOW_THRESHOLD]   = 0.7f;

        cbpsPostFx_.data.data[POST_FX_PARAM_PIXELATION_PIXEL_SIZE]  = 4.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_RADIAL_BLUR_CX]         = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_RADIAL_BLUR_CY]         = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_RADIAL_BLUR_SAMPLES]    = 10.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_RADIAL_BLUR_STRENGTH]   = 0.02f;

        cbpsPostFx_.data.data[POST_FX_PARAM_SWIRL_DISTORT_CX]       = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SWIRL_DISTORT_CY]       = 0.5f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SWIRL_DISTORT_RADIUS]   = 1.0f;
        cbpsPostFx_.data.data[POST_FX_PARAM_SWIRL_DISTORT_ANGLE]    = 1.3f;

        cbpsPostFx_.data.data[POST_FX_PARAM_OLD_TV_DISTORT_STRENGTH] = 0.005f;
        cbpsPostFx_.data.data[POST_FX_PARAM_VIGNETTE_STRENGTH]       = 0.75f;

        // setup default bones transformations (model skinning)
        for (int i = 0; i < MAX_NUM_BONES_PER_CHARACTER; ++i)
        {
            cbvsSkinned_.data.boneTransforms[i] = DirectX::XMMatrixIdentity();
        }


        // load data for const buffers into GPU
        cbViewProj_.ApplyChanges(pContext);
        cbCamera_.ApplyChanges(pContext);
        cbWeather_.ApplyChanges(pContext);
        cbTime_.ApplyChanges(pContext);
        cbDebug_.ApplyChanges(pContext);

        cbvsWorldAndViewProj_.ApplyChanges(pContext);
        cbvsWorldViewProj_.ApplyChanges(pContext);
        cbvsWorldViewOrtho_.ApplyChanges(pContext);
        cbvsWorldInvTranspose_.ApplyChanges(pContext);
        cbvsSkinned_.ApplyChanges(pContext);
        cbvsSprite_.ApplyChanges(pContext);

        cbgsGrassParams_.ApplyChanges(pContext);

        cbpsPerFrame_.ApplyChanges(pContext);
        cbpsRareChanged_.ApplyChanges(pContext);
        cbpsTerrainMaterial_.ApplyChanges(pContext);
        cbpsFontPixelColor_.ApplyChanges(pContext);
        cbpsMaterial_.ApplyChanges(pContext);
        cbpsPostFx_.ApplyChanges(pContext);


        // BIND CONST BUFFERS ---------------------------------------

        // NOTE: we bind some const buffers to the same slot for each stage
        //       (since they are common for each)

        // const buffers for vertex shaders
        ID3D11Buffer* vsCBs[] =
        {
            cbvsWorldAndViewProj_.Get(),            // slot_0: world and viewProj matrix
            cbvsWorldViewProj_.Get(),               // slot_1: world_view_proj matrix
            cbvsWorldViewOrtho_.Get(),              // slot_2: (for 2D rendering) world * basicView * ortho matrix 
            cbvsWorldInvTranspose_.Get(),           // slot_3: world inverse transpose
            cbvsSkinned_.Get(),                     // slot_4: bones transformations (model skinning)
            cbvsSprite_.Get(),                      // slot_5: 2D sprites
            nullptr,                                // slot_6:
            nullptr,                                // slot_7:
            cbDebug_.Get(),                         // slot_8: different stuff for debugging
            cbViewProj_.Get(),                      // slot_9: view * proj matrix
            cbTime_.Get(),                          // slot_10: delta time and game time
            cbCamera_.Get(),                        // slot_11: camera params (matrices, camera position, etc.)
            cbWeather_.Get()                        // slot_12: weather params (wind direction, wind strength, etc.)
        };	

        // const buffers for geometry shaders
        ID3D11Buffer* gsCBs[] =
        {
            cbgsGrassParams_.Get(),                 // slot_0: parameters for grass rendering
            nullptr,                                // slot_1:
            nullptr,                                // slot_2:
            nullptr,                                // slot_3:
            nullptr,                                // slot_4:
            nullptr,                                // slot_5:
            nullptr,                                // slot_6:
            nullptr,                                // slot_7:
            nullptr,                                // slot_8:
            cbViewProj_.Get(),                      // slot_9:  view * proj matrix
            cbTime_.Get(),                          // slot_10: delta time and game time
            cbCamera_.Get(),                        // slot_11: camera params (matrices, camera position, etc.)
            cbWeather_.Get()                        // slot_12: weather params (wind direction, wind strength, etc.)
        };

        // const buffers for pixel shaders
        ID3D11Buffer* psCBs[] =
        {
            cbpsPerFrame_.Get(),                    // slot_0: params which are changed each frame
            cbpsRareChanged_.Get(),                 // slot_1: rare changed params
            cbpsTerrainMaterial_.Get(),             // slot_2: material color data for terrain
            cbpsFontPixelColor_.Get(),              // slot_3: font (2D text) pixel color
            nullptr,                                // slot_4:
            cbpsMaterial_.Get(),                    // slot_5: material color data (common const buffer)
            cbpsSky_.Get(),                         // slot_6: sky data
            cbpsPostFx_.Get(),                      // slot_7: post effects params
            nullptr,                                // slot_8:
            nullptr,                                // slot_9: 
            cbTime_.Get(),                          // slot_10: delta time and game time
            cbCamera_.Get(),                        // slot_11: camera params (matrices, camera position, etc.)
            cbWeather_.Get()                        // slot_12: weather params (wind direction, wind strength, etc.)
        };

        const UINT numBuffersVS = sizeof(vsCBs) / sizeof(ID3D11Buffer*);
        const UINT numBuffersGS = sizeof(gsCBs) / sizeof(ID3D11Buffer*);
        const UINT numBuffersPS = sizeof(psCBs) / sizeof(ID3D11Buffer*);

        // bind constant buffers 
        pContext->VSSetConstantBuffers(0, numBuffersVS, vsCBs);
        pContext->GSSetConstantBuffers(0, numBuffersGS, gsCBs);
        pContext->PSSetConstantBuffers(0, numBuffersPS, psCBs);


        LogMsg(LOG, "all the const buffers are initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize constant buffers");
        return false;
    } 
}

//---------------------------------------------------------
// Desc:   setup and create instances buffer
//---------------------------------------------------------
bool CRender::InitInstancesBuffer(ID3D11Device* pDevice)
{
    constexpr UINT maxInstancesNum = 9000;
    D3D11_BUFFER_DESC vbd;
    memset(&vbd, 0, sizeof(vbd));

    // setup buffer's description
    vbd.Usage               = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth           = (UINT)(sizeof(ConstBufType::InstancedData) * maxInstancesNum);
    vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags           = 0;
    vbd.StructureByteStride = 0;

    HRESULT hr = pDevice->CreateBuffer(&vbd, nullptr, &pInstancedBuffer_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create an instanced buffer");
        return false;
    }

    LogMsg(LOG, "all the const buffers are initialized");
    return true;
}

//---------------------------------------------------------
// Desc:   setup and create sampler states
//---------------------------------------------------------
bool CRender::InitSamplers(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    bool result = false;


    // sampler with anisotropic filtering
    // (it is recreated when we change anisotropy max level: MaxAnisotropy)
    D3D11_SAMPLER_DESC samAnisotropDesc = {};
    samAnisotropDesc.Filter         = D3D11_FILTER_ANISOTROPIC;
    samAnisotropDesc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
    samAnisotropDesc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
    samAnisotropDesc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
    samAnisotropDesc.BorderColor[0] = 1.0f;
    samAnisotropDesc.BorderColor[1] = 0.0f;
    samAnisotropDesc.BorderColor[2] = 0.0f;
    samAnisotropDesc.BorderColor[3] = 1.0f;
    samAnisotropDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samAnisotropDesc.MinLOD         = 0.0f;
    samAnisotropDesc.MaxLOD         = D3D11_FLOAT32_MAX;
    samAnisotropDesc.MaxAnisotropy  = D3D11_REQ_MAXANISOTROPY;
    samAnisotropDesc.MipLODBias     = 0.0f;


    // a sampler is used for sampling a sky texture (it can be a cubemap)
    D3D11_SAMPLER_DESC skySamplerDesc = {};
    skySamplerDesc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    skySamplerDesc.AddressU         = D3D11_TEXTURE_ADDRESS_CLAMP;
    skySamplerDesc.AddressV         = D3D11_TEXTURE_ADDRESS_CLAMP;
    skySamplerDesc.AddressW         = D3D11_TEXTURE_ADDRESS_CLAMP;
    skySamplerDesc.BorderColor[0]   = 1.0f;
    skySamplerDesc.BorderColor[1]   = 0.0f;
    skySamplerDesc.BorderColor[2]   = 0.0f;
    skySamplerDesc.BorderColor[3]   = 1.0f;
    skySamplerDesc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
    skySamplerDesc.MinLOD           = 0.0f;
    skySamplerDesc.MaxLOD           = D3D11_FLOAT32_MAX;
    skySamplerDesc.MaxAnisotropy    = 1;
    skySamplerDesc.MipLODBias       = 0.0f;


    D3D11_SAMPLER_DESC linearClampDesc = {};
    linearClampDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // Controls interpolation between texels
    linearClampDesc.AddressU        = D3D11_TEXTURE_ADDRESS_CLAMP;      // Controls what happens when UVs go outside [0,1]
    linearClampDesc.AddressV        = D3D11_TEXTURE_ADDRESS_CLAMP; 
    linearClampDesc.AddressW        = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearClampDesc.BorderColor[0]  = 0.0f;
    linearClampDesc.BorderColor[1]  = 0.0f;
    linearClampDesc.BorderColor[2]  = 0.0f;
    linearClampDesc.BorderColor[3]  = 0.0f;
    linearClampDesc.ComparisonFunc  = D3D11_COMPARISON_NEVER;
    linearClampDesc.MinLOD          = 0.0f;                             // Restrict mip levels that can be used
    linearClampDesc.MaxLOD          = D3D11_FLOAT32_MAX;
    linearClampDesc.MaxAnisotropy   = 1;
    linearClampDesc.MipLODBias      = 0.0f;


    D3D11_SAMPLER_DESC linearWrapDesc = {};
    linearWrapDesc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linearWrapDesc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
    linearWrapDesc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
    linearWrapDesc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
    linearWrapDesc.BorderColor[0]   = 1.0f;
    linearWrapDesc.BorderColor[1]   = 1.0f;
    linearWrapDesc.BorderColor[2]   = 1.0f;
    linearWrapDesc.BorderColor[3]   = 0.0f;
    linearWrapDesc.ComparisonFunc   = D3D11_COMPARISON_NEVER;
    linearWrapDesc.MinLOD           = 0.0f;
    linearWrapDesc.MaxLOD           = D3D11_FLOAT32_MAX;
    linearWrapDesc.MaxAnisotropy    = 1;
    linearWrapDesc.MipLODBias       = 0.0f;

    //
    // init samplers
    //
    if (!samplerAnisotrop_.Initialize(pDevice))
    {
        LogErr(LOG, "can't init an anisotropic sampler state");
        return false;
    }

    if (!samplerSky_.Initialize(pDevice, &skySamplerDesc))
    {
        LogErr(LOG, "can't init a sky sampler state");
        return false;
    }

    if (!samplerLinearClamp_.Initialize(pDevice, &linearClampDesc))
    {
        LogErr(LOG, "can't init a linear clamp sampler");
        return false;
    }

    if (!samplerLinearWrap_.Initialize(pDevice, &linearWrapDesc))
    {
        LogErr(LOG, "can't init a linear wrap sampler");
        return false;
    }


    //
    // bind samplers
    //

    // bind to vertex shader stage
    pContext->VSSetSamplers(1, 1, samplerSky_.GetAddressOf());
    pContext->VSSetSamplers(2, 1, samplerLinearClamp_.GetAddressOf());
    pContext->VSSetSamplers(3, 1, samplerLinearWrap_.GetAddressOf());
    pContext->VSSetSamplers(4, 1, samplerAnisotrop_.GetAddressOf());


    // set basic sampler (is used as default for pixel shaders)
    pContext->PSSetSamplers(0, 1, samplerAnisotrop_.GetAddressOf());

    // bind to pixel shader stage
    pContext->PSSetSamplers(1, 1, samplerSky_.GetAddressOf());
    pContext->PSSetSamplers(2, 1, samplerLinearClamp_.GetAddressOf());
    pContext->PSSetSamplers(3, 1, samplerLinearWrap_.GetAddressOf());
    pContext->PSSetSamplers(4, 1, samplerAnisotrop_.GetAddressOf());   // duplicate anisotropic at this slot


    LogMsg(LOG, "all the sampler states are initialized");
    return true;
}

//---------------------------------------------------------
// Desc:   execute hot reload of shaders (runtime reinitialization)
//---------------------------------------------------------
bool CRender::ShadersHotReload()
{
    return pShaderMgr_->HotReload(GetDevice());
}


// =================================================================================
//                               updating methods
// =================================================================================

//---------------------------------------------------------
// Desc:  update all the render shit 
//---------------------------------------------------------
void CRender::Update()
{
    if (isChangedPostFxs_)
    {
        cbpsPostFx_.ApplyChanges(Render::g_pContext);
        isChangedPostFxs_ = false;
    }

    // reset the sprites render list before the next frame
    numSpritesToRender_ = 0;
}

//---------------------------------------------------------
// Desc:  update const buffer for this frame
//---------------------------------------------------------
void CRender::UpdatePerFrame(const PerFrameData& data)
{
    try 
    {
        // update light sources data
        UpdateLights(
            data.dirLights,
            data.pointLights,
            data.spotLights,
            data.numDirLights,
            data.numPointLights,
            data.numSpotLights);

        // after all we apply updates
        cbpsPerFrame_.ApplyChanges(GetContext());
    }
    catch (EngineException& e)
    {
        LogErr(e);
    }
    catch (...)
    {
        LogErr(LOG, "something went wrong during updating data for rendering");
    }
}

//---------------------------------------------------------
// Desc:   fill in the instanced buffer with data from input transient buffer
//---------------------------------------------------------
void CRender::UpdateInstancedBuffer(const InstancesBuf& buf)
{
    UpdateInstancedBuffer(
        buf.worlds_,
        buf.materials_,
        buf.GetSize());     // get the number of elements to render
}

//---------------------------------------------------------
// Desc:   fill in the instanced buffer with input data
//---------------------------------------------------------
void CRender::UpdateInstancedBuffer(
    const DirectX::XMMATRIX* worlds,
    const MaterialColors* matColors,
    const int count)
{
    try
    {
        CAssert::True(worlds != nullptr,    "input arr of world matrices == nullptr");
        CAssert::True(matColors != nullptr, "input arr of materials == nullptr");
        CAssert::True(count > 0,            "input number of elements must be > 0");

        ID3D11DeviceContext* pContext = GetContext();

        // map the instanced buffer to write into it
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        CAssert::NotFailed(hr, "can't map the instanced buffer");

        ConstBufType::InstancedData* dataView = (ConstBufType::InstancedData*)mappedData.pData;

        // write data into the subresource
        for (int i = 0; i < count; ++i)
        {
            dataView[i].world = worlds[i];
        }

        for (int i = 0; i < count; ++i)
            dataView[i].matColors = matColors[i];

        pContext->Unmap(pInstancedBuffer_, 0);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't update instanced buffer for rendering");
    }
}


// =================================================================================
//                               rendering methods
// =================================================================================

//---------------------------------------------------------
// Desc:    render input instances batch onto the screen
// Args:    - instances:             batch of instances to render
//          - startInstanceLocation: from where we get instances data
//                                   (world matrices, materials) in the instances buf
//---------------------------------------------------------
void CRender::RenderInstances(
    const InstanceBatch& instances,
    const UINT startInstanceLocation)
{
    try
    {
        const UINT instancesBuffElemSize = (UINT)(sizeof(ConstBufType::InstancedData));

        // bind vertex/index buffers for these instances
        ID3D11Buffer* const vbs[2] = { instances.pVB, pInstancedBuffer_ };
        const UINT strides[2] = { instances.vertexStride, instancesBuffElemSize };
        const UINT offsets[2] = { 0,0 };

        ID3D11DeviceContext* pContext = Render::g_pContext;
        pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
        pContext->IASetIndexBuffer(instances.pIB, DXGI_FORMAT_R32_UINT, 0);

        BindShaderById(instances.shaderId);
        pShaderMgr_->Render(pContext, instances, startInstanceLocation);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't render the mesh instances onto the screen");
    }
    catch (...)
    {
        LogErr(LOG, "can't render mesh instances for some unknown reason :(");
    }
}

//---------------------------------------------------------
// Desc:    depth pre-pass rendering of input instances batch
//---------------------------------------------------------
void CRender::DepthPrepassInstances(
    const InstanceBatch& instances,
    const UINT startInstanceLocation)
{
    constexpr UINT instancesBuffElemSize = (UINT)(sizeof(ConstBufType::InstancedData));

    // bind vertex/index buffers for these instances
    ID3D11Buffer* const vbs[2] = { instances.pVB, pInstancedBuffer_ };
    const UINT strides[2]      = { instances.vertexStride, instancesBuffElemSize };
    const UINT offsets[2]      = { 0,0 };

    ID3D11DeviceContext* pContext = Render::g_pContext;

    pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    pContext->IASetIndexBuffer(instances.pIB, DXGI_FORMAT_R32_UINT, 0);

    
    pShaderMgr_->DepthPrePassBindShaderById(pContext, instances.shaderId);
    pShaderMgr_->Render(pContext, instances, startInstanceLocation);
}

//---------------------------------------------------------
// Desc:   render sky dome (or sphere/box) onto screen
//---------------------------------------------------------
void CRender::RenderSkyDome(
    const SkyInstance& sky,
    const XMMATRIX& worldViewProj)
{
    // prepare input assembler (IA) and update const buffers
    BindVB(&sky.pVB, sky.vertexStride, 0);
    BindIB(sky.pIB, DXGI_FORMAT_R16_UINT);
    UpdateCbWorldViewProj(worldViewProj);

    // bind shader and render sky
    BindShaderByName("SkyDomeShader");
    pShaderMgr_->Render(GetContext(), sky.indexCount);
}

//---------------------------------------------------------
// Desc:  push a new sprite into the sprites render list
//---------------------------------------------------------
void CRender::PushSpriteToRender(const Render::Sprite2D& sprite)
{
    if (numSpritesToRender_ >= MAX_NUM_2D_SPRITES)
        return;

    // check a texture
    if (sprite.pSRV == nullptr)
        return;

    spritesRenderList_[numSpritesToRender_++] = sprite;
}

//---------------------------------------------------------
// Desc:  render 2D sprites onto the screen
//---------------------------------------------------------
void CRender::Render2dSprites()
{
    BindShaderByName("SpriteShader");
    ID3D11DeviceContext* pContext = GetContext();

    for (int i = 0; i < numSpritesToRender_; ++i)
    {
        const Sprite2D& sprite = spritesRenderList_[i];

        // update params which we will use to generate a 2D sprite (in geometry shader)
        cbvsSprite_.data.left   = sprite.left;
        cbvsSprite_.data.top    = sprite.top;
        cbvsSprite_.data.width  = sprite.width;
        cbvsSprite_.data.height = sprite.height;
        cbvsSprite_.ApplyChanges(pContext);

        pContext->PSSetShaderResources(101U, 1, &sprite.pSRV);  // bind texture
        pContext->Draw(1, 0);                                   // render
    }
}

//---------------------------------------------------------
// Desc:   render 2D text onto the screen
//---------------------------------------------------------
void CRender::RenderFont(
    ID3D11Buffer* const* vertexBuffers,
    ID3D11Buffer* pIndexBuf,
    const uint32* indexCounts,
    const size numVertexBuffers,
    const UINT fontVertexStride)
{
    try
    {
        CAssert::True(vertexBuffers,        "input ptr to vertex buffers arr == nullptr");
        CAssert::True(pIndexBuf,            "input ptr to index buffer == nullptr");
        CAssert::True(indexCounts,          "input ptr to index counts arr == nullptr");
        CAssert::True(numVertexBuffers > 0, "input number of vertex buffers must be > 0");

        BindShaderByName("FontShader");
        BindIB(pIndexBuf, DXGI_FORMAT_R32_UINT);

        // render each set of text
        for (index idx = 0; idx < numVertexBuffers; ++idx)
        {
            BindVB(&vertexBuffers[idx], fontVertexStride, 0);
            pShaderMgr_->Render(GetContext(), indexCounts[idx]);
        }
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't render using the shader");
        return;
    }
}


// =================================================================================
//                              effects control
// =================================================================================

//---------------------------------------------------------
// turn on/off the flashlight
//---------------------------------------------------------
void CRender::SwitchFlashLight(const bool state)
{
    cbpsRareChanged_.data.turnOnFlashLight = state;
    cbpsRareChanged_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// turn on/off alpha clipping
//---------------------------------------------------------
void CRender::SwitchAlphaClipping(const bool state)
{
    cbpsRareChanged_.data.alphaClipping = state;
    cbpsRareChanged_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  set how many directional lights we used for lighting of the scene
//---------------------------------------------------------
void CRender::UpdateCbDirLightsCount(const uint numOfLights)
{
    constexpr uint maxNumDirLights = 3;

    if (numOfLights > maxNumDirLights)
    {
        LogErr(LOG, "wrong number (%d) of directed lights (must be in range [0, %d])", (int)numOfLights, (int)maxNumDirLights);
        return;
    }

    cbpsPerFrame_.data.currNumDirLights = numOfLights;
    cbpsPerFrame_.ApplyChanges(GetContext());
}

// =================================================================================
// Grass control
// =================================================================================

//---------------------------------------------------------
// Desc:   set a distance around camera where grass planes are in full size
//---------------------------------------------------------
bool CRender::SetGrassDistFullSize(const float dist)
{
    const float distVisible = cbgsGrassParams_.data.distGrassVisible;

    if ((dist < 0) || (dist > distVisible))
    {
        LogErr(LOG, "can't set distance for full sized grass (input dist == %f; must be <= %f)", dist, distVisible);
        return false;
    }

    cbgsGrassParams_.data.distGrassFullSize = dist;
    cbgsGrassParams_.ApplyChanges(GetContext());
    return true;
}

//---------------------------------------------------------
// Desc:   set a distance after which we don't see any grass
//---------------------------------------------------------
bool CRender::SetGrassDistVisible(const float dist)
{
    const float distFullSize = cbgsGrassParams_.data.distGrassFullSize;

    if ((dist < 0) || (dist < distFullSize))
    {
        LogErr(LOG, "can't set grass visibility distance (input dist == %f; must be >= %f)", dist, distFullSize);
        return false;
    }

    cbgsGrassParams_.data.distGrassVisible = dist;
    cbgsGrassParams_.ApplyChanges(GetContext());
    return true;
}


// =================================================================================
// Fog control
// =================================================================================

//---------------------------------------------------------
// Desc:   turn on/off the fog effect
//---------------------------------------------------------
void CRender::SetFogEnabled(const bool state)
{
    cbpsRareChanged_.data.fogEnabled = state;
    cbpsRareChanged_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   setup where the for starts
//---------------------------------------------------------
void CRender::SetFogStart(const float start)
{
    cbWeather_.data.fogStart = (start > 0) ? start : 0.0f;
    cbWeather_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   setup distance after start where objects will be fully fogged
//
//         you        fog_start      <-- fog_range -->      fully_fogged
//          *-------------*-------------------------------------*
// 
//---------------------------------------------------------
void CRender::SetFogRange(const float range)
{
    cbWeather_.data.fogRange = (range > 1) ? range : 1.0f;
    cbWeather_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   setup RGB color for the fog
//---------------------------------------------------------
void CRender::SetFogColor(const DirectX::XMFLOAT3 color)
{
    cbWeather_.data.fogColor = color;
    cbWeather_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update const buffer;  setup a weather parameter with input value
//---------------------------------------------------------
void CRender::UpdateCbWeatherParam(const eWeatherParam param, const float val)
{
    switch (param)
    {
        case WEATHER_WIND_DIR_X:        cbWeather_.data.windDir.x = val;        break;
        case WEATHER_WIND_DIR_Y:        cbWeather_.data.windDir.y = val;        break;
        case WEATHER_WIND_DIR_Z:        cbWeather_.data.windDir.z = val;        break;
        case WEATHER_WIND_SPEED:        cbWeather_.data.windSpeed = val;        break;

        // swaying
        case WEATHER_WIND_STRENGTH:     cbWeather_.data.windStrength = val;     break;
        case WEATHER_WAVE_AMPLITUDE:    cbWeather_.data.waveAmplitude = val;    break;
        case WEATHER_TURBULENCE:        cbWeather_.data.turbulence = val;       break;
        case WEATHER_GUST_DECAY:        cbWeather_.data.gustDecay = val;        break;
        case WEATHER_GUST_POWER:        cbWeather_.data.gustPower = val;        break;
        case WEATHER_WAVE_FREQUENCY:    cbWeather_.data.waveFrequency = val;    break;
        case WEATHER_BEND_SCALE:        cbWeather_.data.bendScale = val;        break;
        case WEATHER_SWAY_DISTANCE:     cbWeather_.data.swayDistance = val;     break;

        default:
            LogErr(LOG, "unknown weather param: %d", (int)param);
    }

    cbWeather_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  get a weather parameter value by input type
//---------------------------------------------------------
float CRender::GetWeatherParam(const eWeatherParam param) const
{
    switch (param)
    {
        case WEATHER_WIND_DIR_X:        return cbWeather_.data.windDir.x;
        case WEATHER_WIND_DIR_Y:        return cbWeather_.data.windDir.y;
        case WEATHER_WIND_DIR_Z:        return cbWeather_.data.windDir.z;
        case WEATHER_WIND_SPEED:        return cbWeather_.data.windSpeed;

        // swaying
        case WEATHER_WIND_STRENGTH:     return cbWeather_.data.windStrength;
        case WEATHER_WAVE_AMPLITUDE:    return cbWeather_.data.waveAmplitude;
        case WEATHER_TURBULENCE:        return cbWeather_.data.turbulence;
        case WEATHER_GUST_DECAY:        return cbWeather_.data.gustDecay;
        case WEATHER_GUST_POWER:        return cbWeather_.data.gustPower;
        case WEATHER_WAVE_FREQUENCY:    return cbWeather_.data.waveFrequency;
        case WEATHER_BEND_SCALE:        return cbWeather_.data.bendScale;
        case WEATHER_SWAY_DISTANCE:     return cbWeather_.data.swayDistance;

        default:
            LogErr(LOG, "unknown weather param: %d", (int)param);
    }

    return NAN;
}


//---------------------------------------------------------
// Desc:   setup a const buffer with another color for debug text
//---------------------------------------------------------
void CRender::SetDebugFontColor(const DirectX::XMFLOAT3& color)
{
    cbpsFontPixelColor_.data.pixelColor = color;
    cbpsFontPixelColor_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
 // Desc:   setup the sky gradient from the horizon up to the apex
 // Args:   - colorCenter:   sky horizon color
 //         - colorApex:     sky top color
 //---------------------------------------------------------
void CRender::SetSkyGradient(const XMFLOAT3& colorCenter, const XMFLOAT3& colorApex)
{
    cbWeather_.data.skyColorCenter = colorCenter;
    cbWeather_.data.skyColorApex   = colorApex;
    cbWeather_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   setup only the sky horizon color for the gradient
// Args:   - color:   sky horizon color
//---------------------------------------------------------
void CRender::SetSkyColorCenter(const XMFLOAT3& color)
{
    SetSkyGradient(color, cbWeather_.data.skyColorCenter);
}

//---------------------------------------------------------
// Desc:   setup only the sky apex(top) color for the gradient
// Args:   - color:   sky apex color
//---------------------------------------------------------
void CRender::SetSkyColorApex(const XMFLOAT3& color)
{
    SetSkyGradient(cbWeather_.data.skyColorApex, color);
}

//---------------------------------------------------------
// Desc:  update a const buffer which holds bone transformations for model skinning
//---------------------------------------------------------
void CRender::UpdateCbBoneTransforms(const cvector<DirectX::XMMATRIX>& boneTransforms)
{
    assert(boneTransforms.size() <= MAX_NUM_BONES_PER_CHARACTER);

    for (index i = 0; i < boneTransforms.size(); ++i)
        cbvsSkinned_.data.boneTransforms[i] = DirectX::XMMatrixTranspose(boneTransforms[i]);

    cbvsSkinned_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update a const buffer with debug data
//---------------------------------------------------------
void CRender::UpdateCbDebug(const uint currBoneId)
{
    cbDebug_.data.currBoneId = currBoneId;
    cbDebug_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update a const buffer with new material's colors
//---------------------------------------------------------
void CRender::UpdateCbMaterialColors(
    const Vec4& ambient,
    const Vec4& diffuse,
    const Vec4& specular,
    const Vec4& reflect)
{
    cbpsMaterial_.data =
    {
        XMFLOAT4(&ambient.x),
        XMFLOAT4(&diffuse.x),
        XMFLOAT4(&specular.x),
        XMFLOAT4(&reflect.x)
    };

    cbpsMaterial_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update a const buffer with new terrain's material colors
//---------------------------------------------------------
void CRender::UpdateCbTerrainMaterial(
    const Vec4& ambient,
    const Vec4& diffuse,
    const Vec4& specular,
    const Vec4& reflect)
{
    cbpsTerrainMaterial_.data =
    {
        XMFLOAT4(&ambient.x),
        XMFLOAT4(&diffuse.x),
        XMFLOAT4(&specular.x),
        XMFLOAT4(&reflect.x)
    };

    cbpsTerrainMaterial_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   load updated light sources data into const buffers
//---------------------------------------------------------
void CRender::UpdateLights(
    const DirLight* dirLights,
    const PointLight* pointLights,
    const SpotLight* spotLights,
    uint numDirLights,
    uint numPointLights,
    uint numSpotlights)
{
    if (!dirLights || !pointLights || !spotLights)
    {
        LogErr(LOG, "can't update light sources: some of input arr == nullptr");
        return;
    }

    // we want to copy the proper number of point lights
    constexpr uint maxNumDirLights   = 3;
    constexpr uint maxNumPointLights = ARRAYSIZE(cbpsPerFrame_.data.pointLights);
    constexpr uint maxNumSpotlights  = ARRAYSIZE(cbpsPerFrame_.data.spotLights);

    Clamp(numDirLights,   0U, maxNumDirLights);
    Clamp(numPointLights, 0U, maxNumPointLights);
    Clamp(numSpotlights,  0U, maxNumSpotlights);

    cbpsPerFrame_.data.currNumDirLights   = numDirLights;
    cbpsPerFrame_.data.currNumPointLights = numPointLights;
    cbpsPerFrame_.data.currNumSpotLights  = numSpotlights;


    // update light sources data
    for (uint i = 0; i < numDirLights; ++i)
        cbpsPerFrame_.data.dirLights[i] = dirLights[i];

    for (uint i = 0; i < numPointLights; ++i)
        cbpsPerFrame_.data.pointLights[i] = pointLights[i];

    for (uint i = 0; i < numSpotlights; ++i)
        cbpsPerFrame_.data.spotLights[i] = spotLights[i];
}

//---------------------------------------------------------
// Desc:   update a const buffer with view*proj matrix
// NOTE:   the matrix must be already transposed
//---------------------------------------------------------
void CRender::UpdateCbViewProj(const DirectX::XMMATRIX& viewProj)
{
    cbViewProj_.data.viewProj = viewProj;
    cbViewProj_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   update a const buffer with 2 matrices: world and viewProj
//---------------------------------------------------------
void CRender::UpdateCbWorldAndViewProj(
    const DirectX::XMMATRIX& world,
    const DirectX::XMMATRIX& viewProj)
{
    cbvsWorldAndViewProj_.data.world = world;
    cbvsWorldAndViewProj_.data.viewProj = viewProj;
    cbvsWorldAndViewProj_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update a const buffer with new world matrix
//---------------------------------------------------------
void CRender::UpdateCbWorld(const DirectX::XMMATRIX& world)
{
    cbvsWorldAndViewProj_.data.world = world;
    cbvsWorldAndViewProj_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   update a const buffer with world_view_proj matrix (wvp)
//---------------------------------------------------------
void CRender::UpdateCbWorldViewProj(const DirectX::XMMATRIX& wvp)
{
    cbvsWorldViewProj_.data.worldViewProj = wvp;
    cbvsWorldViewProj_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   update a const buffer with world*view*ortho matrix for 2D rendering
// NOTE:   the matrix must be already transposed
//---------------------------------------------------------
void CRender::UpdateCbWorldViewOrtho(const DirectX::XMMATRIX& WVO)
{
    cbvsWorldViewOrtho_.data.worldViewOrtho = WVO;
    cbvsWorldViewOrtho_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update a const buffer with world inverse transposed matrix
//---------------------------------------------------------
void CRender::UpdateCbWorldInvTranspose(const DirectX::XMMATRIX& m)
{
    cbvsWorldInvTranspose_.data.worldInvTranspose = m;
    cbvsWorldInvTranspose_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update a const buffer with new rendering debug type
//---------------------------------------------------------
void CRender::UpdateCbDebugType(const eRndDbgType type)
{
    if (type >= NUM_RENDER_DEBUG_TYPES)
    {
        LogErr(LOG, "invalid debug type: %d", (int)type);
        return;
    }

    cbpsRareChanged_.data.debugType = (int)type;
    cbpsRareChanged_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   update a const buffer for vertex shader stage with time values 
//---------------------------------------------------------
void CRender::UpdateCbTime(const float deltaTime, const float gameTime)
{
    cbTime_.data.deltaTime = deltaTime;
    cbTime_.data.gameTime  += deltaTime;
    cbTime_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   update sky params (sky gradient, sky clouds, etc.)
//---------------------------------------------------------
void CRender::UpdateCbSky(
    const float cloud1TranslationX,
    const float cloud1TranslationZ,
    const float cloud2TranslationX,
    const float cloud2TranslationZ,
    const float cloudBrightness)
{
    cbpsSky_.data.cloud1TranslationX = cloud1TranslationX;
    cbpsSky_.data.cloud1TranslationZ = cloud1TranslationZ;
    cbpsSky_.data.cloud2TranslationX = cloud2TranslationX;
    cbpsSky_.data.cloud2TranslationZ = cloud2TranslationZ;
    cbpsSky_.data.brightness         = cloudBrightness;
    cbpsSky_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   update constant buffer with camera's params
//---------------------------------------------------------
void CRender::UpdateCbCamera(
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& proj,
    const DirectX::XMMATRIX& invView,
    const DirectX::XMMATRIX& invProj,
    const DirectX::XMFLOAT3& cameraPos,
    const float nearZ,
    const float farZ)
{
    cbCamera_.data.view       = view;
    cbCamera_.data.proj       = proj;
    cbCamera_.data.invView    = invView;
    cbCamera_.data.invProj    = invProj;
    cbCamera_.data.cameraPosW = cameraPos;
    cbCamera_.data.nearPlane  = nearZ;
    cbCamera_.data.farPlane   = farZ;
    cbCamera_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  update constant buffer ONLY with camera position
//        (all the rest values remains the same)
//---------------------------------------------------------
void CRender::UpdateCbCameraPos(const DirectX::XMFLOAT3& pos)
{
    cbCamera_.data.cameraPosW = pos;
    cbCamera_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:  get current camera position
//        (which is stored in the const buffer)
//---------------------------------------------------------
DirectX::XMFLOAT3 CRender::GetCameraPos() const
{
    return cbCamera_.data.cameraPosW;
}

//---------------------------------------------------------
// Desc:  update a post effect parameter by input type
// NOTE:  as input param we pass ePostFxParam casted to uint16
//---------------------------------------------------------
void CRender::SetPostFxParam(const uint16 fxParamIdx, const float value)
{
    const ePostFxParam paramType = (ePostFxParam)fxParamIdx;

    if (paramType >= NUM_POST_FX_PARAMS)
    {
        LogErr(LOG, "input post fx index (%d) is invalid", (int)fxParamIdx);
        return;
    }
    
    cbpsPostFx_.data.data[paramType] = value;
    isChangedPostFxs_ = true;
}

//---------------------------------------------------------
// Desc:  get a post effect parameter value by input param type
// NOTE:  as input param we pass ePostFxParam casted to uint16
//---------------------------------------------------------
float CRender::GetPostFxParam(const uint16 fxParamIdx)
{
    const ePostFxParam paramType = (ePostFxParam)fxParamIdx;

    if (paramType >= NUM_POST_FX_PARAMS)
    {
        LogErr(LOG, "input post fx index (%d) is invalid, so return 0.0f", (int)fxParamIdx);
        return 0.0f;
    }

    return cbpsPostFx_.data.data[paramType];
}

//---------------------------------------------------------
// Desc:  get fog parameters
//---------------------------------------------------------
void CRender::GetFogData(DirectX::XMFLOAT3& color, float& start, float& range, bool& enabled)
{
    // cbps - const buffer for pixel shader
    color   = cbWeather_.data.fogColor;
    start   = cbWeather_.data.fogStart;
    range   = cbWeather_.data.fogRange;
    enabled = cbpsRareChanged_.data.fogEnabled;
}

//---------------------------------------------------------

const DirectX::XMFLOAT3& CRender::GetSkyCenterColor() const
{
    return cbWeather_.data.skyColorCenter;
}

//---------------------------------------------------------

const DirectX::XMFLOAT3& CRender::GetSkyApexColor() const
{
    return cbWeather_.data.skyColorApex;
}

//---------------------------------------------------------
// Desc:  check if there is such a shader by ID
//---------------------------------------------------------
bool CRender::ShaderExist(const ShaderID id) const
{
    return (pShaderMgr_->GetShaderById(id) != nullptr);
}

//---------------------------------------------------------
// Desc:  fill in the input arr with ids of all the loaded shaders
//---------------------------------------------------------
void CRender::GetArrShadersIds(cvector<ShaderID>& outIds) const
{
    pShaderMgr_->GetArrShadersIds(outIds);
}

//---------------------------------------------------------
// Desc:  fill in the input arr with names of all the loader shaders
//---------------------------------------------------------
void CRender::GetArrShadersNames(cvector<ShaderName>& outNames) const
{
    pShaderMgr_->GetArrShadersNames(outNames);
}

//---------------------------------------------------------
// Desc:  return shader id by input name
//---------------------------------------------------------
ShaderID CRender::GetShaderIdByName(const char* shaderName) const
{
    return pShaderMgr_->GetShaderIdByName(shaderName);
}

//---------------------------------------------------------
// Desc:  bind a shader by input id
//---------------------------------------------------------
void CRender::BindShaderById(const ShaderID id)
{
    if (id == currShaderId_)
        return;

    Shader* pShader = pShaderMgr_->GetShaderById(id);
    if (!pShader)
        return;

    pShaderMgr_->BindShader(GetContext(), pShader);

    currShaderId_ = id;
    strcpy(currShaderName_, pShader->GetName());
}

//---------------------------------------------------------
// Desc:  bind a shader by input name
//---------------------------------------------------------
void CRender::BindShaderByName(const char* shaderName)
{
    if (StrHelper::IsEmpty(shaderName))
        return;

    if (strcmp(currShaderName_, shaderName) == 0)
        return;

    Shader* pShader = pShaderMgr_->GetShaderByName(shaderName);
    if (!pShader)
        return;

    pShaderMgr_->BindShader(GetContext(), pShader);

    currShaderId_ = pShader->GetId();
    strcpy(currShaderName_, shaderName);
}

}; // namespace
