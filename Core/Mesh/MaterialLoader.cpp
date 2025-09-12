#include <CoreCommon/pch.h>
#include "MaterialLoader.h"
#include "MaterialMgr.h"
#include "../Texture/TextureMgr.h"
#include <Render/CRender.h>          // from the Render module
#include <rapidjson/document.h>      // for parsing json-string/json-files
#include <rapidjson/error/en.h>      // for error handling


namespace Core
{

//---------------------------------------------------------
// forward declarations
//---------------------------------------------------------
bool ParseJson(const char* json, const long jsonSize, rapidjson::Document& doc);
void ReadColors(Material& mat, const rapidjson::Value& colors);
void ReadTextures(Material& mat, const rapidjson::Value& textures);
void ReadRenderStates(Material& mat, const rapidjson::Value& renderStates);


//---------------------------------------------------------
// Desc:   read in a content of json-file and store it into output string
// Args:   - path:      a path to file (relatively to the working directory)
//         - json:      output chars array
//         - jsonSize:  file size in bytes
//---------------------------------------------------------
void ReadMaterialsJSON(const char* path, char** json, long& jsonSize)
{
    if (!path || path[0] == '\0')
    {
        LogErr(LOG, "input path to shaders config file is empty");
        exit(0);
    }

    // open file
    FILE* pFile = fopen(path, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open shaders config file: %s", path);
        exit(0);
    }

    // define a size of the file
    fseek(pFile, 0, SEEK_END);
    jsonSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    // alloc memory for json file content
    *json = NEW char[jsonSize + 1];
    if (!(*json))
    {
        LogErr(LOG, "can't alloc memory for json string from file: %s", path);
        exit(0);
    }
    memset(*json, 0, jsonSize + 1);

    // read in a file content
    fread(*json, sizeof(char), jsonSize, pFile);
}

//---------------------------------------------------------
// Desc:   load materials from json-file by path
//---------------------------------------------------------
bool MaterialLoader::LoadFromFile(const char* path)
{
    if (!path || path[0] == '\0')
    {
        LogErr(LOG, "can't load materials: input path to file is empty");
        return false;
    }

    bool result = false;
    char* json = nullptr;
    long jsonSize = 0;

    ReadMaterialsJSON(path, &json, jsonSize);

    //---------------------------------------
    // parse json
    rapidjson::Document doc;
    if (!ParseJson(json, jsonSize, doc))
    {
        SafeDeleteArr(json);
        LogErr(LOG, "can't parse json-file content: %s", path);
        return false;
    }

    const rapidjson::Value& materials = doc["materials"];
    assert(materials.IsArray());
    const int numMaterials = (int)materials.Size();

    SetConsoleColor(CYAN);
    LogMsg("read materials from file: %s", path);
    LogMsg("num materials:            %d", numMaterials);
    SetConsoleColor(RESET);


    //---------------------------------------
    // read and create each material

    for (int i = 0; i < numMaterials; ++i)
    {
        const rapidjson::Value& matData = materials[i];

        // check some required params
        assert(matData.HasMember("name"));
        assert(matData.HasMember("shader_id"));
        assert(matData["name"].IsString());
        assert(matData["shader_id"].IsUint());


        const rapidjson::Value& name = matData["name"];
        const char* matName = name.GetString();

        // create a new material with this name
        Material& mat = g_MaterialMgr.AddMaterial(matName);

        //---------------------------------------

        mat.shaderId = matData["shader_id"].GetUint();

        if (matData.HasMember("colors"))
        {
            ReadColors(mat, matData["colors"]);
        }

        if (matData.HasMember("textures"))
        {
            ReadTextures(mat, matData["textures"]);
        }

        if (matData.HasMember("states"))
        {
            ReadRenderStates(mat, matData["states"]);
        }
    }

    SafeDeleteArr(json);
    return true;
}

//---------------------------------------------------------
// Desc:   parse input json-string
//---------------------------------------------------------
bool ParseJson(const char* json, const long jsonSize, rapidjson::Document& doc)
{
    rapidjson::ParseResult ok = doc.Parse(json);

    // check if we correctly parsed a json-string
    if (!ok) {
        const int offset = (int)ok.Offset();
        LogErr(LOG, "JSON parse error: %s (%d)", rapidjson::GetParseError_En(ok.Code()), offset);

        // find an end of a line after error
        int endOffset = 0;

        for (int i = offset; i < (int)jsonSize; i++)
        {
            if (json[i] == '\n')
            {
                endOffset = i;
                i = jsonSize;            // get out of for-loop
            }
        }

        // print this line which causes a parsing error
        int len = endOffset - offset;
        LogErr(LOG, "Error happend BEFORE a line: %.*s", len, json + offset);

        exit(0);
    }

    // do some checks
    assert(doc.IsObject());
    assert(doc.HasMember("materials"));
    assert(doc["materials"].IsArray());

    return true;
}

//---------------------------------------------------------
// Desc:   read and setup material's color properties (ambient, diffuse, etc.)
//---------------------------------------------------------
void ReadColors(Material& mat, const rapidjson::Value& colors)
{
    assert(colors.IsArray());

    for (int i = 0; i < (int)colors.Size(); ++i)
    {
        const rapidjson::Value::ConstMemberIterator iter = colors[i].MemberBegin();
        const char* type = iter->name.GetString();

        // ambient
        if (type[0] == 'a')
        {
            const rapidjson::Value& val = iter->value;
            assert(val.IsArray());
            assert(val.Size() == 4);   // RGBA color

            for (int i = 0; i < 4; ++i)
                assert(val[i].IsFloat());


            const float r = val[0].GetFloat();
            const float g = val[1].GetFloat();
            const float b = val[2].GetFloat();
            const float a = val[3].GetFloat();

            mat.SetAmbient(r, g, b, a);
        }

        // diffuse
        else if (type[0] == 'd')
        {
            const rapidjson::Value& val = iter->value;
            assert(val.IsArray());
            assert(val.Size() == 4);   // RGBA color

            for (int i = 0; i < 4; ++i)
                assert(val[i].IsFloat());


            const float r = val[0].GetFloat();
            const float g = val[1].GetFloat();
            const float b = val[2].GetFloat();
            const float a = val[3].GetFloat();

            mat.SetDiffuse(r, g, b, a);
        }

        // specular
        else if (type[0] == 's')
        {
            const rapidjson::Value& val = iter->value;
            assert(val.IsArray());
            assert(val.Size() == 3);   // RGB color

            for (int i = 0; i < 3; ++i)
                assert(val[i].IsFloat());


            const float r = val[0].GetFloat();
            const float g = val[1].GetFloat();
            const float b = val[2].GetFloat();

            mat.SetSpecular(r, g, b);
        }

        // glossiness (specular power)
        else if (type[0] == 'g')
        {
            const rapidjson::Value& val = iter->value;
            assert(val.IsFloat());
            mat.SetGlossiness(val.GetFloat());
        }

        // reflect
        else if (type[0] == 'r')
        {
            const rapidjson::Value& val = iter->value;
            assert(val.IsArray());
            assert(val.Size() == 4);   // RGBA color

            for (int i = 0; i < 4; ++i)
                assert(val[i].IsFloat());


            const float r = val[0].GetFloat();
            const float g = val[1].GetFloat();
            const float b = val[2].GetFloat();
            const float a = val[3].GetFloat();

            mat.SetReflection(r, g, b, a);
        }

        else
        {
            LogErr(LOG, "unknown color property (%s) for material (%s)", type, mat.name);
        }
    }
}

//---------------------------------------------------------
// Desc:    read and setup textures for this material
//---------------------------------------------------------
void ReadTextures(Material& mat, const rapidjson::Value& textures)
{
    assert(textures.IsArray());

    for (int i = 0; i < (int)textures.Size(); ++i)
    {
        const rapidjson::Value::ConstMemberIterator iter = textures[i].MemberBegin();

        const char* texType = iter->name.GetString();
        const char* texPath = iter->value.GetString();


        // load texture from file
        const TexID texId = g_TextureMgr.LoadFromFile(g_RelPathTexDir, texPath);


        if (strcmp(texType, "diff") == 0)
            mat.SetTexture(TEX_TYPE_DIFFUSE, texId);

        else if (strcmp(texType, "norm") == 0)
            mat.SetTexture(TEX_TYPE_NORMALS, texId);
    }
}

//---------------------------------------------------------
// Desc:    read and setup render states for this material
//---------------------------------------------------------
void ReadRenderStates(Material& mat, const rapidjson::Value& renderStates)
{
    assert(renderStates.IsArray());

    for (int i = 0; i < (int)renderStates.Size(); ++i)
    {
        const rapidjson::Value::ConstMemberIterator iter = renderStates[i].MemberBegin();
        const char* key = iter->name.GetString();


        //---------------------------------------
        // render state: alpha clipping
        if (strcmp(key, "alpha_clip") == 0)
        {
            const bool value = iter->value.GetBool();
            mat.SetAlphaClip(value);
        }

        //---------------------------------------
        // render state: fill mode
        else if (strcmp(key, "fill") == 0)
        {
            const char* value = iter->value.GetString();

            // fill: solid
            if (value[0] == 's')
                mat.SetFill(MAT_PROP_FILL_SOLID);

            // fill: wireframe
            else if (value[0] == 'w')
                mat.SetFill(MAT_PROP_FILL_WIREFRAME);

            else
                LogErr(LOG, "can't set fill mode (%s) for material: %s", value, mat.name);
        }

        //---------------------------------------
        // render state: cull mode
        else if (strcmp(key, "cull") == 0)
        {
            const char* value = iter->value.GetString();

            // cull: front
            if (value[0] == 'b')
                mat.SetCull(MAT_PROP_CULL_BACK);

            else if (value[0] == 'f')
                mat.SetCull(MAT_PROP_CULL_FRONT);

            else if (value[0] == 'n')
                mat.SetCull(MAT_PROP_CULL_NONE);

            else
                LogErr(LOG, "can't set cull mode (%s) for material: %s", value, mat.name);
        }

        //---------------------------------------
        // render state: blending
        else if (strcmp(key, "blend") == 0)
        {
            const char* value = iter->value.GetString();
            
            switch (value[0])
            {
                // add or alpha_to_coverage
                case 'a':
                {
                    if (strcmp(value, "add") == 0)
                        mat.SetBlending(MAT_PROP_BS_ADD);

                    else if (strcmp(value, "alpha_to_coverage") == 0)
                        mat.SetBlending(MAT_PROP_BS_ALPHA_TO_COVERAGE);

                    else
                        LogErr(LOG, "can't set a blend state (%s) for material: %s", value, mat.name);

                    break;
                }

                // disable
                case 'd':   mat.SetBlending(MAT_PROP_BS_DISABLE);                 break;

                // enable
                case 'e':   mat.SetBlending(MAT_PROP_BS_ENABLE);                  break;

                // mul
                case 'm':   mat.SetBlending(MAT_PROP_BS_MUL);                     break;

                // no_render_target_writes
                case 'n':   mat.SetBlending(MAT_PROP_BS_NO_RENDER_TARGET_WRITES); break;

                // sub
                case 's':   mat.SetBlending(MAT_PROP_BS_SUB);                     break;

                // transparent
                case 't':   mat.SetBlending(MAT_PROP_BS_TRANSPARENCY);            break;

                default:
                    LogErr(LOG, "can't set a blend state (%s) for material: %s", value, mat.name);
            }
        }

        //---------------------------------------
        // render state: depth_stencil
        else if (strcmp(key, "depth_stencil") == 0)
        {
            const char* value = iter->value.GetString();

            // mark_mirror
            if (value[0] == 'm')
                mat.SetDepthStencil(MAT_PROP_DSS_MARK_MIRROR);

            else
                LogErr(LOG, "can't set depth stencil state (%s) for material: %s", value, mat.name);
        }
    }
}


} // namespace
