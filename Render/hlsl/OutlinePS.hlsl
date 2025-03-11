

//
// TYPEDEFS
//
struct PS_IN
{
	float4   posH      : SV_POSITION;  // homogeneous position
	//float3   posW      : POSITION;     // position in world
	//float3   normalW   : NORMAL;       // normal in world
};

struct PS_OUT
{
	float4 color : SV_Target;
};


//
// PIXEL SHADER
//
float4 PS(PS_IN pin) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
	
}


