#include "LightHelper.hlsli"


//
// GLOBALS
//
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t1);

SamplerState gSampleSky     : register(s0);
SamplerState gSampleType    : register(s1);



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

    float3 gFixedFogColor;       // what is the color of fog?
    float  gFogStart;            // how far from camera the fog starts?
    float  gFogRange;            // how far from camera the object is fully fogged?

    int    gNumOfDirLights;      // current number of directional light sources

    int    gFogEnabled;          // turn on/off the fog effect
    int    gTurnOnFlashLight;    // turn on/off the flashlight
    int    gAlphaClipping;       // turn on/off alpha clipping
};

cbuffer cbTerrain : register(b5)
{
    float4 gAmbient;
    float4 gDiffuse;
    float4 gSpecular;
    float4 gReflect;
};

//
// TYPEDEFS
//
struct PS_IN
{
    float4   posH               : SV_POSITION;  // homogeneous position
    float4   color              : COLOR;
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


    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gEyePosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;

    // --------------------------------------------------

    // HACK to get proper sky pixel
    float3 vec = -toEyeW;
    vec.y -= 990;

    float4 skyTexColor = gCubeMap.Sample(gSampleType, vec);

    // return blended fixed fog color with the sky color at this pixel
    // if the pixel is fully fogged
    if (gFogEnabled && distToEye > (gFogStart + gFogRange))
    {
        return skyTexColor * float4(gFixedFogColor, 1.0f);
    }


    // -------------  SAMPLE TEXTURES  ------------------

    // how many times we scale the detail map
    const float detalizationLvl = 16;

    float4 textureColor   = gTextures[1].Sample(gSampleType, pin.tex);
    float4 lightMapColor  = gTextures[10].Sample(gSampleType, pin.tex);
    float4 detailMapColor = gTextures[16].Sample(gSampleType, pin.tex * detalizationLvl);

    // posW.y / 255 * 10: height based lighting
    //float brightness = pin.posW.y * 0.039215f;

    //float  brightness = 2.0f;

    float4 lightIntensity = lightMapColor * (gDirLights[0].diffuse + gDirLights[0].ambient);

    float4 finalColor = textureColor * detailMapColor * lightIntensity;


    // ---------------------  FOG  ----------------------

    if (gFogEnabled)
    {
        // blend sky texture pixel with fixed fog color
        float4 fogColor = skyTexColor * float4(gFixedFogColor, 1.0f);

        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend using lerp the fog color and the final color
        finalColor = lerp(finalColor, fogColor, fogLerp);
    }

    return finalColor;

    // --------------------  NORMAL MAP   --------------------

    float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

    // compute the bumped normal in the world space
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);


    // --------------------  LIGHTING  --------------------

    Material mat;
    mat.ambient  = gAmbient;
    mat.diffuse  = gDiffuse;
    mat.specular = gSpecular;
    mat.reflect  = gReflect;


    // start with a sum of zero
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    // sum the light contribution from each directional light source
    for (int i = 0; i < gNumOfDirLights; ++i)
    {
        ComputeDirectionalLight(
            mat,
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
            mat,
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
            mat,
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
            mat,
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

    // render depth value as color
    //return float4(pin.posH.z, pin.posH.z, pin.posH.z, 1.0f);

    return litColor;

}
