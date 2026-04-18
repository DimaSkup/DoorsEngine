//==================================================================================
// Desc:  a pixel shader for a night vision (green filter + noise) post effect
//==================================================================================
#include "const_buffers/cb_time.hlsli"
#include "helpers/noise_rand.hlsli"
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
	float4 texColor = gScreenTex.Sample(gSamLinearClamp, pin.tex);
	
	const float resolutionY = GetPostFxParam(POST_FX_PARAM_SCREEN_HEIGHT);
	const float scanline    = 0.2;
	
    const float n   = rand(pin.tex * gGameTime) * 0.1;
    const float lum = dot(texColor.rgb, float3(0.299, 0.587, 0.114));
	
	float4 color = float4(lum * 0.7, lum * 0.8 + n, (lum + n) * 0.6, 1);
	
	// add scanlines
	float scan = sin(pin.tex.y * resolutionY ) * 0.5;
	color *= (1.0 - scanline * (0.25 + 0.75 * scan));

	return color;
}
