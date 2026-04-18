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

        result = d3d_.Init(
            hwnd,
            params.vsyncEnabled,
            params.fullscreen,
            params.enable4xMSAA,
            params.nearZ,
            params.farZ);
        CAssert::True(result, "can't initialize the D3DClass");

        result = InitConstBuffers(params);
        CAssert::True(result, "can't init const buffers");

        result = InitInstancesBuffer();
        CAssert::True(result, "can't init instances buffer");

        result = InitSamplers();
        CAssert::True(result, "can't init samplers");

        pShaderMgr_ = NEW ShaderMgr();
        CAssert::NotNullptr(pShaderMgr_, "can't alloc memory from a shader manager");

        result = pShaderMgr_->Init(GetDevice(), GetContext(), params.worldViewOrtho);
        CAssert::True(result, "can't init shaders manager");
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
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
bool CRender::InitConstBuffers(const InitParams& params)
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

        HRESULT                hr = S_FALSE;
        ID3D11Device*     pDevice = GetDevice();
        ID3D11DeviceContext* pCtx = GetContext();

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

        hr = cbGrass_.Init(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (grass)");


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
        cbViewProj_.ApplyChanges(pCtx);
        cbCamera_.ApplyChanges(pCtx);
        cbWeather_.ApplyChanges(pCtx);
        cbTime_.ApplyChanges(pCtx);
        cbDebug_.ApplyChanges(pCtx);
        cbGrass_.ApplyChanges(pCtx);

        cbvsWorldAndViewProj_.ApplyChanges(pCtx);
        cbvsWorldViewProj_.ApplyChanges(pCtx);
        cbvsWorldViewOrtho_.ApplyChanges(pCtx);
        cbvsWorldInvTranspose_.ApplyChanges(pCtx);
        cbvsSkinned_.ApplyChanges(pCtx);
        cbvsSprite_.ApplyChanges(pCtx);

        cbpsPerFrame_.ApplyChanges(pCtx);
        cbpsRareChanged_.ApplyChanges(pCtx);
        cbpsTerrainMaterial_.ApplyChanges(pCtx);
        cbpsFontPixelColor_.ApplyChanges(pCtx);
        cbpsMaterial_.ApplyChanges(pCtx);
        cbpsPostFx_.ApplyChanges(pCtx);


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
            cbGrass_.Get(),                         // slot_7: grass parameters
            cbDebug_.Get(),                         // slot_8: different stuff for debugging
            cbViewProj_.Get(),                      // slot_9: view * proj matrix
            cbTime_.Get(),                          // slot_10: delta time and game time
            cbCamera_.Get(),                        // slot_11: camera params (matrices, camera position, etc.)
            cbWeather_.Get()                        // slot_12: weather params (wind direction, wind strength, etc.)
        };	

        // const buffers for geometry shaders
        ID3D11Buffer* gsCBs[] =
        {
            nullptr,                                // slot_0:
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
        pCtx->VSSetConstantBuffers(0, numBuffersVS, vsCBs);
        pCtx->GSSetConstantBuffers(0, numBuffersGS, gsCBs);
        pCtx->PSSetConstantBuffers(0, numBuffersPS, psCBs);


        LogMsg(LOG, "all the const buffers are initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't initialize constant buffers");
        return false;
    } 
}

//---------------------------------------------------------
// Desc:   setup and create instances buffer
//---------------------------------------------------------
bool CRender::InitInstancesBuffer()
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

    HRESULT hr = GetDevice()->CreateBuffer(&vbd, nullptr, &pInstancedBuffer_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create an instanced buffer");
        return false;
    }

    LogMsg(LOG, "instances buffer for entities is initialized successfully!");
    return true;
}

//---------------------------------------------------------
// Desc:   setup and create sampler states
//---------------------------------------------------------
bool CRender::InitSamplers()
{
    bool               result = false;
    ID3D11Device*     pDevice = GetDevice();
    ID3D11DeviceContext* pCtx = GetContext();

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
    if (!samplerAnisotrop_.Init(pDevice))
    {
        LogErr(LOG, "can't init an anisotropic sampler state");
        return false;
    }

    if (!samplerSky_.Init(pDevice, &skySamplerDesc))
    {
        LogErr(LOG, "can't init a sky sampler state");
        return false;
    }

    if (!samplerLinearClamp_.Init(pDevice, &linearClampDesc))
    {
        LogErr(LOG, "can't init a linear clamp sampler");
        return false;
    }

    if (!samplerLinearWrap_.Init(pDevice, &linearWrapDesc))
    {
        LogErr(LOG, "can't init a linear wrap sampler");
        return false;
    }


    //
    // bind samplers
    //

    // bind to vertex shader stage
    pCtx->VSSetSamplers(1, 1, samplerSky_.GetAddressOf());
    pCtx->VSSetSamplers(2, 1, samplerLinearClamp_.GetAddressOf());
    pCtx->VSSetSamplers(3, 1, samplerLinearWrap_.GetAddressOf());
    pCtx->VSSetSamplers(4, 1, samplerAnisotrop_.GetAddressOf());


    // set basic sampler (is used as default for pixel shaders)
    pCtx->VSSetSamplers(0, 1, samplerAnisotrop_.GetAddressOf());
    pCtx->PSSetSamplers(0, 1, samplerAnisotrop_.GetAddressOf());

    // bind to pixel shader stage
    pCtx->PSSetSamplers(1, 1, samplerSky_.GetAddressOf());
    pCtx->PSSetSamplers(2, 1, samplerLinearClamp_.GetAddressOf());
    pCtx->PSSetSamplers(3, 1, samplerLinearWrap_.GetAddressOf());
    pCtx->PSSetSamplers(4, 1, samplerAnisotrop_.GetAddressOf());   // duplicate anisotropic at this slot


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

//---------------------------------------------------------
// Desc:  call it to print out a code of HRESULT
//---------------------------------------------------------
void CRender::PrintHRESULT(HRESULT hr)
{
    _com_error err(hr);
    char errMsg[512];

    memset(errMsg, 0, sizeof(errMsg));
    StrHelper::ToStr(err.ErrorMessage(), errMsg);

    LogErr(LOG, "HRESULT: 0x%08X - %s\n\n", hr, errMsg);
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
        LogErr(LOG, e.what());
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
    assert(worlds);
    assert(matColors);
    assert(count > 0);
  
    ID3D11DeviceContext* pCtx = GetContext();
    D3D11_MAPPED_SUBRESOURCE mappedData;

    // map the instanced buffer to write into it
    HRESULT hr = pCtx->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't map the instanced buffer");
        return;
    }

    ConstBufType::InstancedData* data = (ConstBufType::InstancedData*)mappedData.pData;

    // write data into the subresource
    for (int i = 0; i < count; ++i)
    {
        data[i].world = worlds[i];
    }

    for (int i = 0; i < count; ++i)
        data[i].matColors = matColors[i];

    // unmap the buffer
    pCtx->Unmap(pInstancedBuffer_, 0);
}


// =================================================================================
// =================================================================================

//---------------------------------------------------------
// bind render targets and depth-stencil view to the output-merger stage
//---------------------------------------------------------
void CRender::SetRenderTargets(
    const UINT numViews,
    ID3D11RenderTargetView* const* ppRTVs,
    ID3D11DepthStencilView* pDSV)
{
    static ID3D11RenderTargetView* pRTV = nullptr;
    if (numViews == 1)
    {
        // avoid calling GAPI and binding the same render target
        if (pRTV == ppRTVs[0])
            return;

        pRTV = ppRTVs[0];
    }

    GetContext()->OMSetRenderTargets(numViews, ppRTVs, pDSV);
}

//---------------------------------------------------------
// clear input render targets
//---------------------------------------------------------
void CRender::ClearRenderTargets(const UINT numViews, ID3D11RenderTargetView* const* ppRTVs)
{
    assert(ppRTVs);

    ID3D11DeviceContext* pCtx = GetContext();
    const float clearColor[4] = { 0,0,0,1 };

    for (UINT i = 0; i < numViews; ++i)
        pCtx->ClearRenderTargetView(ppRTVs[i], clearColor);
}

//---------------------------------------------------------
// if we will do any postFX stuff we have to call this func before
// the color/lights pass to bind the proper render target view
//---------------------------------------------------------
void CRender::SetRenderTargetsForPostFX()
{
    Render::D3DClass& d3d = GetD3D();
    ID3D11RenderTargetView* pRTV = nullptr;

    // for MSAA we need to bind sufficient render target view (RTV)
    if (d3d.IsEnabled4xMSAA())
        pRTV = d3d.pMSAARTV_;

    // bind non-MSAA render target
    else
        pRTV = d3d.postFxsPassRTV_[0];

    assert(pRTV && "for post process: RTV tex == NULL");

    SetRenderTargets(1, &pRTV, d3d.pDepthStencilView_);
    ClearRenderTargets(1, &pRTV);
}

//---------------------------------------------------------
// Desc:  setup rendering states
// Args:  - rsId:   id of rasterizer state
//        - bsId:   id of blending state
//        - dssId:  id of depth-stencil state
//---------------------------------------------------------
void CRender::SetRenderStates(
    const bool alphaClip,
    const RsID rsId,
    const BsID bsId,
    const DssID dssId)
{
    static bool  prevAlphaClip = 0;
    static RsID  prevRsId = 0;
    static BsID  prevBsId = 0;
    static DssID prevDssId = 0;

    // switch alpha clipping if need...
    if (prevAlphaClip != alphaClip)
    {
        SwitchAlphaClipping(alphaClip);
        prevAlphaClip = alphaClip;
    }

    // switch raster state (fill mode, cull mode, etc.) if need...
    if (prevRsId != rsId)
    {
        GetRenderStates().SetRs(rsId);
        prevRsId = rsId;
    }

    // switch blending state if need...
    if (prevBsId != bsId)
    {
        GetRenderStates().SetBs(bsId);
        prevBsId = bsId;
    }

    // switch depth-stencil state if need...
    if (prevDssId != dssId)
    {
        GetRenderStates().SetDss(dssId, 0);
        prevDssId = dssId;
    }
}

//---------------------------------------------------------
// Desc:  bind shader resource views to a texture slots in VERTEX shader
//---------------------------------------------------------
void CRender::SetTexturesVS(
    const UINT startSlot,
    const UINT numViews,
    ID3D11ShaderResourceView* const* ppSRVs)
{
    GetContext()->VSSetShaderResources(startSlot, numViews, ppSRVs);
}

//---------------------------------------------------------
// Desc:  bind shader resource views to a texture slots in PIXEl shader
//---------------------------------------------------------
void CRender::SetTexturesPS(
    const UINT startSlot,
    const UINT numViews,
    ID3D11ShaderResourceView* const* ppSRVs)
{
    GetContext()->PSSetShaderResources(startSlot, numViews, ppSRVs);
}

// =================================================================================
//                               rendering methods
// =================================================================================

void CRender::RenderInstances(
    ID3D11Buffer* pVB,
    ID3D11Buffer* pIB,
    ID3D11Buffer* pInstancedBuf,
    const UINT vertexStride,
    const UINT instanceStride,
    const DXGI_FORMAT ibFormat,
    const UINT indexCount,
    const UINT numInstances,
    const UINT indexStart,
    const UINT vertexStart,
    const UINT startInstanceLocation)
{
    assert(pVB);
    assert(pIB);
    assert(pInstancedBuf);
    assert(vertexStride > 0);
    assert(instanceStride > 0);

    ID3D11Buffer* const vbs[2] = { pVB, pInstancedBuf };
    const UINT strides[2] = { vertexStride, instanceStride };
    const UINT offsets[2] = { 0, 0 };

    ID3D11DeviceContext* pCtx = GetContext();
    pCtx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    pCtx->IASetIndexBuffer(pIB, ibFormat, 0);

    pCtx->DrawIndexedInstanced(
        indexCount,
        numInstances,
        indexStart,
        vertexStart,
        startInstanceLocation);
}

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
    SetPrimTopology(instances.primTopology);
    BindShaderById(instances.shaderId);

    RenderInstances(
        instances.pVB,                          // vertex buffer
        instances.pIB,                          // index buffer
        pInstancedBuffer_,
        instances.vertexStride,
        sizeof(ConstBufType::InstancedData),    // instanceStride
        DXGI_FORMAT_R32_UINT,                   // index buffer DXGI format
        instances.subset.indexCount,
        instances.numInstances,
        instances.subset.indexStart,
        instances.subset.vertexStart,
        startInstanceLocation);
}

//---------------------------------------------------------
// Desc:    depth pre-pass rendering of input instances batch
//---------------------------------------------------------
void CRender::DepthPrepassInstances(
    const InstanceBatch& instances,
    const UINT startInstanceLocation)
{
    pShaderMgr_->DepthPrePassBindShaderById(GetContext(), instances.shaderId);
    RenderInstances(instances, startInstanceLocation);
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
    ID3D11DeviceContext* pCtx = GetContext();

    for (int i = 0; i < numSpritesToRender_; ++i)
    {
        const Sprite2D& sprite = spritesRenderList_[i];

        // update params which we will use to generate a 2D sprite (in geometry shader)
        cbvsSprite_.data.left   = sprite.left;
        cbvsSprite_.data.top    = sprite.top;
        cbvsSprite_.data.width  = sprite.width;
        cbvsSprite_.data.height = sprite.height;
        cbvsSprite_.ApplyChanges(pCtx);

        pCtx->PSSetShaderResources(101U, 1, &sprite.pSRV);  // bind texture
        pCtx->Draw(1, 0);                                   // render
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
            DrawIndexed(indexCounts[idx], 0, 0);
        }
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't render font");
    }
}

//---------------------------------------------------------
// Desc:  if we have only one post effect in the post process passes queue we
//        can render it directly into the swap chain's RTV
//---------------------------------------------------------
void CRender::RenderPostFxOnePass(const ePostFxType pfxType)
{
    Render::D3DClass&    d3d = GetD3D();
    ID3D11DeviceContext* pCtx = GetContext();

    // bind dst render target and src shader resource view
    assert(d3d.pSwapChainRTV_     && "swap chain RTV is wrong");
    assert(d3d.postFxsPassSRV_[0] && "resolved SRV is wrong");

    pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
    pCtx->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

    BindPostFxShader(pfxType);
    pCtx->Draw(3, 0);

    // bind a depth stencil view back
    pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
}

//---------------------------------------------------------
// Desc:  when we have multiple post process passes we have to flip btw
//        two render targets each time when render a pass;
//        for the final pass we do rendering into the swap chain't RTV
//---------------------------------------------------------
void CRender::RenderPostFxs(const ePostFxType* pfxTypes, const uint8 numPfxsToRender)
{
    assert(pfxTypes);
    assert(numPfxsToRender > 0);

    const Render::D3DClass& d3d = GetD3D();

    if (numPfxsToRender == 1)
    {
        // final pass we render directly into swap chain's RTV
        BindPostFxShader(pfxTypes[0]);
        SetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
        SetTexturesPS(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);
        Draw(3, 0);

        // bind depth stencil back
        SetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
    }

    // we have multiple post FXs in queue
    else
    {
        SetRenderTargets(1, &d3d.postFxsPassRTV_[1], nullptr);
        SetTexturesPS(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

        int i = 0;
        int lastDstIdx = 0;


        for (i = 0; i < numPfxsToRender - 1; ++i)
        {
            // by this index we will get a source SRV for the last post process pass
            lastDstIdx = (i % 2 == 0);

            BindPostFxShader(pfxTypes[i]);
            Draw(3, 0);

            // flip render targets and shader resource views
            // (one target becomes a dst, and another becomes a src)
            const int nextTargetIdx = (i % 2 != 0);
            const int nextSrcIdx    = (i % 2 == 0);

            SetRenderTargets(1, &d3d.postFxsPassRTV_[nextTargetIdx], nullptr);
            SetTexturesPS(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[nextSrcIdx]);
        }

        // final pass we render directly into swap chain's RTV
        BindPostFxShader(pfxTypes[i]);
        SetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
        SetTexturesPS(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[lastDstIdx]);
        Draw(3, 0);

        // bind depth stencil back
        SetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
    }
}

//---------------------------------------------------------
// Desc:  execute Fast Approximate Anti-Aliasing (FXAA)
//---------------------------------------------------------
void CRender::RenderFXAA()
{
    // reset raster state, blend state, depth-stencil state to default
    ResetRenderStates();

    Render::D3DClass& d3d = GetD3D();
    assert(d3d.pSwapChainRTV_ && "swap chain RTV is wrong");
    assert(d3d.postFxsPassSRV_[0] && "resolved SRV is wrong");

    // bind dst render target (+ unbind depth stencil view) and src shader resource view
    SetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
    SetTexturesPS(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

    BindShaderByName("FXAA");
    Draw(3, 0);

    // bind depth stencil view back
    SetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
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
    if ((dist < 0) || (dist > cbGrass_.data.distGrassVisible))
    {
        LogErr(LOG, "can't set distance for full sized grass (input dist == %f; must be <= %f)", dist, cbGrass_.data.distGrassVisible);
        return false;
    }

    cbGrass_.data.distGrassFullSize = dist;
    cbGrass_.ApplyChanges(GetContext());
    return true;
}

//---------------------------------------------------------
// Desc:   set a distance after which we don't see any grass
//---------------------------------------------------------
bool CRender::SetGrassDistVisible(const float dist)
{
    if ((dist < 0) || (dist < cbGrass_.data.distGrassFullSize))
    {
        LogErr(LOG, "can't set grass visibility distance (input dist == %f; must be >= %f)", dist, cbGrass_.data.distGrassFullSize);
        return false;
    }

    cbGrass_.data.distGrassVisible = dist;
    cbGrass_.ApplyChanges(GetContext());
    return true;
}

//---------------------------------------------------------
// Desc:  setup a number of grass texture rows and columns on a texture atlas
//---------------------------------------------------------
void CRender::SetGrassTexRowsCols(const uint cols, const uint rows)
{
    if (rows == 0 || rows > 4 || cols == 0 || rows > 4)
    {
        LogErr(LOG, "invalid grass texture atlas params (rows: %u,  columns: %u)", rows, cols);
        return;
    }

    // prevent useless updating of the const buffer
    if ((uint)cbGrass_.data.numTexColumns == cols &&
        (uint)cbGrass_.data.numTexRows    == rows)
        return;

    cbGrass_.data.numTexColumns = (float)cols;
    cbGrass_.data.numTexRows    = (float)rows;
    cbGrass_.ApplyChanges(GetContext());
    return;
}

// =================================================================================
// Fog control
// =================================================================================

//---------------------------------------------------------
// Desc:   turn on/off the fog effect
//---------------------------------------------------------
void CRender::SetFogEnabled(const bool onOff)
{
    cbWeather_.data.fogEnabled = onOff;
    cbWeather_.ApplyChanges(GetContext());
}

//---------------------------------------------------------
// Desc:   setup where the for starts
//---------------------------------------------------------
void CRender::SetFogStart(const float startDist)
{
    cbWeather_.data.fogStart = (startDist > 0) ? startDist : 0.0f;
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
// Desc:  are we currently using the fog?
//---------------------------------------------------------
bool CRender::IsFogEnabled() const
{
    return cbWeather_.data.fogEnabled;
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
    enabled = cbWeather_.data.fogEnabled;
}

//---------------------------------------------------------
// Desc:  return a distance after which all the geometry
//        is completely fogged
//---------------------------------------------------------
float CRender::GetDistFogged(void) const
{
    return cbWeather_.data.fogStart + cbWeather_.data.fogRange;
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
// Desc:  return shader id/name by input name/id
//---------------------------------------------------------
ShaderID CRender::GetShaderIdByName(const char* shaderName) const
{
    assert(!StrHelper::IsEmpty(shaderName));
    return pShaderMgr_->GetShaderIdByName(shaderName);
}

const char* CRender::GetShaderNameById(const ShaderID id) const
{
    return pShaderMgr_->GetShaderNameById(id);
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
    {
        LogErr(LOG, "no shader by id: %d", (int)id);
        return;
    }

    pShaderMgr_->BindShader(GetContext(), pShader);

    currShaderId_ = id;
    strcpy(currShaderName_, pShader->GetName());
}

//---------------------------------------------------------
// Desc:  bind a shader by input name
//---------------------------------------------------------
void CRender::BindShaderByName(const char* shaderName)
{
    assert(!StrHelper::IsEmpty(shaderName));

    if (strcmp(currShaderName_, shaderName) == 0)
        return;

    Shader* pShader = pShaderMgr_->GetShaderByName(shaderName);
    if (!pShader)
    {
        LogErr(LOG, "no shader by name: %s", shaderName);
        return;
    }

    pShaderMgr_->BindShader(GetContext(), pShader);

    currShaderId_ = pShader->GetId();
    strcpy(currShaderName_, shaderName);
}


//---------------------------------------------------------
// Desc:  bind a shader according to the input post effect's type
//---------------------------------------------------------
void CRender::BindPostFxShader(const ePostFxType fxType)
{
    switch (fxType)
    {
        case POST_FX_VISUALIZE_DEPTH:
            BindShaderByName("DepthResolveShader");
            GetContext()->Draw(3, 0);
            BindShaderByName("VisualizeDepthShader");
            break;

        case POST_FX_GRAYSCALE:
        case POST_FX_INVERT_COLORS:
        case POST_FX_BRIGHT_CONTRAST_ADJ:
        case POST_FX_SEPIA:
        case POST_FX_CHROMATIC_ABERRATION:
        case POST_FX_COLOR_TINT:
        case POST_FX_VIGNETTE_EFFECT:
        case POST_FX_BLOOM_BRIGHT_EXTRACT:
        case POST_FX_EDGE_DETECTION:
        case POST_FX_POSTERIZATION:
        case POST_FX_FILM_GRAIN:
        case POST_FX_CRT_SCANLINES:
        case POST_FX_PIXELATION:
        case POST_FX_COLOR_SHIFT:
        case POST_FX_NEGATIVE_GLOW:
        case POST_FX_THERMAL_VISION:
        case POST_FX_NIGHT_VISION:
        case POST_FX_HEAT_DISTORTION:
        case POST_FX_SHOCKWAVE_DISTORTION:
        case POST_FX_FROST_GLASS_BLUR:
        case POST_FX_OLD_TV_DISTORTION:
        case POST_FX_COLOR_SPLIT:
        case POST_FX_RADIAL_BLUR:
        case POST_FX_SWIRL_DISTORTION:
        case POST_FX_GLITCH:
        case POST_FX_DITHERING_ORDERED:
        {
            const char* shaderName = g_PostFxShaderName[fxType];
            BindShaderByName(shaderName);
            break;
        }

        // gaussian blur we do in 2 passes
        case POST_FX_GAUSSIAN_BLUR:
        {
            const char* shaderName = g_PostFxShaderName[fxType];
            BindShaderByName(shaderName);
            break;
        }

        //-----------------------------------

        default:
            LogErr(LOG, "wrong post effect type (maybe you add a new postFx but forgot to add a new case here?): %d", (int)fxType);
            return;
    }
}

//---------------------------------------------------------
// Desc:   visualize values from the depth buffer (we do it after usual rendering)
//---------------------------------------------------------
void CRender::VisualizeDepthBuffer()
{
    Render::D3DClass& d3d = GetD3D();

    // unbind depth before depth visualization
    d3d.UnbindDepthBuffer();

    if (d3d.IsEnabled4xMSAA())
    {
        ID3D11ShaderResourceView* pSRV_DepthMSAA = d3d.GetDepthSRV();
        SetTexturesPS(TEX_SLOT_DEPTH_MSAA, 1, &pSRV_DepthMSAA);

        BindShaderByName("DepthResolveShader");
        Draw(3, 0);
    }
    else
    {
        // setup depth SRV
        ID3D11ShaderResourceView* pDepthSRV = d3d.GetDepthSRV();
        SetTexturesPS(TEX_SLOT_DEPTH, 1, &pDepthSRV);

        BindShaderByName("VisualizeDepthShader");
        Draw(3, 0);
    }

    // after rendering we bind depth buffer again
    d3d.BindDepthBuffer();
}

}; // namespace
