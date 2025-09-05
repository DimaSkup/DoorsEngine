// =================================================================================
// Filename:     Shader.h
// Description:  a wrapper over HLSL shaders
// 
// Created:      24.08.2025  by DimaSkup
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "InputLayouts.h"
#include "../Common/RenderTypes.h"


namespace Render
{

// a container of initial params for shader
struct ShaderInitParams
{
    ShaderInitParams()
    {
        memset(&inputLayoutDesc, 0, sizeof(inputLayoutDesc));
    }

    // if we have any data in path then we want to init this type of shader
    inline bool NeedInitGS() const
    {
        return gsPath[0] != '\0';
    }


    char name[32]{'\0'};            // a name for shader class
    char vsPath[64]{'\0'};          // path to vertex shader 
    char gsPath[64]{'\0'};          // path to geometry shader 
    char psPath[64]{'\0'};          // path to pixel shader
    char shaderModel[4]{ "5_0" };
    bool loadPrecompiledShaders = true;    // do we use precompiled (.cso) shader files for initialization?

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[32];
    UINT inputLayoutNumElems = 0;
};

//---------------------------------------------------------

class Shader
{
public:
    Shader(ID3D11Device* pDevice, const ShaderInitParams& params);

    // restrict copying of this class instance
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
        
    void HotReload(ID3D11Device* pDevice, const ShaderInitParams& params);

    // getters
    inline const char* GetName() const { return name_; }

    inline VertexShader*   GetVS() const { assert(pVS_ != nullptr); return pVS_; }
    inline GeometryShader* GetGS() const { return pGS_; }
    inline PixelShader*    GetPS() const { assert(pPS_ != nullptr); return pPS_; }

private:
    bool LoadPrecompiled(ID3D11Device* pDevice, const ShaderInitParams& params);
    bool CompileFromFile(ID3D11Device* pDevice, const ShaderInitParams& params);

private:
    VertexShader*   pVS_ = nullptr;
    GeometryShader* pGS_ = nullptr;
    PixelShader*    pPS_ = nullptr;

    char            name_[32]{'\0'};
};

} // namespace
