#include "../Common/pch.h"
#include "Shader.h"

namespace Render
{

//---------------------------------------------------------
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - params:  a container with initialization params
//---------------------------------------------------------
Shader::Shader(ID3D11Device* pDevice, const ShaderInitParams& params)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(params.name),        "input name for shader is empty");
        CAssert::True(!StrHelper::IsEmpty(params.vsPath),      "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(params.psPath),      "input path to pixel shader is empty");
        CAssert::True(!StrHelper::IsEmpty(params.shaderModel), "input shader model value is empty");
        CAssert::True(params.inputLayoutNumElems >= 0,         "input number of layout elements can't be == 0");

        bool result = false;

        // store an id and name
        id_ = (ShaderID)params.shaderId;
        strncpy(name_, params.name, 32);


        // allocate memory for shader classes
        pVS_ = NEW VertexShader;
        CAssert::True(pVS_, "can't allocate memory for vertex shader");

        pPS_ = NEW PixelShader;
        CAssert::True(pPS_, "can't allocate memory for pixel shader");

        if (params.NeedInitGS())
        {
            pGS_ = NEW GeometryShader;
            CAssert::True(pGS_, "can't allocate memory for geometry shader");
        }


        // load shaders from .cso files
        if (!params.runtimeCompilation)
        {
            result = LoadPrecompiled(pDevice, params);
            CAssert::True(result, "can't load precompiled shaders");
        }

        // compile shaders from .hlsl files
        else
        {
            result = CompileFromFile(pDevice, params);
            CAssert::True(result, "can't compile shaders from files");
        }

        LogDbg(LOG, "shader is initialized: %s", params.name);
    }
    catch (EngineException& e)
    {
        SafeDelete(pVS_);
        SafeDelete(pGS_);
        SafeDelete(pPS_);
        LogErr(e);
        LogErr(LOG, "can't init shader: %s", params.name);
    }
}

//---------------------------------------------------------
// Desc:   recompile hlsl shaders from file and reinit shader class object
// Args:   - pDevice:  a ptr to the DirectX11 device
//         - params:   a container of parameters for shader reinitialization 
//---------------------------------------------------------
bool Shader::HotReload(ID3D11Device* pDevice, const ShaderInitParams& params)
{
    // reload vertex shader
    if (pVS_)
    {
        if (params.vsPath[0] == '\0')
        {
            LogErr(LOG, "can't hot reload vertex shader (%s): input path to file is empty", name_);
            return false;
        }

        if (!pVS_->CompileFromFile(
            pDevice,
            params.vsPath,
            "VS",
            params.shaderModel,
            params.inputLayoutDesc,
            params.inputLayoutNumElems))
        {
            LogErr(LOG, "can't recompile vertex shader (%s) from file: %s", name_, params.vsPath);
            return false;
        }
    }

    // reload geometry shader
    if (pGS_)
    {
        if (params.gsPath[0] == '\0')
        {
            LogErr(LOG, "can't hot reload geometry shader (%s): input path to file is empty", name_);
            return false;
        }

        if (!pGS_->CompileFromFile(pDevice, params.gsPath, "GS", params.shaderModel))
        {
            LogErr(LOG, "can't recompile geometry shader (%s) from file: %s", name_, params.gsPath);
            return false;
        }
    }

    // reload pixel shader
    if (pPS_)
    {
        if (params.psPath[0] == '\0')
        {
            LogErr(LOG, "can't hot reload pixel shader (%s): input path to file is empty", name_);
            return false;
        }

        if (!pPS_->CompileFromFile(pDevice, params.psPath, "PS", params.shaderModel))
        {
            LogErr(LOG, "can't recompile pixel shader (%s) from file: %s", name_, params.psPath);
            return false;
        }
    }

    return true;
}


//---------------------------------------------------------
//---------------------------------------------------------
bool Shader::LoadPrecompiled(ID3D11Device* pDevice, const ShaderInitParams& params)
{
    // init vertex shader
    bool result = pVS_->LoadPrecompiled(
        pDevice,
        params.vsPath,
        params.inputLayoutDesc,
        params.inputLayoutNumElems);
    if (!result)
    {
        LogErr(LOG, "can't init a vertex shader");
        return false;
    }

    // init pixel shader
    if (!pPS_->LoadPrecompiled(pDevice, params.psPath))
    {
        LogErr(LOG, "can't init a pixel shader");
        return false;
    }

    // init geometry shader
    if (params.NeedInitGS())
    {
        if (!pGS_->LoadPrecompiled(pDevice, params.gsPath))
        {
            LogErr(LOG, "can't init a geometry shader");
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool Shader::CompileFromFile(ID3D11Device* pDevice, const ShaderInitParams& params)
{
    // init vertex shader
    bool result = pVS_->CompileFromFile(
        pDevice,
        params.vsPath,
        "VS",
        params.shaderModel,
        params.inputLayoutDesc,
        params.inputLayoutNumElems);
    if (!result)
    {
        LogErr(LOG, "can't init a vertex shader");
        return false;
    }

    // init pixel shader
    if (!pPS_->CompileFromFile(pDevice, params.psPath, "PS", params.shaderModel))
    {
        LogErr(LOG, "can't init a pixel shader");
        return false;
    }

    // init geometry shader
    if (params.NeedInitGS())
    {
        if (!pGS_->CompileFromFile(pDevice, params.gsPath, "GS", params.shaderModel))
        {
            LogErr(LOG, "can't init a geometry shader");
            return false;
        }
    }

    return true;
}


} // namespace
