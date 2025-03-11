// =================================================================================
// Filename:      AdapterReader.cpp
// =================================================================================
#include "AdapterReader.h"
#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/MemHelpers.h>
#include <stdexcept>


namespace Core
{

AdapterReader::AdapterReader()
{
	try
	{
		int idx = 0;
		IDXGIFactory* pFactory = nullptr;
		IDXGIAdapter* pAdapter = nullptr;
		IDXGIOutput*  pOutput = nullptr;

		// Create a DXGI Factory
		HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
		Assert::NotFailed(hr, "can't create the DXGI Factory");

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

			// enumerate all the output for this adapter
			//while (pAdapter->EnumOutputs(outputIdx, &pOutput) != DXGI_ERROR_NOT_FOUND)

			pAdapter->EnumOutputs(0, &adaptersData_[idx].pOutput_);
		}
	}
	catch (EngineException& e)
	{
		Log::Error(e, true);
		Shutdown();
		throw EngineException("can't get adapters data");
	}
}

///////////////////////////////////////////////////////////

AdapterData::AdapterData(IDXGIAdapter* pAdapter) : pAdapter_(pAdapter)
{
	// stores a pointer to IDXGI adapter and its description

	Assert::NotNullptr(pAdapter, "ptr to adapter == nullptr");

	HRESULT hr = pAdapter->GetDesc(&description_);
	Assert::NotFailed(hr, "failed to get description for IDXGIAdapter");
}

///////////////////////////////////////////////////////////

AdapterReader::~AdapterReader()
{
	Shutdown();
}



// ====================================================================================
//                              public methods
// ====================================================================================

AdapterData* AdapterReader::GetAdapterDataByIdx(const int idx)
{
	assert((0 <= idx) && (idx < numAdapters_) && "wrong index");

	return adaptersData_ + idx;
}

IDXGIAdapter* AdapterReader::GetDXGIAdapterByIdx(const int idx)
{
	assert((0 <= idx) && (idx < numAdapters_) && "wrong index");

	return adaptersData_[idx].pAdapter_;
}


// ====================================================================================
//                              private methods
// ====================================================================================

void AdapterReader::Shutdown()
{
	// go through each element of the array of adapters data
	// and release the adapters interface pointers

	for (int i = 0; i < numAdapters_; ++i)
	{
		AdapterData& data = adaptersData_[i];

		SafeRelease(&data.pAdapter_);
		SafeRelease(&data.pOutput_);
	}
}

} // namespace Core