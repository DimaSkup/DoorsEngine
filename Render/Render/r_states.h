/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: r_states.h
    Desc:     initialization and work with DX11 render states

    NOTATION:
              rs  - rasterizer state
              bs  - blending state
              dss - depth-stencil state

    Created:            09.09.2024
    Completely remade:  around 19.01.2026
/**********************************************************************************/
#pragma once

#include <types.h>
#include <d3d11.h>


namespace Render
{
enum eRsParamType
{
    RS_PARAM_TYPE_NONE,

    RS_FILL,
    RS_CULL,
    RS_FRONT_COUNTER_CLOCKWISE,
    RS_DEPTH_BIAS,
    RS_DEPTH_BIAS_CLAMP,
    RS_SLOPE_SCALED_DEPTH_BIAS,
    RS_DEPTH_CLIP_ENABLE,
    RS_SCISSOR_ENABLE,
    RS_MULTISAMPLE_ENABLE,
    RS_ANTIALIASED_LINE_ENABLE,
};

//---------------------------------------------------------

enum eBsParamType
{
    BS_PARAM_TYPE_NONE,

    // blending properties (groups)
    BLEND_OPERATION,
    BLEND_FACTOR,
    BLEND_RND_TARGET_WRITE_MASK,

    // blending properties (specific) (for understanding wtf look D3D11_BLEND and D3D11_BLEND_OP)
    BLEND_SRC_BLEND,
    BLEND_DST_BLEND,
    BLEND_OP,

    BLEND_SRC_BLEND_ALPHA,
    BLEND_DST_BLEND_ALPHA,
    BLEND_OP_ALPHA,

    BLEND_IS_ALPHA_TO_COVERAGE,
    BLEND_IS_INDEPENDENT,
    BLEND_IS_ENABLED,
};

//---------------------------------------------------------

enum eDssParamType
{
    DDS_PARAM_TYPE_NONE,

    // depth-stencil properties (groups)
    DSS_DEPTH_WRITE_MASK,
    DSS_COMPARISON_FUNC,
    DSS_STENCIL_OP,

    // depth-stencil props (specific)
    DSS_DEPTH_ENABLED,
    DSS_DEPTH_FUNC,
    DSS_STENCIL_ENABLED,
    DSS_STENCIL_READ_MASK,
    DSS_STENCIL_WRITE_MASK,
    DSS_FRONT_FACE_STENCIL_FAIL_OP,
    DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP,
    DSS_FRONT_FACE_STENCIL_PASS_OP,
    DSS_FRONT_FACE_STENCIL_FUNC,
    DSS_BACK_FACE_STENCIL_FAIL_OP,
    DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP,
    DSS_BACK_FACE_STENCIL_PASS_OP,
    DSS_BACK_FACE_STENCIL_FUNC,
};

//---------------------------------------------------------
// packed structure for rasterizer state description caching
//---------------------------------------------------------
struct RsParams
{
    int    depthBias;
    float  depthBiasClamp;
    float  slopeScaledDepthBias;

    uint32 fillMode                 : 2;
    uint32 cullMode                 : 2;
    uint32 frontCounterClockwise    : 1;
    uint32 depthClipEnable          : 1;
    uint32 scissorEnable            : 1;
    uint32 multisampleEnable        : 1;
    uint32 antialiasedLineEnable    : 1;
};

//---------------------------------------------------------
// packed structure for blending state description caching
//---------------------------------------------------------
struct BsParams
{
    uint32 srcBlend                 : 5;    // source color blend factor
    uint32 dstBlend                 : 5;    // destination color blend factor
    uint32 blendOp                  : 3;    // color blend operation

    uint32 srcBlendAlpha            : 5;    // source alpha blend factor
    uint32 dstBlendAlpha            : 5;    // destination alpha blend factor
    uint32 blendOpAlpha             : 3;    // alpha blend operation

    uint32 writeMask                : 4;    // render target write mask

    uint32 isAlphaToCoverage        : 1;
    uint32 isIndependentBlend       : 1;
    uint32 isBlendEnabled           : 1;
};

//---------------------------------------------------------
// packed structure for depth-stencil state description caching
//---------------------------------------------------------
struct DssParams
{
    uint32 depthEnable              : 1;
    uint32 depthWriteMask           : 1;
    uint32 depthFunc                : 4;
    uint32 stencilEnable            : 1;
    uint32 stencilReadMask          : 8;
    uint32 stencilWriteMask         : 8;

    // ff - front face
    uint32 ffStencilFailOp          : 4;
    uint32 ffStencilDepthFailOp     : 4;
    uint32 ffStencilPassOp          : 4;
    uint32 ffStencilFunc            : 4;

    // bf - back face
    uint32 bfStencilFailOp          : 4;
    uint32 bfStencilDepthFailOp     : 4;
    uint32 bfStencilPassOp          : 4;
    uint32 bfStencilFunc            : 4;
};


//---------------------------------------------------------
// class name: RenderStates
//
// NOTATION:
//           rs  - rasterizer state
//           bs  - blending state
//           dss - depth-stencil state
//---------------------------------------------------------
class RenderStates
{
public:
    RenderStates();
    ~RenderStates();

    void InitAll(ID3D11Device* pDevice, const bool multisampleEnable);
    void DestroyAll(void);


    //-----------------------------------------------------
    // reset render state to default
    //-----------------------------------------------------
    inline void ResetRenderStates(void)
    {
        ResetRs();
        ResetBs();
        ResetDss();
    }

    inline void ResetRs (void) { SetRs(0); }
    inline void ResetBs (void) { SetBs(0); }
    inline void ResetDss(void) { SetDss(0, 0); }


    //-----------------------------------------------------
    // add/get/set rasterizer state (RS)
    //-----------------------------------------------------
    RsID                   AddRs    (const char* name, D3D11_RASTERIZER_DESC& desc);
    RsID                   GetRsId  (const char* rsName) const;
    const char*            GetRsName(const RsID id) const;
    void                   SetRs    (const RsID id);
    ID3D11RasterizerState* GetRs    (const RsID id);
    bool                   IsRsExist(const RsID id) const;

    void UpdateCustomRsParam(const eRsParamType type, const bool value);
    void UpdateCustomRsParam(const eRsParamType type, const char* value);
    void UpdateCustomRsParam(const eRsParamType type, const int value);
    void UpdateCustomRsParam(const eRsParamType type, const float value);

    //-----------------------------------------------------
    // add/get/set blend state (BS)
    //-----------------------------------------------------
    BsID              AddBs    (const char* name, const D3D11_BLEND_DESC& desc);
    BsID              GetBsId  (const char* bsName) const;
    const char*       GetBsName(const BsID id) const;
    void              SetBs    (const BsID id);
    ID3D11BlendState* GetBs    (const BsID id);
    bool              IsBsExist(const BsID id) const;

    void IsBlendEnabled(const BsID id, bool& isTransparent, bool& blendEnabled) const;

    void UpdateCustomBsParam(const eBsParamType type, const bool value);
    void UpdateCustomBsParam(const eBsParamType type, const char* value);

    //-----------------------------------------------------
    // add/get/set depth-stencil state (DSS)
    //-----------------------------------------------------
    DssID                    AddDss    (const char* name, D3D11_DEPTH_STENCIL_DESC& desc);
    DssID                    GetDssId  (const char* dssName) const;
    const char*              GetDssName(const DssID id) const;
    void                     SetDss    (const DssID id, const UINT stencilRef);
    ID3D11DepthStencilState* GetDss    (const DssID id);
    bool                     IsDssExist(const DssID id) const;

    void UpdateCustomDssParam(const eDssParamType type, const bool value);
    void UpdateCustomDssParam(const eDssParamType type, const char* value);

    //-----------------------------------------------------
    // get render states counters and names
    //-----------------------------------------------------
    inline size GetNumRasterStates      (void) const { return rsIds_.size(); }
    inline size GetNumBlendStates       (void) const { return bsIds_.size(); }
    inline size GetNumDepthStencilStates(void) const { return dssIds_.size(); }

    inline const cvector<RenderStateName>& GetRasterStatesNames      (void) const { return rsNames_; }
    inline const cvector<RenderStateName>& GetBlendStatesNames       (void) const { return bsNames_; }
    inline const cvector<RenderStateName>& GetDepthStencilStatesNames(void) const { return dssNames_; }


    //-----------------------------------------------------
    // get render states params couters / names / current values
    //-----------------------------------------------------
    int          GetNumRsParams         (const eRsParamType type) const;
    int          GetNumBsParams         (const eBsParamType type) const;
    int          GetNumDssParams        (const eDssParamType type) const;

    const char** GetArrRsParamsNames    (const eRsParamType type) const;
    const char** GetArrBsParamsNames    (const eBsParamType type) const;
    const char** GetArrDssParamsNames   (const eDssParamType type) const;

    int          GetRsParamInt          (const RsID id, const eRsParamType type) const;
    float        GetRsParamFloat        (const RsID id, const eRsParamType type) const;

    const char*  GetRsParamStr          (const RsID id, const eRsParamType type) const;
    const char*  GetBsParamStr          (const BsID id, const eBsParamType type) const;
    const char*  GetDssParamStr         (const DssID id, const eDssParamType type) const;

    bool         GetRsParamBool         (const RsID id, const eRsParamType type) const;
    bool         GetBsParamBool         (const BsID id, const eBsParamType type) const;
    bool         GetDssParamBool        (const DssID id, const eDssParamType type) const;

    //-----------------------------------------------------
    // debug helpers
    //-----------------------------------------------------
    void PrintDumpRs(void) const;
    void PrintDumpBs(void) const;
    void PrintDumpDss(void) const;

    void PrintDumpRsDesc(const RsID id) const;
    void PrintDumpBsDesc(const BsID id) const;
    void PrintDumpDssDesc(const DssID id) const;


private:
    void LoadRs (const char* cfgFilepath, bool multisampleEnable);
    void LoadBs (const char* cfgFilepath);
    void LoadDss(const char* cfgFilepath);

   

    // get string representation of input parameter
    const char* GetStrByBlendOp       (const D3D11_BLEND_OP op)  const;
    const char* GetStrByBlendFactor   (const D3D11_BLEND factor) const;
    const char* GetStrByBlendWriteMask(const uint8 mask)         const;

    const char* GetStrByCmpFunc       (const D3D11_COMPARISON_FUNC& func) const;
    const char* GetStrStencilOp       (const D3D11_STENCIL_OP& op)        const;


    // cache render states
    void CacheRsDesc (const RsID id,  const D3D11_RASTERIZER_DESC& desc);
    void CacheBsDesc (const BsID id,  const D3D11_BLEND_DESC& desc);
    void CacheDssDesc(const DssID id, const D3D11_DEPTH_STENCIL_DESC& desc);

    // unpack cached description of render state
    D3D11_RASTERIZER_DESC    GetRsDescByParams (const RsParams& params) const;
    D3D11_BLEND_DESC         GetBsDescByParams (const BsParams& params) const;
    D3D11_DEPTH_STENCIL_DESC GetDssDescByParams(const DssParams& params) const;

    void UpdateRs (const RsID id,  const D3D11_RASTERIZER_DESC& desc);
    void UpdateBs (const BsID id,  const D3D11_BLEND_DESC& desc);
    void UpdateDss(const DssID id, const D3D11_DEPTH_STENCIL_DESC& desc);

private:
    ID3D11Device*        pDevice_ = nullptr;
    ID3D11DeviceContext* pCtx_    = nullptr;

    RsID  rsId_ = 0;
    BsID  bsId_ = 0;
    DssID dssId_ = 0;

    // raster states (RS)
    cvector<RsID>                     rsIds_;
    cvector<RenderStateName>          rsNames_;
    cvector<ID3D11RasterizerState*>   rsPtrs_;
    cvector<RsParams>                 rsParams_;

    // blend states (BS)
    cvector<BsID>                     bsIds_;
    cvector<RenderStateName>          bsNames_;
    cvector<ID3D11BlendState*>        bsPtrs_;
    cvector<BsParams>                 bsParams_;

    // depth-stencil states (DSS)
    cvector<DssID>                    dssIds_;
    cvector<RenderStateName>          dssNames_;
    cvector<ID3D11DepthStencilState*> dssPtrs_;
    cvector<DssParams>                dssParams_;
};

}
