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

// flags to define specific properties of the material
enum eMaterialProp : uint32
{
    MAT_PROP_ALPHA_CLIPPING          = (1 << 0),

    MAT_PROP_FILL_SOLID              = (1 << 1),
    MAT_PROP_FILL_WIREFRAME          = (1 << 2),
    
    MAT_PROP_CULL_BACK               = (1 << 3),
    MAT_PROP_CULL_FRONT              = (1 << 4),
    MAT_PROP_CULL_NONE               = (1 << 5),

    MAT_PROP_FRONT_COUNTER_CLOCKWISE = (1 << 6),   // ccw
    MAT_PROP_FRONT_CLOCKWISE         = (1 << 7),

    // blending states
    MAT_PROP_NO_RENDER_TARGET_WRITES = (1 << 8),
    MAT_PROP_ALPHA_DISABLE           = (1 << 9),
    MAT_PROP_ALPHA_ENABLE            = (1 << 10),
    MAT_PROP_ADDING                  = (1 << 11),
    MAT_PROP_SUBTRACTING             = (1 << 12),
    MAT_PROP_MULTIPLYING             = (1 << 13),
    MAT_PROP_TRANSPARENCY            = (1 << 14),
    MAT_PROP_ALPHA_TO_COVERAGE       = (1 << 15),

    // depth stencil states
    MAT_PROP_DEPTH_ENABLED           = (1 << 16),
    MAT_PROP_DEPTH_DISABLED          = (1 << 17),
    MAT_PROP_MARK_MIRROR             = (1 << 18),        // for rendering mirror reflections
    MAT_PROP_DRAW_REFLECTION         = (1 << 19),
    MAT_PROP_NO_DOUBLE_BLEND         = (1 << 20),
    MAT_PROP_SKY_DOME                = (1 << 21),


    ALL_FILL_MODES                   = MAT_PROP_FILL_SOLID | MAT_PROP_FILL_WIREFRAME,
    ALL_CULL_MODES                   = MAT_PROP_CULL_BACK | MAT_PROP_CULL_FRONT | MAT_PROP_CULL_NONE,
    ALL_FRONT_CLOCKWISE_MODES        = MAT_PROP_FRONT_COUNTER_CLOCKWISE | MAT_PROP_FRONT_CLOCKWISE,
    ALL_BLEND_STATES                 = MAT_PROP_NO_RENDER_TARGET_WRITES | MAT_PROP_ALPHA_DISABLE | MAT_PROP_ALPHA_ENABLE | MAT_PROP_ADDING | MAT_PROP_SUBTRACTING | MAT_PROP_MULTIPLYING | MAT_PROP_TRANSPARENCY | MAT_PROP_ALPHA_TO_COVERAGE,
    ALL_DEPTH_STENCIL_STATES         = MAT_PROP_DEPTH_ENABLED | MAT_PROP_DEPTH_DISABLED | MAT_PROP_MARK_MIRROR | MAT_PROP_DRAW_REFLECTION | MAT_PROP_NO_DOUBLE_BLEND | MAT_PROP_SKY_DOME,

    // fill_solid / cull_back / front_clockwise / no_blending / depth_enabled
    MAT_PROP_DEFAULT                 = MAT_PROP_FILL_SOLID | MAT_PROP_CULL_BACK | MAT_PROP_FRONT_CLOCKWISE | MAT_PROP_ALPHA_DISABLE | MAT_PROP_DEPTH_ENABLED,

    NUM_PROPERTIES
};

///////////////////////////////////////////////////////////

struct Material
{
    Float4     ambient  = { 1,1,1,1 };
    Float4     diffuse  = { 1,1,1,1 };
    Float4     specular = { 0,0,0,1 };                              // w-component is a specPower (specular power)
    Float4     reflect  = { .5f, .5f, .5f, 1 };

    MaterialID id = INVALID_MATERIAL_ID;
    char       name[MAX_LENGTH_MATERIAL_NAME] = { "invalid" };

    TexID      textureIDs[NUM_TEXTURE_TYPES]{ INVALID_TEXTURE_ID };
    uint32     properties = 0;                                      // bitfield for materials properties

    //-----------------------------------------------------

    Material() : properties(MAT_PROP_DEFAULT)
    {
    }

    Material(const char* name) : properties(MAT_PROP_DEFAULT)
    {
        SetName(name);
    }

    //-----------------------------------------------------
    // Setters
    //-----------------------------------------------------
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

    //-----------------------------------------------------

    inline void SetTexture(const eTexType type, const TexID id)                          { textureIDs[type] = id; }

    inline void SetAmbient (const float r, const float g, const float b, const float a)  { ambient = Float4(r,g,b,a); }
    inline void SetDiffuse (const float r, const float g, const float b, const float a)  { diffuse = Float4(r,g,b,a); }
    inline void SetSpecular(const float r, const float g, const float b)                 { specular = Float4(r,g,b,specular.w); }   // specular power remains the same
    inline void SetSpecularPower(const float power)                                      { specular.w = power; }

    inline void SetAlphaClip(const bool state)
    {
        properties &= ~(MAT_PROP_ALPHA_CLIPPING);            // reset to 0

        if (state)
            properties |= MAT_PROP_ALPHA_CLIPPING;           // set alpha clipping
    }

    inline void SetFill(eMaterialProp prop)
    {
        if (prop & ALL_FILL_MODES)
        {
            properties &= ~(ALL_FILL_MODES);       // reset all fill modes
            properties |= prop;                    // set fill mode
        }
    }

    inline void SetCull(eMaterialProp prop)
    {
        if (prop & ALL_CULL_MODES)
        {
            properties &= ~(ALL_CULL_MODES);      // reset all cull modes
            properties |= prop;                   // set cull mode
        }
    }

    inline void SetFrontClockwise(eMaterialProp prop)
    {
        if (prop & ALL_FRONT_CLOCKWISE_MODES)
        {
            properties &= ~(ALL_FRONT_CLOCKWISE_MODES);
            properties |= prop;
        }
    }

    inline void SetBlending(eMaterialProp prop)
    {
        if (prop & ALL_BLEND_STATES)
        {
            properties &= ~(ALL_BLEND_STATES);    // reset all blend states
            properties |= prop;                   // set blend state
        }
    }

    inline void SetDepthStencil(const eMaterialProp prop)
    {
        if (prop & ALL_DEPTH_STENCIL_STATES)
        {
            properties &= ~(ALL_DEPTH_STENCIL_STATES);
            properties |= prop;
        }
    }

    //-----------------------------------------------------

    inline bool HasAlphaClip() const { return (bool)(properties & MAT_PROP_ALPHA_CLIPPING); }

};

} // namespace Core
