#pragma once

#include <types.h>
#include <math/vec4.h>
#include <d3d11.h>

struct MaterialData
{
    MaterialID materialId = -1;
    ShaderID   shaderId   = -1;                                 // id of currently selected shader for rendering this material

    Vec4 ambient  = { 1,1,1,1 };
    Vec4 diffuse  = { 1,1,1,1 };
    Vec4 specular = { 0,0,0,1 };                              // w-component is a specPower (specular power)
    Vec4 reflect  = { .5f, .5f, .5f, 1 };

    char name[MAX_LEN_MAT_NAME]{ '\0' };

    TexID                     textureIDs[NUM_TEXTURE_TYPES]{ INVALID_TEX_ID };
    ID3D11ShaderResourceView* textures[NUM_TEXTURE_TYPES]{ nullptr };

    uint currRsId = 0;      // current raster state of this material
    uint currBsId = 0;      // current blend state of this material
    uint currDssId = 0;     // current depth-stencil state of this material
};
