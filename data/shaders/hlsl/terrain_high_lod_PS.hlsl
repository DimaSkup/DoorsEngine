//==================================================================================
// Desc:  a pixel shader for terrain rendering
//==================================================================================
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"


//---------------------------
// GLOBALS
//---------------------------
Texture2D    gTextures[22]  : register(t100);
SamplerState gBasicSampler  : register(s0);
SamplerState gSamLinearWrap : register(s3);


//---------------------------
// CONST BUFFERS
//---------------------------
cbuffer cbTerrain : register(b2)
{
    float4 gAmbient;
    float4 gDiffuse;
    float4 gSpecular;
    float4 gReflect;
};

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
	float4   fogColor   : COLOR0;
};


//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    // -------------  SAMPLE TEXTURES  ------------------
	
	float depthLerpFactor = saturate(pin.posH.z / pin.posH.w * 6);

    // how many times we scale the detail map
    const float scaleDetailMap  = 256.0;
    const float scaleMap2       = 128.0;
    const float scaleMap3       = 128.0;
    const float scaleMap4       = 256.0;

    const float2 uv = float2(pin.texU, pin.texV);
	
          float4 diffMap1   = gTextures[1].Sample(gBasicSampler, uv);
    const float4 diffMap2   = gTextures[2].Sample(gBasicSampler, uv * scaleMap2);
    const float4 diffMap3   = gTextures[3].Sample(gBasicSampler, uv * scaleMap3);
    const float4 diffMap4   = gTextures[4].Sample(gBasicSampler, uv * scaleMap4);
	
	const float4 normalMap1 = gTextures[5].Sample(gBasicSampler, uv * scaleDetailMap);
	const float4 normalMap2 = gTextures[6].Sample(gBasicSampler, uv * scaleMap2);
	const float4 normalMap3 = gTextures[7].Sample(gBasicSampler, uv * scaleMap3);
	const float4 normalMap4 = gTextures[8].Sample(gBasicSampler, uv * scaleMap4);
	
	const float4 specMap1   = gTextures[9].Sample(gBasicSampler,  uv * scaleMap2);
	const float4 specMap2   = gTextures[10].Sample(gBasicSampler, uv * scaleMap2);
	const float4 specMap3   = gTextures[11].Sample(gBasicSampler, uv * scaleMap3);
	const float4 specMap4   = gTextures[12].Sample(gBasicSampler, uv * scaleMap4);
	
    const float4 splatMap   = gTextures[13].Sample(gSamLinearWrap, uv);
	float4 detailMap = gTextures[16].Sample(gBasicSampler, uv * scaleDetailMap);
	
	// to make detail map brighter 
	detailMap *= 2.0;

    // farther from us the influence of detail map is less visible
	detailMap = lerp(float4(1,1,1,1), detailMap, depthLerpFactor); 
	
	// blend with detail map
	diffMap1 *= detailMap;
	
	// blend diffuse maps 
	const float4 pixelDiff1   = lerp(diffMap1,   diffMap2, splatMap.r);
	const float4 pixelDiff2   = lerp(pixelDiff1, diffMap3, splatMap.g);
	const float4 diffMap      = lerp(pixelDiff2, diffMap4, splatMap.b);

	// blend normal maps
	const float4 pixelNorm1   = lerp(normalMap1, normalMap2, splatMap.r);
	const float4 pixelNorm2   = lerp(pixelNorm1, normalMap3, splatMap.g);
	const float4 normalMap    = lerp(pixelNorm2, normalMap4, splatMap.b);
	
	// blend specular maps
	const float4 pixelSpec1   = lerp(specMap1,   specMap2, splatMap.r);
	const float4 pixelSpec2   = lerp(pixelSpec1, specMap3, splatMap.g);
	float specStrength        = lerp(pixelSpec2, specMap4, splatMap.b).r;

    // farther from us the spec strength become closer to 1.0 (so it doesn't do anything)
    specStrength = lerp(1.0, specStrength, depthLerpFactor);

    // --------------------  NORMAL MAP   --------------------

    // normalize the normal vector after interpolation
    const float3 normalW = normalize(pin.normalW);

    // compute the bumped normal in the world space
	const float4 tangent = float4(1,0,0,1);
    float3 normal  = NormalSampleToWorldSpace(normalMap.rgb, normalW, tangent);
	
	normal = lerp(normalW, normal, depthLerpFactor);

    // --------------------  LIGHTING  --------------------

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    const float distToEye = length(toEyeW);

    // normalize
    toEyeW /= distToEye;


    Material mat;
    mat.ambient  = gAmbient;
    mat.diffuse  = gDiffuse;
    mat.specular = gSpecular;
    mat.reflect  = gReflect;

    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;
    float4 spec    = 0;

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    // sum the light contribution from each directional light source
    for (int i = 0; i < gCurrNumDirLights; ++i)
    {
        ComputeDirectionalLight(
            mat,
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
            mat,
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
            mat,
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
            mat,
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
    float4 finalColor = diffMap * (ambient + diffuse) + spec;

    // ---------------------  FOG  ----------------------

    if (gFogEnabled)
    {
        // compute linear interpolation of the fog
        const float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

        // blend using lerp the fog color and the final color
        return lerp(finalColor, pin.fogColor, fogLerp);
    }

    return finalColor;
}
