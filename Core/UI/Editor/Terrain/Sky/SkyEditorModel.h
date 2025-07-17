// =================================================================================
// Filename:    SkyEditorModel.h
// Description: a data container (MVC model) for the editing of the sky
// =================================================================================

#pragma once

#include <UICommon/Color.h>
#include <UICommon/Vectors.h>

#include <cstdint>
#include <unordered_map>


namespace UI
{

class ModelSky
{
public:
    void Init(
        const EntityID skyID,
        const ColorRGB& center,
        const ColorRGB& apex,
        const Vec3& offset)
    {
        id_          = skyID;
        colorCenter_ = center;
        colorApex_   = apex;
        offset_      = offset;
    }

	// ====================================================
	//                     setters
	// ====================================================

	inline void SetColorCenter(const ColorRGB& newColor) { colorCenter_ = newColor;	}
	inline void SetColorApex(const ColorRGB& newColor)   { colorApex_ = newColor;	}
	inline void SetSkyOffset(const Vec3& newOffset)      { offset_ = newOffset; }

	
	// ====================================================
	//                     getters
	// ====================================================
          
    
	inline void GetColorCenter(ColorRGB& outColor) const { outColor = colorCenter_; }
	inline void GetColorApex(ColorRGB& outColor)   const { outColor = colorApex_; }
	inline void GetSkyOffset(Vec3& outOffset)      const { outOffset = offset_; }

    inline EntityID        GetID()                 const { return id_; }
	inline const ColorRGB& GetColorCenter()        const { return colorCenter_; }
	inline const ColorRGB& GetColorApex()          const { return colorApex_; }
	inline const Vec3&     GetSkyOffset()          const { return offset_; }

private:
	std::unordered_map<int, uint32_t> textureIdxToID_;   // map of pairs: [texture_idx => texture_ID]

    EntityID id_;              // sky entity id
	ColorRGB colorCenter_;     // RGB color
	ColorRGB colorApex_;       // RGB color
	Vec3 offset_;              // offset by x,y,z of the sky mesh

};

} // namespace UI
