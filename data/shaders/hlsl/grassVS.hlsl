//==================================================================================
// Desc:  a vertex shader for grass rendering
//==================================================================================

//--------------------------------
// TYPEDEFS
//--------------------------------
struct VS_IN
{
    float3 posW : POSITION;
    float2 size : SIZE;
};

struct VS_OUT
{
    float3 posW : POSITION;
    float2 size : SIZE;
};

//--------------------------------
// VERTEX SHADER
//--------------------------------
VS_OUT VS(VS_IN vin)
{
    // just pass data over to geometry shader
    VS_OUT vout = vin;
    return vout;
}
