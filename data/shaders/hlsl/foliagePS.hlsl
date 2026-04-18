//==================================================================================
// Filename:  fooliageVS.hlsl
// Desc:      pixel shader for foliage rendering
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"

#include "const_buffers/cbps_post_fx2.hlsli"        // to get screen size
#include "helpers/noise_rand.hlsli"
#include "const_buffers/post_fx_params_enum.hlsli"  // to get screen size


//---------------------------
// GLOBALS
//---------------------------
Texture2D    gTextures[22]  : register(t100);
SamplerState gBasicSampler  : register(s0);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    float4x4 material           : MATERIAL;
    float4   posH               : SV_POSITION;  // homogeneous position
    float3   posW               : POSITION;     // position in world
	float    texU               : TEXCOORD0;
	float3   normalL            : NORMAL0;
    float3   normalW            : NORMAL1;      // normal in world
	float    texV               : TEXCOORD1;
    float4   tangentW           : TANGENT;      // tangent in world
    float3   fogColor           : COLOR;
	uint     SampleId 	        : SV_SampleIndex;
    bool     isFrontFace        : SV_IsFrontFace;
};


//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
	bool bHasDistToEye = false;
	float distToEye = 0;
	
    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    float sqrDistToEye = dot(toEyeW, toEyeW);
	
	// distance where model fades in and fades out
	const float2 fadeRange = float2(50.0, 65.0);
	
	// completely discarded if our of range
	if (sqrDistToEye > (fadeRange.y*fadeRange.y))
		discard;
	
	// if we are in "switch" range from model to its LOD (or billboard)
	// so generate a noise for smooth transition
	if (sqrDistToEye > (fadeRange.x*fadeRange.x))
	{
		float2 scrSize;
		scrSize.x = GetPostFxParam(POST_FX_PARAM_SCREEN_WIDTH);
		scrSize.y = GetPostFxParam(POST_FX_PARAM_SCREEN_HEIGHT);
		
		 // fragment_coord / screen_size + sample_id * factor
		const float f = rand(pin.posH.xy / scrSize + pin.SampleId * 37.45128);
		
		distToEye = length(toEyeW);
		
		if (f > 1.0-(distToEye-fadeRange.x)/(fadeRange.y-fadeRange.x))
			discard;
		
		bHasDistToEye = true;
	}
	
	if (!bHasDistToEye)
		distToEye = length(toEyeW);
	
	// normalize a vector from vertex to eye position
	toEyeW /= distToEye;
		
	
	const float2 uv = float2(pin.texU, pin.texV);
    const float4 diffTex = gTextures[1].Sample(gBasicSampler, uv);
    
	clip(diffTex.a - 0.1f);
	
	const float  specPower      = gTextures[2].Sample(gBasicSampler, uv).r;
	const float3 normalMap      = gTextures[6].Sample(gBasicSampler, uv).rgb;

	
    // --------------------  NORMAL  --------------------
	
	// a vector in the world space from vertex to eye pos
    //float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    //float distToEye = length(toEyeW);
	
	//toEyeW /= distToEye;

    // normalize the normal vector after interpolation
	float3 normalL = normalize(pin.normalL);
	
	// for foliage, tree branches, bushes: fix normals (when cull:none)
	float3 normalW = normalize(pin.normalW);
	
	if (pin.isFrontFace)
		normalW = -normalW;

	float3 normalWL = normalize(normalL + normalW);
	
    // compute the bumped normal in the world space
    const float3 normal = NormalSampleToWorldSpace(normalMap, normalWL, pin.tangentW);

	
    // --------------------  LIGHT   --------------------

    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;
    float4 spec    = 0;

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;
    int i = 0;

    Material material = (Material)pin.material;

    // sum the light contribution from each directional light source
    for (i = 0; i < gCurrNumDirLights; ++i)
    {
        ComputeDirectionalLight(
            (Material)pin.material,
            gDirLights[i],
            normal,
            toEyeW,
            specPower,
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
            normal,
            toEyeW,
            specPower,
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
            normal,
            toEyeW,
            specPower,
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
            normal, 
            toEyeW,
            specPower,
            A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }
	
	//return saturate(ambient + diffuse) + spec;
	
	// modulate with late add
    float4 litColor = diffTex * saturate(ambient + diffuse) + spec;
	
	
	// ---------------------  FOG  ----------------------

    if (gFogEnabled)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend the fog color and the lit color
        litColor = lerp(litColor, float4(pin.fogColor, 1.0), fogLerp);
    }
	
   return litColor;
}
