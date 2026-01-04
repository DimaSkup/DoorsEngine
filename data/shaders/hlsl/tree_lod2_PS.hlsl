#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_weather.hlsli"


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
    float4   posH       : SV_POSITION;    // homogeneous position
    float3   matAmbient : COLOR0;
    float    texU       : TEXCOORD0;
    float3   matDiffuse : COLOR1;
    float    texV       : TEXCOORD1;
    float3   posWL      : NORMAL;         // transformed position but translated back to local space
    float    fogLerp    : TEXCOORD2;
    float3   fogColor   : COLOR2;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float2 uv = float2(pin.texU, pin.texV);
    float4 diffTex 	= gTextures[1].Sample(gSamLinearWrap, uv);

    // execute alpha clipping
    clip(diffTex.a - 0.8f);

    float4 lightMap = gTextures[10].Sample(gSamLinearWrap, uv) * 2.5;

   
    // --------------------  LIGHT   --------------------

    // start with a sum of zero
    float4 ambient = 0;
    float4 diffuse = 0;

    // sum the light contribution from each directional light source
    CalcDirLightAmbDiff(
        float4(pin.matAmbient, 1.0),
        float4(pin.matDiffuse, 1.0),
        gDirLights[0],
        pin.posWL,
        ambient,
        diffuse);


    // modulate with late add
    const float  dotSunBranch   = dot(gDirLights[0].direction, -pin.posWL);
    const float  lightGradient  = lerp(0.7, 1.0, dotSunBranch);
    const float4 lightIntensity = saturate(lightMap * lightGradient) * (ambient + diffuse);

    float4 finalColor           = diffTex * lightIntensity;


    // ---------------------  FOG  ----------------------
    if (gFogEnabled)
    {

        // blend the fog color and the lit color
        finalColor = lerp(finalColor, float4(pin.fogColor, 1.0), pin.fogLerp);
    }

    return finalColor;
}
