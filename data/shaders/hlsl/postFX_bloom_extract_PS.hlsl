//==================================================================================
// Desc:  a pixel shader for a bloom (extract bright areas) post effect
//        (this is usually the first pass of a bloom pipeline)
//==================================================================================
#include "const_buffers/cbps_post_fx2.hlsli"
#include "const_buffers/post_fx_params_enum.hlsli"

//---------------------------
// GLOBALS
//---------------------------
Texture2D    gScreenTex      : register(t12);
SamplerState gSamLinearClamp : register(s2);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    float4 color          = gScreenTex.Sample(gSamLinearClamp, pin.tex);
    float  brightness     = max(max(color.r, color.g), color.b);
    float  bloomThreshold = GetPostFxParam(POST_FX_PARAM_BLOOM_THRESHOLD);

    return (brightness > bloomThreshold) ? color : float4(0, 0, 0, 1);

    // and then blur this bright pass and add it back to the original image
}
