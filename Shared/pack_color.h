#pragma once

#include <stdint.h>

//---------------------------------------------------------
// helper for packing RGBA color into uint32 (ABGR)
//---------------------------------------------------------
inline void PackRGBA(
    const float r,
    const float g,
    const float b,
    const float a,
    uint32_t& outColor)
{
    uint32_t R = (uint32_t)(r * 255.0f);
    uint32_t G = (uint32_t)(g * 255.0f);
    uint32_t B = (uint32_t)(b * 255.0f);
    uint32_t A = (uint32_t)(a * 255.0f);

    outColor = 0;
    outColor = (R) | (G << 8) | (B << 16) | (A << 24);
}

//---------------------------------------------------------
// helper for unpacking color from uint32 (ABGR) to float4 (RGBA)
//---------------------------------------------------------
inline void UnpackRGBA(
    const uint32_t c,
    float& r,
    float& g,
    float& b,
    float& a)
{
    // 1.0 / 255.0 == 0.003921f

    r = ((c)       & 0xFF) * 0.003921f;
    g = ((c >> 8)  & 0xFF) * 0.003921f;
    b = ((c >> 16) & 0xFF) * 0.003921f;
    a = ((c >> 24) & 0xFF) * 0.003921f;
}
