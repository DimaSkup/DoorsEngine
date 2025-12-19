//==================================================================================
// Desc:  a pixel shader for a negative glow (inverted bloom) post effect
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
// HELPER: sample neighborhood brightness
//---------------------------
float3 SampleGlow(float2 uv)
{
    const float texelSzX = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_X);
    const float texelSzY = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_Y);

    const float kernel[9] = { 0.05, 0.09, 0.12, 0.15, 0.12, 0.09, 0.05, 0.03, 0.02 };
	float3 sum = 0.0;
	
	[unroll]
	for (int i = -4; i <= 4; ++i)
	{
        float2 tex = uv + float2(texelSzX * i, 0);
		sum += gScreenTex.Sample(gSamLinearClamp, tex).rgb * kernel[i+4];
	}
	
	[unroll]
	for (int j = -4; j <= 4; ++j)
	{
		float2 tex = uv + float2(0, texelSzY * j);
		sum += gScreenTex.Sample(gSamLinearClamp, tex).rgb * kernel[j+4];
	}
	
	return sum;
}

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float3 color = gScreenTex.Sample(gSamLinearClamp, pin.tex).rgb;

    // sample a blurred neighborhood to get glow influence
    const float3 glow = SampleGlow(pin.tex);

	// calc brightness of curr pixel
	const float brightness = dot(color, float3(0.299, 0.587, 0.114));
	
	// invert bright regions for "negative" glow
	const float invGlow = saturate(1.0 - brightness);
	
	// apply threshold and intensity
    const float threshold = GetPostFxParam(POST_FX_PARAM_NEGATIVE_GLOW_THRESHOLD);
    const float strength  = GetPostFxParam(POST_FX_PARAM_NEGATIVE_GLOW_STRENGTH);

	const float3 glowMask = smoothstep(threshold, 1.0, invGlow);
	const float3 result = color + glow * glowMask * strength;
	
	return float4(saturate(result), 1.0);
}
