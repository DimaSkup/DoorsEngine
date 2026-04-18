//==================================================================================
// Filename:  fog.hlsli
// Desc:      a little helper to unify receiving of the fog current color
//==================================================================================
#include "../const_buffers/cb_weather.hlsli"

TextureCube  gCubeMap    : register(t0);
SamplerState gSkySampler : register(s1);

float3 GetFogColor()
{
    // sky_bottom_color * fixed_fog_color
	const float3 uvw = float3(0, -490, 0);
    return gCubeMap.SampleLevel(gSkySampler, uvw, 0).rgb * gFixedFogColor;
}
