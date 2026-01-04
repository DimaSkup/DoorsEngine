//==================================================================================
// Desc:  a pixel shader for 2D sprites rendering
//==================================================================================


//---------------------------
// GLOBALS
//---------------------------
Texture2D    gTextures[22]  : register(t100);
SamplerState gSamLinearWrap : register(s3);

//---------------------------
// TYPEDEFS
//---------------------------
struct PS_INPUT
{
    float4 posH   : SV_POSITION;
    float2 tex    : TEXCOORD;
    uint   primID : SV_PrimitiveID;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_INPUT pin) : SV_TARGET
{
    return gTextures[1].Sample(gSamLinearWrap, pin.tex);
}
