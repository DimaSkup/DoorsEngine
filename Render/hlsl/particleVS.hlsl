//
// TYPEDEFS
//
struct VS_IN
{
    float3   posW          : POSITION;       // particle (point) center position in world
    float    translucency  : TRANSLUCENCY;
    float3   color         : COLOR;
    float2   sizeW         : SIZE;           // width and height of the particle
};

struct VS_OUT
{
    float3   posW          : POSITION;       // particle (point) center position in world
    float    translucency  : TRANSLUCENCY;
    float3   color         : COLOR;
    float2   sizeW         : SIZE;           // width and height of the particle
};


//
// VERTEX SHADER
//
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    // just pass data over to geometry shader
    vout = vin;

    return vout;
}
