
//
// CONSTANT BUFFERS
//
cbuffer cbVSPerFrame : register(b0)
{
    matrix gViewProj;
};

//
// TYPEDEFS
//
struct VS_IN
{
    // data per vertex
    float3   posL       : POSITION;     // vertex position in local space
    float2   tex        : TEXCOORD;
    float3   normalL    : NORMAL;       // vertex normal in local space
    float3   tangentL   : TANGENT;      // tangent in local space
    uint     color      : COLOR;
};

struct VS_OUT
{
    float4   posH       : SV_POSITION;  // homogeneous position
    //float4   color      : COLOR;
    float3   posW       : POSITION;     // position in world
    float3   normalW    : NORMAL;       // normal in world
    float3   tangentW   : TANGENT;      // tangent in world
    float2   tex        : TEXCOORD;
};

//
// VERTEX SHADER
//
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    //vout.color = vin.color;

    // transform pos from local to world space
    vout.posW = vin.posL;// mul(float4(vin.posL, 1.0f), vin.world).xyz;

    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

    // interpolating normal can unnormalize it, so normalize it
    vout.normalW = vin.normalL;// normalize(mul(vin.normalL, (float3x3)vin.worldInvTranspose));

    // calculate the tangent and normalize it
    vout.tangentW = vin.tangentL; // normalize(mul(vin.tangentL, (float3x3)vin.worldInvTranspose));

    // output vertex texture attributes for interpolation across triangle
    vout.tex = vin.tex; // mul(float4(vin.tex, 0.0f, 1.0f), vin.texTransform).xy;

    return vout;
}
