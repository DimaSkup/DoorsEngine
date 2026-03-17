#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "const_buffers/cbps_post_fx2.hlsli"
#include "helpers/noise_rand.hlsli"
#include "const_buffers/post_fx_params_enum.hlsli"


//---------------------------
// GLOBALS
//---------------------------
Texture2D    gTextures[22]  : register(t100);
SamplerState gSamLinearWrap : register(s3);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
	float4x4 material          : MATERIAL;
    matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;
	
    float4  posH    	: SV_POSITION;
	float4  fogColor 	: COLOR;
    float3  posW    	: POSITION;         // billboard corner point in world
    float2  tex0    	: TEXCOORD0;
	float2  tex1    	: TEXCOORD1;
    uint    primID  	: SV_PrimitiveID;
	uint    SampleId 	: SV_SampleIndex;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
	float2 scrSize;
	scrSize.x = GetPostFxParam(POST_FX_PARAM_SCREEN_WIDTH);
	scrSize.y = GetPostFxParam(POST_FX_PARAM_SCREEN_HEIGHT);

	const float f = rand(pin.posH.xy / scrSize + pin.SampleId * 37.45128);
	
    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    const float distToEye = length(toEyeW);
	
	// distance where billboard fades in and fades out
	const float2 fadeRange = float2(35.0, 50.0);
	
	// billboard fade in
	if (distToEye < fadeRange.y)
	{
		if ((1.0-f) > (distToEye-fadeRange.x)/(fadeRange.y-fadeRange.x))
			discard;
	}
	
	
	// dissolve blending
	const float blend = pin.fogColor.w;
	
	float4 diffTex;
	float3 normalMap;
	
	if (f > blend)
	{
		diffTex = gTextures[1].Sample(gSamLinearWrap, pin.tex0);
		
		// execute alpha clipping
		clip(diffTex.a - 0.5f);
		
		normalMap = gTextures[6].Sample(gSamLinearWrap, pin.tex0).rgb;
	}
	else
	{
		diffTex = gTextures[1].Sample(gSamLinearWrap, pin.tex0);
		
		// execute alpha clipping
		clip(diffTex.a - 0.5f);
		
		normalMap = gTextures[6].Sample(gSamLinearWrap, pin.tex0).rgb;
	}
	
	
	// --------------------  NORMAL  --------------------

	// normalize a vector from vertex to eye position
	toEyeW /= distToEye;
	
	// extract normal vector from normal map
    const float3 normalL = 2.0 * normalMap - 1.0;
	
    float3 normal = normalize(mul(normalL, (float3x3)pin.worldInvTranspose));

    if (dot(toEyeW, normal) < 0)
        normal *= -1;

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
