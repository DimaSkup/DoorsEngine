#include "const_buffers/cb_time.hlsli"

//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4   posH       : SV_POSITION;    // homogeneous position
    float3   posW       : POSITION;       // position in world
    float2   tex        : TEXCOORD;
};

float sdfCircle(float2 p, float radius)
{
	return length(p) - radius;
}

float3 palette(float t, float3 a, float3 b, float3 c, float3 d)
{
	return a + b*cos( 6.28318 * (c*t + d) );
}

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
	float3 finalColor = 0;
	float2 uv = pin.tex;
	
	// put 0,0 coords at the center and make range to be in [-1,1]
	uv = uv * 2.0 - 1.0;
	
	float2 uv0 = uv;
	
	
	for (float i = 0.0; i < 4.0; i++)
	{
		uv = frac(uv * 1.5) - 0.5;
			
		float dist = length(uv) * exp(-length(uv0));



		float3 a = float3(0.5, 0.5, 0.5);
		float3 b = float3(0.5, 0.5, 0.5);
		float3 c = float3(1.0, 1.0, 1.0);
		float3 d = float3(0.263, 0.416, 0.557);

		float3 col = palette(length(uv0) + gGameTime*0.4 + i*0.4, a, b, c, d);

		dist = sin(dist*8 + gGameTime) / 8;
		dist = abs(dist);
		
		// increase the contrast
		dist = pow(0.01 / dist, 1.2);

		finalColor += col * dist;
	}

    return float4(finalColor, 1.0);
}
