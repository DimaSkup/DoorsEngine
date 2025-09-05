#include "../Common/pch.h"
#include "ShaderMgr.h"
#include "Shader.h"
#include <rapidjson/document.h>      // for parsing json-string
#include <rapidjson/error/en.h>      // for error handling

namespace Render
{

ShaderMgr::ShaderMgr()
{
}

ShaderMgr::~ShaderMgr()
{
    for (auto& it : idToShader_)
    {
        Shader* pShader = it.second;
        SafeDelete(pShader);
    }

    idToShader_.clear();
}

//---------------------------------------------------------
// Desc:   read in a content of json-file and store it into output string
// Args:   - path:      a path to file (relatively to the working directory)
//         - json:      output chars array
//         - jsonSize:  file size in bytes
//---------------------------------------------------------
void ReadShadersJSON(const char* path, char** json, long& jsonSize)
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
// Desc:   initialize all the shader classes (color, texture, light, etc.)
// Args:   - pDevice:  a ptr to DirectX11 device
//         - pContext: a ptr to DirectX11 device context
//         - WVO:      world * base_view * ortho matrix
//---------------------------------------------------------
bool ShaderMgr::Init(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX& WVO)
{
    SetConsoleColor(YELLOW);
    LogMsg("\n");
    LogMsg("---------------------------------------------------------");
    LogMsg("                INITIALIZATION: SHADERS                  ");
    LogMsg("---------------------------------------------------------");
    LogDbg(LOG, "shaders initialization: start");

    try
    {
        bool result = false;


        // read in shaders declarations from json-config file
        char* json = nullptr;
        long jsonSize = 0;
        ReadShadersJSON("shaders/shaders.json", &json, jsonSize);


        // parse json
        rapidjson::Document doc;
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

            exit(EXIT_FAILURE);
        }

        // do some checks
        assert(doc.IsObject());
        assert(doc.HasMember("shaders"));
        assert(doc["shaders"].IsArray());


        // read in all the shaders definitions
        const rapidjson::Value& shadersData = doc["shaders"];
        
        for (int i = 0; i < (int)shadersData.Size(); ++i)
        {
            const rapidjson::Value& shaderData = shadersData[i];

            // do some checks again
            assert(shaderData.HasMember("name"));
            assert(shaderData.HasMember("vs"));
            assert(shaderData.HasMember("ps"));
            assert(shaderData.HasMember("shader_model"));
            assert(shaderData.HasMember("input_layout"));

            const rapidjson::Value& name        = shaderData["name"];
            const rapidjson::Value& vs          = shaderData["vs"];
            const rapidjson::Value& ps          = shaderData["ps"];
            const rapidjson::Value& shaderModel = shaderData["shader_model"];
            const rapidjson::Value& inputLayout = shaderData["input_layout"];

            assert(name.IsString());
            assert(vs.IsString());
            assert(ps.IsString());
            assert(shaderModel.IsString());
            assert(inputLayout.IsString());

        
#if 1
            printf("name: %s\n",           name.GetString());
            printf("vs: %s\n",             vs.GetString());
            printf("ps: %s\n",             ps.GetString());
            printf("shader_model: %s\n",   shaderModel.GetString());
            printf("input_layout: %s\n\n", inputLayout.GetString());
#endif

            // setup name
            ShaderInitParams initParams;
            strncpy(initParams.name, name.GetString(), 32);

            // setup paths to cso/hlsl shader files
            strncpy(initParams.vsPath, vs.GetString(), 64);
            strncpy(initParams.psPath, ps.GetString(), 64);

            // handle geometry shader "gs" in a separate way
            if (shaderData.HasMember("gs"))
            {
                const rapidjson::Value& gs = shaderData["gs"];
                assert(gs.IsString());
                strncpy(initParams.gsPath, gs.GetString(), 64);
            }

            // setup input layout
            const char* inputLayoutName                = inputLayout.GetString();
            const VertexInputLayout& vertexInputLayout = inputLayoutsMgr_.GetInputLayoutByName(inputLayoutName);
            const UINT numElems                        = (UINT)vertexInputLayout.desc.size();
            initParams.inputLayoutNumElems             = numElems;

            memcpy(initParams.inputLayoutDesc, vertexInputLayout.desc.data(), sizeof(D3D11_INPUT_ELEMENT_DESC) * numElems);


            // add and init a new shader
            result = AddShader(pDevice, &initParams);
            if (!result)
            {
                LogErr(LOG, "can't init a shader class: %s", name.GetString());
            }
        }

        SafeDeleteArr(json);


        LogDbg(LOG, "shaders initialization: finished successfully");
        LogMsg("%s---------------------------------------------------------%s\n", YELLOW, RESET);
    }
    catch (EngineException& e) 
    {
        LogErr(e, true);
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   add and init a new shader
// Args:   - pDevice:      a ptr to DirectX11 device
//         - pInitParams:  a ptr to container with initial params for shader
//---------------------------------------------------------
bool ShaderMgr::AddShader(ID3D11Device* pDevice, const ShaderInitParams* pInitParams)
{
    Shader* pShader = NEW Shader(pDevice, *pInitParams);
    if (!pShader)
    {
        LogErr(LOG, "can't add a new shader");
        return false;
    }

    // generate an ID for this shader
    ShaderID id = lastShaderId_;
    lastShaderId_++;

    idToShader_.insert({ id, pShader });

    return true;
}

//---------------------------------------------------------
// Desc:  bind only one vertex buffer to input assembler (IA) stage
// Args:  - pVB:     a ptr to vertex buffer
//        - stride:  vertex size
//        - offset:  where start to get vertices from the buffer
//---------------------------------------------------------
void ShaderMgr::BindVertexBuffer(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pVB,
    const UINT stride,
    const UINT offset)
{
    pContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
}

//---------------------------------------------------------
// Desc:  bind vertex, index (and maybe instances) buffers to input assember (IA) stage
// Args:  - vbs:       arr of ptrs to vertex and (optional) instances buffers
//        - pIB:       a ptr to index buffer
//        - pStrides:  arr of strides for each element of each buffer from vbs
//        - pOffsets:  arr of offsets for each buffer from vbs
//        - numVBs:    how many elements we have in vbs
//---------------------------------------------------------
void ShaderMgr::BindBuffers(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* const* vbs,
    ID3D11Buffer* pIB,
    const UINT* strides,
    const UINT* offsets,
    int numVBs)
{
    assert(vbs != nullptr);
    assert(pIB != nullptr);
    assert(strides != nullptr);
    assert(offsets != nullptr);
    assert(numVBs > 0);
    //pContext->IASetVertexBuffers(0, 1, &sky.pVB, &sky.vertexStride, &offset);
      //pContext->IASetIndexBuffer(sky.pIB, DXGI_FORMAT_R16_UINT, 0);
    pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    pContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R16_UINT, 0);
}

//---------------------------------------------------------
// Desc:   bind input shader so we will use it for rendering
//---------------------------------------------------------
void ShaderMgr::BindShader(ID3D11DeviceContext* pContext, Shader* pShader)
{
    pContext->IASetInputLayout(pShader->GetVS()->GetInputLayout());
    pContext->VSSetShader(pShader->GetVS()->GetShader(), nullptr, 0);
    pContext->PSSetShader(pShader->GetPS()->GetShader(), nullptr, 0);

    // if we have a geometry shader we bind it
    if (pShader->GetGS())
    {
        pContext->GSSetShader(pShader->GetGS()->GetShader(), nullptr, 0);
    }

    // unbind any geometry shaders for sure
    else
    {
        pContext->GSSetShader(nullptr, nullptr, 0);
    }
}

//---------------------------------------------------------
// Desc:  draw indexed vertices
// NOTE:  all the neccessary preparations must be done before it
//        - materials must be bound (setup render states + bound textures)
//        - InputAssembler stage must be configured (bind buffers, input layouts, etc)
//        - all the shaders must be bound
//---------------------------------------------------------
void ShaderMgr::Render(ID3D11DeviceContext* pContext, const UINT indexCount)
{
    pContext->DrawIndexed(indexCount, 0, 0);
}

//---------------------------------------------------------
// Desc:   render instances starting from startInstanceLocation
//         in the instances buffer
//---------------------------------------------------------
void ShaderMgr::Render(
    ID3D11DeviceContext* pContext,
    const InstanceBatch& instances,
    const UINT startInstanceLocation)
{
    const Subset& subset = instances.subset;

    pContext->DrawIndexedInstanced(
        subset.indexCount,
        instances.numInstances,
        subset.indexStart,
        subset.vertexStart,
        startInstanceLocation);
}

//---------------------------------------------------------
// Desc:   return a ptr to shader by input ID
//         or nullptr if there is no such a shader
//---------------------------------------------------------
Shader* ShaderMgr::GetShaderById(const ShaderID id)
{
    if (idToShader_.contains(id))
        return idToShader_[id];

    else
        return nullptr;
}

//---------------------------------------------------------
// Desc:   return a ptr to shader by input name
//         or nullptr if there is no such a shader
//---------------------------------------------------------
Shader* ShaderMgr::GetShaderByName(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input shader name is empty, so return nullptr");
        return nullptr;
    }

    for (auto& it : idToShader_)
    {
        Shader* pShader = it.second;

        // if name matches we return a ptr to this shader
        if (strcmp(pShader->GetName(), name) == 0)
            return pShader;
    }

    LogErr(LOG, "there is no shader by name: %s\n so return nullptr", name);
    return nullptr;
}

//---------------------------------------------------------
// Desc:  return a shader id by input name
//---------------------------------------------------------
ShaderID ShaderMgr::GetShaderIdByName(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input shader name is empty, so return id == 0");
        return 0;
    }

    for (auto& it : idToShader_)
    {
        Shader* pShader = it.second;

        // if name matches we return an ID of this shader
        if (strcmp(pShader->GetName(), name) == 0)
            return it.first;
    }

    LogErr(LOG, "there is no shader by name: %s\n so return id == 0", name);
    return 0;
}

//---------------------------------------------------------
// Desc:   return a shader name by input id
//---------------------------------------------------------
const char* ShaderMgr::GetShaderNameById(const ShaderID id)
{
    if (idToShader_.contains(id))
        return idToShader_[id]->GetName();

    LogErr(LOG, "there is no shader by id: %" PRIu32 "\n so return name == nullptr", id);
    return nullptr;
}

} // namespace
