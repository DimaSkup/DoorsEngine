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


namespace Core
{

//---------------------------------------------------------
// helpers forward declaration
//---------------------------------------------------------
void CreateNewMat           (const char* buf, FILE* pFile, Material** ppMat);
void ReadCommon             (FILE* pFile, Material& mat);
void ReadColor              (const char* buf, Material& mat);
void ReadTexture            (const char* buf, Material& mat, const char* targetDir);
void ReadFillMode           (const char* buf, Material& mat);
void ReadCullMode           (const char* buf, Material& mat);
void ReadBlendState         (const char* buf, Material& mat);
void ReadDepthStencilState  (const char* buf, Material& mat);

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
                CreateNewMat(buf, pFile, &pMat);
                break;

            case 'b':
                ReadBlendState(buf, *pMat);
                break;

            case 'c':
                ReadCullMode(buf, *pMat);
                break;

            case 'd':
                ReadDepthStencilState(buf, *pMat);
                break;

            case 'f':
                ReadFillMode(buf, *pMat);
                break;

            case 'K':
                ReadColor(buf, *pMat);
                break;

            case 't':
                ReadTexture(buf, *pMat, targetDir);
                break;
        }
    }

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:   create and setup a material with basic params
//---------------------------------------------------------
void CreateNewMat(const char* buf, FILE* pFile, Material** ppMat)
{
    assert(buf && buf[0] != '\0');
    assert(pFile != nullptr);

    char matName[MAX_LEN_MAT_NAME]{ '\0' };
    int count = sscanf(buf, "newmtl %s\n", matName);
    assert(count == 1);

    *ppMat = &g_MaterialMgr.AddMaterial(matName);
    ReadCommon(pFile, **ppMat);
}

//---------------------------------------------------------
// Desc:   read basic info: material id, shader id, render states, etc
//---------------------------------------------------------
void ReadCommon(FILE* pFile, Material& mat)
{
    assert(pFile != nullptr);

    MaterialID tempMatId = 0;
    int count = 0;

    count = fscanf(pFile, "mat_id %" SCNu32, &tempMatId);
    assert(count != EOF);

    count = fscanf(pFile, "\nshader_id %" SCNu32, &mat.shaderId);
    assert(count != EOF);

    count = fscanf(pFile, "\nstates %" SCNu32, &mat.renderStates);
    assert(count != EOF);

    fscanf(pFile, "\n");


    if (mat.renderStates == 0)
        mat.renderStates = MAT_PROP_DEFAULT;
}

//---------------------------------------------------------
// Desc:   read a single color property for input material
//         (ambient, diffuse, specular, etc.)
//---------------------------------------------------------
void ReadColor(const char* buf, Material& mat)
{
    assert(buf != nullptr);
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
void ReadTexture(const char* buf, Material& mat, const char* targetDir)
{
    assert(buf && (buf[0] != '\0'));
    assert(targetDir && (targetDir[0] != '\0'));

    char texType[16]{'\0'};
    char texFileName[128]{'\0'};
    char texFilePath[256]{'\0'};
    
    int count = sscanf(buf, "tex_%s %s\n", texType, texFileName);
    assert(count == 2);

    strcat(texFilePath, targetDir);
    strcat(texFilePath, texFileName);

    const TexID texId = g_TextureMgr.LoadFromFile(texFilePath);

    eTexType type = GetTexType(texType);
    mat.texIds[type] = texId;
}

//---------------------------------------------------------
// Desc:   read in a fill mode for this material
//---------------------------------------------------------
void ReadFillMode(const char* buf, Material& mat)
{
    assert(buf && (buf[0] != '\0'));
    char fill[8]{'\0'};
    
    int count = sscanf(buf, "fill %s\n", fill);
    assert(count == 1);


    if (fill[0] == 's')
        mat.SetFill(MAT_PROP_FILL_SOLID);

    else if (fill[0] == 'w')
        mat.SetFill(MAT_PROP_FILL_WIREFRAME);

    else
        LogErr(LOG, "unknown type of fill mode (%s) for material (%s)", fill, mat.name);
}

//---------------------------------------------------------
// Desc:   read in a cull mode for this material
//---------------------------------------------------------
void ReadCullMode(const char* buf, Material& mat)
{
    assert(buf && (buf[0] != '\0'));
    char cull[8]{'\0'};

    int count = sscanf(buf, "cull %s\n", cull);
    assert(count == 1);

    if (cull[0] == 'b')
        mat.SetCull(MAT_PROP_CULL_BACK);

    else if (cull[0] == 'f')
        mat.SetCull(MAT_PROP_CULL_FRONT);

    else if (cull[0] == 'n')
        mat.SetCull(MAT_PROP_CULL_NONE);

    else
        LogErr(LOG, "unknown type of cull mode (%s) for material (%s)", cull, mat.name);
}

//---------------------------------------------------------
// Desc:   read in a blend state for this material
//---------------------------------------------------------
void ReadBlendState(const char* buf, Material& mat)
{
    assert(buf && (buf[0] != '\0'));
    char bs[32]{'\0'};

    int count = sscanf(buf, "bs %s\n", bs);
    assert(count == 1);

    switch (bs[0])
    {
        case 'a':
        {
            if (strcmp(bs, "add") == 0)
                mat.SetBlending(MAT_PROP_BS_ADD);

            else if (strcmp(bs, "alpha_to_coverage") == 0)
                mat.SetBlending(MAT_PROP_BS_ALPHA_TO_COVERAGE);

            break;
        }

        case 'd':
            mat.SetBlending(MAT_PROP_BS_DISABLE);
            break;

        case 'e':
            mat.SetBlending(MAT_PROP_BS_ENABLE);
            break;

        case 'm':
            mat.SetBlending(MAT_PROP_BS_MUL);
            break;

        case 'n':
            mat.SetBlending(MAT_PROP_BS_NO_RENDER_TARGET_WRITES);
            break;

        case 's':
            mat.SetBlending(MAT_PROP_BS_SUB);
            break;

        case 't':
            mat.SetBlending(MAT_PROP_BS_TRANSPARENCY);
            break;

        default:
            LogErr(LOG, "unknown type of blend state (%s) for material (%s)", bs, mat.name);
    }
}

//---------------------------------------------------------
// Desc:   read in depth stencil state for this material
//---------------------------------------------------------
void ReadDepthStencilState(const char* buf, Material& mat)
{
    assert(buf && (buf[0] != '\0'));
    char dss[32]{ '\0' };

    int count = sscanf(buf, "dss %s\n", dss);
    assert(count == 1);

    switch (dss[0])
    {
         case 'd':
            mat.SetDepthStencil(MAT_PROP_DSS_DEPTH_DISABLED);
            break;

        case 'e':
            mat.SetDepthStencil(MAT_PROP_DSS_DEPTH_ENABLED);
            break;

        case 'm':
            mat.SetDepthStencil(MAT_PROP_DSS_MARK_MIRROR);
            break;

        case 'n':
            mat.SetDepthStencil(MAT_PROP_DSS_NO_DOUBLE_BLEND);
            break;

        case 'r':
            mat.SetDepthStencil(MAT_PROP_DSS_DRAW_REFLECTION);
            break;

        case 's':
            mat.SetDepthStencil(MAT_PROP_DSS_SKY_DOME);
            break;

        default:
            LogErr(LOG, "unknown type of depth-stencil state (%s) for material (%s)", dss, mat.name);
    }
}

} // namespace
