// *********************************************************************************
// Filename:     r_states.cpp
// 
// Created:      09.09.24
// *********************************************************************************
#include "../Common/pch.h"
#include "r_states.h"
#include "r_state_reader.h"
#include <inttypes.h>      // for SCNx8

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
    assert(pDevice);
    pDevice_ = pDevice;

    pDevice->GetImmediateContext(&pCtx_);

    const char* rsCfgFilepath  = "data/rasterizer_states.cfg";
    const char* bsCfgFilepath  = "data/blending_states.cfg";
    const char* dssCfgFilepath = "data/depth_stencil_states.cfg";

    LoadRs (rsCfgFilepath, multisampleEnable);
    LoadBs (bsCfgFilepath);
    LoadDss(dssCfgFilepath);

    ResetRenderStates();

    PrintDumpRs();
    PrintDumpBs();
    PrintDumpDss();
}

//---------------------------------------------------------
// Desc:   destroy all the rasterizer,blending,depth-stencil states
//---------------------------------------------------------
void RenderStates::DestroyAll()
{
    for (ID3D11RasterizerState* pRS : rsPtrs_)
        SafeRelease(&pRS);

    for (ID3D11BlendState* pBS : bsPtrs_)
        SafeRelease(&pBS);

    for (ID3D11DepthStencilState* pDSS : dssPtrs_)
        SafeRelease(&pDSS);

    rsId_ = 0;
    bsId_ = 0;
    dssId_ = 0;

    rsIds_.purge();
    rsNames_.purge();
    rsPtrs_.purge();
    rsParams_.purge();

    bsIds_.purge();
    bsNames_.purge();
    bsPtrs_.purge();
    bsParams_.purge();

    dssIds_.purge();
    dssNames_.purge();
    dssPtrs_.purge();
    dssParams_.purge();
}


//**********************************************************************************
// HANDLERS FOR RASTERIZER STATES
//**********************************************************************************

//---------------------------------------------------------
// Desc:  create and register new rasterizer state
// Args:  - name:  a name for this rs
//        - desc:  description which is used to create this rs
// Ret:   id of created rasterizer state
//---------------------------------------------------------
RsID RenderStates::AddRs(const char* name, D3D11_RASTERIZER_DESC& desc)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name");
        return 0;
    }

    HRESULT hr = S_OK;
    ID3D11RasterizerState* pRS = nullptr;


    assert(pDevice_);
    hr = pDevice_->CreateRasterizerState(&desc, &pRS);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create raster state: %s", name);
        return 0;
    }
    rsPtrs_.push_back(pRS);

    // setup id
    const RsID id = rsId_;
    rsId_++;
    rsIds_.push_back(id);

    // setup name
    rsNames_.push_back(RenderStateName());
    strncpy(rsNames_[id].name, name, MAX_LEN_RND_STATE_NAME);

    // save description of this rasterizer state
    rsParams_.push_back(RsParams());
    CacheRsDesc(id, desc);

    return id;
}

//---------------------------------------------------------
// Desc:  return an ID of rasterizer state by input name
//        or return 0 if there is no such state
//---------------------------------------------------------
RsID RenderStates::GetRsId(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty rs name");
        return 0;
    }

    for (index i = 0; i < rsNames_.size(); ++i)
    {
        if (strcmp(rsNames_[i].name, name) == 0)
            return rsIds_[i];
    }

    LogErr(LOG, "no raster state by name: %s", name);
    PrintDumpRs();
    return 0;
}

//---------------------------------------------------------
// Desc:  return a name of rasterizer state by input id
//---------------------------------------------------------
const char* RenderStates::GetRsName(const RsID id) const
{
    if (!IsRsExist(id))
    {
        LogErr(LOG, "no raster state by id: %d", (int)id);
        return rsNames_[0].name;
    }

    return rsNames_[id].name;
}

//---------------------------------------------------------
// Desc:  bind (set) a raster state by id to output merget stage (OM)
//---------------------------------------------------------
void RenderStates::SetRs(const RsID id)
{
    assert(pCtx_);
    pCtx_->RSSetState(GetRs(id));
}

//---------------------------------------------------------
// Desc:  return a ptr to rasterizer state by input id
//---------------------------------------------------------
ID3D11RasterizerState* RenderStates::GetRs(const RsID id)
{
    if (!IsRsExist(id))
    {
        LogErr(LOG, "invalid raster state id: %d", (int)id);
        return nullptr;
    }

    return rsPtrs_[id];
}

//---------------------------------------------------------
// Desc:  do we have a rasterizer state by input id ?
//---------------------------------------------------------
bool RenderStates::IsRsExist(const RsID id) const
{
    return (id < rsIds_.size());
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" rasterizer state (its id == 1)
//        with input value and recreate this state
//-----------------------------------------------------
void RenderStates::UpdateCustomRsParam(const eRsParamType type, const bool value)
{
    const RsID customRsId = 1;

    if (!IsRsExist(customRsId))
    {
        LogErr(LOG, "no raster state by id: %d", customRsId);
        return;
    }

    RsParams& params = rsParams_[customRsId];

    switch (type)
    {
        case RS_FRONT_COUNTER_CLOCKWISE:  params.frontCounterClockwise = value;  break;
        case RS_DEPTH_CLIP_ENABLE:        params.depthClipEnable = value;        break;
        case RS_SCISSOR_ENABLE:           params.scissorEnable = value;          break;
        case RS_MULTISAMPLE_ENABLE:       params.multisampleEnable = value;      break;
        case RS_ANTIALIASED_LINE_ENABLE:  params.antialiasedLineEnable = value;  break;

        default:
            LogErr(LOG, "you can't setup this rs parameter with bool value (param_type: %d, value: %d)", (int)type, value);
            return;
    }

    UpdateRs(customRsId, GetRsDescByParams(params));
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" rasterizer state (its id == 1)
//        with input value and recreate this state
//-----------------------------------------------------
void RenderStates::UpdateCustomRsParam(const eRsParamType type, const char* value)
{
    const RsID customRsId = 1;

    if (!IsRsExist(customRsId))
    {
        LogErr(LOG, "no raster state by id: %d", customRsId);
        return;
    }
    if (StrHelper::IsEmpty(value))
    {
        LogErr(LOG, "empty value");
        return;
    }


    RsParams& params = rsParams_[customRsId];
    RenderStateReader rStateReader;

    switch (type)
    {
        case RS_FILL:   params.fillMode = rStateReader.GetFillMode(value);  break;
        case RS_CULL:   params.cullMode = rStateReader.GetCullMode(value);  break;

        default:
            LogErr(LOG, "you can't setup this rs parameter with str value (param_type: %d, value: %s)", (int)type, value);
            return;
    }

    UpdateRs(customRsId, GetRsDescByParams(params));
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" rasterizer state (its id == 1)
//        with input value and recreate this state
//-----------------------------------------------------
void RenderStates::UpdateCustomRsParam(const eRsParamType type, const int value)
{
    const RsID customRsId = 1;

    if (!IsRsExist(customRsId))
    {
        LogErr(LOG, "no raster state by id: %d", customRsId);
        return;
    }
    if (type != RS_DEPTH_BIAS)
    {
        LogErr(LOG, "you can't setup this rs param with integer value (param_type: %d, value: %d)", (int)type, value);
        return;
    }

    RsParams& params = rsParams_[customRsId];
    params.depthBias = value;
    UpdateRs(customRsId, GetRsDescByParams(params));
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" rasterizer state (its id == 1)
//        with input value and recreate this state
//-----------------------------------------------------
void RenderStates::UpdateCustomRsParam(const eRsParamType type, const float value)
{
    const RsID customRsId = 1;

    if (!IsRsExist(customRsId))
    {
        LogErr(LOG, "no raster state by id: %d", customRsId);
        return;
    }

    RsParams& params = rsParams_[customRsId];

    switch (type)
    {
        case RS_DEPTH_BIAS_CLAMP:         params.depthBiasClamp       = value;  break;
        case RS_SLOPE_SCALED_DEPTH_BIAS:  params.slopeScaledDepthBias = value;  break;

        default:
            LogErr(LOG, "you can't setup this rs param with float value (param_type: %d, value: %d)", (int)type, value);
            return;
    }

    UpdateRs(customRsId, GetRsDescByParams(params));
}


//**********************************************************************************
// HANDLERS FOR BLEND STATES
//**********************************************************************************

//---------------------------------------------------------
// Desc:  create and register new blending state
// Args:  - name:  a name for this bs
//        - desc:  description which is used to create this bs
// Ret:   id of created blend state
//---------------------------------------------------------
BsID RenderStates::AddBs(const char* name, const D3D11_BLEND_DESC& desc)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name");
        return 0;
    }

    HRESULT hr = S_OK;
    ID3D11BlendState* pBS = nullptr;


    assert(pDevice_);
    hr = pDevice_->CreateBlendState(&desc, &pBS);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create a blend state: %s", name);
        return 0;
    }
    bsPtrs_.push_back(pBS);

    // setup id
    const BsID id = bsId_;
    bsIds_.push_back(id);
    bsId_++;

    // setup name
    bsNames_.push_back(RenderStateName());
    strncpy(bsNames_[id].name, name, MAX_LEN_RND_STATE_NAME);

    // save description of this blend state
    bsParams_.push_back(BsParams());
    CacheBsDesc(id, desc);

    return id;
}

//---------------------------------------------------------
// Desc:  return an ID of blend state by input name
//        or return 0 if there is no such state
//---------------------------------------------------------
BsID RenderStates::GetBsId(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty bs name");
        return 0;
    }

    for (index i = 0; i < bsNames_.size(); ++i)
    {
        if (strcmp(bsNames_[i].name, name) == 0)
            return bsIds_[i];
    }

    LogErr(LOG, "no blend state by name: %s", name);
    return 0;
}

//---------------------------------------------------------
// Desc:  return a name of blend state by input id
//---------------------------------------------------------
const char* RenderStates::GetBsName(const BsID id) const
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "no blend state by id: %d", (int)id);
        return bsNames_[0].name;
    }

    return bsNames_[id].name;
}

//---------------------------------------------------------
// Desc:  bind (set) a blend state by id to output merger stage (OM)
//---------------------------------------------------------
void RenderStates::SetBs(const BsID id)
{
    assert(pCtx_);
    pCtx_->OMSetBlendState(GetBs(id), NULL, 0xFFFFFFFF);
}

//---------------------------------------------------------
// Desc:  return a ptr to blend state by input id
//---------------------------------------------------------
ID3D11BlendState* RenderStates::GetBs(const BsID id)
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "invalid blend state id: %d", (int)id);
        return nullptr;
    }

    return bsPtrs_[id];
}

//---------------------------------------------------------
// Desc:  do we have a blending state by id ?
//---------------------------------------------------------
bool RenderStates::IsBsExist(const BsID id) const
{
    return (id < bsIds_.size());
}

//---------------------------------------------------------
// Desc:  is blending enabled for blend state by id ?
// Out:   - isTransparent:   is for drawing transparent objects?
//        - blendEnabled:    is blending enabled?
//---------------------------------------------------------
void RenderStates::IsBlendEnabled(
    const BsID id,
    bool& isTransparent,
    bool& blendEnabled) const
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "invalid blend state id: %d", (int)id);
        isTransparent = false;
        blendEnabled = false;
        return;
    }

    isTransparent = (bsParams_[id].srcBlend == D3D11_BLEND_SRC_ALPHA &&
                     bsParams_[id].dstBlend == D3D11_BLEND_INV_SRC_ALPHA);

    blendEnabled = bsParams_[id].isBlendEnabled;
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" rasterizer state (its id == 1)
//        with input value and recreate this state
//-----------------------------------------------------
void RenderStates::UpdateCustomBsParam(const eBsParamType type, const bool value)
{
    const BsID customBsId = 1;

    if (!IsBsExist(customBsId))
    {
        LogErr(LOG, "no blend state by id: %d", customBsId);
        return;
    }

    BsParams& params = bsParams_[customBsId];

    switch (type)
    {
        case BLEND_IS_ALPHA_TO_COVERAGE:  params.isAlphaToCoverage = value;  break;
        case BLEND_IS_INDEPENDENT:        params.isIndependentBlend = value; break;
        case BLEND_IS_ENABLED:            params.isBlendEnabled = value;     break;

        default:
            LogErr(LOG, "you can't setup this bs param with bool value (param_type: %d, value: %d)", (int)type, value);
            return;
    }

    UpdateBs(customBsId, GetBsDescByParams(params));
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" rasterizer state (its id == 1)
//        with input value and recreate this state
//-----------------------------------------------------
void RenderStates::UpdateCustomBsParam(const eBsParamType type, const char* value)
{
    const BsID customBsId = 1;

    if (!IsBsExist(customBsId))
    {
        LogErr(LOG, "no blend state by id: %d", customBsId);
        return;
    }

    BsParams& params = bsParams_[customBsId];
    RenderStateReader rStateReader;

    switch (type)
    {
        case BLEND_SRC_BLEND:
            params.srcBlend = rStateReader.GetBlendFactor(value);
            break;

        case BLEND_DST_BLEND:
            params.dstBlend = rStateReader.GetBlendFactor(value);
            break;

        case BLEND_OP:
            params.blendOp = rStateReader.GetBlendOperation(value);
            break;

        case BLEND_SRC_BLEND_ALPHA:
            params.srcBlendAlpha = rStateReader.GetBlendFactor(value);
            break;

        case BLEND_DST_BLEND_ALPHA:
            params.dstBlendAlpha = rStateReader.GetBlendFactor(value);
            break;

        case BLEND_OP_ALPHA:
            params.blendOpAlpha = rStateReader.GetBlendOperation(value);
            break;

        case BLEND_RND_TARGET_WRITE_MASK:
            params.writeMask = rStateReader.GetRenderTargetWriteMask(value);
            break;

        default:
            LogErr(LOG, "you can't setup this bs param with bool value (param_type: %d, value: %d)", (int)type, value);
            return;
    }

    UpdateBs(customBsId, GetBsDescByParams(params));
}


//**********************************************************************************
// HANDLERS FOR DEPTH-STENCIL STATES
//**********************************************************************************

//---------------------------------------------------------
// Desc:  create and register a new depth-stencil state (dss)
// Args:  - name:  a name for this dss
//        - desc:  description which is used to create this dss
// Ret:   id of created depth-stencil state
//---------------------------------------------------------
DssID RenderStates::AddDss(const char* name, D3D11_DEPTH_STENCIL_DESC& desc)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name");
        return 0;
    }

    HRESULT hr = S_OK;
    ID3D11DepthStencilState* pDSS = nullptr;


    assert(pDevice_);
    hr = pDevice_->CreateDepthStencilState(&desc, &pDSS);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create depth-stencil state: %s", name);
        return 0;
    }
    dssPtrs_.push_back(pDSS);

    // setup id
    const DssID id = dssId_;
    dssIds_.push_back(id);
    dssId_++;

    // setup name
    dssNames_.push_back(RenderStateName());
    strncpy(dssNames_[id].name, name, MAX_LEN_RND_STATE_NAME);

    // save description of this depth-stencil state
    dssParams_.push_back(DssParams());
    CacheDssDesc(id, desc);

    return id;
}

//---------------------------------------------------------
// Desc:  return an ID of depth-stencil state by input name
//        or return 0 if there is no such state
//---------------------------------------------------------
DssID RenderStates::GetDssId(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty dss name");
        return 0;
    }

    for (index i = 0; i < dssNames_.size(); ++i)
    {
        if (strcmp(dssNames_[i].name, name) == 0)
            return dssIds_[i];
    }

    LogErr(LOG, "no depth-stencil state by name: %s", name);
    PrintDumpDss();
    return 0;
}

//---------------------------------------------------------
// Desc:  return a name of depth-stencil state by input id
//---------------------------------------------------------
const char* RenderStates::GetDssName(const DssID id) const
{
    if (!IsDssExist(id))
    {
        LogErr(LOG, "no depth-stencil state by id: %d", (int)id);
        return dssNames_[0].name;
    }

    return dssNames_[id].name;
}

//---------------------------------------------------------
// Desc:  return a ptr to depth-stencil state by input id
//---------------------------------------------------------
ID3D11DepthStencilState* RenderStates::GetDss(const DssID id)
{
    if (!IsDssExist(id))
    {
        LogErr(LOG, "invalid depth-stencil state id: %d", (int)id);
        return nullptr;
    }

    return dssPtrs_[id];
}

//---------------------------------------------------------
// Desc:  check that we have a depth-stencil state by input id
//---------------------------------------------------------
bool RenderStates::IsDssExist(const DssID id) const
{
    return (id < dssIds_.size());
}

//---------------------------------------------------------
// Desc:  bind (set) a depth-stencil state by id to output merget stage (OM)
//---------------------------------------------------------
void RenderStates::SetDss(const DssID id, const UINT stencilRef)
{
    assert(pCtx_);
    pCtx_->OMSetDepthStencilState(GetDss(id), stencilRef);
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" depth-stencil (its id == 1)
//        with input boolean value
//-----------------------------------------------------
void RenderStates::UpdateCustomDssParam(const eDssParamType type, const bool value)
{
    const DssID customDssId = 1;

    if (!IsDssExist(customDssId))
    {
        LogErr(LOG, "no custom depth-stencil state by id == 1");
        return;
    }

    DssParams& params = dssParams_[customDssId];

    switch (type)
    {
        case DSS_DEPTH_ENABLED:     params.depthEnable = value;     break;
        case DSS_STENCIL_ENABLED:   params.stencilEnable = value;   break;

        default:
            LogErr(LOG, "invalid depth-stencil param type: %d", (int)type);
            return;
    }

    UpdateDss(customDssId, GetDssDescByParams(params));
}

//-----------------------------------------------------
// Desc:  update a property of the "custom" depth-stencil (its id == 1)
//        with input string value
//-----------------------------------------------------
void RenderStates::UpdateCustomDssParam(const eDssParamType type, const char* value)
{
    const DssID customDssId = 1;

    if (!IsDssExist(customDssId))
    {
        LogErr(LOG, "no depth-stencil state by id: %d", (int)customDssId);
        return;
    }
    if (StrHelper::IsEmpty(value))
    {
        LogErr(LOG, "empty str value");
        return;
    }

    DssParams& params = dssParams_[customDssId];
    RenderStateReader rndStateReader;

    switch (type)
    {
        case DSS_DEPTH_WRITE_MASK:
            params.depthWriteMask = rndStateReader.GetDepthWriteMask(value);
            break;

        case DSS_DEPTH_FUNC:
            params.depthFunc = rndStateReader.GetCmpFunc(value);
            break;

        case DSS_STENCIL_READ_MASK:
        {
            uint8 mask = 0xFF;
            int count = sscanf(value, "%" SCNx8, &mask);
            assert(count == 1);
            params.stencilReadMask = mask;
            break;
        }
        case DSS_STENCIL_WRITE_MASK:
        {
            uint8 mask = 0xFF;
            int count = sscanf(value, "%" SCNx8, &mask);
            assert(count == 1);
            params.stencilWriteMask = mask;
            break;
        }

        case DSS_FRONT_FACE_STENCIL_FAIL_OP:
            params.ffStencilFailOp = rndStateReader.GetStencilOp(value);
            break;

        case DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP:
            params.ffStencilDepthFailOp = rndStateReader.GetStencilOp(value);
            break;

        case DSS_FRONT_FACE_STENCIL_PASS_OP:
            params.ffStencilPassOp = rndStateReader.GetStencilOp(value);
            break;

        case DSS_FRONT_FACE_STENCIL_FUNC:
            params.ffStencilFunc = rndStateReader.GetCmpFunc(value);
            break;

        case DSS_BACK_FACE_STENCIL_FAIL_OP:
            params.bfStencilFailOp = rndStateReader.GetStencilOp(value);
            break;

        case DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP:
            params.bfStencilDepthFailOp = rndStateReader.GetStencilOp(value);
            break;

        case DSS_BACK_FACE_STENCIL_PASS_OP:
            params.bfStencilPassOp = rndStateReader.GetStencilOp(value);
            break;

        case DSS_BACK_FACE_STENCIL_FUNC:
            params.bfStencilFunc = rndStateReader.GetCmpFunc(value);
            break;

        default:
            LogErr(LOG, "invalid depth-stencil param type: %d", (int)type);
            return;
    }

    UpdateDss(customDssId, GetDssDescByParams(params));
}

//-----------------------------------------------------
// get render states params names
//-----------------------------------------------------
#define NUM_RS_FILL_MODES 2
#define NUM_RS_CULL_MODES 3

#define NUM_BS_OPERATIONS 5
#define NUM_BS_FACTORS 13
#define NUM_BS_RENDER_TARGET_WRITE_MASKS 6

#define NUM_DSS_DEPTH_WRITE_MASKS 2
#define NUM_DSS_CMP_FUNCTIONS 8
#define NUM_DSS_STENCIL_OPERATIONS 8


const char* s_InvalidStr[1] = { "invalid" };

const char* s_RasterFillModeNames[NUM_RS_FILL_MODES] = { "wireframe", "solid" };
const char* s_RasterCullModeNames[NUM_RS_CULL_MODES] = { "none", "front", "back"};

const char* s_BlendOpNames[NUM_BS_OPERATIONS] =
{
    "add", "sub", "rev_sub", "min", "max"
};

const char* s_BlendFactorNames[NUM_BS_FACTORS] =
{
    "zero", "one",
    "src_col", "inv_src_col",
    "src_alpha", "inv_src_alpha",
    "dst_alpha", "inv_dst_alpha",
    "dst_col", "inv_dst_col",
    "src_alpha_sat", "blend_factor",
    "inv_blend_factor"
};

const char* s_BlendRenderTargetWriteMaskNames[NUM_BS_RENDER_TARGET_WRITE_MASKS] =
{
    "zero", "red", "green", "blue", "alpha", "all"
};

const char* s_DssDepthWriteMasksNames[NUM_DSS_DEPTH_WRITE_MASKS] =
{
    "zero", "all"
};

const char* s_DssComparisonFuncNames[NUM_DSS_CMP_FUNCTIONS] =
{
    "never", "less", "equal", "less_equal", "greater",
    "not_equal", "greater_equal", "always"
};

const char* s_DssStencilOpsNames[NUM_DSS_STENCIL_OPERATIONS] =
{
    "keep", "zero", "replace", "incr_sat", "decr_sat", "invert", "incr", "decr"
};

//---------------------------------------------------------
// Desc:  return a number of render state params related to particular group
//---------------------------------------------------------
int RenderStates::GetNumRsParams(const eRsParamType type) const
{
    switch (type)
    {
        case RS_FILL:                      return NUM_RS_FILL_MODES;
        case RS_CULL:                      return NUM_RS_CULL_MODES;
    }

    LogErr(LOG, "invalid rs param type: %d", (int)type);
    return 0;
}

//---------------------------------------------------------
// Desc:  return a number of blend state params related to particular group
//---------------------------------------------------------
int RenderStates::GetNumBsParams(const eBsParamType type) const
{
    switch (type)
    {
        case BLEND_OPERATION:              return NUM_BS_OPERATIONS;
        case BLEND_FACTOR:                 return NUM_BS_FACTORS;
        case BLEND_RND_TARGET_WRITE_MASK:  return NUM_BS_RENDER_TARGET_WRITE_MASKS;
    }

    LogErr(LOG, "invalid blend state parameters type: %d", (int)type);
    return 0;
}

//---------------------------------------------------------
// Desc:  return a number of depth-stencil state params related to particular group
//---------------------------------------------------------

int RenderStates::GetNumDssParams(const eDssParamType type) const
{
    switch (type)
    {
        case DSS_DEPTH_WRITE_MASK:         return NUM_DSS_DEPTH_WRITE_MASKS;
        case DSS_COMPARISON_FUNC:          return NUM_DSS_CMP_FUNCTIONS;
        case DSS_STENCIL_OP:               return NUM_DSS_STENCIL_OPERATIONS;
    }

    LogErr(LOG, "invalid depth-stencil param type: %d", (int)type);
    return 0;
}

//---------------------------------------------------------
// Desc:  return a names arr of rasterizer state params related to particular group
//---------------------------------------------------------
const char** RenderStates::GetArrRsParamsNames(const eRsParamType type) const
{
    switch (type)
    {
        case RS_FILL:                      return s_RasterFillModeNames;
        case RS_CULL:                      return s_RasterCullModeNames;
    }

    LogErr(LOG, "invalid rasterizer param type: %d", (int)type);
    return s_InvalidStr;
}

//---------------------------------------------------------
// Desc:  return a names arr of blend state params related to particular group
//---------------------------------------------------------
const char** RenderStates::GetArrBsParamsNames(const eBsParamType type) const
{
    switch (type)
    {
        case BLEND_OPERATION:              return s_BlendOpNames;
        case BLEND_FACTOR:                 return s_BlendFactorNames;
        case BLEND_RND_TARGET_WRITE_MASK:  return s_BlendRenderTargetWriteMaskNames;

    }

    LogErr(LOG, "invalid blend params type: %d", (int)type);
    return s_InvalidStr;
}

//---------------------------------------------------------
// Desc:  return a names arr of depth-stencil state params related to particular group
//---------------------------------------------------------
const char** RenderStates::GetArrDssParamsNames(const eDssParamType type) const
{
    switch (type)
    {
        case DSS_DEPTH_WRITE_MASK:         return s_DssDepthWriteMasksNames;
        case DSS_COMPARISON_FUNC:          return s_DssComparisonFuncNames;
        case DSS_STENCIL_OP:               return s_DssStencilOpsNames;
    }

    LogErr(LOG, "invalid depth-stencil params type: %d", (int)type);
    return s_InvalidStr;
}

//---------------------------------------------------------
// Desc:  get parameter's current value in string-form for rasterizer state by id
//---------------------------------------------------------
const char* RenderStates::GetRsParamStr(const RsID id, const eRsParamType type) const
{
    if (!IsRsExist(id))
    {
        LogErr(LOG, "invalid raster state id: %d", (int)id);
        return s_InvalidStr[0];
    }

    switch (type)
    {
        case RS_FILL:
        {
            const D3D11_FILL_MODE fill = D3D11_FILL_MODE(rsParams_[id].fillMode);

            if (fill == D3D11_FILL_WIREFRAME)
                return s_RasterFillModeNames[0];

            if (fill == D3D11_FILL_SOLID)
                return s_RasterFillModeNames[1];
        }
        case RS_CULL:
        {
            const D3D11_CULL_MODE cull = D3D11_CULL_MODE(rsParams_[id].cullMode);

            if (cull == D3D11_CULL_NONE)
                return s_RasterCullModeNames[0];

            if (cull == D3D11_CULL_FRONT)
                return s_RasterCullModeNames[1];

            if (cull == D3D11_CULL_BACK)
                return s_RasterCullModeNames[2];
        }
    }

    LogErr(LOG, "invalid rasterizer state param type (rs_id: %d, param_type: %d)", (int)id, (int)type);
    return s_InvalidStr[0];
}

//---------------------------------------------------------
// Desc:  get parameter's current value in string-form for blend state by id
//---------------------------------------------------------
const char* RenderStates::GetBsParamStr(const BsID id, const eBsParamType type) const
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "invalid blend state id: %d", (int)id);
        return s_InvalidStr[0];
    }

    const BsParams& params = bsParams_[id];

    switch (type)
    {
        case BLEND_RND_TARGET_WRITE_MASK:
            return GetStrByBlendWriteMask(params.writeMask);

        // blend factors
        case BLEND_SRC_BLEND:
            return GetStrByBlendFactor(D3D11_BLEND(params.srcBlend));

        case BLEND_DST_BLEND:
            return GetStrByBlendFactor(D3D11_BLEND(params.dstBlend));

        case BLEND_SRC_BLEND_ALPHA:
            return GetStrByBlendFactor(D3D11_BLEND(params.srcBlendAlpha));

        case BLEND_DST_BLEND_ALPHA:
            return GetStrByBlendFactor(D3D11_BLEND(params.dstBlendAlpha));

        // blend operations
        case BLEND_OP:
            return GetStrByBlendOp(D3D11_BLEND_OP(params.blendOp));

        case BLEND_OP_ALPHA:
            return GetStrByBlendOp(D3D11_BLEND_OP(params.blendOpAlpha));
    }

    LogErr(LOG, "invalid blend state param type: (bs id: %d;   param: %d)", (int)id, (int)type);
    return s_InvalidStr[0];
}

//---------------------------------------------------------
// Desc:  get parameter's current value in string-form for depth-stencil state by id
//---------------------------------------------------------
const char* RenderStates::GetDssParamStr(const DssID id, const eDssParamType type) const
{
    if (!IsDssExist(id))
    {
        LogErr(LOG, "invalid depth-stencil id: %d", (int)id);
        return s_InvalidStr[0];
    }

    const DssParams& params = dssParams_[id];

    switch (type)
    {
        case DSS_DEPTH_WRITE_MASK:
            return (params.depthWriteMask == 0) ? "zero" : "all";

        case DSS_DEPTH_FUNC:
            return GetStrByCmpFunc(D3D11_COMPARISON_FUNC(params.depthFunc));

        case DSS_STENCIL_READ_MASK:
            return s_InvalidStr[0];

        case DSS_STENCIL_WRITE_MASK:
            return s_InvalidStr[0];


        // -- front face
        case DSS_FRONT_FACE_STENCIL_FAIL_OP:
            return GetStrStencilOp(D3D11_STENCIL_OP(params.ffStencilFailOp));

        case DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP:
            return GetStrStencilOp(D3D11_STENCIL_OP(params.ffStencilDepthFailOp));

        case DSS_FRONT_FACE_STENCIL_PASS_OP:
            return GetStrStencilOp(D3D11_STENCIL_OP(params.ffStencilPassOp));

        case DSS_FRONT_FACE_STENCIL_FUNC:
            return GetStrByCmpFunc(D3D11_COMPARISON_FUNC(params.ffStencilFunc));


        // -- back face
        case DSS_BACK_FACE_STENCIL_FAIL_OP:
            return GetStrStencilOp(D3D11_STENCIL_OP(params.bfStencilFailOp));

        case DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP:
            return GetStrStencilOp(D3D11_STENCIL_OP(params.bfStencilDepthFailOp));

        case DSS_BACK_FACE_STENCIL_PASS_OP:
            return GetStrStencilOp(D3D11_STENCIL_OP(params.bfStencilPassOp));

        case DSS_BACK_FACE_STENCIL_FUNC:
            return GetStrByCmpFunc(D3D11_COMPARISON_FUNC(params.bfStencilFunc));
    }

    LogErr(LOG, "invalid depth-stencil state param type: (dss_id: %d, param_type: %d)", (int)id, (int)type);
    return s_InvalidStr[0];
}

//---------------------------------------------------------
// Desc:  get current boolean-value parameter of blend state by id
//---------------------------------------------------------
bool RenderStates::GetRsParamBool(const RsID id, const eRsParamType type) const
{
    if (!IsRsExist(id))
    {
        LogErr(LOG, "invalid raster state id: %d", (int)id);
        return false;
    }

    switch (type)
    {
        case RS_FRONT_COUNTER_CLOCKWISE:  return rsParams_[id].frontCounterClockwise;
        case RS_DEPTH_CLIP_ENABLE:        return rsParams_[id].depthClipEnable;
        case RS_SCISSOR_ENABLE:           return rsParams_[id].scissorEnable;
        case RS_MULTISAMPLE_ENABLE:       return rsParams_[id].multisampleEnable;
        case RS_ANTIALIASED_LINE_ENABLE:  return rsParams_[id].antialiasedLineEnable;
    }

    LogErr(LOG, "can't get bool param for rs: (%d) %s,  param_type: %d", (int)id, rsNames_[id].name, (int)type);
    return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool RenderStates::GetBsParamBool(const BsID id, const eBsParamType type) const
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "invalid blend state id: %d", (int)id);
        return false;
    }

    switch (type)
    {
        case BLEND_IS_ALPHA_TO_COVERAGE:  return bsParams_[id].isAlphaToCoverage;
        case BLEND_IS_INDEPENDENT:        return bsParams_[id].isIndependentBlend;
        case BLEND_IS_ENABLED:            return bsParams_[id].isBlendEnabled;
    }

    LogErr(LOG, "can't get bool param for bs: (%d) %s,  param_type: %d", (int)id, bsNames_[id].name, (int)type);
    return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool RenderStates::GetDssParamBool(const DssID id, const eDssParamType type) const
{
    if (!IsDssExist(id))
    {
        LogErr(LOG, "invalid depth-stencil state id: %d", (int)id);
        return false;
    }

    if (type == DSS_DEPTH_ENABLED)
        return dssParams_[id].depthEnable;

    if (type == DSS_STENCIL_ENABLED)
        return dssParams_[id].stencilEnable;

    LogErr(LOG, "can't get bool param for dss: (%d) %s,  param_type: %d", (int)id, dssNames_[id].name, (int)type);
    return false;
}


//**********************************************************************************
//                             DEBUG HELPERS
//**********************************************************************************

//---------------------------------------------------------
// Desc:  print a log about loaded rasterizer states
//---------------------------------------------------------
void RenderStates::PrintDumpRs(void) const
{
    LogMsg(LOG, "dump loaded rasterizer states:");
    LogMsg(" - loaded states: %d", (int)rsIds_.size());

    for (index i = 0; i < rsIds_.size(); ++i)
        LogMsg("\t[id: %d] %-20s", (int)i, rsNames_[i].name);
}

//---------------------------------------------------------
// Desc:  print a log about loaded blend states
//---------------------------------------------------------
void RenderStates::PrintDumpBs(void) const
{
    LogMsg(LOG, "dump loaded blend states:");
    LogMsg(" - loaded states: %d", (int)bsIds_.size());

    for (index i = 0; i < bsIds_.size(); ++i)
        LogMsg("\t[id: %d] %-20s", (int)i, bsNames_[i].name);
}

//---------------------------------------------------------
// Desc:  print a log about loaded depth-stencil state
//---------------------------------------------------------
void RenderStates::PrintDumpDss(void) const
{
    LogMsg(LOG, "dump loaded depth-stencil states:");
    LogMsg(" - loaded states: %d", (int)dssIds_.size());

    for (index i = 0; i < dssIds_.size(); ++i)
        LogMsg("\t[id: %d] %-20s", (int)i, dssNames_[i].name);
}

//---------------------------------------------------------
// Desc:  print out current params of the rasterizer state by id
//---------------------------------------------------------
void RenderStates::PrintDumpRsDesc(const RsID id) const
{
    if (!IsRsExist(id))
    {
        LogErr(LOG, "invalid rasterizer state id: %d", (int)id);
        return;
    }

    const char* fmtS = "\t %-28s %s\n";
    const char* fmtD = "\t %-28s %d\n";
    const char* fmtF = "\t %-28s %f\n";

    const RsParams& params = rsParams_[id];

    printf("\n");
    printf("Print a description of a RASTERIZER state: [%d] %s\n", (int)id, rsNames_[id].name);

    printf(fmtS, "fill",                    GetRsParamStr(id, RS_FILL));
    printf(fmtS, "cull",                    GetRsParamStr(id, RS_CULL));
    printf(fmtD, "front_ccw",               params.frontCounterClockwise);

    printf(fmtD, "depth_bias",              params.depthBias);
    printf(fmtF, "depth_bias_clamp",        params.depthBiasClamp);
    printf(fmtF, "slope_scaled_depth bias", params.slopeScaledDepthBias);

    printf(fmtD, "depth_clip_enable",       params.depthClipEnable);
    printf(fmtD, "multisample_enable",      params.multisampleEnable);
    printf(fmtD, "antialiased_line_enable", params.antialiasedLineEnable);
    printf("\n");
}

//---------------------------------------------------------
// Desc:  print out current params of the blend state by id
//---------------------------------------------------------
void RenderStates::PrintDumpBsDesc(const BsID id) const
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "invalid blend state id: %d", (int)id);
        return;
    }

    const char* fmtD = "\t %-28s %d\n";
    const char* fmtS = "\t %-28s %s\n";

    printf("\n");
    printf("Print a description of a BLEND state: [%d] %s\n", (int)id, bsNames_[id].name);

    printf(fmtD, "alpha_to_coverage", GetBsParamBool(id, BLEND_IS_ALPHA_TO_COVERAGE));
    printf(fmtD, "blend_independent", GetBsParamBool(id, BLEND_IS_INDEPENDENT));
    printf(fmtD, "blend_enabled",     GetBsParamBool(id, BLEND_IS_ENABLED));

    printf(fmtS, "blend_src",         GetBsParamStr(id, BLEND_SRC_BLEND));
    printf(fmtS, "blend_dst",         GetBsParamStr(id, BLEND_DST_BLEND));
    printf(fmtS, "blend_op",          GetBsParamStr(id, BLEND_OP));

    printf(fmtS, "blend_src_alpha",   GetBsParamStr(id, BLEND_SRC_BLEND_ALPHA));
    printf(fmtS, "blend_dst_alpha",   GetBsParamStr(id, BLEND_DST_BLEND_ALPHA));
    printf(fmtS, "blend_op_alpha",    GetBsParamStr(id, BLEND_OP_ALPHA));

    printf(fmtS, "render_target_write_mask", GetBsParamStr(id, BLEND_RND_TARGET_WRITE_MASK));
    printf("\n");
}

//---------------------------------------------------------
// Desc:  print out current params of the depth-stencil state by id
//---------------------------------------------------------
void RenderStates::PrintDumpDssDesc(const DssID id) const
{
    if (!IsDssExist(id))
    {
        LogErr(LOG, "invalid depth-stencil state id: %d", (int)id);
        return;
    }

    const char* fmtS = "\t %-28s %s\n";
    const char* fmtD = "\t %-28s %d\n";
    const char* fmtF = "\t %-28s %f\n";
    const char* fmtX = "\t %-28s %" SCNx8 "\n";

    const DssParams& params = dssParams_[id];

    printf("\n");
    printf("Print a description of a DEPTH-STENCIL state: [%d] %s\n", (int)id, dssNames_[id].name);

    printf(fmtD, "depth_enabled",              params.depthEnable);
    printf(fmtS, "depth_write_mask",           GetDssParamStr(id, DSS_DEPTH_WRITE_MASK));
    printf(fmtS, "depth_func",                 GetDssParamStr(id, DSS_DEPTH_FUNC));

    printf(fmtD, "stencil_enabled",            params.stencilEnable);
    printf(fmtX, "stencil_read_mask",          params.stencilReadMask);
    printf(fmtX, "stencil_write_mask",         params.stencilWriteMask);

    printf(fmtS, "front_face.s_fail_op",       GetDssParamStr(id, DSS_FRONT_FACE_STENCIL_FAIL_OP));
    printf(fmtS, "front_face.s_depth_fail_op", GetDssParamStr(id, DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP));
    printf(fmtS, "front_face.s_pass_op",       GetDssParamStr(id, DSS_FRONT_FACE_STENCIL_PASS_OP));
    printf(fmtS, "front_face.s_func",          GetDssParamStr(id, DSS_FRONT_FACE_STENCIL_FUNC));

    printf(fmtS, "back_face.s_fail_op",        GetDssParamStr(id, DSS_BACK_FACE_STENCIL_FAIL_OP));
    printf(fmtS, "back_face.s_depth_fail_op",  GetDssParamStr(id, DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP));
    printf(fmtS, "back_face.s_pass_op",        GetDssParamStr(id, DSS_BACK_FACE_STENCIL_PASS_OP));
    printf(fmtS, "back_face.s_func",           GetDssParamStr(id, DSS_BACK_FACE_STENCIL_FUNC));
    printf("\n");
}


//**********************************************************************************
//                      RENDER STATES LOADING HELPERS
//**********************************************************************************

//---------------------------------------------------------
// Desc:  read in rasterizer states declarations from file and create them
// Args:  - pDevice:            a ptr to DX11 device
//        - cfgFilepath:        a path to config file with declarations
//        - multisampleEnable:  do we use 4xMSAA?
//---------------------------------------------------------
void RenderStates::LoadRs(const char* cfgFilepath, const bool msaaEnabled)
{
    assert(cfgFilepath && cfgFilepath[0] != '\0');

    FILE* pFile = nullptr;
    char buf[128];
    int count = 0;
    RenderStateReader rStateReader;

    pFile = fopen(cfgFilepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", cfgFilepath);
        return;
    }

    // skip comments block
    do {
        fgets(buf, sizeof(buf), pFile);
    } while (buf[0] == ';');


    // read in rasterizer states declarations and create them
    while (!feof(pFile))
    {
        fgets(buf, sizeof(buf), pFile);

        if (buf[0] == '\n')
            continue;

        if (buf[0] == 'r' && buf[1] == 's')
        {
            char rsName[MAX_LEN_RND_STATE_NAME];
            count = sscanf(buf, "rs %s {\n", rsName);
            assert(count == 1);

            // load raster state description
            D3D11_RASTERIZER_DESC rsDesc;  
            rStateReader.LoadRsDesc(pFile, rsDesc, msaaEnabled);

            // create and register a new rasterizer state
            AddRs(rsName, rsDesc);
        }
    }

    fclose(pFile);
}

//---------------------------------------------------------
// Desc:  load blending states descriptions from config file,
//        create them and register in the RenderStates class
//---------------------------------------------------------
void RenderStates::LoadBs(const char* cfgFilepath)
{
    assert(cfgFilepath && cfgFilepath[0] != '\0');

    FILE* pFile = nullptr;
    char buf[128];
    int count = 0;
    BsID bsId = 0;
    RenderStateReader rStateReader;


    pFile = fopen(cfgFilepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", cfgFilepath);
        return;
    }

    // skip comments block
    do {
        fgets(buf, sizeof(buf), pFile);
    } while (buf[0] == ';');

    // read in blend states declarations and create them
    while (!feof(pFile))
    {
        fgets(buf, sizeof(buf), pFile);

        if (buf[0] == '\n')
            continue;

        if (buf[0] == 'b' && buf[1] == 's')
        {
            char bsName[MAX_LEN_RND_STATE_NAME];
            count = sscanf(buf, "bs %s {\n", bsName);
            assert(count == 1);


            // load blending state description
            D3D11_BLEND_DESC bsDesc;
            rStateReader.LoadBsDesc(pFile, bsDesc);

            // create and register a new blend state
            AddBs(bsName, bsDesc);
        }
    }

    fclose(pFile);
}

//---------------------------------------------------------
// Desc:  read in declarations of depth-stencil states from a config file
//        and create them
//---------------------------------------------------------
void RenderStates::LoadDss(const char* cfgFilepath)
{
    assert(cfgFilepath && cfgFilepath[0] != '\0');

    FILE* pFile = nullptr;
    char buf[128];
    int count = 0;
    RenderStateReader rStateReader;


    pFile = fopen(cfgFilepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", cfgFilepath);
        return;
    }

    // skip comments block
    do {
        fgets(buf, sizeof(buf), pFile);
    } while (buf[0] == ';');

    // read in depth-stencil states declarations and create them
    while (!feof(pFile))
    {
        fgets(buf, sizeof(buf), pFile);

        if (buf[0] == '\n')
            continue;

        if (buf[0] == 'd' && buf[1] == 's' && buf[2] == 's')
        {
            char dssName[MAX_LEN_RND_STATE_NAME];
            count = sscanf(buf, "dss %s {\n", dssName);
            assert(count == 1);


            // load depth-stencil state description
            D3D11_DEPTH_STENCIL_DESC dssDesc;
            rStateReader.LoadDssDesc(pFile, dssDesc);

            // create and register a new depth-stencil state
            AddDss(dssName, dssDesc);
        }
    }

    fclose(pFile);
}


//**********************************************************************************
// GET STRING REPRESENTATION OF INPUT PARAMETER
//**********************************************************************************

const char* RenderStates::GetStrByBlendOp(const D3D11_BLEND_OP op) const
{
    const char** names = s_BlendOpNames;

    switch (op)
    {
        case D3D11_BLEND_OP_ADD:          return names[0];
        case D3D11_BLEND_OP_SUBTRACT:     return names[1];
        case D3D11_BLEND_OP_REV_SUBTRACT: return names[2];
        case D3D11_BLEND_OP_MIN:          return names[3];
        case D3D11_BLEND_OP_MAX:          return names[4];
    }

    LogErr(LOG, "invalid blend operation: %d", (int)op);
    return s_InvalidStr[0];
}

//---------------------------------------------------------

const char* RenderStates::GetStrByBlendFactor(const D3D11_BLEND factor) const
{
    const char** names = s_BlendFactorNames;

    switch (factor)
    {
        case D3D11_BLEND_ZERO:              return names[0];
        case D3D11_BLEND_ONE:               return names[1];
        case D3D11_BLEND_SRC_COLOR:         return names[2];
        case D3D11_BLEND_INV_SRC_COLOR:     return names[3];
        case D3D11_BLEND_SRC_ALPHA:         return names[4];
        case D3D11_BLEND_INV_SRC_ALPHA:     return names[5];
        case D3D11_BLEND_DEST_ALPHA:        return names[6];
        case D3D11_BLEND_INV_DEST_ALPHA:    return names[7];
        case D3D11_BLEND_DEST_COLOR:        return names[8];
        case D3D11_BLEND_INV_DEST_COLOR:    return names[9];
        case D3D11_BLEND_SRC_ALPHA_SAT:     return names[10];
        case D3D11_BLEND_BLEND_FACTOR:      return names[11];
        case D3D11_BLEND_INV_BLEND_FACTOR:  return names[12];
        //case D3D11_BLEND_SRC1_COLOR:
        //case D3D11_BLEND_INV_SRC1_COLOR:
        //case D3D11_BLEND_SRC1_ALPHA:
        //case D3D11_BLEND_INV_SRC1_ALPHA:
    }

    LogErr(LOG, "invalid blend factor: %d", (int)factor);
    return s_InvalidStr[0];
}

//---------------------------------------------------------

const char* RenderStates::GetStrByBlendWriteMask(const uint8 mask) const
{
    const char** names = s_BlendRenderTargetWriteMaskNames;

    switch (mask)
    {
        case 0:                              return names[0];
        case D3D11_COLOR_WRITE_ENABLE_RED:   return names[1];
        case D3D11_COLOR_WRITE_ENABLE_GREEN: return names[2];
        case D3D11_COLOR_WRITE_ENABLE_BLUE:  return names[3];
        case D3D11_COLOR_WRITE_ENABLE_ALPHA: return names[4];
        case D3D11_COLOR_WRITE_ENABLE_ALL:   return names[5];
    }

    LogErr(LOG, "invalid blend render target write mask: %x", mask);
    return s_InvalidStr[0];
}

//---------------------------------------------------------
// Desc:  get value of depth/stencil comparison func in string form
//---------------------------------------------------------
const char* RenderStates::GetStrByCmpFunc(const D3D11_COMPARISON_FUNC& func) const
{
    const char** names = s_DssComparisonFuncNames;

    switch (func)
    {
        case D3D11_COMPARISON_NEVER:            return names[0];
        case D3D11_COMPARISON_LESS:             return names[1];
        case D3D11_COMPARISON_EQUAL:            return names[2];
        case D3D11_COMPARISON_LESS_EQUAL:       return names[3];
        case D3D11_COMPARISON_GREATER:          return names[4];
        case D3D11_COMPARISON_NOT_EQUAL:        return names[5];
        case D3D11_COMPARISON_GREATER_EQUAL:    return names[6];
        case D3D11_COMPARISON_ALWAYS:           return names[7];
    }

    LogErr(LOG, "invalid depth/stencil comparison func: %d", (int)func);
    return s_InvalidStr[0];
}

//---------------------------------------------------------
// Desc:  get value of stencil operation in string form
//---------------------------------------------------------
const char* RenderStates::GetStrStencilOp(const D3D11_STENCIL_OP& op) const
{
    const char** names = s_DssStencilOpsNames;

    switch (op)
    {
        case D3D11_STENCIL_OP_KEEP:             return names[0];
        case D3D11_STENCIL_OP_ZERO:             return names[1];
        case D3D11_STENCIL_OP_REPLACE:          return names[2];
        case D3D11_STENCIL_OP_INCR_SAT:         return names[3];
        case D3D11_STENCIL_OP_DECR_SAT:         return names[4];
        case D3D11_STENCIL_OP_INVERT:           return names[5];
        case D3D11_STENCIL_OP_INCR:             return names[6];
        case D3D11_STENCIL_OP_DECR:             return names[7];
    }

    LogErr(LOG, "invalid stencil operation: %d", (int)op);
    return s_InvalidStr[0];
}


//**********************************************************************************
// CACHE (AND PACK) RENDER STATES DESCRIPTIONS
//**********************************************************************************

//---------------------------------------------------------
// Desc:  pack input rasterizer state description into storage structure
//        for a blending state by id
//---------------------------------------------------------
void RenderStates::CacheRsDesc(const RsID id, const D3D11_RASTERIZER_DESC& desc)
{
    if (!IsRsExist(id))
    {
        LogErr(LOG, "invalid rasterizer state id: %d", (int)id);
        return;
    }

    RsParams& params = rsParams_[id];
    ZeroMemory(&params, sizeof(RsParams));

    params.depthBias                = desc.DepthBias;
    params.depthBiasClamp           = desc.DepthBiasClamp;
    params.slopeScaledDepthBias     = desc.SlopeScaledDepthBias;

    params.fillMode                 = desc.FillMode;
    params.cullMode                 = desc.CullMode;
    params.frontCounterClockwise    = desc.FrontCounterClockwise;
    params.depthClipEnable          = desc.DepthClipEnable;
    params.scissorEnable            = desc.ScissorEnable;
    params.multisampleEnable        = desc.MultisampleEnable;
    params.antialiasedLineEnable    = desc.AntialiasedLineEnable;
}

//---------------------------------------------------------
// Desc:  pack input blend state description into storage structure
//        for a blending state by id
//---------------------------------------------------------
void RenderStates::CacheBsDesc(const BsID id, const D3D11_BLEND_DESC& desc)
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "invalid blend state id: %d", (int)id);
        return;
    }

    BsParams& params = bsParams_[id];
    ZeroMemory(&params, sizeof(BsParams));

    // color
    params.srcBlend            = desc.RenderTarget[0].SrcBlend;
    params.dstBlend            = desc.RenderTarget[0].DestBlend;
    params.blendOp             = desc.RenderTarget[0].BlendOp;

    // alpha
    params.srcBlendAlpha       = desc.RenderTarget[0].SrcBlendAlpha;
    params.dstBlendAlpha       = desc.RenderTarget[0].DestBlendAlpha;
    params.blendOpAlpha        = desc.RenderTarget[0].BlendOpAlpha;

    params.writeMask           = desc.RenderTarget[0].RenderTargetWriteMask;
    params.isAlphaToCoverage   = desc.AlphaToCoverageEnable;
    params.isIndependentBlend  = desc.IndependentBlendEnable;
    params.isBlendEnabled      = desc.RenderTarget[0].BlendEnable;
}

//---------------------------------------------------------
// Desc:  pack input depth-stencil state description into storage structure
//        for a blending state by id
//---------------------------------------------------------
void RenderStates::CacheDssDesc(const DssID id, const D3D11_DEPTH_STENCIL_DESC& desc)
{
    if (!IsDssExist(id))
    {
        LogErr(LOG, "invalid depth-stencil state id: %d", (int)id);
        return;
    }

    DssParams& params = dssParams_[id];
    ZeroMemory(&params, sizeof(DssParams));

    params.depthEnable          = desc.DepthEnable;
    params.depthWriteMask       = desc.DepthWriteMask;
    params.depthFunc            = desc.DepthFunc;

    params.stencilEnable        = desc.StencilEnable;
    params.stencilReadMask      = desc.StencilReadMask;
    params.stencilWriteMask     = desc.StencilWriteMask;

    params.ffStencilFailOp      = desc.FrontFace.StencilFailOp;
    params.ffStencilDepthFailOp = desc.FrontFace.StencilDepthFailOp;
    params.ffStencilPassOp      = desc.FrontFace.StencilPassOp;
    params.ffStencilFunc        = desc.FrontFace.StencilFunc;

    params.bfStencilFailOp      = desc.BackFace.StencilFailOp;
    params.bfStencilDepthFailOp = desc.BackFace.StencilDepthFailOp;
    params.bfStencilPassOp      = desc.BackFace.StencilPassOp;
    params.bfStencilFunc        = desc.BackFace.StencilFunc;
}

//---------------------------------------------------------
// Desc:  create a depth-stencil state description by input params
//---------------------------------------------------------
D3D11_RASTERIZER_DESC RenderStates::GetRsDescByParams(const RsParams& params) const
{
    D3D11_RASTERIZER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.DepthBias              = params.depthBias;
    desc.DepthBiasClamp         = params.depthBiasClamp;
    desc.SlopeScaledDepthBias   = params.slopeScaledDepthBias;

    desc.FillMode               = D3D11_FILL_MODE(params.fillMode);
    desc.CullMode               = D3D11_CULL_MODE(params.cullMode);
    desc.FrontCounterClockwise  = params.frontCounterClockwise;
    desc.DepthClipEnable        = params.depthClipEnable;
    desc.ScissorEnable          = params.scissorEnable;
    desc.MultisampleEnable      = params.multisampleEnable;
    desc.AntialiasedLineEnable  = params.antialiasedLineEnable;

    return desc;
}

//---------------------------------------------------------
// Desc:  create a blend state description by input params
//---------------------------------------------------------
D3D11_BLEND_DESC RenderStates::GetBsDescByParams(const BsParams& params) const
{
    D3D11_BLEND_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.AlphaToCoverageEnable                  = params.isAlphaToCoverage;
    desc.IndependentBlendEnable                 = params.isIndependentBlend;
    desc.RenderTarget[0].BlendEnable            = params.isBlendEnabled;
    desc.RenderTarget[0].SrcBlend               = D3D11_BLEND(params.srcBlend);
    desc.RenderTarget[0].DestBlend              = D3D11_BLEND(params.dstBlend);
    desc.RenderTarget[0].BlendOp                = D3D11_BLEND_OP(params.blendOp);
    desc.RenderTarget[0].SrcBlendAlpha          = D3D11_BLEND(params.srcBlendAlpha);
    desc.RenderTarget[0].DestBlendAlpha         = D3D11_BLEND(params.dstBlendAlpha);
    desc.RenderTarget[0].BlendOpAlpha           = D3D11_BLEND_OP(params.blendOpAlpha);
    desc.RenderTarget[0].RenderTargetWriteMask  = (uint8)(params.writeMask);

    return desc;
}

//---------------------------------------------------------
// Desc:  create a depth-stencil state description by input params
//---------------------------------------------------------
D3D11_DEPTH_STENCIL_DESC RenderStates::GetDssDescByParams(const DssParams& params) const
{
    D3D11_DEPTH_STENCIL_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.DepthEnable                    = params.depthEnable;
    desc.DepthWriteMask                 = D3D11_DEPTH_WRITE_MASK(params.depthWriteMask);
    desc.DepthFunc                      = D3D11_COMPARISON_FUNC(params.depthFunc);

    desc.StencilEnable                  = params.stencilEnable;
    desc.StencilReadMask                = params.stencilReadMask;
    desc.StencilWriteMask               = params.stencilWriteMask;

    desc.FrontFace.StencilFailOp        = D3D11_STENCIL_OP(params.ffStencilFailOp);
    desc.FrontFace.StencilDepthFailOp   = D3D11_STENCIL_OP(params.ffStencilDepthFailOp);
    desc.FrontFace.StencilPassOp        = D3D11_STENCIL_OP(params.ffStencilPassOp);
    desc.FrontFace.StencilFunc          = D3D11_COMPARISON_FUNC(params.ffStencilFunc);

    desc.BackFace.StencilFailOp         = D3D11_STENCIL_OP(params.bfStencilFailOp);
    desc.BackFace.StencilDepthFailOp    = D3D11_STENCIL_OP(params.bfStencilDepthFailOp);
    desc.BackFace.StencilPassOp         = D3D11_STENCIL_OP(params.bfStencilPassOp);
    desc.BackFace.StencilFunc           = D3D11_COMPARISON_FUNC(params.bfStencilFunc);

    return desc;
}


//**********************************************************************************
// UPDATE RENDER STATE
//**********************************************************************************

//---------------------------------------------------------
// Desc:  recreate a rasterizer state by id using input description
//---------------------------------------------------------
void RenderStates::UpdateRs(const RsID id, const D3D11_RASTERIZER_DESC& desc)
{
    if (!IsRsExist(id))
    {
        LogErr(LOG, "invalid render state id: %d", (int)id);
        return;
    }

    HRESULT hr = S_OK;
    ID3D11RasterizerState* pRS = nullptr;

    assert(pDevice_);
    hr = pDevice_->CreateRasterizerState(&desc, &pRS);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't recreate a raster state by id: %d", (int)id);
        return;
    }

    // cache this state description
    CacheRsDesc(id, desc);

    rsPtrs_[id]->Release();
    rsPtrs_[id] = pRS;
}

//---------------------------------------------------------
// Desc:  recreate a blending state by id using input description
//---------------------------------------------------------
void RenderStates::UpdateBs(const BsID id, const D3D11_BLEND_DESC& desc)
{
    if (!IsBsExist(id))
    {
        LogErr(LOG, "invalid blend state id: %d", (int)id);
        return;
    }

    HRESULT hr = S_OK;
    ID3D11BlendState* pBS = nullptr;

    assert(pDevice_);
    hr = pDevice_->CreateBlendState(&desc, &pBS);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't recreate a blend state by id: %d", (int)id);
        return;
    }

    // cache blend state description
    CacheBsDesc(id, desc);

    bsPtrs_[id]->Release();
    bsPtrs_[id] = pBS;
}

//---------------------------------------------------------
// Desc:  recreate a depth-stencil state by id using input description
//---------------------------------------------------------
void RenderStates::UpdateDss(const DssID id, const D3D11_DEPTH_STENCIL_DESC& desc)
{
    if (!IsDssExist(id))
    {
        LogErr(LOG, "no depth-stencil state by id: %d", (int)id);
        return;
    }

    HRESULT hr = S_OK;
    ID3D11DepthStencilState* pDSS = nullptr;

    assert(pDevice_);
    hr = pDevice_->CreateDepthStencilState(&desc, &pDSS);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't recreate a depth-stencil state by id: %d", (int)id);
        return;
    }

    // cache this state description
    CacheDssDesc(id, desc);

    dssPtrs_[id]->Release();
    dssPtrs_[id] = pDSS;
}

} // namespace
