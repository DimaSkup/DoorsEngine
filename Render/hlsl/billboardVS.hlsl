//
// TYPEDEFS
//
struct VS_IN
{
	float3 posW          : POSITION;
    float3 color         : COLOR;
	float2 size          : SIZE;
    float  translucency : TRANSLUCENCY;

};

struct VS_OUT
{
	float3  centerW      : POSITION;       // billboard center pos in a world space
    float   translucency : TRANSLUCENCY;
    float3  color        : COLOR;          // rgb
	float2  sizeW        : SIZE;           // width and height of the billboard
};


//
// VERTEX SHADER
//
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

	// just pass data over to geometry shader
	vout.centerW = vin.posW;
    vout.translucency = vin.translucency;
    vout.color = vin.color;
	vout.sizeW = vin.size;

	return vout;
}
