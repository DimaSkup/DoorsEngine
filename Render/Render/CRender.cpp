// =================================================================================
// Filename:     CRender.cpp
// Description:  there are functions for rendering graphics;
// Created:      01.01.23
// =================================================================================
#include "../Common/pch.h"
#include "CRender.h"
#include "d3dclass.h"
#include "../Shaders/Shader.h"

using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMMATRIX = DirectX::XMMATRIX;


namespace Render
{

CRender::CRender() {}
CRender::~CRender() {}

// =================================================================================
//                               public methods
// =================================================================================

bool CRender::Initialize(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    const InitParams& params) 
{
    try
    {
        pContext_ = pContext;
        bool result = true;
        HRESULT hr = S_OK;


        result = InitConstBuffers(pDevice, pContext, params);
        CAssert::True(result, "can't init const buffers");


        result = InitInstancesBuffer(pDevice);
        CAssert::True(result, "can't init instances buffer");


        result = InitSamplers(pDevice, pContext);
        CAssert::True(result, "can't init samplers");

        result = shaderMgr_.Init(pDevice, pContext, params.worldViewOrtho);
        CAssert::True(result, "can't init shaders manager");
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't initialize the CRender module");
        return false;
    }

    return true;
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


        // INIT CONST BUFFERS FOR VERTEX SHADERS --------------------

        hr = cbvsViewProj_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (viewProj; for VS)");

        hr = cbvsWorldAndViewProj_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (world and viewProj; for VS)");

        hr = cbvsWorldViewProj_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (worldViewProj; for VS)");

        hr = cbvsWorldViewOrtho_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (worldViewOrtho; for VS)");


        // INIT CONST BUFFERS FOR GEOMETRY SHADERS ------------------

        hr = cbgsPerFrame_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (per frame; for GS)");

        hr = cbgsGrassParams_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (grass params; for GS)");


        // INIT CONST BUFFERS FOR PIXEL SHADERS ---------------------

        hr = cbpsPerFrame_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (per frame; for PS)");

        hr = cbpsRareChanged_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (rare changed; for PS)");

        hr = cbpsTerrainMaterial_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (terrain material; for PS)");

        hr = cbpsFontPixelColor_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (font pixel color; for PS)");

        hr = cbpsDebugMode_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (debug mode; for PS)");

        hr = cbpsMaterial_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (material: for PS)");


        // SETUP SOME CONST BUFFERS ---------------------------------

        cbvsWorldViewOrtho_.data.worldViewOrtho = params.worldViewOrtho;

        // since fog props is changed very rarely we setup rare changed cbps (const buffer for pixel shader)
        cbpsRareChanged_.data.fogColor = params.fogColor;
        cbpsRareChanged_.data.fogStart = params.fogStart;
        cbpsRareChanged_.data.fogRange = params.fogRange;

        // setup the material colors for terrain
        cbpsTerrainMaterial_.data.ambient = params.terrainMatColors.ambient;
        cbpsTerrainMaterial_.data.diffuse = params.terrainMatColors.diffuse;
        cbpsTerrainMaterial_.data.specular = params.terrainMatColors.specular;
        cbpsTerrainMaterial_.data.reflect = params.terrainMatColors.reflect;

        // setup 2D font color
        cbpsFontPixelColor_.data.pixelColor = { 1,1,1 };


        // load data for const buffers into GPU
        cbvsViewProj_.ApplyChanges(pContext);
        cbvsWorldAndViewProj_.ApplyChanges(pContext);
        cbvsWorldViewProj_.ApplyChanges(pContext);
        cbvsWorldViewOrtho_.ApplyChanges(pContext);

        cbgsPerFrame_.ApplyChanges(pContext);
        cbgsGrassParams_.ApplyChanges(pContext);

        cbpsPerFrame_.ApplyChanges(pContext);
        cbpsRareChanged_.ApplyChanges(pContext);
        cbpsTerrainMaterial_.ApplyChanges(pContext);
        cbpsFontPixelColor_.ApplyChanges(pContext);
        cbpsDebugMode_.ApplyChanges(pContext);
        cbpsMaterial_.ApplyChanges(pContext);


        // BIND CONST BUFFERS ---------------------------------------

        // const buffers for vertex shaders
        ID3D11Buffer* vsCBs[] =
        {
            cbvsViewProj_.Get(),                         // slot_0: is common for color/light/debug shader
            cbvsWorldAndViewProj_.Get(),                 // slot_1: world and viewProj matrix
            cbvsWorldViewProj_.Get(),                    // slot_2: world_view_proj matrix
            cbvsWorldViewOrtho_.Get(),                   // slot_3: world_basicView_ortho matrix (for 2D rendering)
        };	

        // const buffers for geometry shaders
        ID3D11Buffer* gsCBs[] =
        {
            cbgsPerFrame_.Get(),                         // slot_0:
            cbgsGrassParams_.Get(),                      // slot_1:
        };

        // const buffers for pixel shaders
        ID3D11Buffer* psCBs[] =
        {
            cbpsPerFrame_.Get(),                         // slot_0: params which are changed each frame
            cbpsRareChanged_.Get(),                      // slot_1: rare changed params
            cbpsTerrainMaterial_.Get(),                  // slot_2: material color data for terrain
            cbpsFontPixelColor_.Get(),                   // slot_3: font (2D text) pixel color
            cbpsDebugMode_.Get(),                        // slot_4: debug model value (for debug shader)
            cbpsMaterial_.Get(),                         // slot_5: material color data (common const buffer)
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

    // setup description for a sampler state
    D3D11_SAMPLER_DESC skySamplerDesc{};
    skySamplerDesc.Filter           = D3D11_FILTER_ANISOTROPIC;
    skySamplerDesc.AddressU         = D3D11_TEXTURE_ADDRESS_CLAMP;
    skySamplerDesc.AddressV         = D3D11_TEXTURE_ADDRESS_CLAMP;
    skySamplerDesc.AddressW         = D3D11_TEXTURE_ADDRESS_CLAMP;
    skySamplerDesc.BorderColor[0]   = 1.0f;
    skySamplerDesc.BorderColor[1]   = 0.0f;
    skySamplerDesc.BorderColor[2]   = 0.0f;
    skySamplerDesc.BorderColor[3]   = 1.0f;
    skySamplerDesc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;// D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
    skySamplerDesc.MinLOD           = 0.0f;
    skySamplerDesc.MaxLOD           = D3D11_FLOAT32_MAX;
    skySamplerDesc.MaxAnisotropy    = D3D11_REQ_MAXANISOTROPY;
    skySamplerDesc.MipLODBias       = 0.0f;


    // init samplers
    result = basicSampler_.Initialize(pDevice);
    if (!result)
    {
        LogErr(LOG, "can't init a basic sampler state");
        return false;
    }

    result = skySampler_.Initialize(pDevice, &skySamplerDesc);
    if (!result)
    {
        LogErr(LOG, "can't init a sky sampler state");
        return false;
    }


    // bind samplers
    pContext->GSSetSamplers(0, 1, basicSampler_.GetAddressOf());

    pContext->PSSetSamplers(0, 1, basicSampler_.GetAddressOf());
    pContext->PSSetSamplers(1, 1, skySampler_.GetAddressOf());


    LogMsg(LOG, "all the sampler states are initialized");
    return true;
}

//---------------------------------------------------------
// Desc:   execute hot reload of shaders (runtime reinitialization)
//---------------------------------------------------------
bool CRender::ShadersHotReload(ID3D11Device* pDevice)
{
    return shaderMgr_.HotReload(pDevice);
}


// =================================================================================
//                               updating methods
// =================================================================================

//---------------------------------------------------------
// Desc:  update const buffer for this frame
//---------------------------------------------------------
void CRender::UpdatePerFrame(ID3D11DeviceContext* pContext, const PerFrameData& data)
{
    try 
    {
        // update const buffers for vertex shaders
        cbvsViewProj_.data.viewProj = data.viewProj;

        // update const buffers for geometry shaders
        cbgsPerFrame_.data.viewProj = data.viewProj;
        cbgsPerFrame_.data.cameraPosW = data.cameraPos;
        cbgsPerFrame_.data.time = data.totalGameTime;

        // update const buffers for pixel shaders
        cbpsPerFrame_.data.cameraPos = data.cameraPos;
        cbpsPerFrame_.data.time = data.totalGameTime;

        // after all we apply updates
        cbvsViewProj_.ApplyChanges(pContext);
        cbpsPerFrame_.ApplyChanges(pContext);
        cbgsPerFrame_.ApplyChanges(pContext);


        // update light sources data
        UpdateLights(
            data.dirLights,
            data.pointLights,
            data.spotLights,
            data.numDirLights,
            data.numPointLights,
            data.numSpotLights);
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
void CRender::UpdateInstancedBuffer(
    ID3D11DeviceContext* pContext,
    const InstancesBuf& buf)
{
    UpdateInstancedBuffer(
        pContext,
        buf.worlds_,
        buf.materials_,
        buf.GetSize());     // get the number of elements to render
}

//---------------------------------------------------------
// Desc:   fill in the instanced buffer with input data
//---------------------------------------------------------
void CRender::UpdateInstancedBuffer(
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX* worlds,
    const MaterialColors* matColors,
    const int count)
{
    try
    {
        CAssert::True(worlds != nullptr,    "input arr of world matrices == nullptr");
        CAssert::True(matColors != nullptr, "input arr of materials == nullptr");
        CAssert::True(count > 0,            "input number of elements must be > 0");

        // map the instanced buffer to write into it
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        CAssert::NotFailed(hr, "can't map the instanced buffer");

        ConstBufType::InstancedData* dataView = (ConstBufType::InstancedData*)mappedData.pData;

        // write data into the subresource
        for (int i = 0; i < count; ++i)
        {
            dataView[i].world = worlds[i];
            dataView[i].worldInvTranspose = MathHelper::InverseTranspose(worlds[i]);
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
// Args:    - shaderType:            what shader to use
//          - instances:             batch of instances to render
//          - startInstanceLocation: from where we get instances data
//                                   (world matrices, materials) in the instances buf
//---------------------------------------------------------
void CRender::RenderInstances(
    ID3D11DeviceContext* pContext,
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

        pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
        pContext->IASetIndexBuffer(instances.pIB, DXGI_FORMAT_R32_UINT, 0);


        //Shader* pShader = shaderMgr_.GetShaderByName("LightShader");
        Shader* pShader = shaderMgr_.GetShaderById(instances.shaderId);
        constexpr int numVertexBuffers = 2;
        //shaderMgr_.BindBuffers(pContext, vbs, instances.pIB, strides, offsets, numVertexBuffers);
        shaderMgr_.BindShader(pContext, pShader);
        shaderMgr_.Render(pContext, instances, startInstanceLocation);
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
// Desc:   render sky dome (or sphere/box) onto screen
//---------------------------------------------------------
void CRender::RenderSkyDome(
    ID3D11DeviceContext* pContext,
    const SkyInstance& sky,
    const XMMATRIX& worldViewProj)
{
    try 
    {
        // prepare input assembler (IA) stage before the rendering process
        UINT offset = 0;
        pContext->IASetVertexBuffers(0, 1, &sky.pVB, &sky.vertexStride, &offset);
        pContext->IASetIndexBuffer(sky.pIB, DXGI_FORMAT_R16_UINT, 0);

        SetWorldViewProj(pContext, worldViewProj);

        // bind shader and render sky
        Shader* pShader = shaderMgr_.GetShaderByName("SkyDomeShader");
        shaderMgr_.BindShader(pContext, pShader);
        //shaderMgr_.BindBuffers(pContext, &sky.pVB, sky.pIB, &sky.vertexStride, &offset, 1);
        shaderMgr_.Render(pContext, sky.indexCount);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't render the sky dome");
    }
    catch (...)
    {
        LogErr("can't render the sky dome for some unknown reason :(");
    }
}

//---------------------------------------------------------
// Desc:   render 2D text onto the screen
//---------------------------------------------------------
void CRender::RenderFont(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* const* vertexBuffers,
    ID3D11Buffer* pIndexBuffer,
    const uint32* indexCounts,
    const size numVertexBuffers,
    const UINT fontVertexStride,
    SRV* const* ppFontTexSRV)
{
    // renders text onto the screen
    try
    {
        CAssert::True(vertexBuffers,        "input ptr to vertex buffers arr == nullptr");
        CAssert::True(pIndexBuffer,         "input ptr to index buffer == nullptr");
        CAssert::True(indexCounts,          "input ptr to index counts arr == nullptr");
        CAssert::True(numVertexBuffers > 0, "input number of vertex buffers must be > 0");

        // bind shader and render text
        Shader* pShader = shaderMgr_.GetShaderByName("FontShader");
        shaderMgr_.BindShader(pContext, pShader);

        // set textures
        pContext->PSSetShaderResources(2, 1, ppFontTexSRV);

        const UINT offset = 0;
        pContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        for (index idx = 0; idx < numVertexBuffers; ++idx)
        {
            // bind vertex buffer
            pContext->IASetVertexBuffers(0, 1, &vertexBuffers[idx], &fontVertexStride, &offset);

            // render the fonts on the screen
            shaderMgr_.Render(pContext, indexCounts[idx]);
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

void CRender::SwitchFlashLight(ID3D11DeviceContext* pContext, const bool state)
{
    // switch the flashlight state
    cbpsRareChanged_.data.turnOnFlashLight = state;
    cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void CRender::SwitchAlphaClipping(ID3D11DeviceContext* pContext, const bool state)
{
    // turn on/off alpha clipping
    cbpsRareChanged_.data.alphaClipping = state;
    cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void CRender::SwitchDebugState(ID3D11DeviceContext* pContext, const eDebugState state)
{
    // turn on/off debug rendering;
    // if we turn on debug rendering we setup the debug state as well;
    switch (state)
    {
        // use default basic shader
        case DBG_TURN_OFF:
        {
            isDebugMode_ = false;
            break;
        }
        // use debug shader with particular debug state
        case DBG_SHOW_NORMALS:
        case DBG_SHOW_TANGENTS:
        case DBG_SHOW_BUMPED_NORMALS:
        case DBG_SHOW_ONLY_LIGTHING:
        case DBG_SHOW_ONLY_DIRECTED_LIGHTING:
        case DBG_SHOW_ONLY_POINT_LIGHTING:
        case DBG_SHOW_ONLY_SPOT_LIGHTING:
        case DBG_SHOW_ONLY_DIFFUSE_MAP:
        case DBG_SHOW_ONLY_NORMAL_MAP:
        case DBG_WIREFRAME:
        case DBG_SHOW_MATERIAL_AMBIENT:
        case DBG_SHOW_MATERIAL_DIFFUSE:
        case DBG_SHOW_MATERIAL_SPECULAR:
        case DBG_SHOW_MATERIAL_REFLECTION:
        {
            isDebugMode_ = true;
            assert(0 && "FIXME");
            //shadersContainer_.debugShader_.SetDebugType(pContext, state);
            break;
        }
        default:
        {
            isDebugMode_ = false;
            sprintf(g_String, "unknown rendering debug state: %d", state);
            LogErr(g_String);
        }
    }
}

///////////////////////////////////////////////////////////

void CRender::SetDirLightsCount(ID3D11DeviceContext* pContext, int numOfLights)
{
    // set how many directional lights we used for lighting of the scene

    constexpr int maxNumDirLights = 3;

    if ((numOfLights < 0) || (numOfLights > maxNumDirLights))
    {
        LogErr("wrong number of directed lights (must be in range [0, 3])");
        return;
    }

    cbpsRareChanged_.data.numOfDirLights = numOfLights;
    cbpsRareChanged_.ApplyChanges(pContext);
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
    cbgsGrassParams_.ApplyChanges(g_pContext);
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
    cbgsGrassParams_.ApplyChanges(g_pContext);
    return true;
}

// =================================================================================
// Fog control
// =================================================================================

//---------------------------------------------------------
// Desc:   turn on/off the fog effect
//---------------------------------------------------------
void CRender::SetFogEnabled(ID3D11DeviceContext* pContext, const bool state)
{
    cbpsRareChanged_.data.fogEnabled = state;
    cbpsRareChanged_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   setup where the for starts
//---------------------------------------------------------
void CRender::SetFogStart(ID3D11DeviceContext* pContext, const float start)
{
    cbpsRareChanged_.data.fogStart = (start > 0) ? start : 0.0f;
    cbpsRareChanged_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   setup distance after start where objects will be fully fogged
//---------------------------------------------------------
void CRender::SetFogRange(ID3D11DeviceContext* pContext, const float range)
{
    cbpsRareChanged_.data.fogRange = (range > 1) ? range : 1.0f;
    cbpsRareChanged_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   setup RGB color for the fog
//---------------------------------------------------------
void CRender::SetFogColor(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3 color)
{
    cbpsRareChanged_.data.fogColor = color;
    cbpsRareChanged_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   setup a const buffer with another color for debug text
//---------------------------------------------------------
void CRender::SetDebugFontColor(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& color)
{
    cbpsFontPixelColor_.data.pixelColor = color;
    cbpsFontPixelColor_.ApplyChanges(pContext);
}

//---------------------------------------------------------
 // Desc:   setup the sky gradient from the horizon up to the apex
 // Args:   - colorCenter:   sky horizon color
 //         - colorApex:     sky top color
 //---------------------------------------------------------
void CRender::SetSkyGradient(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& colorCenter,
    const DirectX::XMFLOAT3& colorApex)
{
    cbpsRareChanged_.data.colorCenter_ = colorCenter;
    cbpsRareChanged_.data.colorApex_ = colorApex;
    cbpsRareChanged_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   setup only the sky horizon color for the gradient
// Args:   - color:   sky horizon color
//---------------------------------------------------------
void CRender::SetSkyColorCenter(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& color)
{
    SetSkyGradient(pContext, color, cbpsRareChanged_.data.colorCenter_);
}

//---------------------------------------------------------
// Desc:   setup only the sky apex(top) color for the gradient
// Args:   - color:   sky apex color
//---------------------------------------------------------
void CRender::SetSkyColorApex(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& color)
{
    SetSkyGradient(pContext, cbpsRareChanged_.data.colorApex_, color);
}


//---------------------------------------------------------
// Desc:   load updated light sources data into const buffers
//---------------------------------------------------------
void CRender::UpdateLights(
    const DirLight* dirLights,
    const PointLight* pointLights,
    const SpotLight* spotLights,
    const int numDirLights,
    const int numPointLights,
    const int numSpotLights)
{

    // if for some reason we need to update the number of directed light sources
    if (cbpsRareChanged_.data.numOfDirLights != numDirLights)
    {
        cbpsRareChanged_.data.numOfDirLights = numDirLights;
        cbpsRareChanged_.ApplyChanges(pContext_);
    }
    
    // update directional light sources
    for (int i = 0; i < numDirLights; ++i)
        cbpsPerFrame_.data.dirLights[i] = dirLights[i];

    // we want to copy the proper number of point lights
    const int pointLightsCountLimit  = ARRAYSIZE(cbpsPerFrame_.data.pointLights);
    const int spotLightsCountLimit   = ARRAYSIZE(cbpsPerFrame_.data.spotLights);
    const int numPointLightsToUpdate = (numPointLights >= pointLightsCountLimit) ? pointLightsCountLimit : numPointLights;
    const int numSpotLightsToUpdate  = (numSpotLights >= spotLightsCountLimit) ? spotLightsCountLimit : numSpotLights;

    cbpsPerFrame_.data.currNumPointLights = numPointLightsToUpdate;
    cbpsPerFrame_.data.currNumSpotLights = numSpotLightsToUpdate;

    // update point/spot light sources
    for (int i = 0; i < numPointLightsToUpdate; ++i)
        cbpsPerFrame_.data.pointLights[i] = pointLights[i];

    for (int i = 0; i < numSpotLightsToUpdate; ++i)
        cbpsPerFrame_.data.spotLights[i] = spotLights[i];
}

//---------------------------------------------------------
// Desc:   update a const buffer with view*proj matrix
// NOTE:   the matrix must be already transposed
//---------------------------------------------------------
void CRender::SetViewProj(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj)
{
    cbvsViewProj_.data.viewProj = viewProj;
    cbvsViewProj_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   update a const buffer with 2 matrices: world and viewProj
//---------------------------------------------------------
void CRender::SetWorldAndViewProj(
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX& world,
    const DirectX::XMMATRIX& viewProj)
{
    cbvsWorldAndViewProj_.data.world = world;
    cbvsWorldAndViewProj_.data.viewProj = viewProj;
    cbvsWorldAndViewProj_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   update a const buffer with world_view_proj matrix (wvp)
//---------------------------------------------------------
void CRender::SetWorldViewProj(
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX& wvp)
{
    cbvsWorldViewProj_.data.worldViewProj = wvp;
    cbvsWorldViewProj_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   update a const buffer with world*view*ortho matrix for 2D rendering
// NOTE:   the matrix must be already transposed
//---------------------------------------------------------
void CRender::SetWorldViewOrtho(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& WVO)
{
    cbvsWorldViewOrtho_.data.worldViewOrtho = WVO;
    cbvsWorldViewOrtho_.ApplyChanges(pContext);
}


}; // namespace
