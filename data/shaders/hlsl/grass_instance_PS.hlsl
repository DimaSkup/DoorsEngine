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
Texture2D    gTextures[22]    : register(t100);
SamplerState gBasicSampler    : register(s0);

//--------------------------------
// TYPEDEFS
//--------------------------------
struct PS_IN
{
    float4 posH         : SV_POSITION;
    float3 posW         : POSITION;
    float2 tex          : TEXCOORD;
    float3 normal       : NORMAL;
    float3 fogColor     : COLOR;
	float  lightFactor  : LIGHT;
	bool   isFrontFace  : SV_IsFrontFace;
};

//---------------------------------------------------------
// for grass we have to compute directed light contibution in a specific way
//---------------------------------------------------------
void ComputeDirL(
    DirectionalLight L,
    float3 normal,
    out float4 ambient,
    out float4 diffuse)
{
    // initialize outputs
    ambient = 0;
    diffuse = 0;

    // the light vector aims opposite the direction the light rays travel
    float3 lightVec = -L.direction;

    // add ambient term
    ambient = gAmbient * L.ambient;

    // use Lambert's cosine law to define a magnitude of the light intensity
    float diffuseFactor = dot(lightVec, normal);

    // flatten to avoit dynamic branching
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * (gDiffuse * L.diffuse);
    }
    else
    {
        diffuse = 0.5 * (gDiffuse * L.diffuse);
    }
}

//---------------------------------------------------------
// for grass we have to compute point light contibution in a specific way
//---------------------------------------------------------
void ComputePointL(
    PointLight L,
    float3 pos,          // position of the vertex
    float3 normal,
    out float4 ambient,
    out float4 diffuse)
{
    // initialize output
    ambient = 0;
    diffuse = 0;

    float3 lightVec = L.position - pos;

	if (L.range*L.range < dot(lightVec, lightVec))
		return;
	
    float d = length(lightVec);
    lightVec /= d;

    ambient = gAmbient * L.ambient;
	
    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * (gDiffuse * L.diffuse);
    }
	
    // light up the backface of the grass plane
    else
    {
        diffuse = 0.5 * (gDiffuse * L.diffuse);
    }

    float att = 1.0 / dot(L.att, float3(1.0, d, d*d));

    diffuse *= att;
    ambient *= att;
}


//--------------------------------
// PIXEL SHADER
//--------------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
    const float4 texDiff = gTextures[1].Sample(gBasicSampler, pin.tex);
    
    // execute alpha clipping
    clip(texDiff.a - 0.6f);

	float3 normal = normalize(pin.normal);
	
	
	//if (!pin.isFrontFace)
    //    normal = -normal;
	
	//return float4(normal, 1.0);
	
    // --------------------  LIGHT   --------------------

	// a vector in the world space from vertex to eye pos
	float3 toEyeW = gCamPosW - pin.posW;

	// compute the distance to the eye from this surface point
	const float distToEye = length(toEyeW);
		
	// normalize
	toEyeW /= distToEye;
	
    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;
	
    int i = 0;
	
    // calc light from directed light sources
    ComputeDirL(gDirLights[0], normal, A, D);

    ambient += A;
    diffuse += D;

	
		Material mat;
		mat.ambient  = gAmbient;
		mat.diffuse  = gDiffuse;
		mat.specular = gSpecular;
		mat.reflect  = gReflect;
    
    // calc light from point light sources
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
		/*
		ComputePointLight(
			mat,
			gPointLights[i],
			pin.posW,
			normal,
			toEyeW,
			1.0,
			A, D, S);
		*/
        ComputePointL(gPointLights[i], pin.posW, normal, A, D);

        ambient += A;
        diffuse += D;
    }
   
  
    // compute light from the flashlight / spotlights
    if (gTurnOnFlashLight || gCurrNumSpotLights > 1)
    {
	
		
		const float specStrength = 1.0;
		
		if (gTurnOnFlashLight)
		{
			ComputeSpotLight(
				mat,
				gSpotLights[0],
				pin.posW,
				normal,
				toEyeW,
				specStrength,
				A, D, S);	
			
			ambient += A;
			diffuse += D;
		}
			
		// sum the light contribution from each spot light source
		for (i = 1; i < gCurrNumSpotLights; ++i)
		{
			ComputeSpotLight(
				mat,
				gSpotLights[0],
				pin.posW,
				normal,
				toEyeW,
				specStrength,
				A, D, S);	
			
			ambient += A;
			diffuse += D;
		}
    }
	
    // compute final contribution from light sources
    //const float4 light = saturate(ambient + diffuse);

	//float4 color = (texDiff * light) * pin.lightFactor;
	float4 color = (texDiff * (ambient + diffuse)) * pin.lightFactor;

    // ------------------------------------------

    if (gFogEnabled)
    {		
        // fog intensity
        const float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        color = lerp(color, float4(pin.fogColor, 1.0), fogLerp);
    }

    return color;
}
