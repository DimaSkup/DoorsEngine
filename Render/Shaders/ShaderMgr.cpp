#include "../Common/pch.h"
#include "ShaderMgr.h"
#include "Shader.h"
#include <rapidjson/document.h>      // for parsing json-string
#include <rapidjson/error/en.h>      // for error handling
#include <Shlwapi.h>

namespace Render
{
//---------------------------------------------------------
// Desc:   a container for shaders common params
//         (is used during initialization or hot reload)
//---------------------------------------------------------
struct ShaderCommonParams
{
    bool runtimeCompilation = false;
    char dirPathShaders[32]{ '\0' };
    char shaderFileExt[8]{ '\0' };
    char shaderModel[4]{ '\0' };
};

//---------------------------------------------------------
// Forward declaration of initialization helpers
//---------------------------------------------------------
bool ShaderDirExists(const char* dirPath)
{
    if (!dirPath || dirPath[0] == '\0')
    {
        LogErr(LOG, "input path to shaders directory is empty!");
        return false;
    }

    DWORD ftyp = GetFileAttributesA(dirPath);

    if (ftyp == INVALID_FILE_ATTRIBUTES)
    {
        LogErr(LOG, "something is wrong with this path: %s", dirPath);
        return false;
    }

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;

    return false;
}

void ReadJsonIntoBuffer              (const char* path, char** json, long& jsonSize);
void ParseJson                       (rapidjson::Document& doc, const char* json, const int jsonSize);

void ReadShadersInitCommonParams     (const rapidjson::Value& rawData, ShaderCommonParams& outParams);
void ReadShadersHotReloadCommonParams(const rapidjson::Value& rawData, ShaderCommonParams& outParams);
void PrepareShaderInitParams         (const rapidjson::Value& rawData, const ShaderCommonParams& commonParams, const VertexInputLayoutMgr& mgr, ShaderInitParams& outParams);
void PrepareShaderHotReloadParams    (const rapidjson::Value& rawData, const ShaderCommonParams& commonParams, const VertexInputLayoutMgr& mgr, ShaderInitParams& outParams);

//---------------------------------------------------------
// Desc:   just a constructor and destructor
//---------------------------------------------------------
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
// Desc:   initialize all the shader classes (color, texture, light, etc.)
// Args:   - pDevice:                 a ptr to DirectX11 device
//         - pContext:                a ptr to DirectX11 device context
//         - WVO:                     world * base_view * ortho matrix
//         - forceRuntimeCompilation: (optional) is used when hot reload
// Ret:    true if everything is ok
//---------------------------------------------------------
bool ShaderMgr::Init(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX& WVO,
    const bool forceRuntimeCompilation)
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
        ReadJsonIntoBuffer("data/shaders/shaders.json", &json, jsonSize);

        // parse json
        rapidjson::Document doc;
        ParseJson(doc, json, jsonSize);
       
        // do some checks
        assert(doc.IsObject());
        assert(doc.HasMember("params"));
        assert(doc.HasMember("shaders"));
        assert(doc["params"].IsObject());
        assert(doc["shaders"].IsArray());


        ShaderCommonParams commonParams;
        ReadShadersInitCommonParams(doc["params"], commonParams);


        // read in all the shaders definitions
        const rapidjson::Value& shadersData = doc["shaders"];
        
        for (int i = 0; i < (int)shadersData.Size(); ++i)
        {
            ShaderInitParams initParams;
            PrepareShaderInitParams(shadersData[i], commonParams, inputLayoutsMgr_, initParams);

            // if there is already a shader with such ID or it is invalid
            const bool mgrHasId  = idToShader_.contains(initParams.shaderId);
            const bool idIsValid = (size_t)initParams.shaderId >= idToShader_.size();

            if (mgrHasId || !idIsValid)
            {
                LogErr(LOG, "there is already a shader with such id: %" PRIu32, initParams.shaderId);
                initParams.shaderId = INVALID_SHADER_ID;
            }
           
            // add and init a new shader
            result = AddShader(pDevice, &initParams);
            if (!result)
            {
                LogErr(LOG, "can't init a shader class: %s", initParams.name);
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
// Desc:   shaders hot reload
//---------------------------------------------------------
bool ShaderMgr::HotReload(ID3D11Device* pDevice)
{
    try
    {
        bool result = false;

        // read in shaders declarations from json-config file
        char* json = nullptr;
        long jsonSize = 0;
        ReadJsonIntoBuffer("data/shaders/shaders.json", &json, jsonSize);

        // parse json
        rapidjson::Document doc;
        ParseJson(doc, json, jsonSize);

        // do some checks
        assert(doc.IsObject());
        assert(doc.HasMember("params"));
        assert(doc.HasMember("shaders"));
        assert(doc["params"].IsObject());
        assert(doc["shaders"].IsArray());


        ShaderCommonParams commonParams;
        ReadShadersHotReloadCommonParams(doc["params"], commonParams);

        // check if there is such a directory of shaders 
        const char* dirPath = commonParams.dirPathShaders;

        if (!ShaderDirExists(dirPath))
        {
            LogErr(LOG, "can't hot reload shaders since there is no directory for shaders: %s",
                        (dirPath) ? dirPath : "unknown_dir_path");

            SafeDeleteArr(json);
            return false;
        }

        // read in all the shaders definitions
        const rapidjson::Value& shadersData = doc["shaders"];

        for (int i = 0; i < (int)shadersData.Size(); ++i)
        {
            ShaderInitParams hotReloadParams;
            PrepareShaderHotReloadParams(shadersData[i], commonParams, inputLayoutsMgr_, hotReloadParams);

            ShaderID shaderId = (ShaderID)hotReloadParams.shaderId;
            Shader* pShader   = GetShaderById(shaderId);

            // there is no shader by such ID
            if (!pShader)
            {
                const rapidjson::Value& data = shadersData[i];
                const char* fmt = "there is no shader by id: %s (its name: %s)";

                if (data.HasMember("name"))
                    LogErr(LOG, fmt, shaderId, data["name"].GetString());

                else
                    LogErr(LOG, fmt, shaderId, "unknown_shader");

                continue;
            }

            // try to hot reload this shader
            if (!pShader->HotReload(pDevice, hotReloadParams))
            {
                LogErr(LOG, "can't hot reload a shader: %s", pShader->GetName());
                continue;
            }
        }

        SafeDeleteArr(json);

        LogDbg(LOG, "shaders hot reload: finished successfully");
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
    if (!pInitParams)
    {
        LogErr(LOG, "can't add a new shader: input parameters container == nullptr");
        return false;
    }

    Shader* pShader = NEW Shader(pDevice, *pInitParams);
    if (!pShader)
    {
        LogErr(LOG, "can't add a new shader: %s", pInitParams->name);
        return false;
    }

    idToShader_.insert({ (ShaderID)pInitParams->shaderId, pShader });

    return true;
}

//---------------------------------------------------------
// Desc:  bind only one vertex buffer to input assembler (IA) stage
// Args:  - pVB:     a ptr to vertex buffer
//        - stride:  vertex size
//        - offset:  where start to get vertices from the buffer
//---------------------------------------------------------
void ShaderMgr::BindVB(
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

    pContext->IASetVertexBuffers(0, (UINT)numVBs, vbs, strides, offsets);
    pContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R16_UINT, 0);
}

//---------------------------------------------------------
// Desc:   bind input shader so we will use it for rendering
//---------------------------------------------------------
void ShaderMgr::BindShader(ID3D11DeviceContext* pContext, Shader* pShader)
{
    if (pShader)
    {
        pContext->VSSetShader     (pShader->GetVS()->GetShader(), nullptr, 0);
        pContext->IASetInputLayout(pShader->GetVS()->GetInputLayout());
        pContext->PSSetShader     (pShader->GetPS()->GetShader(), nullptr, 0);

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
}

//---------------------------------------------------------
// Desc:  is used for depth pre-pass
//---------------------------------------------------------
void ShaderMgr::DepthPrePassBindShaderById(ID3D11DeviceContext* pContext, const ShaderID id)
{
    DepthPrePassBindShader(pContext, GetShaderById(id));
}

//---------------------------------------------------------
// Desc:  is used for depth pre-pass
//---------------------------------------------------------
void ShaderMgr::DepthPrePassBindShaderByName(ID3D11DeviceContext* pContext, const char* name)
{
    DepthPrePassBindShader(pContext, GetShaderByName(name));
}

//---------------------------------------------------------
// Desc:  is used for depth pre-pass: we need only vertex shader, and
//        geometry shader if we have such
//---------------------------------------------------------
void ShaderMgr::DepthPrePassBindShader(ID3D11DeviceContext* pContext, Shader* pShader)
{
    if (pShader)
    {
        pContext->VSSetShader(pShader->GetVS()->GetShader(), nullptr, 0);
        pContext->IASetInputLayout(pShader->GetVS()->GetInputLayout());
        pContext->PSSetShader(nullptr, nullptr, 0);

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
Shader* ShaderMgr::GetShaderById(const ShaderID id) const
{
    const auto it = idToShader_.find(id);

    if (it == idToShader_.end())
    {
        LogErr(LOG, "there is no shader by id: %" PRIu32, ", so return nullptr");
        return nullptr;
    }

    return it->second;
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

//---------------------------------------------------------
// Desc:  fill in the input arr with ids of all the loaded shaders
//---------------------------------------------------------
void ShaderMgr::GetArrShadersIds(cvector<ShaderID>& outIds) const
{
    outIds.resize(idToShader_.size(), INVALID_SHADER_ID);

    for (int i = 0; const auto& it : idToShader_)
        outIds[i++] = it.first;
}

//---------------------------------------------------------
// Desc:  fill in the input arr with names of all the loader shaders
//---------------------------------------------------------
void ShaderMgr::GetArrShadersNames(cvector<ShaderName>& outNames) const
{
    outNames.resize(idToShader_.size());

    for (int i = 0; const auto & it : idToShader_)
    {
        strcpy(outNames[i].name, it.second->GetName());
        ++i;
    }
}


//==================================================================================
// (PRIVATE) initialization helpers
//==================================================================================

//---------------------------------------------------------
// Desc:   read in a content of json-file and store it into output string
// Args:   - path:      a path to file (relatively to the working directory)
//         - json:      output chars array
//         - jsonSize:  file size in bytes
//---------------------------------------------------------
void ReadJsonIntoBuffer(const char* path, char** json, long& jsonSize)
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
// Desc:   read in parameters which are related to each shader
// Args:   - rawData:    array of parameters to read in
//         - outParams:  output parameters which will be used for initialization
//---------------------------------------------------------
void ReadShadersInitCommonParams(
    const rapidjson::Value& rawData,
    ShaderCommonParams& outParams)
{
    assert(rawData.HasMember("runtime_compilation"));
    assert(rawData.HasMember("shader_model"));
    assert(rawData.HasMember("dir_path_to_precompiled"));
    assert(rawData.HasMember("dir_path_to_hlsl"));

    const rapidjson::Value& runtimeCompilation   = rawData["runtime_compilation"];
    const rapidjson::Value& shaderModel          = rawData["shader_model"];
    const rapidjson::Value& dirPathToPrecompiled = rawData["dir_path_to_precompiled"];
    const rapidjson::Value& dirPathToHLSL        = rawData["dir_path_to_hlsl"];

    assert(runtimeCompilation.IsBool());
    assert(shaderModel.IsString());
    assert(dirPathToPrecompiled.IsString());
    assert(dirPathToHLSL.IsString());


    outParams.runtimeCompilation = runtimeCompilation.GetBool();

    if (outParams.runtimeCompilation)
    {
        strncpy(outParams.dirPathShaders, dirPathToHLSL.GetString(), 32);
        strncpy(outParams.shaderFileExt, ".hlsl", 8);
        strncpy(outParams.shaderModel, shaderModel.GetString(), 3);
    }
    else
    {
        strncpy(outParams.dirPathShaders, dirPathToPrecompiled.GetString(), 32);
        strncpy(outParams.shaderFileExt, ".cso", 8);
        strncpy(outParams.shaderModel, shaderModel.GetString(), 3);
    }


    // check if output params are valid
    const char* sm = outParams.shaderModel;

    assert(outParams.dirPathShaders[0] != '\0');
    assert(outParams.shaderFileExt[0] == '.');
    assert(isdigit(sm[0]) && (sm[1] == '_') && isdigit(sm[2]));
}

//---------------------------------------------------------
// Desc:   read in parameters which are related to each shader
// Args:   - rawData:    array of parameters to read in
//         - outParams:  output parameters which will be used for hot reload
//---------------------------------------------------------
void ReadShadersHotReloadCommonParams(
    const rapidjson::Value& rawData,
    ShaderCommonParams& outParams)
{
    assert(rawData.HasMember("shader_model"));
    assert(rawData.HasMember("dir_path_to_hlsl"));

    const rapidjson::Value& shaderModel   = rawData["shader_model"];
    const rapidjson::Value& dirPathToHLSL = rawData["dir_path_to_hlsl"];

    assert(shaderModel.IsString());
    assert(dirPathToHLSL.IsString());


    // force runtime compilation since we want to hot reload
    outParams.runtimeCompilation = true;

    strncpy(outParams.dirPathShaders, dirPathToHLSL.GetString(), 32);
    strncpy(outParams.shaderFileExt, ".hlsl", 8);
    strncpy(outParams.shaderModel, shaderModel.GetString(), 3);


    // check if output params are valid
    const char* sm = outParams.shaderModel;

    assert(outParams.dirPathShaders[0] != '\0');
    assert(isdigit(sm[0]) && (sm[1] == '_') && isdigit(sm[2]));
}

//---------------------------------------------------------
// Desc:   parse input json-string into document and check if thsi json is valid
// Args:   - doc:       will contain parsed json elements
//         - json:      json-content
//         - jsonSize:  number of characters in the json
//---------------------------------------------------------
void ParseJson(rapidjson::Document& doc, const char* json, const int jsonSize)
{
    rapidjson::ParseResult ok = doc.Parse<rapidjson::kParseCommentsFlag>(json);

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
}

//---------------------------------------------------------
// Desc:   prepare parameters for shader initialization
// Args:   - rawData:    container of raw parameters
//         - outParams:  container of prepared parameters
//---------------------------------------------------------
void PrepareShaderInitParams(
    const rapidjson::Value& rawData,
    const ShaderCommonParams& commonParams,
    const VertexInputLayoutMgr& mgr,
    ShaderInitParams& outParams)
{
    // check some required members
    assert(rawData.HasMember("id"));
    assert(rawData.HasMember("name"));
    assert(rawData.HasMember("vs"));
    assert(rawData.HasMember("ps"));
    assert(rawData.HasMember("input_layout"));

    const rapidjson::Value& id          = rawData["id"];
    const rapidjson::Value& name        = rawData["name"];
    const rapidjson::Value& vs          = rawData["vs"];
    const rapidjson::Value& ps          = rawData["ps"];
    const rapidjson::Value& inputLayout = rawData["input_layout"];

    assert(id.IsUint());
    assert(name.IsString());
    assert(vs.IsString());
    assert(ps.IsString());
    assert(inputLayout.IsString());


    outParams.runtimeCompilation = commonParams.runtimeCompilation;
    outParams.shaderId           = (int)id.GetUint();

    strncpy(outParams.name, name.GetString(), 32);

    // generate a full path to vertex shader file
    strcat(outParams.vsPath, commonParams.dirPathShaders);
    strcat(outParams.vsPath, vs.GetString());
    strcat(outParams.vsPath, commonParams.shaderFileExt);

    // generate a full path to pixel shader file
    strcat(outParams.psPath, commonParams.dirPathShaders);
    strcat(outParams.psPath, ps.GetString());
    strcat(outParams.psPath, commonParams.shaderFileExt);

    // generate a full path to geometry shader file if necessary
    if (rawData.HasMember("gs"))
    {
        const rapidjson::Value& gs = rawData["gs"];
        assert(gs.IsString());

        strcat(outParams.gsPath, commonParams.dirPathShaders);
        strcat(outParams.gsPath, gs.GetString());
        strcat(outParams.gsPath, commonParams.shaderFileExt);
    }

    // setup input layout
    const char* inputLayoutName                 = inputLayout.GetString();
    const VertexInputLayout& vertexInputLayout  = mgr.GetInputLayoutByName(inputLayoutName);
    const UINT numElems                         = (UINT)vertexInputLayout.desc.size();
    outParams.inputLayoutNumElems               = numElems;

    memcpy(outParams.inputLayoutDesc, vertexInputLayout.desc.data(), sizeof(D3D11_INPUT_ELEMENT_DESC) * numElems);

    // setup shader model
    strncpy(outParams.shaderModel, commonParams.shaderModel, 3);
}

//---------------------------------------------------------
// Desc:   prepare parameters for shader hot reload
// Args:   - rawData:    container of raw parameters
//         - outParams:  container of prepared parameters
//---------------------------------------------------------
void PrepareShaderHotReloadParams(
    const rapidjson::Value& rawData,
    const ShaderCommonParams& commonParams,
    const VertexInputLayoutMgr& mgr,
    ShaderInitParams& outParams)
{
    // check some required members
    assert(rawData.HasMember("id"));
    assert(rawData.HasMember("vs"));
    assert(rawData.HasMember("ps"));
    assert(rawData.HasMember("input_layout"));

    const rapidjson::Value& id          = rawData["id"];
    const rapidjson::Value& vs          = rawData["vs"];
    const rapidjson::Value& ps          = rawData["ps"];
    const rapidjson::Value& inputLayout = rawData["input_layout"];

    assert(id.IsUint());
    assert(vs.IsString());
    assert(ps.IsString());
    assert(inputLayout.IsString());


    outParams.runtimeCompilation = commonParams.runtimeCompilation;
    outParams.shaderId           = (int)id.GetUint();


    // generate a full path to vertex shader file
    strcat(outParams.vsPath, commonParams.dirPathShaders);
    strcat(outParams.vsPath, vs.GetString());
    strcat(outParams.vsPath, commonParams.shaderFileExt);

    // generate a full path to pixel shader file
    strcat(outParams.psPath, commonParams.dirPathShaders);
    strcat(outParams.psPath, ps.GetString());
    strcat(outParams.psPath, commonParams.shaderFileExt);

    // generate a full path to geometry shader file if necessary
    if (rawData.HasMember("gs"))
    {
        const rapidjson::Value& gs = rawData["gs"];
        assert(gs.IsString());

        strcat(outParams.gsPath, commonParams.dirPathShaders);
        strcat(outParams.gsPath, gs.GetString());
        strcat(outParams.gsPath, commonParams.shaderFileExt);
    }

    // setup input layout
    const char* inputLayoutName                 = inputLayout.GetString();
    const VertexInputLayout& vertexInputLayout  = mgr.GetInputLayoutByName(inputLayoutName);
    const UINT numElems                         = (UINT)vertexInputLayout.desc.size();
    outParams.inputLayoutNumElems               = numElems;

    memcpy(outParams.inputLayoutDesc, vertexInputLayout.desc.data(), sizeof(D3D11_INPUT_ELEMENT_DESC) * numElems);

    // setup shader model
    strncpy(outParams.shaderModel, commonParams.shaderModel, 3);
}

} // namespace
