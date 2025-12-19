#include "../Common/pch.h"
#include "InputLayouts.h"
#include <rapidjson/document.h>      // for parsing json-string
#include <rapidjson/error/en.h>      // for error handling
#pragma warning (disable : 4996)

namespace Render
{

// some functions declarations
void ReadInputLayoutsJSON(const char* path, char** json, long& jsonSize);
void AddPerVertexInputLayoutElems(VertexInputLayout& layout, const rapidjson::Value& arrElements, const UINT inputSlotIdx);
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
const VertexInputLayout& VertexInputLayoutMgr::GetInputLayoutByName(const char* name) const
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
    const char* path = "data/shaders/input_layouts.json";
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


    // read in "None" input layout in a separate way (is used when vertex shader has no input)
    inputLayouts.push_back(VertexInputLayout());
    VertexInputLayout& layout = inputLayouts.back();


    // read in all the rest of input layouts
    const rapidjson::Value& layouts = doc["layouts"];

    for (int i = 1; i < (int)layouts.Size(); ++i)
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
// enum of supported DXGI format
// (it is here because we use it strictly for internal purposes)
//---------------------------------------------------------
enum eSupportFmtDXGI
{
    DXGI_UNKNOWN,
    DXGI_F1,          // DXGI_FORMAT_R32_FLOAT
    DXGI_F2,          // DXGI_FORMAT_R32G32_FLOAT
    DXGI_F3,          // DXGI_FORMAT_R32G32B32_FLOAT
    DXGI_F4,          // DXGI_FORMAT_R32G32B32A32_FLOAT
    DXGI_F16,         // matrix of four float4 (world, world inv transpose, material, etc.)
    DXGI_U32,         // DXGI_FORMAT_R8G8B8A8_UINT
    DXGI_UN32,        // DXGI_FORMAT_R8G8B8A8_UNORM
    DXGI_R32_UINT,    // DXGI_FORMAT_R32_UINT
    DXGI_RGBA32_UINT, // DXGI_FORMAT_R32G32B32A32_UINT
};

//---------------------------------------------------------
// Desc:  get DXGI code (internal) by input format string
//---------------------------------------------------------
inline eSupportFmtDXGI GetDXGI_FloatFormat(const char* fmt, UINT& outNumElems)
{
    outNumElems = 1;

    if (strcmp(fmt, "f3") == 0)
        return DXGI_F3;

    else if (strcmp(fmt, "f2") == 0)
        return DXGI_F2;

    else if (strcmp(fmt, "f1") == 0)
        return DXGI_F1;

    else if (strcmp(fmt, "f4") == 0)
        return DXGI_F4;

    // matrix (16 floats)
    else if (strcmp(fmt, "f16") == 0)
    {
        outNumElems = 4;
        return DXGI_F16;
    }

    else
        return DXGI_UNKNOWN;
}

//---------------------------------------------------------
// Desc:  get DXGI code (internal) by input format string
//---------------------------------------------------------
eSupportFmtDXGI GetDXGI_UnsignedFormat(const char* fmt, UINT& outNumElems)
{
    outNumElems = 1;

    if (strcmp(fmt, "r32") == 0)
        return DXGI_R32_UINT;

    else if (strcmp(fmt, "un32") == 0)
        return DXGI_UN32;

    else if (strcmp(fmt, "u32") == 0)
        return DXGI_U32;

    else if (strcmp(fmt, "rgba32u") == 0)
        return DXGI_RGBA32_UINT;

    else
        return DXGI_UNKNOWN;
}

//---------------------------------------------------------
// Desc:  define DXGI_FORMAT and number of elements by input fmt string
//---------------------------------------------------------
void GetFormatDXGI(const char* fmt, DXGI_FORMAT& outFmt, uint& outNumElems)
{
    outFmt = DXGI_FORMAT_UNKNOWN;

    if (StrHelper::IsEmpty(fmt))
    {
        LogErr(LOG, "input layout element format is empty!");
        return;
    }

    eSupportFmtDXGI formatCode = DXGI_UNKNOWN;

    // check if float format
    if (fmt[0] == 'f')
        formatCode = GetDXGI_FloatFormat(fmt, outNumElems);

    // check if unsigned format
    else if (fmt[0] == 'u' || fmt[0] == 'r')
        formatCode = GetDXGI_UnsignedFormat(fmt, outNumElems);

    else
    {
        LogErr(LOG, "uknown input layout element format: %s", fmt);
        return;
    }

    if (formatCode == DXGI_UNKNOWN)
    {
        LogErr(LOG, "uknown input layout element format: %s", fmt);
        return;
    }
   

    switch (formatCode)
    {
        case DXGI_F1:          outFmt = DXGI_FORMAT_R32_FLOAT;             break;
        case DXGI_F2:          outFmt = DXGI_FORMAT_R32G32_FLOAT;          break;
        case DXGI_F3:          outFmt = DXGI_FORMAT_R32G32B32_FLOAT;       break;
        case DXGI_F4:          outFmt = DXGI_FORMAT_R32G32B32A32_FLOAT;    break;
        case DXGI_F16:         outFmt = DXGI_FORMAT_R32G32B32A32_FLOAT;    break;
        case DXGI_U32:         outFmt = DXGI_FORMAT_R8G8B8A8_UINT;         break;
        case DXGI_UN32:        outFmt = DXGI_FORMAT_R8G8B8A8_UNORM;        break;
        case DXGI_R32_UINT:    outFmt = DXGI_FORMAT_R32_UINT;              break;
        case DXGI_RGBA32_UINT: outFmt = DXGI_FORMAT_R32G32B32A32_UINT;     break;

        default:
        {
            LogErr(LOG, "uknown input layout element format: %s", fmt);
            return;
        }
    }
}


//---------------------------------------------------------
// Desc:   go through each element from input array,
//         create a new input layout element (per vertex) and add it into layout
// Args:   - layout:    vertex input layout to fill in with elements
//         - arrElems:  array of parsed raw elements from file
//---------------------------------------------------------
void AddPerVertexInputLayoutElems(
    VertexInputLayout& layout,
    const rapidjson::Value& arrElements,
    const UINT inputSlotIdx)
{
    
    for (UINT i = 0; i < (UINT)arrElements.Size(); i++)
    {
        const rapidjson::Value& arrElem      = arrElements[i];
        const rapidjson::Value& semanticName = arrElem[0];
        const rapidjson::Value& format       = arrElem[1];

        assert(semanticName.IsString());
        assert(format.IsString());


        // define a DXGI format
        DXGI_FORMAT     dxgiFormat = DXGI_FORMAT_UNKNOWN;
        UINT            numElems   = 1;
        const char*     fmt = format.GetString();

        GetFormatDXGI(fmt, dxgiFormat, numElems);

        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            continue;


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
        desc.InputSlot              = inputSlotIdx;
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
        assert(format.IsString());


        // define a DXGI format
        DXGI_FORMAT     dxgiFormat = DXGI_FORMAT_UNKNOWN;
        UINT            numElems = 1;
        const char*     fmt = format.GetString();

        GetFormatDXGI(fmt, dxgiFormat, numElems);

        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            continue;


        // add elements into the input layout
        for (UINT i = 0; i < numElems; ++i)
        {
            layout.desc.push_back(D3D11_INPUT_ELEMENT_DESC());
            D3D11_INPUT_ELEMENT_DESC& desc = layout.desc.back();

            const char* elemSemanticName = semanticName.GetString();
            const size_t semanticNameLen = strlen(elemSemanticName);

            // setup description
            desc.SemanticName = new char[semanticNameLen + 1]{ '\0' };
            strcpy((char*)desc.SemanticName, elemSemanticName);

            desc.SemanticIndex          = i;
            desc.Format                 = dxgiFormat;
            desc.InputSlot              = 1;
            desc.AlignedByteOffset      = D3D11_APPEND_ALIGNED_ELEMENT;
            desc.InputSlotClass         = D3D11_INPUT_PER_INSTANCE_DATA;
            desc.InstanceDataStepRate   = 1;
        }
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
    
    // if we want to get data from several input slots (bind 2+ vertex buffers)
    if (layoutData.HasMember("num_input_slots"))
    {
        assert(layoutData["num_input_slots"].IsUint());
        const UINT numInputSlots = layoutData["num_input_slots"].GetUint();

        // check if we have proper number of slots
        for (UINT inputSlotIdx = 0; inputSlotIdx < numInputSlots; ++inputSlotIdx)
        {
            char buf[32]{'\0'};
            sprintf(buf, "per_vertex_slot_%u", inputSlotIdx);
            assert(layoutData.HasMember(buf));

            AddPerVertexInputLayoutElems(layout, layoutData[buf], inputSlotIdx);
        }
    }
   
    else
    {
        // setup "per vertex" layout only for a single input slot (0)
        assert(layoutData.HasMember("per_vertex"));
        assert(layoutData["per_vertex"].IsArray());

        constexpr UINT inputSlotIdx = 0;
        AddPerVertexInputLayoutElems(layout, layoutData["per_vertex"], inputSlotIdx);
    }

  


    //-----------------------------------------------------
    // setup "per instance" layout input elements

    if (layoutData.HasMember("per_instance"))
    {
        assert(layoutData["per_instance"].IsArray());
        AddPerInstanceInputLayoutElems(layout, layoutData["per_instance"]);
    }
}

} // namespace
