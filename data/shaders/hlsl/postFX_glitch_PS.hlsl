//==================================================================================
// Desc:  a pixel shader for a "glitch" post effect
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
    const float resolutionY = GetPostFxParam(POST_FX_PARAM_SCREEN_HEIGHT);
    const float intensity   = GetPostFxParam(POST_FX_PARAM_GLITCH_INTENSITY);
    const float colorSplit  = GetPostFxParam(POST_FX_PARAM_GLITCH_COLOR_SPLIT);
    const float blockSize   = GetPostFxParam(POST_FX_PARAM_GLITCH_BLOCK_SIZE);
    const float speed       = GetPostFxParam(POST_FX_PARAM_GLITCH_SPEED);
    const float scanline    = GetPostFxParam(POST_FX_PARAM_GLITCH_SCANLINE);
    const float noiseAmount = GetPostFxParam(POST_FX_PARAM_GLITCH_NOISE_AMOUNT);

	float2 uv = pin.tex;

	
	// --- 1. Basic glitch blocks ---
	float block    = blockSize * (0.5 + 0.5 * sin(gGameTime * speed * 2.0));
	float2 blockUV = floor(uv / block) * block;
	float noise    = rand(blockUV + floor(gGameTime * speed));
	
	// create horizontal tearing based on noise threshold
	float glitchLine = step(0.8, noise) * (rand(float2(noise, gGameTime)) - 0.5) * 2.0;
	uv.x += glitchLine * intensity * 0.25;
	
	
	// --- 2. Vertical strip offset (simulate digital tearing) ---
	if (rand(float2(floor(uv.y * 20.0), floor(gGameTime * speed))) > 0.85)
	{
		uv.x += 0.1 * (rand(float2(uv.y, gGameTime)) - 0.5) * intensity;
	}
	
	
	// --- 3. Color split (RGB offset)) ---
	float2 chromaShift = float2(intensity * 0.005 * colorSplit, 0.0);
	float4 colR = gScreenTex.Sample(gSamLinearClamp, uv + chromaShift);
	float4 colG = gScreenTex.Sample(gSamLinearClamp, uv);
	float4 colB = gScreenTex.Sample(gSamLinearClamp, uv - chromaShift);
	
	float3 color = float3(colR.r, colG.g, colB.b);
	
	
	// --- 4. Add scanlines ---
	if (scanline > 0.0)
	{
		float scan = sin(uv.y * resolutionY * 1.5) * 0.5 + 0.5;
		color *= (1.0 - scanline * (0.25 + 0.75 * scan));
	}
	
	
	// --- 5. Add random flicker ---
	float flicker = (rand(float2(gGameTime, uv.y)) - 0.5) * noiseAmount * 0.2;
	color += flicker;
	
	
	return float4(saturate(color), 1.0);
}
