// *********************************************************************************
// Filename:     RenderStates.h
// 
// Created:      09.09.24
// *********************************************************************************
#pragma once

#include <d3d11.h>
#include <map>
#include <set>


namespace Render
{


#define NUM_FILL_MODES 2
#define NUM_CULL_MODES 3
#define NUM_WINDING_ORDERS 2
#define NUM_BLEND_STATES 8
#define NUM_DEPTH_STENCIL_STATES 6

enum eRenderState // : byte
{
    // fill modes
    R_FILL_SOLID,
    R_FILL_WIREFRAME,

    // cull modes
    R_CULL_BACK,
    R_CULL_FRONT,
    R_CULL_NONE,

    // winding orders
    R_FRONT_COUNTER_CLOCKWISE,  // CCW
    R_FRONT_CLOCKWISE,

    // blending states
    R_NO_RENDER_TARGET_WRITES,
    R_ALPHA_DISABLE,
    R_ALPHA_ENABLE,
    R_ADDING,
    R_SUBTRACTING,
    R_MULTIPLYING,
    R_TRANSPARENCY,
    R_ALPHA_TO_COVERAGE,

    // depth stencil states
    R_DEPTH_PREPASS,
    R_DEPTH_ENABLED,
    R_DEPTH_DISABLED,
    R_MARK_MIRROR,                  // for rendering mirror reflections
    R_DRAW_REFLECTION,
    R_NO_DOUBLE_BLEND,
    R_SKY_DOME,

    R_NUM_RENDER_STATES,            // to make possible iteration over the enum

    eRenderState_forcedword = UINT(-1)
};

///////////////////////////////////////////////////////////

class RenderStates
{
public:
    RenderStates();
    ~RenderStates();

    void InitAll(ID3D11Device* pDevice, const bool multisampleEnable);
    void DestroyAll();


    uint GetNumFillModes()                    const { return NUM_FILL_MODES; }
    uint GetNumCullModes()                    const { return NUM_CULL_MODES; }
    uint GetNumWindingOrders()                const { return NUM_WINDING_ORDERS; }
    uint GetNumBlendStates()                  const { return NUM_BLEND_STATES; }
    uint GetNumDepthStencilStates()           const { return NUM_DEPTH_STENCIL_STATES; }

    const char** GetFillModesNames()          const;
    const char** GetCullModesNames()          const;
    const char** GetWindingOrdersNames()      const;
    const char** GetBlendStatesNames()        const;
    const char** GetDepthStencilStatesNames() const;


    inline void ResetRS (ID3D11DeviceContext* pDeviceContext) { SetRS(pDeviceContext, { R_FILL_SOLID, R_CULL_BACK, R_FRONT_CLOCKWISE }); }
    inline void ResetBS (ID3D11DeviceContext* pDeviceContext) { SetBS(pDeviceContext, R_ALPHA_DISABLE); }
    inline void ResetDSS(ID3D11DeviceContext* pDeviceContext) { pDeviceContext->OMSetDepthStencilState(0, 0); }

    // get blend state / raster state / depth stencil state
    ID3D11BlendState*        GetBS(const eRenderState state);
    ID3D11RasterizerState*   GetRS(const std::set<eRenderState>& states);
    ID3D11DepthStencilState* GetDSS(const eRenderState state);

    // returns a hash to the pointer of the current rasterizer state
    inline uint8_t GetCurrentRSHash() const { return rasterStateHash_; }

    // set raster state
    void SetRS       (ID3D11DeviceContext* pDeviceContext, const eRenderState state);
    void SetRS       (ID3D11DeviceContext* pDeviceContext, const std::set<eRenderState>& states);
    void SetRSByHash (ID3D11DeviceContext* pDeviceContext, const uint8_t hash);

    // set blend state / depth stencil state
    void SetBS       (ID3D11DeviceContext* pDeviceContext, const eRenderState key);
    void SetDSS      (ID3D11DeviceContext* pDeviceContext, const eRenderState key, const UINT stencilRef);

private:
    void InitAllRasterParams      (ID3D11Device* pDevice, bool multisampleEnable);
    void InitAllBlendStates       (ID3D11Device* pDevice);
    void InitAllDepthStencilStates(ID3D11Device* pDevice);

    inline void ResetRasterStateHash() { rasterStateHash_ &= 0; }
    inline void TurnOnRasterParam(const eRenderState rsParam) { rasterStateHash_ |= (1 << rsParam);  }

    ID3D11RasterizerState* GetRasterStateByHash(const uint8_t hash);

    uint8 UpdateRSHash(const eRenderState rsParam);
    uint8 UpdateRSHash(const std::set<eRenderState>& rsParams);
    void PrintErrAboutRSHash(const uint8_t bitfield);

private:
    std::map<eRenderState, ID3D11BlendState*>         blendStates_;
    std::map<uint8_t,      ID3D11RasterizerState*>    rasterStates_;
    std::map<eRenderState, ID3D11DepthStencilState*>  depthStencilStates_;

    // rasterizer state related stuff
    uint8_t rasterStateHash_     { 0b0000'0000 };              // hash to particular rasterizer state
    uint8_t turnOffCullModesHash_{ 0b1111'1111 };              // using this hash we turn off ALL the CULL modes at the same time
    uint8_t turnOffFillModesHash_{ 0b1111'1111 };              // using this hash we turn off ALL the FILL modes at the same time
};

}
