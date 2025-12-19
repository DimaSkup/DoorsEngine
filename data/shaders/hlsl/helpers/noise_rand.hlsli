//==================================================================================
// helpers for different shaders: generation of rand values or noise
//==================================================================================

float hash21(float2 p)
{
    p = frac(p * float2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return frac(p.x * p.y);
}

//---------------------------

float noise(float3 p)
{
    // simple trilinear-interpolated values noise
    float3 i = floor(p);
    float3 f = frac(p);

    float n000 = hash21(i.xy + float2(0, 0) + i.z * 12.9898);
    float n100 = hash21(i.xy + float2(1, 0) + i.z * 12.9898);
    float n010 = hash21(i.xy + float2(0, 1) + i.z * 12.9898);
    float n110 = hash21(i.xy + float2(1, 1) + i.z * 12.9898);

    float nx00 = lerp(n000, n100, f.x);
    float nx10 = lerp(n010, n110, f.x);
    float nxy0 = lerp(nx00, nx10, f.y);

    // shift z layer
    float n001 = hash21(i.xy + float2(0, 0) + (i.z + 1) * 12.9898);
    float n101 = hash21(i.xy + float2(1, 0) + (i.z + 1) * 12.9898);
    float n011 = hash21(i.xy + float2(0, 1) + (i.z + 1) * 12.9898);
    float n111 = hash21(i.xy + float2(1, 1) + (i.z + 1) * 12.9898);

    float nx01 = lerp(n001, n101, f.x);
    float nx11 = lerp(n011, n111, f.x);
    float nxy1 = lerp(nx01, nx11, f.y);

    return lerp(nxy0, nxy1, f.z) * 2.0f - 1.0f;  // range [-1, 1]
}

//---------------------------
float noise(float2 co)
{
    return (frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453) - 0.5) * 0.05;
}

//---------------------------
// simple hash-based 2d random (fast, decent quality)
//---------------------------
float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

//---------------------------
// two-octave noise
//---------------------------
float noise2(float2 uv)
{
    float n = rand(uv);
    n += 0.5 * rand(uv * 1.7 + 17.0);
    return n * 0.66;
}
