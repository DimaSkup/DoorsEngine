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
};

cbuffer cbTerrain     : register(b5)
{
    float4x4 gMaterial;
};


//
// TYPEDEFS
//
struct PS_IN
{
    float4   posH               : SV_POSITION;  // homogeneous position
    //float4   color              : COLOR;
    float3   posW               : POSITION;     // position in world
    float3   normalW            : NORMAL;       // normal in world
    float3   tangentW           : TANGENT;      // tangent in world
    float2   tex                : TEXCOORD;
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
    float4 textureColor = gTextures[1].Sample(gSampleType, pin.tex);
    return textureColor;
    float4 detailMapColor = gTextures[16].Sample(gSampleType, pin.tex*8);
    //float brightness = pin.posW.y * 0.0039215f;    // posW.y / 255
    textureColor = textureColor * detailMapColor * 3.0f;
    return textureColor;
    //textureColor = float4(textureColor.xyz * gDirLights[0].diffuse.xyz, 1.0f);

    // --------------------  NORMAL MAP   --------------------

    float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

    // compute the bumped normal in the world space
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);

  
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
            (Material)gMaterial,
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
            (Material)gMaterial,
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
            (Material)gMaterial,
            gSpotLights[0],
            pin.posW,
            bumpedNormalW, //bumpedNormalW, // pin.normalW,
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
            (Material)gMaterial, 
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
    float4 litColor = textureColor * (ambient + diffuse) + spec;

    // common to take alpha from diffuse material and texture
    //litColor.a = ((Material)pin.material).diffuse.a * textureColor.a;


    // ---------------------  FOG  ----------------------

    if (gFogEnabled)
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
