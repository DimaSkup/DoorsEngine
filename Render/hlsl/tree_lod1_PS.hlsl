#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"


//---------------------------
// GLOBALS
//---------------------------
Texture2D    gTextures[22]  : register(t100);
SamplerState gBasicSampler  : register(s0);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4x4 material          : MATERIAL;
    matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;

    float4   posH        : SV_POSITION;  // homogeneous position
    float3   posW        : POSITION;     // position in world
    float2   tex         : TEXCOORD;
    float4   fogColor    : COLOR;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    float4 diffTex = gTextures[1].Sample(gBasicSampler, pin.tex);
    
    // execute alpha clipping
    clip(diffTex.a - 0.8f);
    
    
    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    //toEyeW /= distToEye;

    float3 normalMap = gTextures[6].Sample(gBasicSampler, pin.tex).rgb;
  

    // --------------------  NORMAL MAP   --------------------

    float3 normalL = 2.0 * normalMap - 1.0;
    float3 normalW = normalize(mul(normalL, (float3x3)pin.worldInvTranspose));

    if (dot(toEyeW, normalW) < 0)
        normalW *= -1;

    float3 normal = normalW;

    // --------------------  LIGHT   --------------------

    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;
    int i = 0;

    Material material = (Material)pin.material;

    
    // sum the light contribution from each directional light source
    CalcDirLightAmbDiff(
        material.ambient,
        material.diffuse,
        gDirLights[0],
        normal,
        A, D);

    ambient += A;
    diffuse += D;

    // sum the light contribution from each point light source
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
       CalcPointLightAmbDiff(
           material.ambient,
           material.diffuse,
           gPointLights[i],
           pin.posW,
           normal,
           A, D);

       ambient += A;
       diffuse += D;
    }

    // compute light from the flashlight
    if (gTurnOnFlashLight)
    {
        CalcSpotLightAmbDiff(
            material.ambient,
            material.diffuse,
            gSpotLights[0],
            pin.posW,
            normal,
            A, D);

       ambient += A;
       diffuse += D;
    }

    // sum the light contribution from each spot light source
    for (i = 1; i < gCurrNumSpotLights; ++i)
    {
        CalcSpotLightAmbDiff(
            material.ambient,
            material.diffuse,
            gSpotLights[i],
            pin.posW,
            normal,
            A, D);

       ambient += A;
       diffuse += D;
    }
    

    // modulate with late add
    float4 litColor = diffTex * (ambient + diffuse);

    
    // ---------------------  FOG  ----------------------
    if (gFogEnabled)
    {
       float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

       // blend the fog color and the lit color
       litColor = lerp(litColor, pin.fogColor, fogLerp);
    }

    return litColor;
}
