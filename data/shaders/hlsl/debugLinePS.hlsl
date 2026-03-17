// =================================================================================
// Filename:  debugLinePS.hlsl
// Desc:      pixel shader for rendering debug shapes (AABB, OBB, spheres, etc.)
// =================================================================================

#define div_255_fast(x) (((x) * 32897) >> 23)
#define div_255(x) ((x) * 0.003921f)

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_INPUT
{
    float4 posH   : SV_POSITION;    // homogeneous position
    uint   color  : COLOR;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_INPUT pin) : SV_TARGET
{
    float r = div_255((pin.color >> 24) & 0xFF);
    float g = div_255((pin.color >> 16) & 0xFF);
    float b = div_255((pin.color >> 8)  & 0xFF);

    return float4(r, g, b, 1.0f);
};
