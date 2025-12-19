//==================================================================================
// Desc:  a pixel shader for a shockwave distortion post effect
//==================================================================================
#include "const_buffers/cb_time.hlsli"
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
    const float  cx        = GetPostFxParam(POST_FX_PARAM_SHOCKWAVE_DISTORT_CX);
    const float  cy        = GetPostFxParam(POST_FX_PARAM_SHOCKWAVE_DISTORT_CY);
    const float  speed     = GetPostFxParam(POST_FX_PARAM_SHOCKWAVE_DISTORT_SPEED);
    const float  thickness = GetPostFxParam(POST_FX_PARAM_SHOCKWAVE_DISTORT_THICKNESS);
    const float  amplitude = GetPostFxParam(POST_FX_PARAM_SHOCKWAVE_DISTORT_AMPLITUDE);
    const float  sharpness = GetPostFxParam(POST_FX_PARAM_SHOCKWAVE_DISTORT_SHARPNESS);

    const float fade = sin(gGameTime);        // fade out (0..1)
	float2 uv = pin.tex;
	float2 dir = uv - float2(cx, cy);
	float dist = length(dir);
	
	// current radius of the expanding ring
	float radius = sin(gGameTime * speed);
	
	// radial distance from the wavefront
	float delta = dist - radius;
	
	// Gaussian-style falloff for ring profile
	float wave = exp(-pow(abs(delta / thickness), sharpness));
	
	// optional fade (multiply amplitude down over time)
	float amp = amplitude * (1.0 - fade);
	
	// offset direction (normalized)
	float2 dirN = (dist > 1e-6) ? normalize(dir) : float2(0,0);
	
	// apply radial distorion (outward or inward depending on sign)
	float2 offset = dirN * wave * amp;
	
	// Refractive Wave + Chromatic Edge
	// (Add a slight RGB split only on the ring edge)
	float2 uvR = uv + offset * 1.0;
	float2 uvG = uv + offset * 0.8;
	float2 uvB = uv + offset * 0.6;
	float3 col;
	col.r = gScreenTex.Sample(gSamLinearClamp, uvR).r;
	col.g = gScreenTex.Sample(gSamLinearClamp, uvG).g;
	col.b = gScreenTex.Sample(gSamLinearClamp, uvB).b;
	float4 color = float4(col, 1.0);
	
	return color;
	
	// sample distorted color (without refraction + chromatic edge)
	return gScreenTex.Sample(gSamLinearClamp, uv + offset);
}
