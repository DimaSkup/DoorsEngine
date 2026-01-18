//==================================================================================
// Desc:  a pixel shader for particles rendering
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
    float4   posH          : SV_POSITION;
    float3   posW          : POSITION;       // particle center position in world
    float    translucency  : TRANSLUCENCY;
    float3   color         : COLOR;
    float    normalX       : NORMAL_X;
    float2   tex           : TEXCOORD;
    float    normalY       : NORMAL_Y;
    float    normalZ       : NORMAL_Z;
};

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_INPUT pin) : SV_TARGET
{
    float4 tex = gTextures[1].Sample(gSamLinearWrap, pin.tex);
    return float4(tex.rgb * pin.color, pin.translucency * tex.a);
}
