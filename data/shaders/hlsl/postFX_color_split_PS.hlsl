//==================================================================================
// Desc:  a pixel shader for a color split (chromatic aberration - strong) post effect
//==================================================================================
#include "const_buffers/cbps_post_fx2.hlsli"
#include "const_buffers/post_fx_params_enum.hlsli"

//---------------------------
// GLOBALS
//---------------------------
Texture2D    gScreenTex      : register(t12);
SamplerState gSamLinearClamp : register(s2);

// clamp samples to a small compile-time maximum for loops
static const int MAX_SAMPLES = 8;

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

//---------------------------
// HELPER: linear interpolate and average N samples btw uv and uv + offset
//---------------------------
float4 SampleAlong(const float2 uv0, const float2 uv1, int samples)
{
    // simple box sampling along the segment
    float4 accum = float4(0, 0, 0, 0);

    if (samples == 1)
    {
        return gScreenTex.Sample(gSamLinearClamp, uv1);
    }
    else
    {
        const float invSamples = 1.0 / (samples - 1);

        // samples evenly spaced including endpoint
        for (int i = 0; i < samples; ++i)
        {
            float t = (i == 1) ? 1.0 : (float(i) * invSamples); // 0..1
            float2 uv = lerp(uv0, uv1, t);
            accum += gScreenTex.Sample(gSamLinearClamp, uv);
        }

        return accum / samples;
    }
}

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float cx          = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_CX);
    const float cy          = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_CY);
    const float intensity   = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_INTENSITY);
    const float radialPower = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_RADIAL_POWER);
    const float chromaMulR  = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_R);
    const float chromaMulG  = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_G);
    const float chromaMulB  = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_B);
    const float samples     = GetPostFxParam(POST_FX_PARAM_COLOR_SPLIT_SAMPLES);


    float2 uv = pin.tex;

    // direction from center, normalized
    float2 dir  = uv - float2(cx, cy);
    float  dist = length(dir);   // 0..~0.707 for corners if center=(0.5,0.5)
    float2 dirN = (dist > 1e-6) ? (dir / dist) : float2(0,0);

    // radial-based displacement magnitude
    // stronger at edges. Tweak intensity and radialPower for look
    float disp = intensity * pow(abs(dist * radialPower), intensity);

    // compute per-channel target UVs
    float2 uvR = uv + dirN * (disp * chromaMulR);
    float2 uvG = uv + dirN * (disp * chromaMulG);
    float2 uvB = uv + dirN * (disp * chromaMulB);
	
	// clamp UVs slightly to avoid sampling outside - optional
    // (let the sampler address mode handle edges if desired)
    // uvR = clamp(uvR, float2(0.0,0.0), float2(1.0,1.0));
    // uvG = clamp(uvG, float2(0.0,0.0), float2(1.0,1.0));
    // uvB = clamp(uvB, float2(0.0,0.0), float2(1.0,1.0));

    // sample count
    int sampleCount = (int)round(clamp(samples, 1.0, (float)MAX_SAMPLES));

    // optionally smooth each channel by sampling along the segment from uv to uvX
    float4 colR = SampleAlong(uv, uvR, sampleCount);
    float4 colG = SampleAlong(uv, uvG, sampleCount);
    float4 colB = SampleAlong(uv, uvB, sampleCount);

    // assemble final color
    float3 color = float3(colR.r, colG.g, colB.b);

    // preserve alpha (take average alpha)
    float alpha = (colR.a + colG.a + colB.a) * 0.333;

    return float4(color, alpha);
}
