//==================================================================================
// Desc:  a pixel shader for FXAA (Fast Approximate Anti-Aliasing)
// 
//        it is used as post-processing shader that smooths out jagged edges after
//        rendering your scene. FXAA doesn't require multi-sampling (MSAA) and
//        works entirely as a screen-space filter
//==================================================================================
#include "const_buffers/cbps_post_fx2.hlsli"
#include "const_buffers/post_fx_params_enum.hlsli"

//---------------------------
// GLOBALS
//---------------------------
Texture2D    gScreenTex    : register(t12);
SamplerState gSamAnisotrop : register(s4);


// The minimum amount of local contrast required to apply algorithm
//   1/3  - too little
//   1/4  - low quality
//   1/8  - high quality
//   1/16 - overkill    
#define FXAA_EDGE_THRESHOLD (1.0 / 8.0)

// trims the algorithm from processing darks
//   1/32 - visible limit
//   1/16 - high quality
//   1/12 - upper limit (start of visible unfiltered edges)
#define FXAA_EDGE_THRESHOLD_MIN (1.0 / 24.0)

// controls removal of sub-pixel aliasing:
//   1/2 - low removal
//   1/3 - medium removal
//   1/4 - default removal
//   1/8 - high removal
//   0   - complete removal
#define FXAA_SUBPIX_TRIM (1.0 / 4.0)

// insures fine detail is not completely removed
// (this partly overrides FXAA_SUBPIX_TRIM)
//   3/4 - default amount of filtering
//   7/8 - high amount of filtering
//   1   - no capping of filtering
#define FXAA_SUBPIX_CAP        (3.0/4.0)
#define FXAA_SUBPIX_TRIM_SCALE (1.0 / (1.0 - FXAA_SUBPIX_TRIM))

// controls the max number of search steps
#define FXAA_SEARCH_STEPS  32

// how much to accelerate search using anisotropic filtering
//   1 - no acceleration
//   2 - skip by 2 pixels
//   3 - skip by 3 pixels
//   4 - skip by 4 pixels (hard upper limit)
#define FXAA_SEARCH_ACCELERATION 1

// controls when to stop searching
//   1/4 - seems best quality wise
#define FXAA_SEARCH_THRESHOLD (1.0 / 4.0)



//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

//---------------------------
// calc luminance: is estimated strictly from Red and Green channels
// using a single fused multiply add operation. In practice pure blue
// aliasing rarely appears in typical game content
//---------------------------
float FxaaLuma(float3 rgb)
{
    return dot(rgb, float3(0.299, 0.587, 0.114));
    return rgb.y * (0.587 / 0.299) + rgb.x;
}

//---------------------------
//---------------------------
float4 FxaaTexLod0(const float2 uv)
{
    return gScreenTex.SampleLevel(gSamAnisotrop, uv, 0.0);
}

//---------------------------
//---------------------------
float3 FxaaLerp3(float3 a, float3 b, float amountOfA)
{
    return (float3(-amountOfA, 0, 0) * b) + ((a * float3(amountOfA, 0, 0)) + b);
}

//---------------------------
//---------------------------
float4 FxaaTexOff(const float2 uv, const int2 off)
{
    return gScreenTex.SampleLevel(gSamAnisotrop, uv, 0.0, off);
}

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float texelX = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_X);
    const float texelY = GetPostFxParam(POST_FX_PARAM_TEXEL_SIZE_Y);

    const float2 rcpFrame = float2(texelX, texelY);
    const float2 uv = pin.tex;

    // North, South, East, West
    const float3 rgbN = FxaaTexOff(uv, int2( 0, -1)).rgb;
    const float3 rgbW = FxaaTexOff(uv, int2(-1,  0)).rgb;
    const float3 rgbM = FxaaTexOff(uv, int2( 0,  0)).rgb;
    const float3 rgbE = FxaaTexOff(uv, int2( 1,  0)).rgb;
    const float3 rgbS = FxaaTexOff(uv, int2( 0,  1)).rgb;

    // luma - luminance
    float lumaN = FxaaLuma(rgbN);
    float lumaW = FxaaLuma(rgbW);
    float lumaM = FxaaLuma(rgbM);
    float lumaE = FxaaLuma(rgbE);
    float lumaS = FxaaLuma(rgbS);

    float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));

    float range = rangeMax - rangeMin;

    // if the difference in local max and min luma (contrast) is lower than
    // a threshold proportional to the maximum local luma, then the shader
    // early exits (no visible aliasing). This threshold is clamped at a min value
    // to avoid processing in really dark areas
    if (range < max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD))
    {
        return float4(rgbM, 1.0);
    }
 

    // --- 2. Sub-pixel aliasing test ---
    float lumaL  = (lumaN + lumaW + lumaE + lumaS) * 0.25;
    float rangeL = abs(lumaL - lumaM);
    float blendL = max(0.0, (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
    blendL = min(FXAA_SUBPIX_CAP, blendL);

    // lowpass value used to filter sub-pixel aliasing at the end of the algorithm 
    // is a box filter of the complete 3x3 pixel neighborhood
    float3 rgbNW = FxaaTexOff(uv, int2(-1, -1)).rgb;
    float3 rgbNE = FxaaTexOff(uv, int2( 1, -1)).rgb;
    float3 rgbSW = FxaaTexOff(uv, int2(-1,  1)).rgb;
    float3 rgbSE = FxaaTexOff(uv, int2( 1,  1)).rgb;

    float3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
    rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
    rgbL *= float3(1.0 / 9.0, 0, 0);


    // --- 3. Vertical/horizontal edge test ---
    float lumaNW = FxaaLuma(rgbNW);
    float lumaNE = FxaaLuma(rgbNE);
    float lumaSW = FxaaLuma(rgbSW);
    float lumaSE = FxaaLuma(rgbSE);

    float edgeVert =
        abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
        abs((0.50 * lumaW)  + (-1.0 * lumaM) + (0.50 * lumaE)) +
        abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));

    float edgeHoriz =
        abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
        abs((0.50 * lumaN)  + (-1.0 * lumaM) + (0.50 * lumaS)) +
        abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));

    bool horizSpan = (edgeHoriz >= edgeVert);

    float lengthSign = horizSpan ? -rcpFrame.y : -rcpFrame.x;

    if (!horizSpan)
    {
        lumaN = lumaW;
        lumaS = lumaE;
    }
	
    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);
    lumaN = (lumaN + lumaM) * 0.5;
    lumaS = (lumaS + lumaM) * 0.5;


    // choose side of pixel where gradient is highest
    bool pairN = (gradientN >= gradientS);

    if (!pairN)
    {
        lumaN = lumaS;
        gradientN = gradientS;
        lengthSign *= -1.0;
    }

    float2 posN = uv;
	
    if (horizSpan)
        posN.y += lengthSign * 0.5;
    else
        posN.x += lengthSign * 0.5;
	

    // choose search limit values
    gradientN *= FXAA_SEARCH_THRESHOLD;

    // search in both direction until find luma pair average is out of range
    float2 posP = posN;
    float2 offNP = (horizSpan) ? float2(rcpFrame.x, 0) : float2(0, rcpFrame.y);

    float lumaEndN = lumaN;
    float lumaEndP = lumaN;

    bool doneN = false;
    bool doneP = false;

#if FXAA_SEARCH_ACCELERATION == 1
    posN -= offNP;
    posP += offNP;
#endif

    for (int i = 0; i < FXAA_SEARCH_STEPS; ++i)
    {
#if FXAA_SEARCH_ACCELERATION == 1
        if (!doneN)  lumaEndN = FxaaLuma(FxaaTexLod0(posN).rgb);
        if (!doneP)  lumaEndP = FxaaLuma(FxaaTexLod0(posP).rgb);
#else
        if (!doneN)  lumaEndN = FxaaLuma(FxaaTexGrad(uv, offNP).rgb);
        if (!doneP)  lumaEndP = FxaaLuma(FxaaTexGrad(uv, offNP).rgb);
#endif

        doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
        doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);

        if (doneN && doneP)  break;

        if (!doneN)  posN -= offNP;
        if (!doneP)  posP += offNP;
    }



    // handle if center is on positive or negative side
    float dstN;
    float dstP;

    if (horizSpan)
    {
        dstN = uv.x - posN.x;
        dstP = posP.x - uv.x;
    }
    else
    {
        dstN = uv.y - posN.y;
        dstP = posP.y - uv.y;
    }
   

    bool dirN = dstN < dstP;
    lumaEndN = (dirN) ? lumaEndN : lumaEndP;


    // check if pixel is in section of span which gets no filtering
    if (((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0))
        lengthSign = 0.0;

    float spanLen = (dstP + dstN);
    dstN = dirN ? dstN : dstP;
   
    float subPixelOff = (0.5 + (dstN * (-1.0 / spanLen))) * lengthSign;

    float2 rgbFuv = uv;

    if (horizSpan)
        rgbFuv.y += subPixelOff;
    else
        rgbFuv.x += subPixelOff;

    float3 rgbF = FxaaTexLod0(rgbFuv).rgb;
    float3 rgb  = FxaaLerp3(rgbL, rgbF, blendL);

	return float4(rgb, 1.0);
}
