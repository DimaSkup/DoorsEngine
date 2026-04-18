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
	float4x4 material   : MATERIAL;
	matrix   world      : WORLD;
    float4   posH    	: SV_POSITION;
	float3   fogColor	: COLOR;
	
    float3   posW    	: POSITION;         // billboard corner point in world
	float    texU       : TEXCOORD0;
	
	float3   normal     : NORMAL;
	float    texV       : TEXCOORD1;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
	const float2 uv      = float2(pin.texU, pin.texV);
	const float4 diffTex = gTextures[1].Sample(gSamLinearWrap, uv);
		
	// execute alpha clipping
	clip(diffTex.a - 0.6f);
	
	// a vector in the world space from vertex to eye pos
	float3 toEyeW = gCamPosW - pin.posW;
	
	// compute the distance to the eye from this surface point
	const float distToEye = length(toEyeW);
	
	
	// --------------------  NORMAL  --------------------
	
	const float3 normalMap = gTextures[6].Sample(gSamLinearWrap, uv).rgb;

	// extract normal vector from normal map
    const float3 normalL = 2.0 * normalMap - 1.0;
	
    float3 normal = normalize(mul(normalL, (float3x3)pin.world));
	
	if (dot(toEyeW, normal) < 0)
		normal = -normal;  
	
	
    // --------------------  LIGHT   --------------------

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D;

    Material material = (Material)pin.material;

    // sum the light contribution from each directional light source
    CalcDirLightAmbDiff(
        material.ambient,
        material.diffuse,
        gDirLights[0],
        normal,
        A, D);

    // modulate with late add
    float4 litColor = diffTex * (A + D);

    
    // ---------------------  FOG  ----------------------
    if (gFogEnabled)
    {
		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		// blend the fog color and the lit color
		litColor = lerp(litColor, float4(pin.fogColor, 1.0), fogLerp);
    }

    return litColor;
}
