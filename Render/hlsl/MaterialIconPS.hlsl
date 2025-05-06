#include "LightHelper.hlsli"

//
// GLOBALS
//
Texture2D    gTextures[128] : register(t0);
SamplerState gSampleType    : register(s0);


//
// CONSTANT BUFFERS
//
cbuffer cbMaterialPerObj : register(b5)
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
    float4 posH      : SV_POSITION;   // homogeneous position
    float3 normalW   : NORMAL;        // normal in world
    float3 tangentW  : TANGENT;       // tangent in world
    float3 binormalW : BINORMAL;      // binormal in world
    float2 tex       : TEXCOORD;      
};

//
// PIXEL SHADER
//
float4 PS(PS_IN pin) : SV_Target
{
    float4 textureColor = gTextures[1].Sample(gSampleType, pin.tex);

    // execute alpha clipping
    if (true)
        clip(textureColor.a - 0.1f);

    
    // --------------------  LIGHT   --------------------

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = float3(0,0,-2.0f) - pin.posH.xyz;

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


    /*
    Material mat;
    mat.ambient  = float4(0.3f, 0.3f, 0.3f, 1.0f);
    mat.diffuse  = float4(1.0f, 1.0f, 1.0f, 1.0f);
    mat.specular = float4(0.0f, 0.0f, 0.0f, 2.0f);
    mat.reflect  = float4(.5f, .5f, .5f, 1.0f);
    */

    DirectionalLight dirLight;
    dirLight.ambient  = float4(1,1,1,1);
    dirLight.diffuse  = float4(1,1,1,1);
    dirLight.specular = float4(1,1,1,1);
    dirLight.direction = float3(-1.0f, -1.0f, 1.0f);

    Material mat;
    mat.ambient = gAmbient;
    mat.diffuse = gDiffuse;
    mat.specular = gSpecular;
    mat.reflect = gReflect;

    ComputeDirectionalLight(
        mat,
        dirLight,
        pin.normalW,
        toEyeW,
        0.0f,
        A, D, S);

    ambient += A;
    diffuse += D;
    spec += S;

    // modulate with late add
    float4 litColor = textureColor * (ambient + diffuse) + spec;
    litColor.a = 1.0f;

    // common to take alpha from diffuse material and texture
    //litColor.a = ((Material)pin.material).diffuse.a * textureColor.a;

    return litColor;

    
}
