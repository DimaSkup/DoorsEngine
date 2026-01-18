// ********************************************************************************
// Filename:      Material.h
// Description:   data structure of a Material (surface properties + textures)
// 
// Created:       28.10.24
// ********************************************************************************
#pragma once

#include <math/vec4.h>
#include <Types.h>
#include "../Texture/enum_texture_types.h"
#include <string.h>

#pragma warning (disable : 4996)


namespace Core
{

struct Material
{
    Vec4       ambient  = { 1,1,1,1 };
    Vec4       diffuse  = { 1,1,1,1 };
    Vec4       specular = { 0,0,0,1 };                                // w-component is a glossiness (specPower, specular power)
    Vec4       reflect  = { 0,0,0,0 };

    MaterialID id = INVALID_MATERIAL_ID;
    ShaderID   shaderId = INVALID_SHADER_ID;                          // default "Invalid" shader with id == 0
    char       name[MAX_LEN_MAT_NAME] = { "invalid" };
    
    TexID      texIds[NUM_TEXTURE_TYPES]{ INVALID_TEX_ID };

    // render states
    RsID       rsId = 0;
    BsID        bsId = 0;
    DssID dssId = 0;
    bool                alphaClip = false;

    //-----------------------------------------------------

    Material()
    {
    }

    Material(const char* name)
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
            //LogErr("can't set name for material: input name is empty!");
            return;
        }

        size_t len = strlen(inName);

        if (len > MAX_LEN_MAT_NAME-1)
            len = MAX_LEN_MAT_NAME-1;

        strncpy(name, inName, len);
        name[len] = '\0';
    }

    inline bool HasAlphaClip() const { return alphaClip; }

    //-----------------------------------------------------

    inline void SetTexture(const eTexType type, const TexID id)                           { texIds[type] = id; }

    inline void SetAmbient (const float r, const float g, const float b, const float a)   { ambient = Vec4(r,g,b,a); }
    inline void SetDiffuse (const float r, const float g, const float b, const float a)   { diffuse = Vec4(r,g,b,a); }

    //-----------------------------------------------------
    // Desc:  setup specular RGB color (NOTE: specular power(glossiness remains the same)
    //-----------------------------------------------------
    inline void SetSpecular(const float r, const float g, const float b)                  { specular = Vec4(r,g,b,specular.w); }   

    //-----------------------------------------------------
    // Desc:  setup specular power (glossiness)
    //-----------------------------------------------------
    inline void SetSpecularPower(const float power)                                       { specular.w = power; }
    inline void SetGlossiness(const float gloss)                                          { specular.w = gloss; }

    //-----------------------------------------------------
   // Desc:  setup reflection RGB color
   //-----------------------------------------------------
    inline void SetReflection(const float r, const float g, const float b, const float a) { reflect = Vec4(r,g,b,a); }


    inline void SetAlphaClip(const bool state) { alphaClip = true; }
};

} // namespace Core
