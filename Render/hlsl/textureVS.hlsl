//////////////////////////////////
// Filename: texture.vs
//////////////////////////////////


//////////////////////////////////
// GLOBALS
//////////////////////////////////
cbuffer cbPerFrame : register(b0)
{
	matrix gViewProj;
	float3 gCameraPosition;
};

//////////////////////////////////
// TYPEDEFS
//////////////////////////////////
struct VS_INPUT
{
	// data per instance
	row_major matrix world        : WORLD;
	row_major matrix texTransform : TEXTRANSFORM;
	uint             instanceID   : SV_InstanceID;

	// data per vertex
	float3 posL         : POSITION;      // vertex position in a local space
	float2 tex          : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 posH        : SV_POSITION;  // homogeneous position of the vertex
	float3 posW        : POSITION;     // world position of the vertex
	float2 tex         : TEXCOORD0;
};


//////////////////////////////////
// VERTEX SHADER
//////////////////////////////////
VS_OUTPUT VS(VS_INPUT vin)
{
	VS_OUTPUT vout;

	// transform pos from local to world space
	vout.posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

	// transform to homogeneous clip space
	vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);
	
	// output vertex attributes for interpolation across triangle
	vout.tex = mul(float4(vin.tex, 0.0f, 1.0f), vin.texTransform).xy;
	//vout.tex = vin.tex;



	return vout;
}