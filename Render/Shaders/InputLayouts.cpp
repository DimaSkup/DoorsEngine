#include "../Common/pch.h"
#include "InputLayouts.h"
#include <rapidjson/document.h>      // for parsing json-string
#include <rapidjson/error/en.h>      // for error handling
#pragma warning (disable : 4996)

namespace Render
{

// some functions declarations
void ReadInputLayoutsJSON(const char* path, char** json, long& jsonSize);
void AddPerVertexInputLayoutElems(VertexInputLayout& layout, const rapidjson::Value& arrElements);
void AddPerInstanceInputLayoutElems(VertexInputLayout& layout, const rapidjson::Value& arrElems);
void AddInputLayout(const rapidjson::Value& layoutData, VertexInputLayoutMgr& mgr);


//---------------------------------------------------------
// Desc:  just default constructor
//---------------------------------------------------------
VertexInputLayoutMgr::VertexInputLayoutMgr()
{
    LoadAndCreateInputLayouts();
}

//---------------------------------------------------------
// Desc:  return a ref to vertex input layout by its name
//        or return InputLayout by idx 0 if there is no such name
//---------------------------------------------------------
const VertexInputLayout& VertexInputLayoutMgr::GetInputLayoutByName(const char* name)
{
    // check input name
    if (!name || name[0] == '\0')
    {
        LogErr(LOG, "input name is empty");
        return inputLayouts[0];
    }

    // find layout by name
    for (const VertexInputLayout& layout : inputLayouts)
    {
        if (strcmp(layout.name, name) == 0)
        {
            return layout;
        }
    }

    return inputLayouts[0];
}

//---------------------------------------------------------
// Desc:   load declarations of vertex input layouts from a json file
//         and create them
//---------------------------------------------------------
void VertexInputLayoutMgr::LoadAndCreateInputLayouts()
{
    // read in input layouts declarations from json-config file
    const char* path = "shaders/input_layouts.json";
    char* json = nullptr;
    long jsonSize = 0;
    ReadInputLayoutsJSON(path, &json, jsonSize);
    

    // parse json
    rapidjson::Document doc;
    rapidjson::ParseResult ok = doc.Parse(json);

    // check if we correctly parsed a json-string
    if (!ok) {
        size_t offset = ok.Offset();
        LogErr(LOG, "JSON parse error: %s (%zu)", rapidjson::GetParseError_En(ok.Code()), offset);

        // find an end of a line after error
        size_t endOffset = 0;

        for (size_t i = offset; i < jsonSize; i++)
        {
            if (json[i] == '\n')
            {
                endOffset = i;
                i = jsonSize;            // get out of for-loop
            }
        }

        // print this line which causes a parsing error
        size_t len = endOffset - offset;
        LogErr(LOG, "Error happend BEFORE a line: %.*s", len, json + offset);

        exit(EXIT_FAILURE);
    }

    // do some checks
    assert(doc.IsObject());
    assert(doc.HasMember("layouts"));
    assert(doc["layouts"].IsArray());


    // read in all the input layouts
    const rapidjson::Value& layouts = doc["layouts"];

    for (int i = 0; i < (int)layouts.Size(); ++i)
    {
        const rapidjson::Value& layoutData = layouts[i];
        AddInputLayout(layoutData, *this);
    }

    // release buffer
    SafeDeleteArr(json);
}

//==================================================================================
//==================================================================================
//==================================================================================

//---------------------------------------------------------
// Desc:   read in a content of json-file and store it into output string
// Args:   - path:      a path to file (relatively to the working directory)
//         - json:      output chars array
//         - jsonSize:  file size in bytes
//---------------------------------------------------------
void ReadInputLayoutsJSON(const char* path, char** json, long& jsonSize)
{
    if (!path || path[0] == '\0')
    {
        LogErr(LOG, "input path to input layouts config file is empty");
        exit(0);
    }

    // open file
    FILE* pFile = fopen(path, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open input layouts config file: %s", path);
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
// Desc:   go through each element from input array,
//         create a new input layout element (per vertex) and add it into layout
// Args:   - layout:    vertex input layout to fill in with elements
//         - arrElems:  array of parsed raw elements from file
//---------------------------------------------------------
void AddPerVertexInputLayoutElems(VertexInputLayout& layout, const rapidjson::Value& arrElements)
{
    
    for (UINT i = 0; i < (UINT)arrElements.Size(); i++)
    {
        const rapidjson::Value& arrElem      = arrElements[i];
        const rapidjson::Value& semanticName = arrElem[0];
        const rapidjson::Value& format       = arrElem[1];

        assert(semanticName.IsString());
        assert(format.IsInt());


        // define a DXGI format
        DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;

        switch (format.GetInt())
        {
            // float
            case 1:
                dxgiFormat = DXGI_FORMAT_R32_FLOAT;
                break;

            // float2
            case 2:   
                dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
                break;

            // float3
            case 3:   
                dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
                break;

            // float4
            case 4:   
                dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
                break;

            default:
            {
                LogErr(LOG, "unknown dxgi format for input layout elem desc per vertex: %d", format.GetInt());
            }
        }

        // add a new input element description into layout
        layout.desc.push_back(D3D11_INPUT_ELEMENT_DESC());
        D3D11_INPUT_ELEMENT_DESC& desc = layout.desc.back();

        // setup description
        const char* elemSemanticName = semanticName.GetString();
        const size_t semanticNameLen = strlen(elemSemanticName);

        desc.SemanticName = new char[semanticNameLen + 1]{ '\0' };
        strcpy((char*)desc.SemanticName, elemSemanticName);

        desc.SemanticIndex          = 0;
        desc.Format                 = dxgiFormat;
        desc.InputSlot              = 0;
        desc.AlignedByteOffset      = D3D11_APPEND_ALIGNED_ELEMENT;
        desc.InputSlotClass         = D3D11_INPUT_PER_VERTEX_DATA;
        desc.InstanceDataStepRate   = 0;
    }
}

//---------------------------------------------------------
// Desc:   go through each element from input array,
//         create a new input layout element (per instance) and add it into layout
// Args:   - layout:    vertex input layout to fill in with elements
//         - arrElems:  array of parsed raw elements from file
//---------------------------------------------------------
void AddPerInstanceInputLayoutElems(VertexInputLayout& layout, const rapidjson::Value& arrElems)
{
    for (UINT i = 0; i < (UINT)arrElems.Size(); i++)
    {
        const rapidjson::Value& arrElem      = arrElems[i];
        const rapidjson::Value& semanticName = arrElem[0];
        const rapidjson::Value& format       = arrElem[1];

        assert(semanticName.IsString());
        assert(format.IsInt());


        // define a DXGI format
        DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;

        switch (format.GetInt())
        {
             // float
            case 1:
                dxgiFormat = DXGI_FORMAT_R32_FLOAT;
                break;

            // float2
            case 2:
                dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
                break;

            // float3
            case 3:
                dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
                break;

            // float4
            case 4:
                
                dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
                break;

            // matrix (16 floats)
            case 16:
            {
                // matrix consists of four float4 elements
                for (UINT i = 0; i < 4; ++i)
                {
                    layout.desc.push_back(D3D11_INPUT_ELEMENT_DESC());
                    D3D11_INPUT_ELEMENT_DESC& desc = layout.desc.back();

                    const char* elemSemanticName = semanticName.GetString();
                    const size_t semanticNameLen = strlen(elemSemanticName);

                    // setup description
                    desc.SemanticName           = new char[semanticNameLen + 1]{'\0'};
                    strcpy((char*)desc.SemanticName, elemSemanticName);

                    desc.SemanticIndex          = i;
                    desc.Format                 = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    desc.InputSlot              = 1;
                    desc.AlignedByteOffset      = D3D11_APPEND_ALIGNED_ELEMENT;
                    desc.InputSlotClass         = D3D11_INPUT_PER_INSTANCE_DATA;
                    desc.InstanceDataStepRate   = 1;
                }

                break;
            }

            default:
            {
                LogErr(LOG, "unknown dxgi format for input layout elem desc per instance: %d", format.GetInt());
            }
        } // switch
    } // for
}

//-----------------------------------------------------
// Desc:   add a new input layout into layouts manager
// Args:   - layoutData:  raw data for layout initialization
//         - mgr:         vertex input layouts manager
//-----------------------------------------------------
void AddInputLayout(const rapidjson::Value& layoutData, VertexInputLayoutMgr& mgr)
{
    // add a new empty vertex input layout
    mgr.inputLayouts.push_back(VertexInputLayout());
    VertexInputLayout& layout = mgr.inputLayouts.back();

    assert(layoutData.HasMember("name"));
    strncpy(layout.name, layoutData["name"].GetString(), 32);


    //-----------------------------------------------------
    // setup "per vertex" layout input elements

    assert(layoutData.HasMember("per_vertex"));
    assert(layoutData["per_vertex"].IsArray());
    AddPerVertexInputLayoutElems(layout, layoutData["per_vertex"]);


    //-----------------------------------------------------
    // setup "per instance" layout input elements

    if (layoutData.HasMember("per_instance"))
    {
        assert(layoutData["per_instance"].IsArray());
        AddPerInstanceInputLayoutElems(layout, layoutData["per_instance"]);
    }
}

} // namespace
