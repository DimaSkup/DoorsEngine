//==================================================================================
// Filename:  visualizeDepthPS.hlsl
// Desc:      pixel shader for depth visualization
//==================================================================================
#include "const_buffers/cb_camera.hlsli"

//---------------------------
// GLOBALS
//---------------------------
Texture2D<float> gDepthTex     : register(t10);
SamplerState     gBasicSampler : register(s0);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

//---------------------------
// HELPERS
//---------------------------
float LinearizeDepth(float depth)
{
    return (gNearZ * gFarZ) / (gFarZ - depth * (gFarZ - gNearZ));
}

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    float depth = gDepthTex.Sample(gBasicSampler, pin.tex);

    // convert to linear depth
    float linearDepth = LinearizeDepth(depth);
    float normalized = saturate((linearDepth - gNearZ) / (gFarZ - gNearZ));

    // invert for clarity (while = near, black = far)
    //normalized = 1.0 - normalized;   

    return float4(normalized.xxx, 1.0);
	
	// add color ramp
	//float3 color = lerp(float3(0, 0, 1), float3(1, 0, 0), normalized);
    //return float4(color, 1.0);
}
