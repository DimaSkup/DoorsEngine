//
// TYPEDEFS
//
struct VS_IN
{
	// data per instance
	float4x4 material    : MATERIAL;
	float3   posW        : POSITION;       // billboard center pos in a world space
	float2   sizeW       : SIZE;           // width and height of the billboard
	uint     instanceID  : SV_InstanceID;

	float3 posL : POSITION_L;
	float2 size : SIZE_L;
};

struct VS_OUT
{
	float4x4 material    : MATERIAL;
	float3   centerW     : POSITION;       // billboard center pos in a world space
	float2   sizeW       : SIZE;           // width and height of the billboard
};


//
// VERTEX SHADER
//
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

	// just pass data over to geometry shader
	vout.material = vin.material;
	vout.centerW = vin.posW;
	vout.centerW.y += (vin.sizeW.y * 0.5f);
	vout.sizeW = vin.sizeW;

	return vout;
}
