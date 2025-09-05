
//////////////////////////////////
// CONSTANT BUFFERS
//////////////////////////////////
cbuffer cbVSRareChanged : register(b1)
{
    matrix gWorld;
    matrix gViewProj;
}


//////////////////////////////////
// TYPEDEFS
//////////////////////////////////
struct VS_IN
{
    // data per vertex
    float3 posL      : POSITION;        // vertex position in local space
    float2 tex       : TEXCOORD;
    float3 normalL   : NORMAL;          // vertex normal in local space
    float3 tangentL  : TANGENT;         // tangent in local space
};

struct VS_OUT
{
    float4 posH      : SV_POSITION;     // homogeneous position
    float3 posW      : POSITION;        // position in world
    float3 normalW   : NORMAL;          // normal in world
    float3 tangentW  : TANGENT;         // tangent in world
    float2 tex       : TEXCOORD;
};

//////////////////////////////////
// VERTEX SHADER
//////////////////////////////////
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    // transform pos from local to world space
    vout.posW       = mul(float4(vin.posL, 1.0f), gWorld).xyz;

    // transform to homogeneous clip space
    vout.posH       = mul(float4(vout.posW, 1.0f), gViewProj);

    // just copy the rest values into the output struct
    vout.normalW    = mul(float4(vin.normalL, 1.0f), gWorld).xyz;
    vout.tangentW   = mul(float4(vin.tangentL, 1.0f), gWorld).xyz;
    vout.tex        = vin.tex;

    return vout;
}
