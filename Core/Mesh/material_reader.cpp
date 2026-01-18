/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: material_reader.cpp

    Created:  19.10.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "material_reader.h"
#include "material_mgr.h"
#include "material.h"
#include <Texture/texture_mgr.h>
#include <Render/CRender.h>          // include it here because we need to get ShaderID by name 

namespace Core
{

//---------------------------------------------------------
// helpers forward declaration
//---------------------------------------------------------
void CreateNewMat           (const char* buf, Material** ppMat, FILE* pFile);
void ReadCommon             (FILE* pFile,     Material* pMat);
void ReadColor              (const char* buf, Material* pMat);
void ReadTexture            (const char* buf, Material* pMat, const char* targetDir);
void ReadRasterState        (const char* buf, Material* pMat);
void ReadBlendState         (const char* buf, Material* pMat);
void ReadDepthStencilState  (const char* buf, Material* pMat);

//---------------------------------------------------------
// Desc:   read data for multiple materials from a file
//         and add them into the material manager
//---------------------------------------------------------
bool MaterialReader::Read(const char* filePath)
{
    // check input args
    if (!filePath || (filePath[0] == '\0'))
    {
        LogErr(LOG, "input filepath is empty");
        return false;
    }


    FILE* pFile = fopen(filePath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", filePath);
        return false;
    }


    constexpr int bufSize = 128;
    char buf[bufSize]{ '\0' };
    char targetDir[256]{ '\0' };
    Material* pMat = nullptr;
    FileSys::GetParentPath(filePath, targetDir);

    while (fgets(buf, bufSize, pFile))
    {
        switch (buf[0])
        {
            // alpha clip
            case 'a':
                if (pMat)
                {
                    int alphaClip = 0;
                    int count = sscanf(buf, "alpha_clip %d\n", &alphaClip);
                    assert(count = 1);

                    pMat->SetAlphaClip((bool)alphaClip);
                }
                break;

            // newmtl
            case 'n':
                CreateNewMat(buf, &pMat, pFile);
                break;

            case 'b':
                ReadBlendState(buf, pMat);
                break;

            case 'd':
                ReadDepthStencilState(buf, pMat);
                break;

            case 'K':
                ReadColor(buf, pMat);
                break;

            case 'r':
                ReadRasterState(buf, pMat);
                break;

            case 't':
                ReadTexture(buf, pMat, targetDir);
                break;
        }
    }

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:   create and setup a material with basic params
//---------------------------------------------------------
void CreateNewMat(const char* buf, Material** ppMat, FILE* pFile)
{
    assert(buf && buf[0] != '\0');
    assert(ppMat);
    assert(pFile);

    char matName[MAX_LEN_MAT_NAME]{ '\0' };
    int count = sscanf(buf, "newmtl %s\n", matName);
    assert(count == 1);

    *ppMat = &g_MaterialMgr.AddMaterial(matName);
    ReadCommon(pFile, *ppMat);
}

//---------------------------------------------------------
// Desc:   read basic info: material id, shader id, render states, etc
//---------------------------------------------------------
void ReadCommon(FILE* pFile, Material* pMat)
{
    assert(pFile);
    assert(pMat);

    char shaderName[MAX_LEN_SHADER_NAME];
    int count = fscanf(pFile, "\nshader %s\n", shaderName);
    assert(count == 1);

    fscanf(pFile, "\n");

    pMat->shaderId = Render::g_Render.GetShaderIdByName(shaderName);
}

//---------------------------------------------------------
// Desc:   read a single color property for input material
//         (ambient, diffuse, specular, etc.)
//---------------------------------------------------------
void ReadColor(const char* buf, Material* pMat)
{
    assert(buf && buf[0] == 'K');
    assert(pMat);

    Material& mat = *pMat;
    int count = 0;

    switch (buf[1])
    {
        // ambient
        case 'a':
            count = sscanf(buf, "Ka %f %f %f %f\n", &mat.ambient.r, &mat.ambient.g, &mat.ambient.b, &mat.ambient.a);
            assert(count == 4);
            break;

        // diffuse
        case 'd':
            count = sscanf(buf, "Kd %f %f %f %f\n", &mat.diffuse.r, &mat.diffuse.g, &mat.diffuse.b, &mat.diffuse.a);
            assert(count == 4);
            break;

        // specular
        case 's':
            count = sscanf(buf, "Ks %f %f %f\n", &mat.specular.r, &mat.specular.g, &mat.specular.b);
            assert(count == 3);
            break;

        // glossiness
        case 'g':
            count = sscanf(buf, "Kg %f\n", &mat.specular.w);
            assert(count == 1);
            break;

        // reflect
        case 'r':
            count = sscanf(buf, "Kr %f %f %f %f\n", &mat.reflect.r, &mat.reflect.g, &mat.reflect.b, &mat.reflect.a);
            assert(count == 4);
            break;

        default:
            LogErr(LOG, "uknown material color type: %c", buf[1]);
    }
}

//---------------------------------------------------------
// Desc:  return a texture type code by input string
//---------------------------------------------------------
eTexType GetTexType(const char* texType)
{
    if (strcmp(texType, "diff") == 0)
        return TEX_TYPE_DIFFUSE;

    else if (strcmp(texType, "spec") == 0)
        return TEX_TYPE_SPECULAR;

    else if (strcmp(texType, "amb") == 0)
        return TEX_TYPE_AMBIENT;

    else if (strcmp(texType, "emiss") == 0)
        return TEX_TYPE_EMISSIVE;

    else if (strcmp(texType, "height") == 0)
        return TEX_TYPE_HEIGHT;

    else if (strcmp(texType, "norm") == 0)
        return TEX_TYPE_NORMALS;

    else if (strcmp(texType, "shin") == 0)
        return TEX_TYPE_SHININESS;

    else if (strcmp(texType, "opac") == 0)
        return TEX_TYPE_OPACITY;

    else if (strcmp(texType, "displ") == 0)
        return TEX_TYPE_DISPLACEMENT;

    else if (strcmp(texType, "light") == 0)
        return TEX_TYPE_LIGHTMAP;

    else if (strcmp(texType, "refl") == 0)
        return TEX_TYPE_REFLECTION;

    else if (strcmp(texType, "base") == 0)
        return TEX_TYPE_BASE_COLOR;

    else if (strcmp(texType, "ncam") == 0)
        return TEX_TYPE_NORMAL_CAMERA;

    else if (strcmp(texType, "emiss_col") == 0)
        return TEX_TYPE_EMISSION_COLOR;

    else if (strcmp(texType, "metal") == 0)
        return TEX_TYPE_METALNESS;

    else if (strcmp(texType, "rough") == 0)
        return TEX_TYPE_DIFFUSE_ROUGHNESS;

    else if (strcmp(texType, "ao") == 0)
        return TEX_TYPE_AMBIENT_OCCLUSION;

    else if (strcmp(texType, "sheen") == 0)
        return TEX_TYPE_SHEEN;

    else if (strcmp(texType, "clcoat") == 0)
        return TEX_TYPE_CLEARCOAT;

    else if (strcmp(texType, "trmiss") == 0)
        return TEX_TYPE_TRANSMISSION;

    return TEX_TYPE_NONE;
}

//---------------------------------------------------------
// Desc:   load a single texture from file and bind it to input material
//---------------------------------------------------------
void ReadTexture(const char* buf, Material* pMat, const char* targetDir)
{
    assert(buf && buf[0] != '\0');
    assert(pMat);
    assert(targetDir && targetDir[0] != '\0');

    char texType[16]{'\0'};
    char texName[MAX_LEN_TEX_NAME]{'\0'};
    
    int count = sscanf(buf, "tex_%s %s\n", texType, texName);
    assert(count == 2);

    const TexID texId = g_TextureMgr.GetTexIdByName(texName);
    if (texId == INVALID_TEX_ID)
        LogErr(LOG, "no texture by name: %s", texName);

    eTexType type = GetTexType(texType);
    pMat->texIds[type] = texId;
}

//---------------------------------------------------------
// Desc:  read in a rasterizer state for this material
//---------------------------------------------------------
void ReadRasterState(const char* buf, Material* pMat)
{
    assert(buf && buf[0] != '\0');
    assert(pMat);

    char rsName[32]{ '\0' };
    int count = sscanf(buf, "rs %s\n", rsName);
    assert(count == 1);

    pMat->rsId = Render::g_Render.GetRenderStates().GetRsId(rsName);
}

//---------------------------------------------------------
// Desc:   read in a blend state for this material
//---------------------------------------------------------
void ReadBlendState(const char* buf, Material* pMat)
{
    assert(buf && (buf[0] != '\0'));
    assert(pMat);

    char bsName[32]{'\0'};

    int count = sscanf(buf, "bs %s\n", bsName);
    assert(count == 1);

    pMat->bsId = Render::g_Render.GetRenderStates().GetBsId(bsName);
}

//---------------------------------------------------------
// Desc:   read in depth stencil state for this material
//---------------------------------------------------------
void ReadDepthStencilState(const char* buf, Material* pMat)
{
    assert(buf && buf[0] != '\0');
    assert(pMat);

    char dssName[32]{ '\0' };
    int count = sscanf(buf, "dss %s\n", dssName);
    assert(count == 1);

    pMat->dssId = Render::g_Render.GetRenderStates().GetDssId(dssName);
}

} // namespace
