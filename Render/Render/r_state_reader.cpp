/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: r_state_reader.cpp
    Desc:     implementation of the RenderStateReader class

    Created:  17.01.2026 by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "r_state_reader.h"
#include <inttypes.h>      // for SCNx8

#pragma warning(disable : 4996)


namespace Render
{

//---------------------------------------------------------
// common helpers
//---------------------------------------------------------
inline bool GetBool(const char* str)
{
    assert(str && str[0] != '\0');
    return (str[0] == 't');         // if == 't' then true, else false
}
    
//**********************************************************************************
//                         RASTERIZER STATES LOADING
//**********************************************************************************

//---------------------------------------------------------
// Desc:  read in a rasterizer state description from file
//---------------------------------------------------------
void RenderStateReader::LoadRsDesc(
    FILE* pFile,
    D3D11_RASTERIZER_DESC& desc,
    const bool msaaEnable)
{
    assert(pFile);

    char buf[128];
    char propName[32];
    char propValue[32];
    int count = 0;
    ZeroMemory(&desc, sizeof(desc));

    while (fgets(buf, sizeof(buf), pFile))
    {
        if (buf[0] == '}')
            return;

        count = sscanf(buf, " %s %s", propName, propValue);
        assert(count == 2);

        GetRasterStateDescParam(propName, propValue, msaaEnable, desc);
    }
}

//---------------------------------------------------------
// Desc:  get parameter value for rasterizer state description
//---------------------------------------------------------
void RenderStateReader::GetRasterStateDescParam(
    const char* propName,
    const char* propValue,
    const bool msaaEnable,
    D3D11_RASTERIZER_DESC& desc)
{
    assert(propName && propName[0] != '\0');
    assert(propValue && propValue[0] != '\0');

    switch (propName[0])
    {
        case 'a':
        {
            desc.AntialiasedLineEnable = GetBool(propValue);
            break;
        }
        case 'd':
        {
            // depth_
            if (strcmp(propName, "depth_bias") == 0)
                desc.DepthBias = atoi(propValue);

            else if (strcmp(propName, "depth_bias_clamp") == 0)
                desc.DepthBiasClamp = (float)atof(propValue);

            else if (strcmp(propName, "depth_clip_enable") == 0)
                desc.DepthClipEnable = GetBool(propValue);

            break;
        }
        case 'c':
        {
            desc.CullMode = GetCullMode(propValue);
            break;
        }
        case 'f':
        {
            // fill / front_ccw
            if (propName[1] == 'i')
                desc.FillMode = GetFillMode(propValue);
            else
                desc.FrontCounterClockwise = GetBool(propValue);
            break;
        }
        case 'm':
        {
            // multisambple_enable
            desc.MultisampleEnable = GetBool(propValue) || msaaEnable;
            break;
        }
        case 's':
        {
            if (strcmp(propName, "slope_scaled_depth_bias") == 0)
                desc.SlopeScaledDepthBias = (float)atof(propValue);

            else if (strcmp(propName, "scissor_enable") == 0)
                desc.ScissorEnable = GetBool(propValue);

            break;
        }
    }
}

//---------------------------------------------------------

D3D11_FILL_MODE RenderStateReader::GetFillMode(const char* fill)
{
    assert(fill && fill[0] != '\0');

    if (strcmp(fill, "solid") == 0)
        return D3D11_FILL_SOLID;

    if (strcmp(fill, "wireframe") == 0)
        return D3D11_FILL_WIREFRAME;

    LogErr(LOG, "unknown fill mode param: %s", fill);
    return D3D11_FILL_SOLID;
}

//---------------------------------------------------------

D3D11_CULL_MODE RenderStateReader::GetCullMode(const char* cull)
{
    assert(cull && cull[0] != '\0');

    switch (cull[0])
    {
        case 'b':
            if (strcmp(cull, "back") == 0)
                return D3D11_CULL_BACK;
            break;

        case 'f':
            if (strcmp(cull, "front") == 0)
                return D3D11_CULL_FRONT;
            break;

        case 'n':
            if (strcmp(cull, "none") == 0)
                return D3D11_CULL_NONE;
            break;
    }

    LogErr(LOG, "unknown cull mode param: %s", cull);
    return D3D11_CULL_BACK;
}

    
//**********************************************************************************
//                          BLEND STATES LOADING
//**********************************************************************************

//---------------------------------------------------------
// Desc:  read in blend state description params from file
//        and setup the output description structure
//---------------------------------------------------------
void RenderStateReader::LoadBsDesc(FILE* pFile, D3D11_BLEND_DESC& desc)
{
    assert(pFile);

    char buf[128];
    char prop[32];
    char propValue[32];
    int count = 0;
    ZeroMemory(&desc, sizeof(desc));

    D3D11_RENDER_TARGET_BLEND_DESC& rtbd = desc.RenderTarget[0];


    while (fgets(buf, sizeof(buf), pFile))
    {
        if (buf[0] == '}')
            return;

        // read in: blending property and its value
        count = sscanf(buf, "%s %s", prop, propValue);
        assert(count == 2);

        // get property type: skip first 7 symbols "blend_"
        const char* propType = &buf[7];

        switch (propType[0])
        {
            case 'a':
                desc.AlphaToCoverageEnable = GetBool(propValue);
                break;

            // destination color / alpha
            case 'd':
                if (strcmp(prop, "blend_dst") == 0)
                    rtbd.DestBlend = GetBlendFactor(propValue);

                else if (strcmp(prop, "blend_dst_alpha") == 0)
                    rtbd.DestBlendAlpha = GetBlendFactor(propValue);

                break;

            case 'e':
                rtbd.BlendEnable = GetBool(propValue);
                break;

            case 'i':
                desc.IndependentBlendEnable = GetBool(propValue);
                break;

            // blend operation
            case 'o':
                if (strcmp(prop, "blend_op") == 0)
                    rtbd.BlendOp = GetBlendOperation(propValue);

                else if (strcmp(prop, "blend_op_alpha") == 0)
                    rtbd.BlendOpAlpha = GetBlendOperation(propValue);

                break;

            // render target write mask
            case 'r':
                rtbd.RenderTargetWriteMask = GetRenderTargetWriteMask(propValue);
                break;

            // source color / alpha
            case 's':
                if (strcmp(prop, "blend_src") == 0)
                    rtbd.SrcBlend = GetBlendFactor(propValue);

                else if (strcmp(prop, "blend_src_alpha") == 0)
                    rtbd.SrcBlendAlpha = GetBlendFactor(propValue);

                break;
        }
    }
}

//---------------------------------------------------------
// Desc:  return D3D_BLEND_OP value by input name of the blend operation 
//---------------------------------------------------------
D3D11_BLEND_OP RenderStateReader::GetBlendOperation(const char* op)
{
    assert(op && op[0] != '\0');

    switch (op[0])
    {
        case 'a': return D3D11_BLEND_OP_ADD;
        case 's': return D3D11_BLEND_OP_SUBTRACT;
        case 'r': return D3D11_BLEND_OP_REV_SUBTRACT;
        case 'm': return (op[1] == 'a') ? D3D11_BLEND_OP_MAX : D3D11_BLEND_OP_MIN;
    }

    LogErr(LOG, "failed to read blend operation: %s", op);
    return D3D11_BLEND_OP_ADD;
}

//---------------------------------------------------------
// Desc:  return D3D11_BLEND value by input name of the blend factor
//---------------------------------------------------------
D3D11_BLEND RenderStateReader::GetBlendFactor(const char* f)
{
    assert(f && f[0] != '\0');

    switch (f[0])
    {
        case 'b':
            return D3D11_BLEND_BLEND_FACTOR;

        case 'd':
        {
            if (strcmp(f, "dst_col") == 0)          return D3D11_BLEND_DEST_COLOR;
            if (strcmp(f, "dst_alpha") == 0)        return D3D11_BLEND_DEST_ALPHA;
        }

        case 'i':
        {
            if (strcmp(f, "inv_src_col") == 0)      return D3D11_BLEND_INV_SRC_COLOR;
            if (strcmp(f, "inv_src_alpha") == 0)    return D3D11_BLEND_INV_SRC_ALPHA;
            if (strcmp(f, "inv_dst_col") == 0)      return D3D11_BLEND_DEST_COLOR;
            if (strcmp(f, "inv_dst_alpha") == 0)    return D3D11_BLEND_DEST_ALPHA;
            if (strcmp(f, "inv_blend_factor") == 0) return D3D11_BLEND_INV_BLEND_FACTOR;
        }

        case 'o':
            return D3D11_BLEND_ONE;

        case 's':
        {
            if (strcmp(f, "src_col") == 0)          return D3D11_BLEND_SRC_COLOR;
            if (strcmp(f, "src_alpha") == 0)        return D3D11_BLEND_SRC_ALPHA;
            if (strcmp(f, "src_alpha_sat") == 0)    return D3D11_BLEND_SRC_ALPHA_SAT;
        }

        case 'z':
            return D3D11_BLEND_ZERO;
    }

    LogErr(LOG, "failed to read blend factor: %s", f);
    return D3D11_BLEND_ONE;
}

//---------------------------------------------------------
// Desc:  return D3D11_COLOR_WRITE_ENABLE value by input name of mask
//---------------------------------------------------------
D3D11_COLOR_WRITE_ENABLE RenderStateReader::GetRenderTargetWriteMask(const char* mask)
{
    assert(mask && mask[0] != '\0');

    switch (mask[0])
    {
        case 'a':   return D3D11_COLOR_WRITE_ENABLE_ALL;
        case 'b':   return D3D11_COLOR_WRITE_ENABLE_BLUE;
        case 'g':   return D3D11_COLOR_WRITE_ENABLE_GREEN;
        case 'r':   return D3D11_COLOR_WRITE_ENABLE_RED;
        case 'z':   return D3D11_COLOR_WRITE_ENABLE(0);
    }

    LogErr(LOG, "failed to read render target view mask: %s", mask);
    return D3D11_COLOR_WRITE_ENABLE_ALL;
}


//**********************************************************************************
//                  DEPTH-STENCIL STATES LOADING
//**********************************************************************************

//---------------------------------------------------------
// helpers for depth-stencil description setup
//---------------------------------------------------------
void RenderStateReader::LoadDssDesc(FILE* pFile, D3D11_DEPTH_STENCIL_DESC& desc)
{
    assert(pFile);

    char buf[128];
    char propName[32];
    char propValue[32];
    int count = 0;
    ZeroMemory(&desc, sizeof(desc));


    while (fgets(buf, sizeof(buf), pFile))
    {
        if (buf[0] == '}')
            return;

        count = sscanf(buf, " %s %s", propName, propValue);
        assert(count == 2);

        switch (propName[0])
        {
            case 'b':   ReadDssBackFaceParam (propName, propValue, desc);   break;
            case 'd':   ReadDssDepthParam    (propName, propValue, desc);   break;
            case 'f':   ReadDssFrontFaceParam(propName, propValue, desc);   break;
            case 's':   ReadDssStencilParam  (propName, propValue, desc);   break;
        }
    }
}

//---------------------------------------------------------
// Desc:  setup parameter for depth-stencil operation related to back faces
//---------------------------------------------------------
void RenderStateReader::ReadDssBackFaceParam(
    const char* propName,
    const char* propValue,
    D3D11_DEPTH_STENCIL_DESC& desc)
{
    assert(propName && propName[0] != '\0');
    assert(propValue && propValue[0] != '\0');

    if (strcmp(propName, "back_face.s_fail_op") == 0)
        desc.BackFace.StencilFailOp = GetStencilOp(propValue);
    
    else if (strcmp(propName, "back_face.s_depth_fail_op") == 0)
        desc.BackFace.StencilDepthFailOp = GetStencilOp(propValue);
    
    else if (strcmp(propName, "back_face.s_pass_op") == 0)
        desc.BackFace.StencilPassOp = GetStencilOp(propValue);

    else if (strcmp(propName, "back_face.s_func") == 0)
        desc.BackFace.StencilFunc = GetCmpFunc(propValue);

    else
        // somewhere we crapped ourself 
        assert(0 && "failed to read dss back face param");
}

//---------------------------------------------------------
// Desc:  get depth parameter for depth-stencil description
//---------------------------------------------------------
void RenderStateReader::ReadDssDepthParam(
    const char* propName,
    const char* propValue,
    D3D11_DEPTH_STENCIL_DESC& desc)
{
    assert(propName && propName[0] != '\0');
    assert(propValue && propValue[0] != '\0');

    if (strcmp(propName, "d_enabled") == 0)
        desc.DepthEnable = GetBool(propValue);

    else if (strcmp(propName, "d_write_mask") == 0)
        desc.DepthWriteMask = GetDepthWriteMask(propValue);
    
    else if (strcmp(propName, "d_func") == 0)
        desc.DepthFunc = GetCmpFunc(propValue);

    else
        // somewhere we crapped ourself 
        assert(0 && "failed to read dss depth param");
}

//---------------------------------------------------------
// Desc:  setup parameter for depth-stencil operation related to front faces
//---------------------------------------------------------
void RenderStateReader::ReadDssFrontFaceParam(
    const char* propName,
    const char* propValue,
    D3D11_DEPTH_STENCIL_DESC& desc)
{
    assert(propName && propName[0] != '\0');
    assert(propValue && propValue[0] != '\0');

    if (strcmp(propName, "front_face.s_fail_op") == 0)
        desc.FrontFace.StencilFailOp = GetStencilOp(propValue);
    
    else if (strcmp(propName, "front_face.s_depth_fail_op") == 0)
        desc.FrontFace.StencilDepthFailOp = GetStencilOp(propValue);
    
    else if (strcmp(propName, "front_face.s_pass_op") == 0)
        desc.FrontFace.StencilPassOp = GetStencilOp(propValue);
    
    else if (strcmp(propName, "front_face.s_func") == 0)
        desc.FrontFace.StencilFunc = GetCmpFunc(propValue);
    
    else
        // somewhere we crapped ourself 
        assert(0 && "failed to read dss front face param");
}

//---------------------------------------------------------
// Desc:  get stencil parameter for depth-stencil description
//---------------------------------------------------------
void RenderStateReader::ReadDssStencilParam(
    const char* propName,
    const char* propValue,
    D3D11_DEPTH_STENCIL_DESC& desc)
{
    assert(propName && propName[0] != '\0');
    assert(propValue && propValue[0] != '\0');

    if (strcmp(propName, "s_enabled") == 0)
    {
        desc.StencilEnable = GetBool(propValue);
    }
    else if (strcmp(propName, "s_read_mask") == 0)
    {
        int count = sscanf(propValue, "%" SCNx8, &desc.StencilReadMask);
        assert(count == 1);
    }
    else if (strcmp(propName, "s_write_mask") == 0)
    {
        int count = sscanf(propValue, "%" SCNx8, &desc.StencilWriteMask);
        assert(count == 1);
    }
    else
    {
        // somewhere we crapped ourself 
        assert(0 && "failed to read dss stencil param");
    }
}

//---------------------------------------------------------
// Desc:  get stencil operation by input str
//---------------------------------------------------------
D3D11_STENCIL_OP RenderStateReader::GetStencilOp(const char* op)
{
    assert(op && op[0] != '\0');

    switch (op[0])
    {
        // decr_sat, decr
        case 'd':
            if (strcmp(op, "decr_sat") == 0)
                return D3D11_STENCIL_OP_DECR_SAT;

            if (strcmp(op, "decr") == 0)
                return D3D11_STENCIL_OP_DECR;

            break;

        // incr_sat, invert, incr
        case 'i':
            if (strcmp(op, "incr_sat") == 0)
                return D3D11_STENCIL_OP_INCR_SAT;

            if (strcmp(op, "incr") == 0)
                return D3D11_STENCIL_OP_INCR;

            if (strcmp(op, "invert") == 0)
                return D3D11_STENCIL_OP_INVERT;

            break;

        case 'k':  return D3D11_STENCIL_OP_KEEP;
        case 'r':  return D3D11_STENCIL_OP_REPLACE;
        case 'z':  return D3D11_STENCIL_OP_ZERO;
    }

    // somewhere we crapped ourself 
    LogErr(LOG, "failed to read stencil operation: %s", op);
    return D3D11_STENCIL_OP_KEEP;
}

//---------------------------------------------------------
// Desc:  get comparison function by input str
//---------------------------------------------------------
D3D11_COMPARISON_FUNC RenderStateReader::GetCmpFunc(const char* func)
{
    assert(func && func[0] != '\0');

    switch (func[0])
    {
        case 'a':
            if (strcmp(func, "always") == 0)
                return D3D11_COMPARISON_ALWAYS;
            break;

        case 'e':
            if (strcmp(func, "equal") == 0)
                return D3D11_COMPARISON_EQUAL;
            break;

        // greater, greater_equal
        case 'g':
            if (strcmp(func, "greater") == 0)
                return D3D11_COMPARISON_GREATER;

            if (strcmp(func, "greater_equal") == 0)
                return D3D11_COMPARISON_GREATER_EQUAL;

            break;

        // less, less_equal
        case 'l':
            if (strcmp(func, "less") == 0)
                return D3D11_COMPARISON_LESS;

            if (strcmp(func, "less_equal") == 0)
                return D3D11_COMPARISON_LESS_EQUAL;

            break;

        // never, not_equal
        case 'n':
            if (strcmp(func, "never") == 0)
                return D3D11_COMPARISON_NEVER;

            if (strcmp(func, "not_equal") == 0)
                return D3D11_COMPARISON_NOT_EQUAL;

            break;
    }

    // somewhere we crapped ourself 
    LogErr(LOG, "failed to read comparison func: %s", func);
    return D3D11_COMPARISON_EQUAL;
}

//---------------------------------------------------------
// Desc:  return depth write mask by input str
//---------------------------------------------------------
D3D11_DEPTH_WRITE_MASK RenderStateReader::GetDepthWriteMask(const char* mask)
{
    assert(mask && mask[0] != '\0');

    if (strcmp(mask, "zero") == 0)
        return D3D11_DEPTH_WRITE_MASK_ZERO;

    if (strcmp(mask, "all") == 0)
        return D3D11_DEPTH_WRITE_MASK_ALL;

    // somewhere we crapped ourself 
    LogErr(LOG, "failed to read depth write mask: %s", mask);
    return D3D11_DEPTH_WRITE_MASK_ALL;
}


} // namespace
