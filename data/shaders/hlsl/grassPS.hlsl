//==================================================================================
// Desc:  a pixel shader for grass rendering
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "const_buffers/cbps_material_colors.hlsli"


//--------------------------------
// GLOBALS
//--------------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);

//--------------------------------
// TYPEDEFS
//--------------------------------
struct PS_IN
{
    float4 posH     : SV_POSITION;
    float3 posW     : POSITION;
    float  texU     : TEXCOORD0;
    float3 normal   : NORMAL;
    float  texV     : TEXCOORD1;
};

//---------------------------------------------------------
// for grass we have to compute directed light contibution in a specific way
//---------------------------------------------------------
void ComputeDirLight(
    Material mat,
    DirectionalLight L,
    float3 normal,
    float3 toEye,
    float specularPower,
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // this HLSL function outputs the lit color of a point given a material, directional 
    // light source, surface normal, and the unit vector from the surface being lit to the eye

    // initialize outputs
    ambient = 0;
    diffuse = 0;
    spec    = 0;


    // the light vector aims opposite the direction the light rays travel
    float3 lightVec = -L.direction;

    // add ambient term
    ambient = mat.ambient * L.ambient;

    // use Lambert's cosine law to define a magnitude of the light intensity
    float diffuseFactor = dot(lightVec, normal);

    // flatten to avoit dynamic branching
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 R = reflect(lightVec, normal);
        float specFactor = pow(saturate(dot(R, toEye)), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
    else
    {
        diffuse = (-1.0 * diffuseFactor) * mat.diffuse * L.diffuse;
    }
}

//---------------------------------------------------------
// for grass we have to compute point light contibution in a specific way
//---------------------------------------------------------
void ComputePoint(
    Material mat,
    PointLight L,
    float3 pos,      // position of the vertex
    float3 normal,
    float3 toEye,
    float3 specularMap,
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // initialize output
    ambient = 0;
    diffuse = 0;
    spec    = 0;

    float3 lightVec = L.position - pos;

    float d = length(lightVec);
    lightVec /= d;

    ambient = mat.ambient * L.ambient;

    float diffuseFactor = dot(lightVec, normal);

    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
    // light up the backface of the grass plane
    else
    {
        diffuse = mat.diffuse * L.diffuse * 0.6f;
    }

    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    ambient *= att;
    spec *= att;
}


//--------------------------------
// PIXEL SHADER
//--------------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
    float4 skyBottomColor = gCubeMap.Sample(gSkySampler, float3(0,-490, 0));
    float4 texColor = gTextures[1].Sample(gBasicSampler, float2(pin.texU, pin.texV));


    // execute alpha clipping
    clip(texColor.a - 0.1f);

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;


    // --------------------  LIGHT   --------------------

    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;
    float4 spec    = 0;

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    Material material;
    material.ambient  = gAmbient;
    material.diffuse  = gDiffuse;
    material.specular = gSpecular;               // w-component is a specular power (glossiness)
    material.reflect  = gReflect;

    int i = 0;

    // sum the light contribution from each directional light source
    ComputeDirLight(
        material,
        gDirLights[0],
        pin.normal,
        toEyeW,
        1.0f,             // specular map value
        A, D, S);

    ambient += A;
    diffuse += D;
    spec += S;

	// sum the light contribution from each point light source
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
		 ComputePoint(
			material,
			gPointLights[i],
			pin.posW,
			pin.normal,
			toEyeW,
			1.0f,             // specular map value
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
            pin.normal,
            toEyeW,
            1.0f,             // specular map value
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
            pin.normal,
            toEyeW,
            1.0f,             // specular map value
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    // compute light contribution
    float4 color = texColor * (ambient + diffuse) + spec;

    // ------------------------------------------

    // make grass darker when closer to the ground
    float topColor    = 1.0;
    float bottomColor = 0.5;
    color *= lerp(topColor, bottomColor, pin.texV);
    color.a = texColor.a;

    // ------------------------------------------

    if (gFogEnabled)
    {
        // blend sky pixel color with fixed fog color
        float4 fogColor = skyBottomColor * float4(gFixedFogColor, 1.0f) + 0.1f;
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        color = lerp(color, fogColor, fogLerp);
    }

    return color;
}
