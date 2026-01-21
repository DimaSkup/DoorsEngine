//==================================================================================
// Desc:  a vertex shader for particles rendering
//==================================================================================

//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    float4   color         : COLOR;
    float3   posW          : POSITION;       // particle (point) center position in world
    float2   uv0           : TEXCOORD0;
    float2   uv1           : TEXCOORD1;    
    float2   sizeW         : TEXCOORD2;           // width and height of the particle
};

struct VS_OUT
{
    float4   color         : COLOR;
    float3   posW          : POSITION;       // particle (point) center position in world
    float2   uv0           : TEXCOORD0;
    float2   uv1           : TEXCOORD1;
    float2   sizeW         : TEXCOORD2;           // width and height of the particle
};

//---------------------------
// Vertex shader
//---------------------------
VS_OUT VS(VS_IN vin)
{
    // just pass data over to geometry shader
    VS_OUT vout = vin;
    return vout;
}
