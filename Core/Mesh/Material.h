// ********************************************************************************
// Filename:      Material.h
// Description:   data structure of a Material (surface properties + textures)
// 
// Created:       28.10.24
// ********************************************************************************
#pragma once

#include <Types.h>
#include <log.h>
#include "../Texture/TextureTypes.h"

#pragma warning (disable : 4996)


namespace Core
{

// flags to define specific properties of the material
enum eMaterialProp : uint32
{
    MAT_PROP_ALPHA_CLIPPING                 = (1 << 0),

    MAT_PROP_FILL_SOLID                     = (1 << 1),
    MAT_PROP_FILL_WIREFRAME                 = (1 << 2),
    
    MAT_PROP_CULL_BACK                      = (1 << 3),
    MAT_PROP_CULL_FRONT                     = (1 << 4),
    MAT_PROP_CULL_NONE                      = (1 << 5),

    MAT_PROP_FRONT_COUNTER_CLOCKWISE        = (1 << 6),   // ccw
    MAT_PROP_FRONT_CLOCKWISE                = (1 << 7),

    // BS -- blending states
    MAT_PROP_BS_NO_RENDER_TARGET_WRITES  = (1 << 8),
    MAT_PROP_BS_DISABLE                  = (1 << 9),
    MAT_PROP_BS_ENABLE                   = (1 << 10),
    MAT_PROP_BS_ADD                      = (1 << 11),   // adding
    MAT_PROP_BS_SUB                      = (1 << 12),   // subtraction
    MAT_PROP_BS_MUL                      = (1 << 13),   // multiplication
    MAT_PROP_BS_TRANSPARENCY             = (1 << 14),
    MAT_PROP_BS_ALPHA_TO_COVERAGE        = (1 << 15),

    // DSS -- depth stencil states
    MAT_PROP_DSS_DEPTH_ENABLED                  = (1 << 16),
    MAT_PROP_DSS_DEPTH_DISABLED                 = (1 << 17),
    MAT_PROP_DSS_MARK_MIRROR                    = (1 << 18),        // for rendering mirror reflections
    MAT_PROP_DSS_DRAW_REFLECTION                = (1 << 19),
    MAT_PROP_DSS_NO_DOUBLE_BLEND                = (1 << 20),
    MAT_PROP_DSS_SKY_DOME                       = (1 << 21),


    ALL_FILL_MODES                   = MAT_PROP_FILL_SOLID | MAT_PROP_FILL_WIREFRAME,
    ALL_CULL_MODES                   = MAT_PROP_CULL_BACK | MAT_PROP_CULL_FRONT | MAT_PROP_CULL_NONE,
    ALL_FRONT_CLOCKWISE_MODES        = MAT_PROP_FRONT_COUNTER_CLOCKWISE | MAT_PROP_FRONT_CLOCKWISE,
    ALL_BLEND_STATES                 = MAT_PROP_BS_NO_RENDER_TARGET_WRITES | MAT_PROP_BS_DISABLE | MAT_PROP_BS_ENABLE | MAT_PROP_BS_ADD | MAT_PROP_BS_SUB | MAT_PROP_BS_MUL | MAT_PROP_BS_TRANSPARENCY | MAT_PROP_BS_ALPHA_TO_COVERAGE,
    ALL_DEPTH_STENCIL_STATES         = MAT_PROP_DSS_DEPTH_ENABLED | MAT_PROP_DSS_DEPTH_DISABLED | MAT_PROP_DSS_MARK_MIRROR | MAT_PROP_DSS_DRAW_REFLECTION | MAT_PROP_DSS_NO_DOUBLE_BLEND | MAT_PROP_DSS_SKY_DOME,

    // fill_solid / cull_back / front_clockwise / no_blending / depth_enabled
    MAT_PROP_DEFAULT                 = MAT_PROP_FILL_SOLID | MAT_PROP_CULL_BACK | MAT_PROP_FRONT_CLOCKWISE | MAT_PROP_BS_DISABLE | MAT_PROP_DSS_DEPTH_ENABLED,

    MAT_BLEND_NOT_TRANSPARENT        = MAT_PROP_BS_ADD | MAT_PROP_BS_SUB | MAT_PROP_BS_MUL | MAT_PROP_BS_ALPHA_TO_COVERAGE,

    // material groups
    MAT_MASKED_GROUP = MAT_PROP_ALPHA_CLIPPING | MAT_PROP_FILL_SOLID | MAT_PROP_CULL_NONE | MAT_PROP_FRONT_CLOCKWISE | MAT_PROP_BS_DISABLE | MAT_PROP_DSS_DEPTH_ENABLED,
    MAT_OPAQUE_GROUP =                           MAT_PROP_FILL_SOLID | MAT_PROP_CULL_BACK | MAT_PROP_FRONT_CLOCKWISE | MAT_PROP_BS_DISABLE | MAT_PROP_DSS_DEPTH_ENABLED,
    MAT_BLEND_GROUP  =                           MAT_PROP_FILL_SOLID | MAT_PROP_CULL_BACK | MAT_PROP_FRONT_CLOCKWISE | ALL_BLEND_STATES    | MAT_PROP_DSS_DEPTH_ENABLED,

    NUM_PROPERTIES
};

///////////////////////////////////////////////////////////

struct Material
{
    Float4     ambient  = { 1,1,1,1 };
    Float4     diffuse  = { 1,1,1,1 };
    Float4     specular = { 0,0,0,1 };                                // w-component is a glossiness (specPower, specular power)
    Float4     reflect  = { 0,0,0,0 };

    MaterialID id = INVALID_MATERIAL_ID;
    ShaderID   shaderId = INVALID_SHADER_ID;                          // default "Invalid" shader with id == 0
    char       name[MAX_LENGTH_MATERIAL_NAME] = { "invalid" };
    
    TexID      texIds[NUM_TEXTURE_TYPES]{ INVALID_TEXTURE_ID };
    uint32     renderStates = 0;                                      // bitfield for materials properties


    //-----------------------------------------------------

    Material() : renderStates(MAT_PROP_DEFAULT)
    {
    }

    Material(const char* name) : renderStates(MAT_PROP_DEFAULT)
    {
        SetName(name);
    }

    //-----------------------------------------------------
    // Setters
    //-----------------------------------------------------

    inline void SetShaderId(ShaderID id) { shaderId = id; }

    void SetName(const char* inName)
    {
        if ((inName == nullptr) || (inName[0] == '\0'))
        {
            LogErr("can't set name for material: input name is empty!");
            return;
        }

        size_t len = strlen(inName);

        if (len > MAX_LENGTH_MATERIAL_NAME-1)
            len = MAX_LENGTH_MATERIAL_NAME-1;

        strncpy(name, inName, len);
        name[len] = '\0';
    }

    inline bool HasAlphaClip() const { return renderStates & MAT_PROP_ALPHA_CLIPPING; }

    //-----------------------------------------------------

    inline void SetTexture(const eTexType type, const TexID id)                           { texIds[type] = id; }

    inline void SetAmbient (const float r, const float g, const float b, const float a)   { ambient = Float4(r,g,b,a); }
    inline void SetDiffuse (const float r, const float g, const float b, const float a)   { diffuse = Float4(r,g,b,a); }

    //-----------------------------------------------------
    // Desc:  setup specular RGB color (NOTE: specular power(glossiness remains the same)
    //-----------------------------------------------------
    inline void SetSpecular(const float r, const float g, const float b)                  { specular = Float4(r,g,b,specular.w); }   

    //-----------------------------------------------------
    // Desc:  setup specular power (glossiness)
    //-----------------------------------------------------
    inline void SetSpecularPower(const float power)                                       { specular.w = power; }
    inline void SetGlossiness(const float gloss)                                          { specular.w = gloss; }

    //-----------------------------------------------------
   // Desc:  setup reflection RGB color
   //-----------------------------------------------------
    inline void SetReflection(const float r, const float g, const float b, const float a) { reflect = Float4(r,g,b,a); }



    inline void SetAlphaClip(const bool state)
    {
        renderStates &= ~(MAT_PROP_ALPHA_CLIPPING);            // reset to 0

        if (state)
            renderStates |= MAT_PROP_ALPHA_CLIPPING;           // set alpha clipping
    }

    inline void SetFill(eMaterialProp prop)
    {
        if (prop & ALL_FILL_MODES)
        {
            renderStates &= ~(ALL_FILL_MODES);       // reset all fill modes
            renderStates |= prop;                    // set fill mode
        }
    }

    inline void SetCull(eMaterialProp prop)
    {
        if (prop & ALL_CULL_MODES)
        {
            renderStates &= ~(ALL_CULL_MODES);      // reset all cull modes
            renderStates |= prop;                   // set cull mode
        }
    }

    inline void SetFrontClockwise(eMaterialProp prop)
    {
        if (prop & ALL_FRONT_CLOCKWISE_MODES)
        {
            renderStates &= ~(ALL_FRONT_CLOCKWISE_MODES);
            renderStates |= prop;
        }
    }

    inline void SetBlending(eMaterialProp prop)
    {
        if (prop & ALL_BLEND_STATES)
        {
            renderStates &= ~(ALL_BLEND_STATES);    // reset all blend states
            renderStates |= prop;                   // set blend state
        }
    }

    inline void SetDepthStencil(const eMaterialProp prop)
    {
        if (prop & ALL_DEPTH_STENCIL_STATES)
        {
            renderStates &= ~(ALL_DEPTH_STENCIL_STATES);
            renderStates |= prop;
        }
    }

    //-----------------------------------------------------
    // setup render state by idx of property inside its own group
    // (groups: fill, cull, blending, depth-stencil, etc.)
    //-----------------------------------------------------
    inline void SetFillByIdx(const uint32 propIdx)
    {
        SetFill(eMaterialProp(MAT_PROP_FILL_SOLID << propIdx));
    }

    inline void SetCullByIdx(const uint32 propIdx)
    {
        SetCull(eMaterialProp(MAT_PROP_CULL_BACK << propIdx));
    }

    inline void SetBlendingByIdx(const uint32 propIdx)
    {
        SetBlending(eMaterialProp(MAT_PROP_BS_NO_RENDER_TARGET_WRITES << propIdx));
    }

    inline void SetDepthStencilByIdx(const uint32 propIdx)
    {
        SetDepthStencil(eMaterialProp(MAT_PROP_DSS_DEPTH_ENABLED << propIdx));
    }
};

} // namespace Core
