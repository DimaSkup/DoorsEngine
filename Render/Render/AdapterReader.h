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
//#pragma comment (lib, "d3d11.lib")
//#pragma comment (lib, "d3dx11.lib")
//#pragma comment (lib, "d3dx10.lib")

/////////////////////////////
// INCLUDES
/////////////////////////////
//#include <d3d11.h>
#include <dxgi.h>	// a DirectX graphic interface header


namespace Render
{

class AdapterReader
{
public:
    AdapterReader() {};

    static void          EnumerateAdapters     (cvector<IDXGIAdapter*>& outAdapters);
    static IDXGIAdapter* FindAnySuitableAdapter(const cvector<IDXGIAdapter*>& adapters);
};

} // namespace Core
