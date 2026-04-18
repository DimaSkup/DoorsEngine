//==================================================================================
// Desc:  a pixel shader for a film grain post effect
//        (adds subtle noise for a cinematic feel)
//==================================================================================
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cbps_post_fx2.hlsli"
#include "helpers/noise_rand.hlsli"
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
    float4 color = gScreenTex.Sample(gSamLinearClamp, pin.tex);

    const float grain    = rand(pin.tex * gGameTime) * 2.0 - 1.0;
    const float strength = GetPostFxParam(POST_FX_PARAM_FILM_GRAIN_STRENGTH);

    return saturate(color + grain * strength);
}
