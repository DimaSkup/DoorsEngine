// =================================================================================
// Filename:     CRender.cpp
// Description:  there are functions for rendering graphics;
// Created:      01.01.23
// =================================================================================
#include "CRender.h"
#include <MathHelper.h>
#include <log.h>
#include <CAssert.h>
#include "InitRender.h"

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
        InitRender init;

        result = init.InitializeShaders(
            pDevice, 
            pContext,
            shadersContainer_,
            params.worldViewOrtho);
        CAssert::True(result, "can't initialize shaders");

        // --------------------------------------------

        // create instances buffer
        const UINT maxInstancesNum = 9000;
        D3D11_BUFFER_DESC vbd;

        // setup buffer's description
        vbd.Usage               = D3D11_USAGE_DYNAMIC;
        vbd.ByteWidth           = static_cast<UINT>(sizeof(ConstBufType::InstancedData) * maxInstancesNum);
        vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        vbd.MiscFlags           = 0;
        vbd.StructureByteStride = 0;

        hr = pDevice->CreateBuffer(&vbd, nullptr, &pInstancedBuffer_);
        CAssert::NotFailed(hr, "can't create an instanced buffer");


        // ------------------------ CONSTANT BUFFERS ------------------------------ 

        hr = cbvsPerFrame_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (VS)");

        hr = cbpsPerFrame_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (PS)");

        hr = cbpsRareChanged_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer (PS)");

        // const buffers for geometry shader
        hr = cbgsPerFrame_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't initialize const buffer for GS");

        //hr = cbgsPerObject_.Initialize(pDevice, pContext);
        //CAssert::NotFailed(hr, "can't initialize const buffer for GS");

        //hr = cbgsFixed_.Initialize(pDevice, pContext);
        //CAssert::NotFailed(hr, "can't initialize const buffer for GS");

        // init fog params
        InitFogParams(pContext, params.fogColor, params.fogStart, params.fogRange);

        // load rare changed data (default values if we didn't setup any) into GPU 
        cbpsRareChanged_.ApplyChanges(pContext);

        ColorShader&    colorShader   = shadersContainer_.colorShader_;
        LightShader&    lightShader   = shadersContainer_.lightShader_;
        SkyDomeShader&  skyDomeShader = shadersContainer_.skyDomeShader_;
        FontShader&     fontShader    = shadersContainer_.fontShader_;
        DebugShader&    debugShader   = shadersContainer_.debugShader_;
        TerrainShader&  terrainShader = shadersContainer_.terrainShader_;
        
        // const buffers for vertex shaders (vsCB)
        ID3D11Buffer* vsCBs[3] = 
        {
            cbvsPerFrame_.Get(),                         // slot_0: is common for color/light/debug shader
            skyDomeShader.GetConstBufferVSPerFrame(),    // slot_1: sky dome
            fontShader.GetConstBufferVS(),               // slot_2: font shader
        };	

        // const buffers for geometry shaders (gsCB)
        ID3D11Buffer* gsCBs[2] =
        {
            cbgsPerFrame_.Get(),
            //cbgsPerObject_.Get(),
            //cbgsFixed_.Get(),
        };

        // const buffers for pixel shaders (psCB)
        ID3D11Buffer* psCBs[] =
        {
            cbpsPerFrame_.Get(),                         // slot_0: light / debug shader
            cbpsRareChanged_.Get(),                      // slot_1: light / debug shader
            skyDomeShader.GetConstBufferPSRareChanged(), // slot_2: sky dome shader
            fontShader.GetConstBufferPS(),               // slot_3: font shader 
            debugShader.GetConstBufferPSRareChanged(),   // slot_4: debug shader
            terrainShader.GetConstBufferPS(),            // slot_5: terrain shader
        };

        const UINT numBuffersVS = sizeof(vsCBs) / sizeof(ID3D11Buffer*);
        const UINT numBuffersGS = sizeof(gsCBs) / sizeof(ID3D11Buffer*);
        const UINT numBuffersPS = sizeof(psCBs) / sizeof(ID3D11Buffer*);

        // bind constant buffers 
        pContext->VSSetConstantBuffers(0, numBuffersVS, vsCBs);
        pContext->GSSetConstantBuffers(0, numBuffersGS, gsCBs);
        pContext->PSSetConstantBuffers(0, numBuffersPS, psCBs);
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't initialize the CRender module");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

bool CRender::ShadersHotReload(ID3D11Device* pDevice)
{
    try
    {
        shadersContainer_.lightShader_.ShaderHotReload       (pDevice, "shaders/hlsl/LightVS.hlsl",        "shaders/hlsl/LightPS.hlsl");
        shadersContainer_.materialIconShader_.ShaderHotReload(pDevice, "shaders/hlsl/MaterialIconVS.hlsl", "shaders/hlsl/MaterialIconPS.hlsl");
        shadersContainer_.terrainShader_.ShaderHotReload     (pDevice, "shaders/hlsl/TerrainVS.hlsl",      "shaders/hlsl/TerrainPS.hlsl");
        LogDbg(LOG, "shaders are hot reloaded");

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        return false;
    }
}


// =================================================================================
//                               updating methods
// =================================================================================
void CRender::UpdatePerFrame(
    ID3D11DeviceContext* pContext,
    const PerFrameData& data)
{
    // update the common data for shaders for this frame
    // update constant buffers for this frame

    try 
    {
        // view * proj matrix must be already transposed
        cbvsPerFrame_.data.viewProj = data.viewProj;
        cbgsPerFrame_.data.viewProj = data.viewProj;

        // update the camera pos
        cbpsPerFrame_.data.cameraPos = data.cameraPos;
        cbgsPerFrame_.data.cameraPosW = data.cameraPos;

        // update light sources data
        UpdateLights(
            data.dirLights,
            data.pointLights,
            data.spotLights,
            data.numDirLights,
            data.numPointLights,
            data.numSpotLights);

        // after all we apply updates
        cbvsPerFrame_.ApplyChanges(pContext);
        cbpsPerFrame_.ApplyChanges(pContext);
        cbgsPerFrame_.ApplyChanges(pContext);

    }
    catch (EngineException& e)
    {
        LogErr(e);
    }
    catch (...)
    {
        LogErr("something went wrong during updating data for rendering");
    }
}

///////////////////////////////////////////////////////////

void CRender::UpdateInstancedBuffer(
    ID3D11DeviceContext* pContext,
    const InstBuffData& data)
{
    UpdateInstancedBuffer(
        pContext,
        data.worlds_,
        data.texTransforms_,
        data.materials_,
        data.GetSize());     // get the number of elements to render
}

///////////////////////////////////////////////////////////

void CRender::UpdateInstancedBuffer(
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX* worlds,
    const DirectX::XMMATRIX* texTransforms,
    const Material* materials,
    const int count)
{
    // fill in the instanced buffer with data
    try
    {
        CAssert::True(worlds != nullptr,        "input arr of world matrices == nullptr");
        CAssert::True(texTransforms != nullptr, "input arr of texture transformations == nullptr");
        CAssert::True(materials != nullptr,     "input arr of materials == nullptr");
        CAssert::True(count > 0,                "input number of elements must be > 0");

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
            dataView[i].texTransform = texTransforms[i];

        for (int i = 0; i < count; ++i)
            dataView[i].material = materials[i];

        pContext->Unmap(pInstancedBuffer_, 0);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't update instanced buffer for rendering");
    }
}

///////////////////////////////////////////////////////////

void CRender::UpdateInstancedBufferWorlds(
    ID3D11DeviceContext* pContext,
    cvector<DirectX::XMMATRIX>& worlds)
{
    try
    {
        // map the instanced buffer to write to it
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        CAssert::NotFailed(hr, "can't map the instanced buffer");

        ConstBufType::InstancedData* dataView = (ConstBufType::InstancedData*)mappedData.pData;

        // write data into the subresource
        for (index i = 0; i < std::ssize(worlds); ++i)
        {
            dataView[i].world = worlds[i];
            dataView[i].worldInvTranspose = MathHelper::InverseTranspose(worlds[i]);
        }

        pContext->Unmap(pInstancedBuffer_, 0);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't update instanced buffer WORLDS for rendering");
    }
    catch (...)
    {
        LogErr("can't update instanced buffer for rendering for some unknown reason :)");
    }
}

///////////////////////////////////////////////////////////

void CRender::UpdateInstancedBufferMaterials(
    ID3D11DeviceContext* pContext,
    cvector<Material>& materials)
{
    try
    {
        // map the instanced buffer to write to it
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        CAssert::NotFailed(hr, "can't map the instanced buffer");

        ConstBufType::InstancedData* dataView = (ConstBufType::InstancedData*)mappedData.pData;

        // write data into the subresource
        for (index i = 0; i < std::ssize(materials); ++i)
            dataView[i].material = materials[i];

        pContext->Unmap(pInstancedBuffer_, 0);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't update instanced buffer MATERIALS for rendering");
    }
    catch (...)
    {
        LogErr("can't update instanced buffer for rendering for some unknown reason :)");
    }
}




// =================================================================================
//                               rendering methods
// =================================================================================

void CRender::RenderBoundingLineBoxes(
    ID3D11DeviceContext* pContext,
    const Instance* instances,
    const int numModels)
{
    try
    {
        const UINT instancedBuffElemSize = static_cast<UINT>(sizeof(ConstBufType::InstancedData));

        shadersContainer_.colorShader_.Render(
            pContext,
            pInstancedBuffer_,
            instances,
            numModels,
            instancedBuffElemSize);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't render the bounding line boxes onto the screen");
    }
    catch (...)
    {
        LogErr("can't render the bounding line boxes for some unknown reason :(");
    }
}

///////////////////////////////////////////////////////////

void CRender::RenderInstances(
    ID3D11DeviceContext* pContext,
    const ShaderTypes type,
    const Instance* instances,
    const int numInstances)
{
    try
    {
        const UINT instancedBuffElemSize = static_cast<UINT>(sizeof(ConstBufType::InstancedData));

        if (isDebugMode_)
        {
            shadersContainer_.debugShader_.Render(
                pContext,
                pInstancedBuffer_,
                instances,
                numInstances,
                instancedBuffElemSize);

            return;
        }

        switch (type)
        {
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            case COLOR:
            {
                shadersContainer_.colorShader_.Render(
                    pContext,
                    pInstancedBuffer_,
                    instances,
                    numInstances,
                    instancedBuffElemSize);

                break;
            }
            case TEXTURE:
            {
                shadersContainer_.textureShader_.Render(
                    pContext,
                    pInstancedBuffer_,
                    instances,
                    numInstances,
                    instancedBuffElemSize);
                break;
            }
#if 0
            case BILLBOARD:
            {
                shadersContainer_.billboardShader_.CRender(
                    pContext,
                    pInstancedBuffer_,
                    instances,
                    numInstances,
                    instancedBuffElemSize);
                break;
            }
#endif
            case LIGHT:
            {
                shadersContainer_.lightShader_.Render(
                    pContext,
                    pInstancedBuffer_,
                    instances,
                    numInstances,
                    instancedBuffElemSize);

                break;
            }
            case OUTLINE:
            {
                shadersContainer_.outlineShader_.Render(
                    pContext,
                    pInstancedBuffer_,
                    instances,
                    numInstances,
                    instancedBuffElemSize);

                break;
            }
            default:
            {
                sprintf(g_String, "unknown shader type: %d", type);
                LogErr(g_String);
            }
        }
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't render the mesh instances onto the screen");
    }
    catch (...)
    {
        LogErr("can't render mesh instances for some unknown reason :(");
    }
}

///////////////////////////////////////////////////////////

void CRender::RenderSkyDome(
    ID3D11DeviceContext* pContext,
    const SkyInstance& sky,
    const XMMATRIX& worldViewProj)
{
    try 
    {
        shadersContainer_.skyDomeShader_.Render(
            pContext,
            sky,
            worldViewProj);
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
        {
            isDebugMode_ = true;
            shadersContainer_.debugShader_.SetDebugType(pContext, state);
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
// Fog control
// =================================================================================
void CRender::SetFogEnabled(ID3D11DeviceContext* pContext, const bool state)
{
    // turn on/off the fog effect
    cbpsRareChanged_.data.fogEnabled = state;
    cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void CRender::SetFogStart(ID3D11DeviceContext* pContext, const float start)
{
    // setup where the for starts
    cbpsRareChanged_.data.fogStart = (start > 0) ? start : 0.0f;
    cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void CRender::SetFogRange(ID3D11DeviceContext* pContext, const float range)
{
    // setup distance after start where objects will be fully fogged
    cbpsRareChanged_.data.fogRange = (range > 1) ? range : 1.0f;
    cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void CRender::SetFogColor(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3 color)
{
    cbpsRareChanged_.data.fogColor = color;
    cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void CRender::SetWorldViewOrtho(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& WVO)
{
    // update the WVO (world * base_view * orthographic) matrix for 2D rendering
    shadersContainer_.fontShader_.SetWorldViewOrtho(pContext, WVO);
}

///////////////////////////////////////////////////////////

void CRender::InitFogParams(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& fogColor,
    const float fogStart,
    const float fogRange)
{
    const float start = (fogStart) ? fogStart : 0.0f;
    const float range = (fogRange) ? fogRange : 10.0f;

    // since fog props is changed very rarely we setup rare changed cbps (const buffer for pixel shader)
    cbpsRareChanged_.data.fogColor = fogColor;
    cbpsRareChanged_.data.fogStart = start;
    cbpsRareChanged_.data.fogRange = range;

    cbpsRareChanged_.ApplyChanges(pContext);
}


// =================================================================================
// Sky control
// =================================================================================
void CRender::SetSkyGradient(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& colorCenter,
    const DirectX::XMFLOAT3& colorApex)
{
    try
    {
        shadersContainer_.skyDomeShader_.SetSkyGradient(pContext, colorCenter, colorApex);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't update the sky gradient");
    }
}

///////////////////////////////////////////////////////////

void CRender::SetSkyColorCenter(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& color)
{
    try
    {
        shadersContainer_.skyDomeShader_.SetSkyColorCenter(pContext, color);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't update the sky center (horizon) color");
    }
}

///////////////////////////////////////////////////////////

void CRender::SetSkyColorApex(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& color)
{
    try
    {
        shadersContainer_.skyDomeShader_.SetSkyColorApex(pContext, color);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't update the sky apex (top) color");
    }
}


///////////////////////////////////////////////////////////

void CRender::UpdateLights(
    const DirLight* dirLights,
    const PointLight* pointLights,
    const SpotLight* spotLights,
    const int numDirLights,
    const int numPointLights,
    const int numSpotLights)
{
    // load updated light sources data into const buffers

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

///////////////////////////////////////////////////////////

void CRender::SetViewProj(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj)
{
    // view * proj matrix must be already transposed
    cbvsPerFrame_.data.viewProj = viewProj;
    cbvsPerFrame_.ApplyChanges(pContext);
}



}; // namespace CRender
