#include "LightHelper.hlsli"


// ==========================
// GLOBALS
// ==========================
TextureCube  gCubeMap       : register(t0);
Texture2D    gPerlinNoise   : register(t1);
Texture2D    gTextures[22]  : register(t10);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);


// ==========================
// CONSTANT BUFFERS
// ==========================
cbuffer cbPerFrame    : register(b0)
{
    // light sources data
    DirectionalLight  gDirLights[3];
    PointLight        gPointLights[25];
    SpotLight         gSpotLights[25];
    float3            gEyePosW;                // eye position in world space
    float             gTime;
    int               gCurrNumPointLights;
    int               gCurrNumSpotLights;
};

cbuffer cbRareChanged : register(b1)
{
    // some flags for controlling the rendering process and
    // params which are changed very rarely

    float3 gFixedFogColor;       // what is the color of fog?
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
    float2   tex                : TEXCOORD;
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
    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gEyePosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;


    // ------------------------------------------

    // TEMP: hacky fix for the vector to sample the proper pixel of sky
    float3 vec = -toEyeW;
    vec.y -= 990;

    

    // blend sky pixel color with fixed fog color
    float4 skyBottomColor = gCubeMap.Sample(gSkySampler, vec);
    float4 fogColor    = skyBottomColor * float4(gFixedFogColor, 1.0f);


    // return blended fixed fog color with the sky color at this pixel
    // if the pixel is fully fogged
    if (gFogEnabled && distToEye > (gFogStart + gFogRange))
    {
        return fogColor;
    }

    float4 textureColor = gTextures[1].Sample(gBasicSampler, pin.tex);
    float4 specularColor = gTextures[2].Sample(gBasicSampler, pin.tex);

    // execute alpha clipping
    if (gAlphaClipping)
        clip(textureColor.a - 0.1f);


    // --------------------  NORMAL MAP   --------------------

    float3 normalMap = gTextures[6].Sample(gBasicSampler, pin.tex).rgb;

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

    // compute the bumped normal in the world space
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);

    // --------------------  LIGHT   --------------------

    // start with a sum of zero
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    Material material = (Material)pin.material;
    
    // sum the light contribution from each directional light source
    for (int i = 0; i < gNumOfDirLights; ++i)
    {
        ComputeDirectionalLight(
            material,
            gDirLights[i],
            bumpedNormalW,
            toEyeW,
            0.0f,             // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    
    // sum the light contribution from each point light source
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
        ComputePointLight(
            material,
            gPointLights[i],
            pin.posW,
            bumpedNormalW,
            toEyeW,
            0.0f,           // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    
    // compute light from the flashlight
    if (gTurnOnFlashLight)
    {
        ComputeSpotLight(
            material,
            gSpotLights[0],
            pin.posW,
            bumpedNormalW, 
            toEyeW,
            0.0f,      // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    
    // sum the light contribution from each spot light source
    for (i = 1; i < gCurrNumSpotLights; ++i)
    {
        ComputeSpotLight(
            material,
            gSpotLights[i],
            pin.posW,
            bumpedNormalW, //bumpedNormalW, // pin.normalW,
            toEyeW,
            0.0f,   // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    
    // modulate with late add
    float4 finalSpecular = spec;
    float4 litColor = textureColor * (ambient + diffuse) + finalSpecular;

    // reflection
    float3 incident = -toEyeW;
    float3 reflectionVec = reflect(incident, normalW);
    float4 reflectionColor = gCubeMap.Sample(gSkySampler, reflectionVec);

    litColor += material.reflect * reflectionColor;

    // common to take alpha from diffuse material and texture
    //litColor.a = ((Material)pin.material).diffuse.a * textureColor.a;


    // ---------------------  FOG  ----------------------

    if (gFogEnabled)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        litColor = lerp(litColor, fogColor, fogLerp);
    }

    return litColor;
    
}
