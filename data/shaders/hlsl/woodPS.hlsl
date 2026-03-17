//==================================================================================
// Filename:  woodPS.hlsl
// Desc:      pixel shader for rendering a wood material
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "types/ps_in_basic_inst.hlsli"


//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
	const float2 uv = float2(pin.texU, pin.texV);
	
    const float4 skyBottomColor = gCubeMap.Sample(gSkySampler, float3(0, -490, 0));
    const float4 diffTex        = gTextures[1].Sample(gBasicSampler, uv);
    const float  specPower      = gTextures[2].Sample(gBasicSampler, uv).r;
    const float3 normalMap      = gTextures[6].Sample(gBasicSampler, uv).rgb;


    // --------------------  NORMAL MAP  --------------------

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);
    
    // compute the bumped normal in the world space
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);
	
    // --------------------  LIGHT   --------------------

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;


    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;
    float4 spec    = 0;

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    // sum the light contribution from each directional light source
    for (int i = 0; i < gCurrNumDirLights; ++i)
    {
        ComputeDirectionalLight(
            (Material)pin.material,
            gDirLights[i],
            bumpedNormalW,
            toEyeW,
            specPower,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    // sum the light contribution from each point light source
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
        ComputePointLight(
            (Material)pin.material,
            gPointLights[i],
            pin.posW,
            bumpedNormalW,
            toEyeW,
            specPower,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }


    // compute light from the flashlight
    if (gTurnOnFlashLight)
    {
        ComputeSpotLight(
            (Material)pin.material,
            gSpotLights[0],
            pin.posW,
            bumpedNormalW,
            toEyeW,
            specPower,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }


    // sum the light contribution from each spot light source
    for (i = 1; i < gCurrNumSpotLights; ++i)
    {
        ComputeSpotLight(
            (Material)pin.material,
            gSpotLights[i],
            pin.posW,
            bumpedNormalW, 
            toEyeW,
            specPower,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    // modulate with late add
    float4 litColor = diffTex * (ambient + diffuse) + spec;

    // ---------------------  FOG  ----------------------

    if (gFogEnabled)
    {
        // blend sky pixel color with fixed fog color
        float4 fogColor = skyBottomColor * float4(gFixedFogColor, 1.0f) + 0.1f;

        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        litColor = lerp(litColor, fogColor, fogLerp);
    }

    return litColor;
}
