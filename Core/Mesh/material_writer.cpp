/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: material_writer.cpp
    Desc:     write materials into file

    Created:  19.10.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "material.h"
#include "material_writer.h"
#include "material_mgr.h"
#include <Texture/texture_mgr.h>



namespace Core
{
//---------------------------------------------------------
// helpers forward declaration
//---------------------------------------------------------
void WriteCommon  (FILE* pFile, const Material& mat);
void WriteColors  (FILE* pFile, const Material& mat);
void WriteTextures(FILE* pFile, const Material& mat);


//---------------------------------------------------------
// Desc:  write data of multiple materials into a single file
//---------------------------------------------------------
bool MaterialWriter::Write(
    const Material* materials,
    const int numMaterials,
    const char* filePath)
{
    // check input args
    if (!materials || (numMaterials <= 0))
    {
        LogErr(LOG, "input materials are invalid");
        return false;
    }
    if (!filePath || filePath[0] == '\0')
    {
        LogErr(LOG, "input filepath is empty!");
        return false;
    }


    FILE* pFile = fopen(filePath, "w");
    if (!pFile)
    {
        LogErr(LOG, "can't open file for materials writing: %s", filePath);
        return false;
    }

    // go through each material and write it into file
    for (int i = 0; i < numMaterials; ++i)
    {
        const Material& mat = materials[i];

        WriteCommon(pFile, mat);
        WriteColors(pFile, mat);
        WriteTextures(pFile, mat);
        fprintf(pFile, "\n");
    }

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:   write material id, shader id, name, render states
//---------------------------------------------------------
void WriteCommon(FILE* pFile, const Material& mat)
{
    assert(pFile != nullptr);

    ShaderID shaderId = 0;

    // if we don't have any shader for this material use this as default 
    if (mat.shaderId == INVALID_SHADER_ID)
        shaderId = g_MaterialMgr.GetMatIdByName("LightShader");

    else
        shaderId = mat.shaderId;

  
    fprintf(pFile, "newmtl %s\n",    mat.name);
    fprintf(pFile, "mat_id %d\n",    (int)mat.id);
    fprintf(pFile, "shader_id %d\n", (int)shaderId);
    fprintf(pFile, "states %u\n",    mat.renderStates);
}

//---------------------------------------------------------
// Desc:   write ambient, diffuse, specular, reflect color values
//---------------------------------------------------------
void WriteColors(FILE* pFile, const Material& mat)
{
    assert(pFile != nullptr);

    // a - ambient, d - diffuse, s - specular, g - glossiness, r - reflect
    fprintf(pFile, "Ka %.2f %.2f %.2f %.2f\n", mat.ambient.r, mat.ambient.g, mat.ambient.b, mat.ambient.a);
    fprintf(pFile, "Kd %.2f %.2f %.2f %.2f\n", mat.diffuse.r, mat.diffuse.g, mat.diffuse.b, mat.diffuse.a);
    fprintf(pFile, "Ks %.2f %.2f %.2f\n",      mat.specular.r, mat.specular.g, mat.specular.b);
    fprintf(pFile, "Kg %.2f\n",                mat.specular.w);
    fprintf(pFile, "Kr %.2f %.2f %.2f %.2f\n", mat.reflect.r, mat.reflect.g, mat.reflect.b, mat.reflect.a);
}

//---------------------------------------------------------
// Desc:   write textures data related to this material
//---------------------------------------------------------
void WriteTextures(FILE* pFile, const Material& mat)
{
    for (int i = 0; i < NUM_TEXTURE_TYPES; ++i)
    {
        if (mat.texIds[i] == INVALID_TEXTURE_ID)
            continue;

        switch (eTexType(i))
        {
            case TEX_TYPE_DIFFUSE:           fprintf(pFile, "tex_diff "); break;
            case TEX_TYPE_SPECULAR:          fprintf(pFile, "tex_spec "); break;
            case TEX_TYPE_AMBIENT:           fprintf(pFile, "tex_amb "); break;
            case TEX_TYPE_EMISSIVE:          fprintf(pFile, "tex_emiss "); break;
            case TEX_TYPE_HEIGHT:            fprintf(pFile, "tex_height "); break;
            case TEX_TYPE_NORMALS:           fprintf(pFile, "tex_norm "); break;
            case TEX_TYPE_SHININESS:         fprintf(pFile, "tex_shin "); break;
            case TEX_TYPE_OPACITY:           fprintf(pFile, "tex_opac "); break;
            case TEX_TYPE_DISPLACEMENT:      fprintf(pFile, "tex_displ "); break;
            case TEX_TYPE_LIGHTMAP:          fprintf(pFile, "tex_light "); break;
            case TEX_TYPE_REFLECTION:        fprintf(pFile, "tex_refl "); break;
            case TEX_TYPE_BASE_COLOR:        fprintf(pFile, "tex_base "); break;
            case TEX_TYPE_NORMAL_CAMERA:     fprintf(pFile, "tex_ncam "); break;
            case TEX_TYPE_EMISSION_COLOR:    fprintf(pFile, "tex_emiss_col "); break;
            case TEX_TYPE_METALNESS:         fprintf(pFile, "tex_metal "); break;
            case TEX_TYPE_DIFFUSE_ROUGHNESS: fprintf(pFile, "tex_rough "); break;
            case TEX_TYPE_AMBIENT_OCCLUSION: fprintf(pFile, "tex_ao "); break;
            case TEX_TYPE_SHEEN:             fprintf(pFile, "tex_sheen "); break;
            case TEX_TYPE_CLEARCOAT:         fprintf(pFile, "tex_clcoat "); break;
            case TEX_TYPE_TRANSMISSION:      fprintf(pFile, "tex_trmiss "); break;

            default:
            {
                LogErr(LOG, "unknown texture type (%d) for material: %s", i, mat.name);
                continue;
            }
        }

        Texture& tex = g_TextureMgr.GetTexByID(mat.texIds[i]);
        fprintf(pFile, "%s.dds\n", tex.GetName().c_str());
    }
}

} // namespace
