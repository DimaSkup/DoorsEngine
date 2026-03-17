//==================================================================================
// Desc:  a pixel shader for terrain rendering (for LOD1,LOD2,etc.)
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
    float4 gMatAmbient;
    float4 gMatDiffuse;
    float4 gMatSpecular;
    float4 gMatReflect;
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
	float4   splatMap   : COLOR0;
	float4   fogColor   : COLOR1;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    // -------------  SAMPLE TEXTURES  ------------------

    // how many times we scale the detail map
    const float scaleDetailMap  = 256.0;
    const float scaleMap2       = 128.0;
    const float scaleMap3       = 128.0;
    const float scaleMap4       = 256.0;

    const float2 uv = float2(pin.texU, pin.texV);

          float4 diffMap1 = gTextures[1].Sample(gSamLinearWrap, uv);
    const float4 diffMap2 = gTextures[2].Sample(gSamLinearWrap, uv * scaleMap2);
    const float4 diffMap3 = gTextures[3].Sample(gSamLinearWrap, uv * scaleMap3);
    const float4 diffMap4 = gTextures[4].Sample(gSamLinearWrap, uv * scaleMap4);      


    // blend diffuse maps 
    const float4 pixelDiff1 = lerp(diffMap1,   diffMap2, pin.splatMap.r);
    const float4 pixelDiff2 = lerp(pixelDiff1, diffMap3, pin.splatMap.g);
    const float4 diffMap    = lerp(pixelDiff2, diffMap4, pin.splatMap.b);

	
    // --------------------  NORMAL   --------------------

    // normalize the normal vector after interpolation
    const float3 normal = normalize(pin.normalW);


    // --------------------  LIGHTING  --------------------

	// a vector in the world space from vertex to eye pos
	float3 toEyeW = gCamPosW - pin.posW;

    // compute the distance to the eye from this surface point
    const float distToEye = length(toEyeW);


    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D;


    // sum the light contribution from each directional light source
    for (int i = 0; i < gCurrNumDirLights; ++i)
    {
        CalcDirLightAmbDiff(
            gMatAmbient,
            gMatDiffuse,
            gDirLights[i],
            normal,
            A, D);

        ambient += A;
        diffuse += D;
    }

    // sum the light contribution from each point light source
    for (i = 0; i < gCurrNumPointLights; ++i)
    {
        CalcPointLightAmbDiff(
            gMatAmbient,
            gMatDiffuse,
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
            gMatAmbient,
            gMatDiffuse,
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
            gMatAmbient,
            gMatDiffuse,
			gSpotLights[i],
			pin.posW,
			normal,
			A, D);

        ambient += A;
        diffuse += D;
    }


    // modulate with late add
    float4 finalColor = diffMap * (ambient + diffuse);

	
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
