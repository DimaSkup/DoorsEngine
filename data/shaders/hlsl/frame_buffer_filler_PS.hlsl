//==================================================================================
// Desc:  a pixel shader for filling a frame buffer with some color
//        (render a fullscreen plane)
//==================================================================================


//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;  // homogeneous position
    float2 tex  : TEXCOORD;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
