// =================================================================================
// Filename:      AdapterReader.cpp
// =================================================================================
#include "../Common/pch.h"
#include "AdapterReader.h"


namespace Render
{

//---------------------------------------------------------
// Desc:   empty constructor
//         - create DXGI factory
//         - enumerate adapters
//         - enumerate outputs of each adapter
//---------------------------------------------------------
AdapterReader::AdapterReader()
{
    int idx = 0;
    IDXGIFactory* pFactory = nullptr;
    IDXGIAdapter* pAdapter = nullptr;
    IDXGIOutput*  pOutput  = nullptr;

    // Create a DXGI Factory
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create the DXGI Factory");
        exit(0);
    }

    // go through all the available graphics adapters and get a ptr to it and get data
    while (SUCCEEDED(pFactory->EnumAdapters(idx, &pAdapter)))
    {
        adaptersData_[idx] = AdapterData(pAdapter);
        numAdapters_++;
        idx++;
    }

    // enumerate outputs for each available graphics adapter
    for (int idx = 0, outputIdx = 0; idx < numAdapters_; ++idx)
    {
        pAdapter = adaptersData_[idx].pAdapter_;

        // enumerate the outputs for this adapter
        pAdapter->EnumOutputs(0, &adaptersData_[idx].pOutput_);
    }
}

//---------------------------------------------------------
// Desc:  a constructor: stores a pointer to IDXGI adapter and its description
//---------------------------------------------------------
AdapterData::AdapterData(IDXGIAdapter* pAdapter)
    : pAdapter_(pAdapter)
{
    if (!pAdapter)
    {
        LogErr(LOG, "input ptr to adapter == nullptr");
        return;
    }

    HRESULT hr = pAdapter->GetDesc(&description_);
    if (FAILED(hr))
    {
        LogErr(LOG, "failed to get description for IDXGIAdapter");
        return;
    }
}

//---------------------------------------------------------
// Desc:   just destructor
//---------------------------------------------------------
AdapterReader::~AdapterReader()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:   return a ptr to adapter data by input index
//---------------------------------------------------------
AdapterData* AdapterReader::GetAdapterDataByIdx(const int idx)
{
    if ((idx < 0) || (idx >= numAdapters_))
    {
        LogErr(LOG, "input index is wrong (%d), but expected to be in range [0; %d)", idx, numAdapters_);
        return nullptr;
    }

    return adaptersData_ + idx;
}

//---------------------------------------------------------
// Desc:   return a ptr to adapter by input index
//---------------------------------------------------------
IDXGIAdapter* AdapterReader::GetDXGIAdapterByIdx(const int idx)
{
    if ((idx < 0) || (idx >= numAdapters_))
    {
        LogErr(LOG, "input index is wrong (%d), but expected to be in range [0; %d)", idx, numAdapters_);
        return nullptr;
    }

    return adaptersData_[idx].pAdapter_;
}

//---------------------------------------------------------
// Desc:   go through each element of the array of adapters data
//         and release the adapters interface pointers
//---------------------------------------------------------
void AdapterReader::Shutdown()
{

    for (int i = 0; i < numAdapters_; ++i)
    {
        AdapterData& data = adaptersData_[i];

        SafeRelease(&data.pAdapter_);
        SafeRelease(&data.pOutput_);
    }
}

} // namespace
