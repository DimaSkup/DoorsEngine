// *********************************************************************************
// Filename:     RenderStates.h
// Description:  defines render state objects
// 
// Created:      09.09.24
// *********************************************************************************
#pragma once

#include <d3d11.h>
#include <map>
#include <vector>

class RenderStates
{
public:
	enum STATES
	{
		// rasterizer params
		FILL_SOLID,
		FILL_WIREFRAME,
		CULL_BACK,
		CULL_FRONT,
		CULL_NONE,
		FRONT_COUNTER_CLOCKWISE,  // CCW
		FRONT_CLOCKWISE,
		

		// blending states
		NO_RENDER_TARGET_WRITES,
		ALPHA_DISABLE,
		ALPHA_ENABLE,
		ADDING,
		SUBTRACTING,
		MULTIPLYING,
		TRANSPARENCY,

		// depth stencil states
		DEPTH_ENABLED,
		DEPTH_DISABLED,
		MARK_MIRROR,     // for rendering mirror reflections
		DRAW_REFLECTION,
		NO_DOUBLE_BLEND,
		SKY_DOME,

		LAST   // to make possible iteration over the enum
	};

public:
	RenderStates();
	~RenderStates();

	void InitAll(ID3D11Device* pDevice, bool multisampleEnable);
	void DestroyAll();

	inline void ResetRS (ID3D11DeviceContext* pDeviceContext) { SetRS(pDeviceContext, { FILL_SOLID, CULL_BACK, FRONT_COUNTER_CLOCKWISE }); }
	inline void ResetBS (ID3D11DeviceContext* pDeviceContext) { SetBS(pDeviceContext, ALPHA_DISABLE); }
	inline void ResetDSS(ID3D11DeviceContext* pDeviceContext) { pDeviceContext->OMSetDepthStencilState(0, 0); }

	ID3D11BlendState* GetBS(const STATES state);
	ID3D11RasterizerState* GetRS(const std::vector<STATES>& states);
	ID3D11DepthStencilState* GetDSS(const STATES state);

	// returns a hash to the pointer of the current rasterizer state
	inline uint8_t GetCurrentRSHash() const { return rasterStateHash_; }

	void SetRS(ID3D11DeviceContext* pDeviceContext, const STATES state);
	void SetRS(ID3D11DeviceContext* pDeviceContext, const std::vector<STATES>& states);
	void SetRSByHash(ID3D11DeviceContext* pDeviceContext, const uint8_t hash);

	void SetBS(ID3D11DeviceContext* pDeviceContext, const STATES key);
	void SetDSS(ID3D11DeviceContext* pDeviceContext, const STATES key, const UINT stencilRef);

private:
	void InitAllRasterParams(ID3D11Device* pDevice, bool multisampleEnable);
	void InitAllBlendStates(ID3D11Device* pDevice);
	void InitAllDepthStencilStates(ID3D11Device* pDevice);

	inline void ResetRasterStateHash() { rasterStateHash_ &= 0; }
	inline void TurnOnRasterParam(const STATES rsParam) { rasterStateHash_ |= (1 << rsParam);  }
	//inline void turnOffRasterParam(const STATES rsParam) { rasterStateHash_ &= ~(1 << rsParam); }

	ID3D11RasterizerState* GetRasterStateByHash(const uint8_t hash);

	void UpdateRSHash(const std::vector<STATES>& rsParams);
	void PrintErrAboutRSHash(const uint8_t hash);

private:
	std::map<STATES,  ID3D11BlendState*>         blendStates_;
	std::map<uint8_t, ID3D11RasterizerState*>    rasterStates_;
	std::map<STATES,  ID3D11DepthStencilState*>  depthStencilStates_;

	// rasterizer state related stuff
	uint8_t rasterStateHash_     { 0b0000'0000 };              // hash to particular rasterizer state
	uint8_t turnOffCullModesHash_{ 0b1111'1111 };              // using this hash we turn off ALL the CULL modes at the same time
	uint8_t turnOffFillModesHash_{ 0b1111'1111 };              // using this hash we turn off ALL the FILL modes at the same time

};
