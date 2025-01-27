#pragma once

#include "../../UICommon/Color.h"
#include "../../UICommon/Vectors.h"
#include "../../../Common/log.h"

#include <cstdint>
#include <stdexcept>
#include <unordered_map>



// Everything that is storage related (and used for the editor)
namespace Model
{

class Sky
{
public:

	// ====================================================
	//                     setters
	// ====================================================

	void SetTextureID(const int idx, const uint32_t textureID)
	{
		if (idx >= 0)
			textureIdxToID_[idx] = textureID;
		else 
			Log::Error("wrong texture idx: " + std::to_string(idx));
	}

	inline void SetColorCenter(const ColorRGB& newColor) { colorCenter_ = newColor;	}
	inline void SetColorApex(const ColorRGB& newColor)   { colorApex_ = newColor;	}
	inline void SetSkyOffset(const Vec3& newOffset)      { offset_ = newOffset; }

	
	// ====================================================
	//                     getters
	// ====================================================

	inline uint32_t GetTextureID(const int idx) const
	{ 
		try
		{
			return textureIdxToID_.at(idx);
		}
		catch (const std::out_of_range& e)
		{
			Log::Error(e.what());
			Log::Error("try to get a texture ID by wrong idx: " + std::to_string(idx));
			return 0;  // return invalid texture ID
		}
	}

	inline void GetColorCenter(ColorRGB& outColor) const { outColor = colorCenter_; }
	inline void GetColorApex(ColorRGB& outColor)   const { outColor = colorApex_; }
	inline void GetSkyOffset(Vec3& outOffset)      const { outOffset = offset_; }

	inline const ColorRGB& GetColorCenter()        const { return colorCenter_; }
	inline const ColorRGB& GetColorApex()          const { return colorApex_; }
	inline const Vec3& GetSkyOffset()              const { return offset_; }

private:
	std::unordered_map<int, uint32_t> textureIdxToID_;   // map of pairs: [texture_idx => texture_ID]

	ColorRGB colorCenter_;     // RGB color
	ColorRGB colorApex_;       // RGB color
	Vec3 offset_;              // offset by x,y,z of the sky mesh

};

}
