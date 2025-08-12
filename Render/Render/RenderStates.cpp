// *********************************************************************************
// Filename:     RenderStates.h
// Description:  constains implementation of the RenderStates class' functional
// 
// Created:      09.09.24
// *********************************************************************************
#include "../Common/pch.h"
#include "RenderStates.h"
#include <stdexcept>

#pragma warning(disable : 4996)

namespace Render
{
    
RenderStates::RenderStates() 
{
}

RenderStates::~RenderStates() 
{ 
    DestroyAll(); 
}

//---------------------------------------------------------
// Desc:   initialize all the rasterizer,blending,depth-stencil states
//---------------------------------------------------------
void RenderStates::InitAll(ID3D11Device* pDevice, const bool multisampleEnable)
{
    InitAllRasterParams(pDevice, multisampleEnable);
    InitAllBlendStates(pDevice);
    InitAllDepthStencilStates(pDevice);

    // init some hashes to use it later during switching between some states
    turnOffFillModesHash_ &= ~(1 << R_FILL_SOLID);
    turnOffFillModesHash_ &= ~(1 << R_FILL_WIREFRAME);

    turnOffCullModesHash_ &= ~(1 << R_CULL_BACK);
    turnOffCullModesHash_ &= ~(1 << R_CULL_FRONT);
    turnOffCullModesHash_ &= ~(1 << R_CULL_NONE);
}

//---------------------------------------------------------
// Desc:   destroy all the rasterizer,blending,depth-stencil states
//---------------------------------------------------------
void RenderStates::DestroyAll()
{
    for (auto& it : blendStates_)
        SafeRelease(&it.second);

    for (auto& it : rasterStates_)
        SafeRelease(&it.second);

    for (auto& it : depthStencilStates_)
        SafeRelease(&it.second);
        
    blendStates_.clear();
    rasterStates_.clear();
    depthStencilStates_.clear();
}

//---------------------------------------------------------
// Desc:   return a ptr to the blending state by input key
//---------------------------------------------------------
ID3D11BlendState* RenderStates::GetBS(const eRenderState state)
{
    try
    {
        return blendStates_.at(state);
    }
    catch (const std::out_of_range& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "unknown blend state key: %d", state);
        return nullptr;
    }
}

//---------------------------------------------------------
// return a ptr to the rasterizer state by input keys
//---------------------------------------------------------
ID3D11RasterizerState* RenderStates::GetRS(const std::set<eRenderState>& states)
{
    return nullptr;
}

//---------------------------------------------------------
// Desc:   return a ptr to the depth stencil state by input key
//---------------------------------------------------------
ID3D11DepthStencilState* RenderStates::GetDSS(const eRenderState state)
{
    try
    {
        return depthStencilStates_.at(state);
    }
    catch (const std::out_of_range&)
    {
        LogErr(LOG, "there is no depth stencil state by key: %d", state);
        return nullptr;
    }
}

//---------------------------------------------------------
// Desc:   setup raster state according to the input state
//---------------------------------------------------------
void RenderStates::SetRS(ID3D11DeviceContext* pContext, const eRenderState state)
{
    SetRS(pContext, std::set<eRenderState>{state});
}

//---------------------------------------------------------
// Desc:   set up a raster state according to the input states params
//---------------------------------------------------------
void RenderStates::SetRS(ID3D11DeviceContext* pContext, const std::set<eRenderState>& states)
{
    UpdateRSHash(states);
    pContext->RSSetState(GetRasterStateByHash(GetCurrentRSHash()));
}

//---------------------------------------------------------
// Desc:   set up a raster state according to the input hash
//---------------------------------------------------------
void RenderStates::SetRSByHash(ID3D11DeviceContext* pContext, const uint8_t hash)
{
    ID3D11RasterizerState* pRS = nullptr;

    // if the RS by input hash is valid AND we don't want to set the same RS
    if ((rasterStateHash_ != hash) && (pRS = GetRasterStateByHash(hash)))
    {
        pContext->RSSetState(pRS);
        rasterStateHash_ = hash;
    }
}

//---------------------------------------------------------
// Desc:   set a blending state by input key
//---------------------------------------------------------
void RenderStates::SetBS(ID3D11DeviceContext* pContext, const eRenderState state)
{
    try
    {
        ID3D11BlendState* pBS = blendStates_.at(state);

        switch (state)
        {
            case R_ALPHA_DISABLE:
            case R_ALPHA_ENABLE:
            case R_ADDING:
            case R_SUBTRACTING:
            case R_MULTIPLYING:
            case R_ALPHA_TO_COVERAGE:
            {
                pContext->OMSetBlendState(pBS, NULL, 0xFFFFFFFF);
                break;
            }
            case R_TRANSPARENCY:
            case R_NO_RENDER_TARGET_WRITES:
            {
                //float blendFactor[4] = { 0,0,0,0 };
                float blendFactor[4] = { 0, 0, 0, 0 };
                pContext->OMSetBlendState(pBS, blendFactor, 0xFFFFFFFF);
                break;
            }
            default:
            {
                LogErr(LOG, "there is no case for handling blend state: %d", state);
            }
        }
    }
    catch (const std::out_of_range&)
    {
        LogErr(LOG, "there is no blend state (BS) by key: %d", state);
        pContext->OMSetBlendState(nullptr, NULL, 0xFFFFFFFF);
    }

}

//---------------------------------------------------------
// Desc:   set a depth stencil state by input key
//---------------------------------------------------------
void RenderStates::SetDSS(
    ID3D11DeviceContext* pContext, 
    const eRenderState state, 
    const UINT stencilRef)
{
    try
    {
        pContext->OMSetDepthStencilState(depthStencilStates_.at(state), stencilRef);
    }
    catch (const std::out_of_range&)
    {
        LogErr(LOG, "there is no depth stencil state (DSS) by key: %d", state);
        pContext->OMSetDepthStencilState(0, 0);
    }
}

//---------------------------------------------------------
// create/set up the rasterizer state objects
// 
// firstly we set up a description for a raster state, then we create it
// after creation we generate a hash for this particular raster state
// and insert into the map created pairs ['hash' => 'ptr_to_raster_state']
//---------------------------------------------------------
void RenderStates::InitAllRasterParams(ID3D11Device* pDevice, bool multisampleEnable)
{
    try
    {
        HRESULT                hr = S_OK;
        ID3D11RasterizerState* pRasterState = nullptr;
        D3D11_RASTERIZER_DESC  desc;
        ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));

        //
        // 1. DEFAULT: fill solid + cull back + front cw
        //
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;
        desc.MultisampleEnable = multisampleEnable;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        UpdateRSHash({ R_FILL_SOLID, R_CULL_BACK, R_FRONT_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // 2. fill_solid + cull_back + front ccw
        //
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = true;
        desc.DepthClipEnable = true;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_SOLID, R_CULL_BACK, R_FRONT_COUNTER_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // 3. fill_solid + cull_none + front ccw
        //
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FrontCounterClockwise = true;
        desc.DepthClipEnable = true;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_SOLID, R_CULL_NONE, R_FRONT_COUNTER_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });

        //
        // 4. fill_solid + cull_none + front cw
        //
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_SOLID, R_CULL_NONE, R_FRONT_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // 5. fill solid + cull front + front cw
        //
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_FRONT;
        desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_SOLID, R_CULL_FRONT, R_FRONT_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // 6. fill wireframe + cull back + front cw
        //
        desc.FillMode = D3D11_FILL_WIREFRAME;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_WIREFRAME, R_CULL_BACK, R_FRONT_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // 7. fill wireframe + cull front + front cw
        //
        desc.FillMode = D3D11_FILL_WIREFRAME;
        desc.CullMode = D3D11_CULL_FRONT;
        desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_WIREFRAME, R_CULL_FRONT, R_FRONT_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // 8. fill wireframe + cull none + front cw
        //
        desc.FillMode = D3D11_FILL_WIREFRAME;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;

        hr = pDevice->CreateRasterizerState(&desc, &pRasterState);
        CAssert::NotFailed(hr, "can't create a raster state");

        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_WIREFRAME, R_CULL_NONE, R_FRONT_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // AFTER ALL: reset the rasterizer state hash after initialization
        //            and set the default params
        ResetRasterStateHash();
        UpdateRSHash({ R_FILL_SOLID, R_CULL_BACK, R_FRONT_CLOCKWISE });
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        throw EngineException("can't initialize the rasterizer state");
    }
}

//---------------------------------------------------------
// Desc:   init different blending states
//---------------------------------------------------------
void RenderStates::InitAllBlendStates(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
    D3D11_BLEND_DESC blendDesc { 0 };
    D3D11_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];

    //
    // No Render Target Writes
    //
    blendDesc.AlphaToCoverageEnable  = false;
    blendDesc.IndependentBlendEnable = false;

    rtbd.BlendEnable    = false;
    rtbd.SrcBlend       = D3D11_BLEND_ONE;
    rtbd.DestBlend      = D3D11_BLEND_ZERO;
    rtbd.BlendOp        = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha  = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = 0;

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[R_NO_RENDER_TARGET_WRITES]);
    CAssert::NotFailed(hr, "can't create a no_render_target_writes blending state");
    
    //
    // Disabled blending
    //
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    rtbd.BlendEnable    = FALSE;
    rtbd.SrcBlend       = D3D11_BLEND_SRC_ALPHA;
    rtbd.DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.BlendOp        = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha  = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[R_ALPHA_DISABLE]);
    CAssert::NotFailed(hr, "can't create an alpha disabled blending state");

    //
    // Enabled blending (for rendering fonts, sky plane, ...)
    //
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    rtbd.BlendEnable    = TRUE;
    rtbd.SrcBlend       = D3D11_BLEND_ONE;
    rtbd.DestBlend      = D3D11_BLEND_ONE;
    rtbd.BlendOp        = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha  = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[R_ALPHA_ENABLE]);
    CAssert::NotFailed(hr, "can't create an alpha enabled blending state");

    //
    // Adding BS
    //
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    rtbd.BlendEnable    = TRUE;
    //rtbd.SrcBlend       = D3D11_BLEND_ONE;
    rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtbd.DestBlend      = D3D11_BLEND_ONE;
    rtbd.BlendOp        = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha  = D3D11_BLEND_ZERO;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[R_ADDING]);
    CAssert::NotFailed(hr, "can't create an adding blend state");

    //
    // Subtracting BS
    //
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    rtbd.BlendEnable    = TRUE;
    rtbd.SrcBlend       = D3D11_BLEND_ONE;
    rtbd.DestBlend      = D3D11_BLEND_ONE;
    rtbd.BlendOp        = D3D11_BLEND_OP_SUBTRACT;
    rtbd.SrcBlendAlpha  = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[R_SUBTRACTING]);
    CAssert::NotFailed(hr, "can't create a subtracting blend state");

    //
    // Multiplying BS
    //
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    rtbd.BlendEnable    = TRUE;
    rtbd.SrcBlend       = D3D11_BLEND_ZERO;
    rtbd.DestBlend      = D3D11_BLEND_SRC_COLOR;
    rtbd.BlendOp        = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha  = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[R_MULTIPLYING]);
    CAssert::NotFailed(hr, "can't create a multiplying blend state");

    // 
    // Transparent BS
    //
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    rtbd.BlendEnable    = true;
    rtbd.SrcBlend       = D3D11_BLEND_DEST_COLOR;
    rtbd.DestBlend      = D3D11_BLEND_ZERO;
    rtbd.BlendOp        = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha  = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[R_TRANSPARENCY]);
    CAssert::NotFailed(hr, "can't create a transparent blend state");

    //
    // alpha to coverage
    //
    D3D11_BLEND_DESC a2CDesc = { 0 };
    a2CDesc.AlphaToCoverageEnable = true;
    a2CDesc.IndependentBlendEnable = false;
    a2CDesc.RenderTarget[0].BlendEnable = false;
    a2CDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&a2CDesc, &blendStates_[R_ALPHA_TO_COVERAGE]);
    CAssert::NotFailed(hr, "can't create a blend state (alpha to coverage)");
}

//---------------------------------------------------------
// Desc:   initialize different depth stencil states
//---------------------------------------------------------
void RenderStates::InitAllDepthStencilStates(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;

    //
    // depth ENABLED
    //
    CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(D3D11_DEFAULT);
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilStates_[R_DEPTH_ENABLED]);
    CAssert::NotFailed(hr, "can't create a depth stencil state");

    //
    // depth DISABLED (for 2D rendering)
    //
    CD3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc(D3D11_DEFAULT);
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledStencilDesc.DepthEnable = false;

    hr = pDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &depthStencilStates_[R_DEPTH_DISABLED]);
    CAssert::NotFailed(hr, "can't create the depth disabled stencil state");


    // mark mirror:
    // this state is used to mark the position of a mirror on the stencil buffer, 
    // without changing the depth buffer. We will pair this with a new BlendState 
    // (noRenderTargetWritesBS) which will disable writing any color information 
    // to the backbuffer, so that we will have the combined effect which will 
    // be used to write only to the stencil.

    D3D11_DEPTH_STENCIL_DESC mirrorDesc;

    mirrorDesc.DepthEnable      = true;
    mirrorDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;
    mirrorDesc.DepthFunc        = D3D11_COMPARISON_LESS;
    mirrorDesc.StencilEnable    = true;
    mirrorDesc.StencilReadMask  = 0xff;
    mirrorDesc.StencilWriteMask = 0xff;

    mirrorDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
    mirrorDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

    mirrorDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    mirrorDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
    mirrorDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

    hr = pDevice->CreateDepthStencilState(&mirrorDesc, &depthStencilStates_[R_MARK_MIRROR]);
    CAssert::NotFailed(hr, "can't create a mark mirror depth stencil state");


    // draw reflection:
    // this state will be used to draw the geometry that should appear as a reflection
    // in mirror. We will set the stencil test up so that we will only render pixels
    // if they have been previously marked as part of the mirror by the MarkMirrorDSS.

    CD3D11_DEPTH_STENCIL_DESC drawReflectionDesc(D3D11_DEFAULT);

    drawReflectionDesc.DepthEnable      = true;
    drawReflectionDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
    drawReflectionDesc.DepthFunc        = D3D11_COMPARISON_LESS;
    drawReflectionDesc.StencilEnable    = true;
    drawReflectionDesc.StencilReadMask  = 0xff;
    drawReflectionDesc.StencilWriteMask = 0xff;

    drawReflectionDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_EQUAL;

    drawReflectionDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilFunc         = D3D11_COMPARISON_EQUAL;

    hr = pDevice->CreateDepthStencilState(&drawReflectionDesc, &depthStencilStates_[R_DRAW_REFLECTION]);
    CAssert::NotFailed(hr, "can't create a draw reflection depth stencil state");


    // no double blending:
    // this state will be used to draw our shadows. Because we are drawing shadows as
    // partially transparent black using alpha-blending, if we were to simply draw the 
    // shadow geometry, we would have darker patches where multiple surfaces of the shadow
    // object are projected to the shadow plane, a condition known as shadow-acne. Instead,
    // we setup the stencil test to check that the current stencil value is equal to the 
    // reference value, and increment on passes. Thus, the first time a projected pixel is
    // drawn, it will pass the stencil test, increment the stencil value, and be rendered.
    // On subsequent draws, the pixel will fail the stencil test.

    D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;

    noDoubleBlendDesc.DepthEnable      = true;
    noDoubleBlendDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
    noDoubleBlendDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
    noDoubleBlendDesc.StencilEnable    = true;
    noDoubleBlendDesc.StencilReadMask  = 0xff;
    noDoubleBlendDesc.StencilWriteMask = 0xff;

    noDoubleBlendDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_INCR;
    noDoubleBlendDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_EQUAL;

    noDoubleBlendDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_INCR;
    noDoubleBlendDesc.BackFace.StencilFunc         = D3D11_COMPARISON_EQUAL;

    hr = pDevice->CreateDepthStencilState(&noDoubleBlendDesc, &depthStencilStates_[R_NO_DOUBLE_BLEND]);
    CAssert::NotFailed(hr, "can't create a no double blend depth stencil state");

    //
    // for the SKY DOME rendering
    //
#if 1
    CD3D11_DEPTH_STENCIL_DESC skyDesc(D3D11_DEFAULT);
    skyDesc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;
    skyDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    hr = pDevice->CreateDepthStencilState(&skyDesc, &depthStencilStates_[R_SKY_DOME]);
    CAssert::NotFailed(hr, "can't create a depth stencil state");

#else
    D3D11_DEPTH_STENCIL_DESC skyDesc;
    
    skyDesc.DepthEnable                   = true;
    skyDesc.DepthWriteMask                = D3D11_DEPTH_WRITE_MASK_ZERO;
    skyDesc.DepthFunc                     = D3D11_COMPARISON_LESS_EQUAL;
    skyDesc.StencilEnable                 = true;
    skyDesc.StencilReadMask               = 0xff;
    skyDesc.StencilWriteMask              = 0xff;

    skyDesc.FrontFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    skyDesc.FrontFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    skyDesc.FrontFace.StencilPassOp       = D3D11_STENCIL_OP_INCR;
    skyDesc.FrontFace.StencilFunc         = D3D11_COMPARISON_EQUAL;

    skyDesc.BackFace.StencilFailOp        = D3D11_STENCIL_OP_KEEP;
    skyDesc.BackFace.StencilDepthFailOp   = D3D11_STENCIL_OP_KEEP;
    skyDesc.BackFace.StencilPassOp        = D3D11_STENCIL_OP_INCR;
    skyDesc.BackFace.StencilFunc          = D3D11_COMPARISON_EQUAL;

    hr = pDevice->CreateDepthStencilState(&skyDesc, &depthStencilStates_[SKY_DOME]);
    CAssert::NotFailed(hr, "can't create a depth stencil state");

#endif
}

//---------------------------------------------------------
// Desc:   return a pointer to some rasterizer state by input hash
//---------------------------------------------------------
ID3D11RasterizerState* RenderStates::GetRasterStateByHash(const uint8_t hash)
{
    try
    {
        return rasterStates_.at(hash);
    }
    catch (const std::out_of_range&)
    {
        LogErr(LOG, "there is no rasterizer state by hash: %x", hash);
        PrintErrAboutRSHash(hash);
        return nullptr;
    }
}

//---------------------------------------------------------
// Desc:  setup the rasterizer state hash according to the input params 
//---------------------------------------------------------
void RenderStates::UpdateRSHash(const std::set<eRenderState>& rsParams)
{
    for (const eRenderState param : rsParams)
    {
        switch (param)
        {
            // switch between rasterizer fill modes
            case R_FILL_SOLID:
            case R_FILL_WIREFRAME:
            {
                rasterStateHash_ &= turnOffFillModesHash_;   // turn off all the fill modes
                TurnOnRasterParam(param);
                break;
            }
            // switch between rasterizer culling modes
            case R_CULL_BACK:
            case R_CULL_FRONT:
            case R_CULL_NONE:
            {
                rasterStateHash_ &= turnOffCullModesHash_;   // turn off all the cull modes
                TurnOnRasterParam(param);
                break;
            }
            case R_FRONT_CLOCKWISE:
            {
                // &= ~(1 << rsParam);
                rasterStateHash_ &= ~(1 << R_FRONT_COUNTER_CLOCKWISE);  // turn off
                rasterStateHash_ |=  (1 << R_FRONT_CLOCKWISE);          // turn on
                break;
            }
            case R_FRONT_COUNTER_CLOCKWISE:
            {
                rasterStateHash_ &= ~(1 << R_FRONT_CLOCKWISE);          // turn off
                rasterStateHash_ |=  (1 << R_FRONT_COUNTER_CLOCKWISE);  // turn on
                break;
            }
            default:
            {
                LogErr(LOG, "unknown rasterizer state parameter: %d", param);
                return;
            }
        }
    }	
}

//---------------------------------------------------------
// Desc:   if we got somewhere some wrong hash we call this method to 
//         print an error message about it
//---------------------------------------------------------
void RenderStates::PrintErrAboutRSHash(const uint8_t bitfield)
{
    char bitfieldChars[8]{ '\0' };
    char rasterStatesNamesBuf[256] {'\0'};

    const std::map<eRenderState, const char*> rasterParamsNames =
    {
        {R_FILL_SOLID,              "FILL_SOLID"},
        {R_FILL_WIREFRAME,          "FILL_WIREFRAME"},
        {R_CULL_BACK,               "CULL_BACK"},
        {R_CULL_FRONT,              "CULL_FRONT"},
        {R_CULL_NONE,               "CULL_NONE"},
        {R_FRONT_COUNTER_CLOCKWISE, "FRONT_COUNTER_CLOCKWISE"},
        {R_FRONT_CLOCKWISE,         "FRONT_CLOCKWISE"},
    };

    // convert bitfield into string
    itoa(bitfield, bitfieldChars, 2);

    // go through each bitflag
    for (int i = 7; i >= 0; i--)
    {
        // get a name for this bigflag if it == 1
        if (bitfieldChars[i] == '1')
        {
            strcat(rasterStatesNamesBuf, rasterParamsNames.at((eRenderState)i));
            strcat(rasterStatesNamesBuf, ", ");
        }
    }

    // print error info about not existent rasterizer state
    LogErr(LOG, "can't get a raster state ptr by hash: %s", bitfieldChars);

    SetConsoleColor(RED);
    LogMsg("which is responsible to such params(at the same time) :");
    LogMsg(rasterStatesNamesBuf);
    SetConsoleColor(RESET);
}

} // namespace
