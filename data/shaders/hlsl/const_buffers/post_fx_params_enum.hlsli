//---------------------------------------------------------
// post effects parameters
//---------------------------------------------------------

#ifndef __cplusplus

// common
#define POST_FX_PARAM_SCREEN_WIDTH 0                     // viewport dimensions
#define POST_FX_PARAM_SCREEN_HEIGHT 1
#define POST_FX_PARAM_TEXEL_SIZE_X 2                     // (1.0 / screen_width, 1.0 / screen_height)
#define POST_FX_PARAM_TEXEL_SIZE_Y 3

// shockwave distortion
#define POST_FX_PARAM_SHOCKWAVE_DISTORT_CX 4             // shockwave origin in UV (0..1)
#define POST_FX_PARAM_SHOCKWAVE_DISTORT_CY 5             // shockwave origin in UV (0..1)
#define POST_FX_PARAM_SHOCKWAVE_DISTORT_SPEED 6          // speed of wave expansion (e.g. 1.0)
#define POST_FX_PARAM_SHOCKWAVE_DISTORT_THICKNESS 7      // ring thickness in UV units (e.g. 0.1)
#define POST_FX_PARAM_SHOCKWAVE_DISTORT_AMPLITUDE 8      // distortion strength (e.g. 0.05)
#define POST_FX_PARAM_SHOCKWAVE_DISTORT_SHARPNESS 9      // how sharp the ring edge is (e.g. 4-8)

// glitch
#define POST_FX_PARAM_GLITCH_INTENSITY   10              // 0-1, global glitch strength
#define POST_FX_PARAM_GLITCH_COLOR_SPLIT 11              // RGB channel split strength (e.g. 1.5-4.0)
#define POST_FX_PARAM_GLITCH_BLOCK_SIZE  12              // pixel block size for displacement (e.g. 0.05)
#define POST_FX_PARAM_GLITCH_SPEED 13                     // glitch speed
#define POST_FX_PARAM_GLITCH_SCANLINE 14                  // scanline intensity (0-1)
#define POST_FX_PARAM_GLITCH_NOISE_AMOUNT 15              // random flicker intensity (0-1)

// posterization
#define POST_FX_PARAM_POSTERIZATION_LEVELS 16

// bloom extract
#define POST_FX_PARAM_BLOOM_THRESHOLD 17

// brightness/contrast adjust
#define POST_FX_PARAM_BRIGHTNESS 18                       // increases brightness
#define POST_FX_PARAM_CONTRAST 19                         // increases contrast

// chromatic aberration - simple
#define POST_FX_PARAM_CHROMATIC_ABERRATION_STRENGTH 20

#define POST_FX_PARAM_COLOR_SHIFT_HUE 21                  // in radians

// color split (chromatic aberration - strong)
#define POST_FX_PARAM_COLOR_SPLIT_CX 22                   // center of aberration in UV (0..1) - default (0.5, 0.5)
#define POST_FX_PARAM_COLOR_SPLIT_CY 23
#define POST_FX_PARAM_COLOR_SPLIT_INTENSITY 24            // base strength (0..1). Strong default ~0.03-0.08
#define POST_FX_PARAM_COLOR_SPLIT_RADIAL_POWER 25         // how dispersion grows with radius (>=0). 1 = linear
#define POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_R 26         // per-channel multiplier for displacement (RGB)
#define POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_G 27
#define POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_B 28
#define POST_FX_PARAM_COLOR_SPLIT_SAMPLES 29              // smoothing samples (1..8). 1 = no smoothing

// color tint
#define POST_FX_PARAM_COLOR_TINT_R 30                     // e.g. float3(1.0, 0.5, 0.5) for red tint
#define POST_FX_PARAM_COLOR_TINT_G 31
#define POST_FX_PARAM_COLOR_TINT_B 32
#define POST_FX_PARAM_COLOR_TINT_INTENSITY 33             // 0 = no tint, 1 = full tint

// CRT curvature + scanlines
#define POST_FX_PARAM_CRT_CURVATURE 34                    // e.g. 0.2

// film grain
#define POST_FX_PARAM_FILM_GRAIN_STRENGTH 35              // e.g. 0.3

// frosten glass blur
#define POST_FX_PARAM_FROST_GLASS_BLUR_STRENGTH 36        // e.g. 5.0

// heat distortion
#define POST_FX_PARAM_HEAT_DISTORT_STRENGTH 37            // e.g. 0.01

// negative glow
#define POST_FX_PARAM_NEGATIVE_GLOW_STRENGTH 38   // e.g. 0.5
#define POST_FX_PARAM_NEGATIVE_GLOW_THRESHOLD 39  // e.g. 0.7

// pixelation
#define POST_FX_PARAM_PIXELATION_PIXEL_SIZE 40    // e.g. 4.0

// radial blur
#define POST_FX_PARAM_RADIAL_BLUR_CX 41           // horizon center in UV (0..1)
#define POST_FX_PARAM_RADIAL_BLUR_CY 42           // vertical center in UV (0..1)
#define POST_FX_PARAM_RADIAL_BLUR_SAMPLES 43      // e.g. 10
#define POST_FX_PARAM_RADIAL_BLUR_STRENGTH 44     // e.g. 0.02

// swirl distortion
#define POST_FX_PARAM_SWIRL_DISTORT_CX 45
#define POST_FX_PARAM_SWIRL_DISTORT_CY 46
#define POST_FX_PARAM_SWIRL_DISTORT_RADIUS 47
#define POST_FX_PARAM_SWIRL_DISTORT_ANGLE 48     // in radians

// old TV distortion
#define POST_FX_PARAM_OLD_TV_DISTORT_STRENGTH 49       // e.g. 0.003

// vignette effect
#define POST_FX_PARAM_VIGNETTE_STRENGTH 50  // e.g. 0.75

#endif // !defined(__cplusplus)

#ifdef __cplusplus

enum ePostFxParam
{
    // common
    POST_FX_PARAM_SCREEN_WIDTH,                     // viewport dimensions
    POST_FX_PARAM_SCREEN_HEIGHT,
    POST_FX_PARAM_TEXEL_SIZE_X,                     // (1.0 / screen_width, 1.0 / screen_height)
    POST_FX_PARAM_TEXEL_SIZE_Y,

    // shockwave distortion
    POST_FX_PARAM_SHOCKWAVE_DISTORT_CX,             // shockwave origin in UV (0..1)
    POST_FX_PARAM_SHOCKWAVE_DISTORT_CY,             // shockwave origin in UV (0..1)
    POST_FX_PARAM_SHOCKWAVE_DISTORT_SPEED,          // speed of wave expansion (e.g. 1.0)
    POST_FX_PARAM_SHOCKWAVE_DISTORT_THICKNESS,      // ring thickness in UV units (e.g. 0.1)
    POST_FX_PARAM_SHOCKWAVE_DISTORT_AMPLITUDE,      // distortion strength (e.g. 0.05)
    POST_FX_PARAM_SHOCKWAVE_DISTORT_SHARPNESS,      // how sharp the ring edge is (e.g. 4-8)

    // glitch
    POST_FX_PARAM_GLITCH_INTENSITY,                 // 0-1, global glitch strength
    POST_FX_PARAM_GLITCH_COLOR_SPLIT,               // RGB channel split strength (e.g. 1.5-4.0)
    POST_FX_PARAM_GLITCH_BLOCK_SIZE,                // pixel block size for displacement (e.g. 0.05)
    POST_FX_PARAM_GLITCH_SPEED,                     // glitch speed
    POST_FX_PARAM_GLITCH_SCANLINE,                  // scanline intensity (0-1)
    POST_FX_PARAM_GLITCH_NOISE_AMOUNT,              // random flicker intensity (0-1)

    // posterization
    POST_FX_PARAM_POSTERIZATION_LEVELS,

    // bloom extract
    POST_FX_PARAM_BLOOM_THRESHOLD,

    // brightness/contrast adjust
    POST_FX_PARAM_BRIGHTNESS,                       // increases brightness
    POST_FX_PARAM_CONTRAST,                         // increases contrast

    // chromatic aberration - simple
    POST_FX_PARAM_CHROMATIC_ABERRATION_STRENGTH,

    POST_FX_PARAM_COLOR_SHIFT_HUE,                  // in radians

    // color split (chromatic aberration - strong)
    POST_FX_PARAM_COLOR_SPLIT_CX,                   // center of aberration in UV (0..1) - default (0.5, 0.5)
    POST_FX_PARAM_COLOR_SPLIT_CY,
    POST_FX_PARAM_COLOR_SPLIT_INTENSITY,            // base strength (0..1). Strong default ~0.03-0.08
    POST_FX_PARAM_COLOR_SPLIT_RADIAL_POWER,         // how dispersion grows with radius (>=0). 1 = linear
    POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_R,         // per-channel multiplier for displacement (RGB)
    POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_G,
    POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_B,
    POST_FX_PARAM_COLOR_SPLIT_SAMPLES,              // smoothing samples (1..8). 1 = no smoothing

    // color tint
    POST_FX_PARAM_COLOR_TINT_R,                     // e.g. float3(1.0, 0.5, 0.5) for red tint
    POST_FX_PARAM_COLOR_TINT_G,
    POST_FX_PARAM_COLOR_TINT_B,
    POST_FX_PARAM_COLOR_TINT_INTENSITY,             // 0 = no tint, 1 = full tint

    // CRT curvature + scanlines
    POST_FX_PARAM_CRT_CURVATURE,                    // e.g. 0.2

    // film grain
    POST_FX_PARAM_FILM_GRAIN_STRENGTH,              // e.g. 0.3

    // frosten glass blur
    POST_FX_PARAM_FROST_GLASS_BLUR_STRENGTH,        // e.g. 5.0

    // heat distortion
    POST_FX_PARAM_HEAT_DISTORT_STRENGTH,            // e.g. 0.01

    // negative glow
    POST_FX_PARAM_NEGATIVE_GLOW_STRENGTH,   // e.g. 0.5
    POST_FX_PARAM_NEGATIVE_GLOW_THRESHOLD,  // e.g. 0.7

    // pixelation
    POST_FX_PARAM_PIXELATION_PIXEL_SIZE,    // e.g. 4.0

    // radial blur
    POST_FX_PARAM_RADIAL_BLUR_CX,           // horizon center in UV (0..1)
    POST_FX_PARAM_RADIAL_BLUR_CY,           // vertical center in UV (0..1)
    POST_FX_PARAM_RADIAL_BLUR_SAMPLES,      // e.g. 10
    POST_FX_PARAM_RADIAL_BLUR_STRENGTH,     // e.g. 0.02

    // swirl distortion
    POST_FX_PARAM_SWIRL_DISTORT_CX,
    POST_FX_PARAM_SWIRL_DISTORT_CY,
    POST_FX_PARAM_SWIRL_DISTORT_RADIUS,
    POST_FX_PARAM_SWIRL_DISTORT_ANGLE,      // in radians

    // old TV distortion
    POST_FX_PARAM_OLD_TV_DISTORT_STRENGTH,        // e.g. 0.003

    // vignette effect
    POST_FX_PARAM_VIGNETTE_STRENGTH,        // e.g. 0.75

    NUM_POST_FX_PARAMS
};

#endif // defined(__cplusplus)
