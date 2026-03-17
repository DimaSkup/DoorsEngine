//==================================================================================
// Filename:  decalPS.hlsl
// Desc:      a pixel shader for 3d decals rendering
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"       // lights data
#include "const_buffers/cbps_material_colors.hlsli" // colors: diffuse, ambient, etc.
#include "const_buffers/cb_weather.hlsli"           // fog data
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"


//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);

SamplerState gSkySampler    : register(s1);
SamplerState gSamLinearWrap : register(s3);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float2 tex  : TEXCOORD;
    float3 normW : NORMAL;
    float  translucency : TRANSLUCENCY;
    /*float4 posH  : SV_POSITION;
    float  texU  : TEXCOORD0;
    float3 normW : NORMAL;
    float  texV  : TEXCOORD1;*/
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
    int i;
    const float4 diffTex        = gTextures[1].Sample(gSamLinearWrap, pin.tex);
    const float4 skyBottomColor = gCubeMap.Sample(gSkySampler, float3(0, -490, 0));
    
    const float specStrength    = 1.0;
    const float alpha           = diffTex.a;
    
    // a vector in world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // the distance between the eye and vertex
    const float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;

   
    // --------------------  LIGHT   --------------------

    Material material;
    material.ambient = gAmbient;
    material.diffuse = gDiffuse;
    material.specular = gSpecular;

    float4 ambient = 0;
    float4 diffuse = 0;
    float4 spec    = 0;

    // sum up the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;


    // ...from each directional light source
    for (i = 0; i < gCurrNumDirLights; ++i)
    {
        ComputeDirectionalLight(
            material,
            gDirLights[i],
            pin.normW,
            toEyeW,
            specStrength,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec    += S;
    }
    
    // ...from each point light source
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
        ComputePointLight2(
            material,
            gPointLights[i],
            pin.posW,
            pin.normW,
            toEyeW,
            1.0,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec    += S;
    }
    
    // ...from the flashlight (spotlight)
    if (gTurnOnFlashLight)
    {
        ComputeSpotLight(
            material,
            gSpotLights[0],
            pin.posW,
            pin.normW,
            toEyeW,
            specStrength,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec    += S;
    }
    
    // ...from each spotlight source
    // (start from 1 because we've already calculated flashlight)
    for (i = 1; i < gCurrNumSpotLights; ++i)
    {
        ComputeSpotLight(
            material,
            gSpotLights[i],
            pin.posW,
            pin.normW,
            toEyeW,
            specStrength,
            A, D, S);
            
        ambient += A;
        diffuse += D;
        spec    += S;
    }
    
    // modulate with late add
    float4 litColor = (diffTex * (ambient + diffuse) + spec);

    // calc how much this pixel is fogged
    if (gFogEnabled)
    {
        CalcFog(skyBottomColor, gFixedFogColor, distToEye, gFogStart, gFogRange, litColor);
    }
    
    return float4(litColor.xyz, pin.translucency * alpha);
}
