// =================================================================================
// Filename:      AdapterReader.h
// Description:   a container for DXGI factory, adapters and outputs
// 
// Created:       21.10.22
// =================================================================================
#pragma once


/////////////////////////////
// LINKING
/////////////////////////////
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

/////////////////////////////
// INCLUDES
/////////////////////////////
#include <d3d11.h>
#include <dxgi.h>	// a DirectX graphic interface header


namespace Core
{

class AdapterData
{
public:
	AdapterData() {};
	AdapterData(IDXGIAdapter* pAdapter);

	IDXGIAdapter* pAdapter_ = nullptr;
	IDXGIOutput*  pOutput_ = nullptr;
	DXGI_ADAPTER_DESC description_ = {};
};

class AdapterReader
{
public:
	AdapterReader();
	~AdapterReader();

	AdapterData* GetAdapterDataByIdx(const int idx);
	IDXGIAdapter* GetDXGIAdapterByIdx(const int idx);

	inline int GetNumAdapters() const { return numAdapters_; }
	//inline IDXGIFactory* GetFactory() { return pFactory_; }



private:
	void Shutdown();
	void InitOutputsPerAdapter();

private:

	AdapterData  adaptersData_[3];
	int    numAdapters_ = 0;


	//AdapterData*  adaptersData_ = nullptr;   // array of adapters and related data
};

} // namespace Core