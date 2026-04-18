// =================================================================================
// Filename:  skyCloudPS.hlsl
// Desc:      pixel shader for clouds rendering
// =================================================================================
#include "const_buffers/cbps_sky.hlsli"
#include "const_buffers/cb_camera.hlsli"


//-------------------------------------
// GLOBALS
//-------------------------------------
Texture2D    gCloudTex1     : register(t4);
Texture2D    gCloudTex2     : register(t5);
SamplerState gSamLinearWrap : register(s3);

//-------------------------------------
// TYPEDEFS
//-------------------------------------
struct PS_IN
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float2 tex  : TEXCOORD;
};

//-------------------------------------
// PIXEL SHADER
//-------------------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
    // calc tex sampling coords for each cloud
    float2 texUV1 = pin.tex;
    float2 texUV2 = pin.tex;

    texUV1.x += gFirstTranslationX;
    texUV1.y += gFirstTranslationZ;

    texUV2.x += gSecondTranslationX;
    texUV2.y += gSecondTranslationZ;

    // sample clouds pixels
    float4 texColor1 = gCloudTex1.Sample(gSamLinearWrap, texUV1);
    float4 texColor2 = gCloudTex2.Sample(gSamLinearWrap, texUV2);

    // combine the two cloud textures evenly
    float4 finalColor = lerp(texColor1, texColor2, 0.5f);

    // reduce brightness
    finalColor *= gBrightness;

    // near the horizon clouds are completely faded
    float3 eyeToCloud = pin.posW - gCamPosW;
    float3 up = float3(0, 1, 0);

    return finalColor * saturate(dot(eyeToCloud, up));
}
