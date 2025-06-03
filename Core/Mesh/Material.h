// ********************************************************************************
// Filename:      Material.h
// Description:   data structure of a Material (surface properties + textures)
// 
// Created:       28.10.24
// ********************************************************************************
#pragma once

#include <Types.h>
#include <log.h>
#include <CAssert.h>
#include "../Texture/TextureTypes.h"


#pragma warning (disable : 4996)


namespace Core
{

// flags to define specific property of the material
enum eMaterialProp : uint32
{
    ALPHA_CLIPPING,
    NUM_PROPERTIES,
};

///////////////////////////////////////////////////////////

struct Material
{
    Float4   ambient  = { 1,1,1,1 };
    Float4   diffuse  = { 1,1,1,1 };
    Float4   specular = { 0,0,0,1 };                              // w-component is a specPower (specular power)
    Float4   reflect  = { .5f, .5f, .5f, 1 };

    char     name[MAX_LENGTH_MATERIAL_NAME]{ '\0' };
    TexID    textureIDs[NUM_TEXTURE_TYPES]{ INVALID_TEXTURE_ID };
    uint32   properties = 0;                                      // bitfield for materials properties

    // ----------------------------------------------------

    Material() {}

    void SetName(const char* inName)
    {
        if ((inName == nullptr) || (inName[0] == '\0'))
        {
            LogErr("can't set name for material: input name is empty!");
            return;
        }

        size_t length = strlen(inName);

        CAssert::True(inName != nullptr, "input ptr to name string == nullptr");
        CAssert::True(length > 0,        "length of input name string must be > 0");

        if (length > MAX_LENGTH_MATERIAL_NAME)
            length = MAX_LENGTH_MATERIAL_NAME;

        strncpy(name, inName, length);
    }

    inline void SetTexture(const eTexType type, const TexID id)                          { textureIDs[type] = id; }

    inline void SetAmbient (const float r, const float g, const float b, const float a)  { ambient = Float4(r,g,b,a); }
    inline void SetDiffuse (const float r, const float g, const float b, const float a)  { diffuse = Float4(r,g,b,a); }
    inline void SetSpecular(const float r, const float g, const float b)                 { specular = Float4(r,g,b,specular.w); }   // specular power remains the same
    inline void SetSpecularPower(const float power)                                      { specular.w = power; }


    inline void SetFlag(const eMaterialProp prop, const bool state)
    {
        properties &= ~(1 << prop);            // reset flag to 0
        properties |= ((int)(state) << prop);  // setup flag
    }

    inline void SetAlphaClip(const bool state) { SetFlag(ALPHA_CLIPPING, state); }

};

} // namespace Core
