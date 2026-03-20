// =================================================================================
// Filename:      AdapterReader.cpp
// =================================================================================
#include "../Common/pch.h"
#include "AdapterReader.h"

namespace Render
{

//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void PrintAdapterInfo(IDXGIAdapter* pAdapter);
void PrintHRESULT(HRESULT hr);

//---------------------------------------------------------
// enumerate each available video adapter and push its ptr into output array
//---------------------------------------------------------
void AdapterReader::EnumerateAdapters(cvector<IDXGIAdapter*>& outAdapters)
{
    IDXGIFactory* pFactory = nullptr;
    IDXGIAdapter* pAdapter = nullptr;

    outAdapters.clear();

    // Create a DXGI Factory object
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
    if (FAILED(hr))
    {
        PrintHRESULT(hr);
        LogErr(LOG, "can't create the DXGI Factory");
        SafeRelease(&pFactory);
    }

    SetConsoleColor(YELLOW);
    LogMsg("\n");
    LogMsg("Video adapters list:");

    // enumerate each available video adapter
    for (UINT i = 0;
        pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
        ++i)
    {
        outAdapters.push_back(pAdapter);
        PrintAdapterInfo(pAdapter);
    }

    SafeRelease(&pFactory);
}

//---------------------------------------------------------
//---------------------------------------------------------
IDXGIAdapter* AdapterReader::FindAnySuitableAdapter(const cvector<IDXGIAdapter*>& adapters)
{
    int lastMemorySize = 0;
    int index = -1;

    for (int i = 0; i < adapters.size(); i++)
    {
        DXGI_ADAPTER_DESC desc;
        adapters[i]->GetDesc(&desc);

        if (desc.DedicatedVideoMemory > lastMemorySize)
        {
            lastMemorySize = (int)desc.DedicatedVideoMemory;
            index = i;
        }
    }

    if (index != -1 && index <= adapters.size())
        return adapters[index];

    return nullptr;
}

//---------------------------------------------------------
// Desc:  a constructor: stores a pointer to IDXGI adapter and its description
//---------------------------------------------------------
void PrintAdapterInfo(IDXGIAdapter* pAdapter)
{
    if (!pAdapter)
    {
        LogErr(LOG, "ptr to adapter == NULL");
        return;
    }

    HRESULT hr = S_OK;
    DXGI_ADAPTER_DESC desc;

    hr = pAdapter->GetDesc(&desc);
    if (FAILED(hr))
    {
        PrintHRESULT(hr);
        LogErr(LOG, "failed to get description for IDXGIAdapter");
        return;
    }

    char adapterDesc[128];
    memset(adapterDesc, 0, sizeof(adapterDesc));

    StrHelper::ToStr(desc.Description, adapterDesc);

    SetConsoleColor(GREEN);
    LogMsg("\n");

    // common info
    LogMsg("Adapter:        %s", adapterDesc);
    LogMsg("VendorId:       %zu", desc.VendorId);
    LogMsg("DeviceId:       %zu", desc.DeviceId);
    LogMsg("Sub sys id:     %zu", desc.SubSysId);

    // memory info
    constexpr ULONG bytesInMB = 1048576;

    LogMsg("Video mem:      %lu bytes;  %lu MB", desc.DedicatedVideoMemory,  desc.DedicatedVideoMemory  / bytesInMB);
    LogMsg("Sys mem:        %lu bytes;  %lu MB", desc.DedicatedSystemMemory, desc.DedicatedSystemMemory / bytesInMB);
    LogMsg("Shared sysmem:  %lu bytes;  %lu MB", desc.SharedSystemMemory,    desc.SharedSystemMemory    / bytesInMB);

    LogMsg("\n");
    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// if HRESULT isn't S_OK we call this func and print hr error code
//---------------------------------------------------------
void PrintHRESULT(HRESULT hr)
{
    _com_error err(hr);
    char errMsg[512];

    memset(errMsg, 0, sizeof(errMsg));
    StrHelper::ToStr(err.ErrorMessage(), errMsg);

    LogErr(LOG, "HRESULT: 0x%08X - %s\n\n", hr, errMsg);
}

} // namespace
