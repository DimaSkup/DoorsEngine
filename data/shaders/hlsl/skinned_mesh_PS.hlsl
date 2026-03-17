//==================================================================================
// Desc:  a pixel shader which is used as default (texturing + lighting)
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "const_buffers/cbps_material_colors.hlsli"

//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);

SamplerState gBasicSampler  : register(s0);
SamplerState gSkySampler    : register(s1);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4   posH       : SV_POSITION;  // homogeneous position
    float3   posW       : POSITION;     // position in world
    float    texU       : TEXCOORD0;
    float3   normalW    : NORMAL;       // normal in world
    float    texV       : TEXCOORD1;
    float4   tangentW   : TANGENT;      // tangent in world
    float4   weights    : WEIGHTS;
    uint4    boneIds    : BONEINDICES;
    uint     currBoneId : TEXCOORD2;
};


//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{

    const float2 uv             = float2(pin.texU, pin.texV);
    const float4 diffMap        = gTextures[1].Sample(gBasicSampler, uv);
	const float4 skyBottomColor = gCubeMap.Sample(gSkySampler, float3(0, -490, 0));
	const float specStrength    = 1.0f;
	
	// --------------------  NORMAL  --------------------

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;


    // normalize the normal vector after interpolation
    float3 normal = normalize(pin.normalW);
	
	//return float4(normal, 1.0);
	
    // --------------------  LIGHT   --------------------ddd

    // start with a sum of zero
    float4 ambient = float4(0,0,0,0);
    float4 diffuse = float4(0,0,0,0);
    float4 spec    = float4(0,0,0,0);

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    Material material;
	material.ambient  = gAmbient;
	material.diffuse  = gDiffuse;
	material.specular = gSpecular;
	material.reflect  = gReflect;

    // sum the light contribution from each directional light source
    for (int i = 0; i < gCurrNumDirLights; ++i)
    {
        ComputeDirectionalLight(
            material,
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
            material,
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
            material,
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
            material,
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
    float4 litColor = (diffMap * (ambient + diffuse) + spec);

    // ---------------------  FOG  ----------------------

    if (gFogEnabled)
    {
        // blend sky pixel color with fixed fog color
        float4 fogColor = skyBottomColor * float4(gFixedFogColor, 1.0f) + 0.1f;

        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        litColor = lerp(litColor, fogColor, fogLerp);
    }

    return litColor;

	
	
	
	
	/*
	// DEBUG BONE WEIGHTS (VISUALIZE IT)
	
    if (pin.boneIds.x == pin.currBoneId)
    {
        if (pin.weights.x >= 0.7)
        {
            return float4(1,0,0,1) * pin.weights.x;
        }
        else if (pin.weights.x >= 0.4 && pin.weights.x <= 0.6)
        {
            return float4(0,1,0,1) * pin.weights.x;
        }
        else if (pin.weights.x >= 0.1)
        {
            return float4(1,1,0,1) * pin.weights.x;
        }
    }

    if (pin.boneIds.y == pin.currBoneId)
    {
        if (pin.weights.y >= 0.7)
        {
            return float4(1,0,0,1) * pin.weights.y;
        }
        else if (pin.weights.y >= 0.4 && pin.weights.y <= 0.6)
        {
            return float4(0,1,0,1) * pin.weights.y;
        }
        else if (pin.weights.y >= 0.1)
        {
            return float4(1,1,0,1) * pin.weights.y;
        }
    }

    if (pin.boneIds.z == pin.currBoneId)
    {
        if (pin.weights.z >= 0.7)
        {
            return float4(1,0,0,1) * pin.weights.z;
        }
        else if (pin.weights.z >= 0.4 && pin.weights.z <= 0.6)
        {
            return float4(0,1,0,1) * pin.weights.z;
        }
        else if (pin.weights.z >= 0.1)
        {
            return float4(1,1,0,1) * pin.weights.z;
        }
    }

    if (pin.boneIds.w == pin.currBoneId)
    {
        if (pin.weights.w >= 0.7)
        {
            return float4(1,0,0,1) * pin.weights.w;
        }
        else if (pin.weights.w >= 0.4 && pin.weights.w <= 0.6)
        {
            return float4(0,1,0,1) * pin.weights.w;
        }
        else if (pin.weights.w >= 0.1)
        {
            return float4(1,1,0,1) * pin.weights.w;
        }
    }

    return float4(0,0,1,1);
	*/
}
