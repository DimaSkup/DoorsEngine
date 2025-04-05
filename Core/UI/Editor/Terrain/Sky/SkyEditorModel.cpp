// =================================================================================
// Filename:    SkyEditorModel.h
// 
// Created:     21.02.25  by DimaSkup
// =================================================================================
#include "SkyEditorModel.h"
#include <CoreCommon/log.h>

namespace UI
{

// =================================================================================
// setters
// =================================================================================

void ModelSky::SetTextureID(const int idx, const uint32_t textureID)
{
	if (idx >= 0)
		textureIdxToID_[idx] = textureID;
	else 
		Core::Log::Error("wrong texture idx: " + std::to_string(idx));
}


// =================================================================================
// getters
// =================================================================================

uint32_t ModelSky::GetTextureID(const int idx) const
{
	try
	{
		return textureIdxToID_.at(idx);
	}
	catch (const std::out_of_range& e)
	{
		Core::Log::Error(e.what());
		Core::Log::Error("try to get a texture ID by wrong idx: " + std::to_string(idx));
		return 0;  // return invalid texture ID
	}
}

}