// *********************************************************************************
// Filename:     RenderStates.h
// Description:  constains implementation of the RenderStates class' functional
// 
// Created:      09.09.24
// *********************************************************************************
#include <CoreCommon/pch.h>
#include "RenderStates.h"

#pragma warning(disable : 4996)

namespace Core
{
    
RenderStates::RenderStates() 
{
}

RenderStates::~RenderStates() 
{ 
    DestroyAll(); 
}

// ********************************************************************************
//                            PUBLIC METHODS
// ********************************************************************************
void RenderStates::InitAll(ID3D11Device* pDevice, const bool multisampleEnable)
{
    InitAllRasterParams(pDevice, multisampleEnable);
    InitAllBlendStates(pDevice);
    InitAllDepthStencilStates(pDevice);

    // init some hashes to use it later during switching between some states
    turnOffFillModesHash_ &= ~(1 << FILL_SOLID);
    turnOffFillModesHash_ &= ~(1 << FILL_WIREFRAME);

    turnOffCullModesHash_ &= ~(1 << CULL_BACK);
    turnOffCullModesHash_ &= ~(1 << CULL_FRONT);
    turnOffCullModesHash_ &= ~(1 << CULL_NONE);
}

///////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////

ID3D11BlendState* RenderStates::GetBS(const eRenderState state)
{
    // return a ptr to the blending state by input key
    try
    {
        return blendStates_.at(state);
    }
    catch (const std::out_of_range&)
    {
        LogErr(LOG, "unknown blend state key: %d", state);
        return nullptr;
    }
}

///////////////////////////////////////////////////////////

ID3D11RasterizerState* RenderStates::GetRS(const std::set<eRenderState>& states)
{
    // return a ptr to the rasterizer state by input key
    return nullptr;
}

///////////////////////////////////////////////////////////

ID3D11DepthStencilState* RenderStates::GetDSS(const eRenderState state)
{
    // return a ptr to the depth stencil state by state enum key
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

///////////////////////////////////////////////////////////

void RenderStates::SetRS(ID3D11DeviceContext* pContext, const eRenderState state)
{
    SetRS(pContext, std::set<eRenderState>{state});
}

///////////////////////////////////////////////////////////

void RenderStates::SetRS(ID3D11DeviceContext* pContext, const std::set<eRenderState>& states)
{
    // set up a raster state according to the input states params

    UpdateRSHash(states);
    pContext->RSSetState(GetRasterStateByHash(GetCurrentRSHash()));
}

///////////////////////////////////////////////////////////

void RenderStates::SetRSByHash(ID3D11DeviceContext* pContext, const uint8_t hash)
{
    // set up a raster state according to the input hash

    ID3D11RasterizerState* pRS = nullptr;

    // if the RS by input hash is valid AND we don't want to set the same RS
    if ((rasterStateHash_ != hash) && (pRS = GetRasterStateByHash(hash)))
    {
        pContext->RSSetState(pRS);
        rasterStateHash_ = hash;
    }
}

///////////////////////////////////////////////////////////

void RenderStates::SetBS(ID3D11DeviceContext* pContext, const eRenderState state)
{
    // set a blending state by input key
    try
    {
        ID3D11BlendState* pBS = blendStates_.at(state);

        switch (state)
        {
            case ALPHA_DISABLE:
            case ALPHA_ENABLE:
            case ADDING:
            case SUBTRACTING:
            case MULTIPLYING:
            case ALPHA_TO_COVERAGE:
            {
                pContext->OMSetBlendState(pBS, NULL, 0xFFFFFFFF);
                break;
            }
            case TRANSPARENCY:
            case NO_RENDER_TARGET_WRITES:
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

///////////////////////////////////////////////////////////

void RenderStates::SetDSS(
    ID3D11DeviceContext* pContext, 
    const eRenderState state, 
    const UINT stencilRef)
{
    // set a depth stencil state by input key
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


// ********************************************************************************
//                            PRIVATE METHODS
// ********************************************************************************

void InitFillRasterParams()
{

}
void InitCullRasterParams();

void RenderStates::InitAllRasterParams(ID3D11Device* pDevice, bool multisampleEnable)
{
    // create/set up the rasterizer state objects
    // 
    // firstly we set up a description for a raster state, then we create it
    // after creation we generate a hash for this particular raster state
    // and insert into the map created pairs ['hash' => 'ptr_to_raster_state']

    try
    {
        HRESULT hr = S_OK;
        ID3D11RasterizerState* pRasterState = nullptr;
        D3D11_RASTERIZER_DESC desc;
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

        UpdateRSHash({ FILL_SOLID, CULL_BACK, FRONT_CLOCKWISE });
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
        UpdateRSHash({ FILL_SOLID, CULL_BACK, FRONT_COUNTER_CLOCKWISE });
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
        UpdateRSHash({ FILL_SOLID, CULL_NONE, FRONT_COUNTER_CLOCKWISE });
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
        UpdateRSHash({ FILL_SOLID, CULL_NONE, FRONT_CLOCKWISE });
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
        UpdateRSHash({ FILL_SOLID, CULL_FRONT, FRONT_CLOCKWISE });
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
        UpdateRSHash({ FILL_WIREFRAME, CULL_BACK, FRONT_CLOCKWISE });
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
        UpdateRSHash({ FILL_WIREFRAME, CULL_FRONT, FRONT_CLOCKWISE });
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
        UpdateRSHash({ FILL_WIREFRAME, CULL_NONE, FRONT_CLOCKWISE });
        rasterStates_.insert({ GetCurrentRSHash(), pRasterState });


        //
        // AFTER ALL: reset the rasterizer state hash after initialization
        //            and set the default params
        ResetRasterStateHash();
        UpdateRSHash({ FILL_SOLID, CULL_BACK, FRONT_CLOCKWISE });
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        throw EngineException("can't initialize the rasterizer state");
    }
}

///////////////////////////////////////////////////////////

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

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[NO_RENDER_TARGET_WRITES]);
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

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[ALPHA_DISABLE]);
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

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[ALPHA_ENABLE]);
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

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[ADDING]);
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

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[SUBTRACTING]);
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

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[MULTIPLYING]);
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

    hr = pDevice->CreateBlendState(&blendDesc, &blendStates_[TRANSPARENCY]);
    CAssert::NotFailed(hr, "can't create a transparent blend state");

    //
    // alpha to coverage
    //
    D3D11_BLEND_DESC a2CDesc = { 0 };
    a2CDesc.AlphaToCoverageEnable = true;
    a2CDesc.IndependentBlendEnable = false;
    a2CDesc.RenderTarget[0].BlendEnable = false;
    a2CDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = pDevice->CreateBlendState(&a2CDesc, &blendStates_[ALPHA_TO_COVERAGE]);
    CAssert::NotFailed(hr, "can't create a blend state (alpha to coverage)");
}

///////////////////////////////////////////////////////////

void RenderStates::InitAllDepthStencilStates(ID3D11Device* pDevice)
{
    // initialize different depth stencil states
    
    HRESULT hr = S_OK;

    //
    // depth ENABLED
    //
    CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(D3D11_DEFAULT);
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilStates_[DEPTH_ENABLED]);
    CAssert::NotFailed(hr, "can't create a depth stencil state");

    //
    // depth DISABLED (for 2D rendering)
    //
    CD3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc(D3D11_DEFAULT);
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledStencilDesc.DepthEnable = false;

    hr = pDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &depthStencilStates_[DEPTH_DISABLED]);
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

    hr = pDevice->CreateDepthStencilState(&mirrorDesc, &depthStencilStates_[MARK_MIRROR]);
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

    hr = pDevice->CreateDepthStencilState(&drawReflectionDesc, &depthStencilStates_[DRAW_REFLECTION]);
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

    hr = pDevice->CreateDepthStencilState(&noDoubleBlendDesc, &depthStencilStates_[NO_DOUBLE_BLEND]);
    CAssert::NotFailed(hr, "can't create a no double blend depth stencil state");

    //
    // for the SKY DOME rendering
    //
    CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
    desc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    hr = pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilStates_[SKY_DOME]);
    CAssert::NotFailed(hr, "can't create a depth stencil state");
}

///////////////////////////////////////////////////////////

ID3D11RasterizerState* RenderStates::GetRasterStateByHash(const uint8_t hash)
{
    // return a pointer to some rasterizer state by hash
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

///////////////////////////////////////////////////////////

void RenderStates::UpdateRSHash(const std::set<eRenderState>& rsParams)
{
    // setup the rasterizer state hash according to the params 

    for (const eRenderState param : rsParams)
    {
        switch (param)
        {
            // switch between rasterizer fill modes
            case FILL_SOLID:
            case FILL_WIREFRAME:
            {
                rasterStateHash_ &= turnOffFillModesHash_;   // turn off all the fill modes
                TurnOnRasterParam(param);
                break;
            }
            // switch between rasterizer culling modes
            case CULL_BACK:
            case CULL_FRONT:
            case CULL_NONE:
            {
                rasterStateHash_ &= turnOffCullModesHash_;   // turn off all the cull modes
                TurnOnRasterParam(param);
                break;
            }
            case FRONT_CLOCKWISE:
            {
                // &= ~(1 << rsParam);
                rasterStateHash_ &= ~(1 << FRONT_COUNTER_CLOCKWISE); // turn off
                rasterStateHash_ |= (1 << FRONT_CLOCKWISE);          // turn on
                break;
            }
            case FRONT_COUNTER_CLOCKWISE:
            {
                rasterStateHash_ &= ~(1 << FRONT_CLOCKWISE);         // turn off
                rasterStateHash_ |= (1 << FRONT_COUNTER_CLOCKWISE);  // turn on
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

///////////////////////////////////////////////////////////

void RenderStates::PrintErrAboutRSHash(const uint8_t bitfield)
{
    // if we got somewhere some wrong hash we call this method to 
    // print an error message about it

    char bitfieldChars[8]{ '\0' };
    char rasterStatesNamesBuf[256] {'\0'};

    const std::map<eRenderState, const char*> rasterParamsNames =
    {
        {FILL_SOLID,              "FILL_SOLID"},
        {FILL_WIREFRAME,          "FILL_WIREFRAME"},
        {CULL_BACK,               "CULL_BACK"},
        {CULL_FRONT,              "CULL_FRONT"},
        {CULL_NONE,               "CULL_NONE"},
        {FRONT_COUNTER_CLOCKWISE, "FRONT_COUNTER_CLOCKWISE"},
        {FRONT_CLOCKWISE,         "FRONT_CLOCKWISE"},
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

} // namespace Core
