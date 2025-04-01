#include "LightHelper.hlsli"


//
// GLOBALS
//
Texture2D    gTextures[128] : register(t0);
SamplerState gSampleType   : register(s0);


//
// CONSTANT BUFFERS
//
cbuffer cbPerFrame    : register(b0)
{
    // light sources data
    DirectionalLight  gDirLights[3];
    PointLight        gPointLights[25];
    SpotLight         gSpotLights[25];
    float3            gEyePosW;                // eye position in world space  
    int               gCurrNumPointLights;
    int               gCurrNumSpotLights;
};

cbuffer cbRareChanged : register(b1)
{
    // some flags for controlling the rendering process and
    // params which are changed very rarely

    float3 gFogColor;            // what is the color of fog?
    float  gFogStart;            // how far from camera the fog starts?
    float  gFogRange;            // how far from camera the object is fully fogged?

    int    gNumOfDirLights;      // current number of directional light sources

    int   gFogEnabled;          // turn on/off the fog effect
    int   gTurnOnFlashLight;    // turn on/off the flashlight
    int   gAlphaClipping;       // turn on/off alpha clipping
}

//
// TYPEDEFS
//
struct PS_IN
{
    float4x4 material           : MATERIAL;
    float4   posH               : SV_POSITION;  // homogeneous position
    float3   posW               : POSITION;     // position in world
    float3   normalW            : NORMAL;       // normal in world
    float3   tangentW           : TANGENT;      // tangent in world
    float3   binormalW          : BINORMAL;     // binormal in world
    float2   tex                : TEXCOORD;
    uint     textureSubsetIdx   : TEX_SUBSET_IDX;
    uint     instanceID         : SV_InstanceID;
};

struct PS_OUT
{
    float4 color : SV_Target;
    float  depth : SV_Depth;
};


//
// PIXEL SHADER
//
float4 PS(PS_IN pin) : SV_Target
{
    float4 textureColor;

    //if (pin.instanceID == 0)
     //   textureColor = gTextures[1].Sample(gSampleType, pin.tex);
    //else if (pin.instanceID == 1)
    //    textureColor = gTextures[23].Sample(gSampleType, pin.tex);
    //else if (pin.instanceID == 2)
    //    textureColor = gTextures[44].Sample(gSampleType, pin.tex);
    //else
        textureColor = gTextures[1].Sample(gSampleType, pin.tex);
    
    // execute alpha clipping
    if (gAlphaClipping)
        clip(textureColor.a - 0.1f);

    float4 specularMap = gTextures[2].Sample(gSampleType, pin.tex);
    float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;
    float4 roughnessMap = gTextures[16].Sample(gSampleType, pin.tex);

    float3 normalW = normalize(pin.normalW);

    normalMap = (normalMap * 2.0f) - 1.0f;
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);
    bumpedNormalW = normalize(bumpedNormalW);
    

    // --------------------  LIGHT   --------------------

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gEyePosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;

    // start with a sum of zero
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;
    
    // sum the light contribution from each directional light source
    for (int i = 0; i < gNumOfDirLights; ++i)
    {
        ComputeDirectionalLight(
            (Material)pin.material,
            gDirLights[i],
            normalW,
            toEyeW,
            specularMap.x,
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
            normalW,
            toEyeW,
            specularMap.x,
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
            normalW, //bumpedNormalW, // pin.normalW,
            toEyeW,
            specularMap.x,
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
            normalW, //bumpedNormalW, // pin.normalW,
            toEyeW,
            specularMap.x,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    
    // modulate with late add
    float4 litColor = textureColor * (ambient + diffuse) + spec;

    // common to take alpha from diffuse material and texture
    litColor.a = ((Material)pin.material).diffuse.a;// *textureColor.a;


    // ---------------------  FOG  ----------------------

    if (true)
    {
        //float distToEye = length(gEyePosW - pin.posW);
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        litColor = lerp(litColor, float4(gFogColor, 1.0f), fogLerp);
    }

    // render depth value as color
    //return float4(pin.posH.z, pin.posH.z, pin.posH.z, 1.0f);

    return litColor;
}
