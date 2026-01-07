//==================================================================================
// Filename:  decalPS.hlsl
// Desc:      a pixel shader for 3d decals rendering
//==================================================================================


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
    float4 posH : SV_POSITION;
    float2 tex  : TEXCOORD;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_TARGET
{
    return gTextures[1].Sample(gSamLinearWrap, pin.tex);
}
