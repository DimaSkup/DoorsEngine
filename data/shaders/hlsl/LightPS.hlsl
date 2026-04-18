//==================================================================================
// Desc:  a pixel shader which is used as default (texturing + lighting)
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"     // lights data
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "types/ps_in_basic_inst.hlsli"
#include "helpers/fog.hlsli"                      // to get a fog color


//---------------------------
// GLOBALS
//---------------------------
Texture2D    gTextures[22]  : register(t100);
SamplerState gBasicSampler  : register(s0);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_OUT
{
    float4 color : SV_Target;
    float  depth : SV_Depth;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float2 uv = float2(pin.texU, pin.texV);

    const float3 fogColor     = GetFogColor();
    const float4 diffTex      = gTextures[1].Sample(gBasicSampler, uv);
    const float3 normalMap    = gTextures[6].Sample(gBasicSampler, uv).rgb;
	const float  specStrength = gTextures[7].Sample(gBasicSampler, uv).r;
    
    // --------------------  NORMAL  --------------------

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;


    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

    // compute the bumped normal in the world space
    float3 normal = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);
	
    // --------------------  LIGHT   --------------------

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
            normal,
            toEyeW,
            specStrength,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    // sum the light contribution from each point light source
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
        ComputePointLight2(
            (Material)pin.material,
            gPointLights[i],
            pin.posW,
            normal,
            toEyeW,
            specStrength,
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
            normal,
            toEyeW,
            specStrength,
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
            normal, 
            toEyeW,
            specStrength,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    // modulate with late add
    float4 litColor = (diffTex * (ambient + diffuse) + spec);


    // calc how much this pixel is fogged
    if (gFogEnabled)
    {
        const float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        litColor = lerp(litColor, float4(fogColor, 1.0), fogLerp);
    }

    return litColor;
}
