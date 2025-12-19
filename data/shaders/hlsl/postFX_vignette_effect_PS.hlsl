//==================================================================================
// Desc:  a pixel shader for a vignette post effect
//        (darkens the edges of the image)
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
	float4 color   = gScreenTex.Sample(gSamLinearClamp, pin.tex);
    float strength = GetPostFxParam(POST_FX_PARAM_VIGNETTE_STRENGTH);

    float2 dir = pin.tex - 0.5;
    float dist = length(dir);   // 0..~0.707 for corners if center=(0.5,0.5)
	
	 // smoothstep style vignette: 0 near center -> 1 near edges
	float vig = smoothstep(0.0, 1.0, dist * (1.0 + strength * 2.0));

	// invert so center = 1, edges darken
	float vignetteFactor = lerp(1.0, 1.0 - strength, vig);
    
    return color * vignetteFactor;
}
