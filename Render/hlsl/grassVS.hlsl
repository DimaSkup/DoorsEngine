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
    VS_OUT vout;

    // just pass data over to geometry shader
    vout = vin;

    return vout;
}
