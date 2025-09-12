#include "LightHelper.hlsli"


//--------------------------------
// GLOBALS
//--------------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gPerlinNoise   : register(t1);
Texture2D    gTextures[22]  : register(t10);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);


//--------------------------------
// CONSTANT BUFFERS
//--------------------------------
cbuffer cbPerFrame        : register(b0)
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

    float3 gFixedFogColor;       // what is the color of fog?
    float  gFogStart;            // how far from camera the fog starts?
    float  gFogRange;            // how far from camera the object is fully fogged?

    int    gNumOfDirLights;      // current number of directional light sources

    int    gFogEnabled;          // turn on/off the fog effect
    int    gTurnOnFlashLight;    // turn on/off the flashlight
    int    gAlphaClipping;       // turn on/off alpha clipping

    float3 gSkyColorCenter;
    float  padding0;
    float3 gSkyColorApex;
    float  padding1;
}

//--------------------------------
// TYPEDEFS
//--------------------------------
struct PS_IN
{
    float4 posH     : SV_POSITION;
    float3 posW     : POSITION;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD;
    uint   primID   : SV_PrimitiveID;
};


//--------------------------------
// PIXEL SHADER
//--------------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
    float4 texColor = gTextures[1].Sample(gBasicSampler, pin.tex);
   
    // execute alpha clipping
    clip(texColor.a - 0.1f);
    
    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gEyePosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;

    // --------------------  LIGHT   --------------------

    // start with a sum of zero
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    Material material;

    material.ambient  = float4(1, 1, 1, 1);
    material.diffuse  = float4(1, 1, 1, 1);
    material.specular = float4(0, 0, 0, 1);                                // w-component is a specPower (specular power)
    material.reflect  = float4(0, 0, 0, 0);

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normal);

    // sum the light contribution from each directional light source
    for (int i = 0; i < gNumOfDirLights; ++i)
    {
        ComputeDirectionalLight(
            material,
            gDirLights[i],
            normalW,
            toEyeW,
            0.0f,             // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    ComputePointLight(
        material,
        gPointLights[0],
        pin.posW,
        normalW,
        toEyeW,
        0.0f,             // specular map value
        A, D, S);

    ambient += A;
    diffuse += D;
    spec += S;

    // sum the light contribution from each point light source


    // compute light from the flashlight
    if (gTurnOnFlashLight)
    {
        ComputeSpotLight(
            material,
            gSpotLights[0],
            pin.posW,
            normalW,
            toEyeW,
            0.0f,             // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    ComputeSpotLight(
        material,
        gSpotLights[1],
        pin.posW,
        normalW,
        toEyeW,
        0.0f,             // specular map value
        A, D, S);

    ComputeSpotLight(
        material,
        gSpotLights[2],
        pin.posW,
        normalW,
        toEyeW,
        0.0f,             // specular map value
        A, D, S);


    ambient += A;
    diffuse += D;
    spec += S;

    
    // sum the light contribution from each spot light source
    for (i = 1; i < gCurrNumSpotLights; ++i)
    {
        ComputeSpotLight(
            material,
            gSpotLights[i],
            pin.posW,
            normalW, 
            toEyeW,
            0.0f,             // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    //float texAlpha = texColor.a;
    float4 color = float4(texColor.xyz * (ambient + diffuse).xyz, texColor.a);
    //color.a = texAlpha;

    // ------------------------------------------

    // TEMP: hacky fix for the vector to sample the proper pixel of sky
    float3 vec = -toEyeW;
    vec.y -= 490;


    // blend sky pixel color with fixed fog color
    float4 skyBottomColor = gCubeMap.Sample(gSkySampler, vec);
    float4 fogColor = skyBottomColor * float4(gFixedFogColor, 1.0f);


    if (gFogEnabled)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        color = lerp(color, fogColor, fogLerp);
    }

    return color;
}
