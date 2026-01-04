// =================================================================================
// Filename:     SkyDomePS.hlsl
// Description:  pixel shader for rendering of the sky
// =================================================================================
#include "const_buffers/cbps_rare_changed.hlsli"

//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
SamplerState gSkySampler    : register(s1);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_INPUT
{
    float4 posH   : SV_POSITION;    // homogeneous position
    float3 posL   : POSITION;       // position of the vertex in local space
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_INPUT pin) : SV_TARGET
{
	float4 texColor = gCubeMap.Sample(gSkySampler, pin.posL);
	
    float height = pin.posL.y * (pin.posL.y > 0.0);
	
    // determine the gradient colour by interpolating between the apex and center 
    // based on the height of the pixel in the sky dome
    //float3 gradient = lerp(gSkyColorCenter, gSkyColorApex, height);
    
    return texColor;// *float4(gradient, 1.0f);
};
