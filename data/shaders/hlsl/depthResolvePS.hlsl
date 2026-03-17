//==================================================================================
// Desc:   reads the MSAA depth texture and writes an average (or nearest) depth
//         value into the resolved single-sample texture
//         (we use it only when 4xMSAA is enabled)
//==================================================================================

Texture2DMS<float, 4> gDepthTexMSAA : register(t11);
Texture2DMS<float4>     gScreenTex  : register(t12);
SamplerState          gBasicSampler : register(s0);


cbuffer cbCameraParams : register(b7)
{
    matrix gView;
    matrix gProj;
    matrix gInvProj;
    matrix gInvView;
    float3 gCamPos;
    float  gNearZ;
    float  gFarZ;
}


struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};


float LinearizeDepth(float depth)
{
    return (gNearZ * gFarZ) / (gFarZ - depth * (gFarZ - gNearZ));
}

float GetDepthMSAA(int2 pixelCoord)
{
    // average or min of all samples
    float depth = 0.0f;

    [unroll]
    for (int i = 0; i < 4; ++i)
        depth += gDepthTexMSAA.Load(pixelCoord, i);
    depth *= 0.25;

    return depth;
}


float4 PS(PS_IN pin) : SV_Target
{
    float depth = GetDepthMSAA(pin.posH.xy);

    float linearDepth = LinearizeDepth(depth);
    float normalized = saturate((linearDepth - gNearZ) / (gFarZ - gNearZ));

    //return float4(normalized.xxx, 1.0);

    // add color ramp
    float3 color = lerp(float3(0, 0, 0), float3(1, 1, 0), normalized);
    return float4(color, 1.0);
}
