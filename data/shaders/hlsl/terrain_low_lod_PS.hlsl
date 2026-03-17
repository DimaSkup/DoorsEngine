//==================================================================================
// Desc:  a pixel shader for terrain rendering of patches which are completely fogged
//==================================================================================


//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4   posH       : SV_POSITION;  // homogeneous position
    float4   fogColor   : COLOR;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    return pin.fogColor;
}
