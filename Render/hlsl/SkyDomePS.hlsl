// =================================================================================
// Filename:     SkyDomePS.hlsl
// Description:  pixel shader which is used to shade sky dome
// =================================================================================


//
// GLOBALS
//
// nonnumeric values cannot be added to a cbuffer
TextureCube  gCubeMap       : register(t0);
SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);


//
// CONSTANT BUFFERS
//
cbuffer cbRareChanged     : register(b2)
{
	// these values can be changed once per several game minutes
	float3 gColorCenter;
	float  padding1;
	float3 gColorApex;
	float  padding2;
};


//
// TYPEDEFS
//
struct PS_INPUT
{
	float4 posH   : SV_POSITION;    // homogeneous position
	float3 posL   : POSITION;       // position of the vertex in local space
};


//
// PIXEL SHADER
//
float4 PS(PS_INPUT pin) : SV_TARGET
{
	float height = pin.posL.y; 

	// the value ranges from -radius to +radius (or height of the skybox)
	// so change it to only positive values
	// NOTE: since radius of the sky dome sphere radius can be thousands 
	// we must "normalize" through multiplication by (1 / radius)
	// to compute the proper interpolation btw apex and center color;
	height = clamp(height, 0.0f, 3.0f);

	// determine the gradient colour by interpolating between the apex and center 
	// based on the height of the pixel in the sky dome
	float3 color = lerp(gColorCenter, gColorApex, height);


	// determine the position on the sky dome where this pixel is located
	// and mix it with the gradient color
    // 
	//float3 vec = float3(0, 1, 0);
    //return float4(color, 1.0f) * gCubeMap.Sample(gSampleType, vec);

    //return float4(color, 1.0f) * gCubeMap.Sample(gSampleType, pin.posL);

    float4 texColor = gCubeMap.Sample(gSkySampler, pin.posL);
    return texColor * float4(color, 1.0f);
};
