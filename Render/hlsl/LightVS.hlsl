
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
    // data per instance
    row_major matrix   world             : WORLD;
    row_major matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;
    row_major matrix   texTransform      : TEX_TRANSFORM;
    row_major float4x4 material          : MATERIAL;
    uint               instanceID        : SV_InstanceID;

    // data per vertex
    float3   posL       : POSITION;     // vertex position in local space
    float2   tex        : TEXCOORD;
    float3   normalL    : NORMAL;       // vertex normal in local space
    float3   tangentL   : TANGENT;      // tangent in local space
    float3   binormalL  : BINORMAL;     // binormal in local space
};

struct VS_OUT
{
    float4x4 material   : MATERIAL;
    float4   posH       : SV_POSITION;  // homogeneous position
    float3   posW       : POSITION;     // position in world
    float3   normalW    : NORMAL;       // normal in world
    float3   tangentW   : TANGENT;      // tangent in world
    float3   binormalW  : BINORMAL;     // binormal in world
    float2   tex        : TEXCOORD;
    uint     instanceID : SV_InstanceID;
};

//
// VERTEX SHADER
//
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;

    vout.material = vin.material;

    // transform pos from local to world space
    vout.posW = mul(float4(vin.posL, 1.0f), vin.world).xyz;

    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0f), gViewProj);

    // interpolating normal can unnormalize it, so normalize it
    vout.normalW = normalize(mul(vin.normalL, (float3x3)vin.worldInvTranspose));

    // calculate the tangent and normalize it
    vout.tangentW = normalize(mul(vin.tangentL, (float3x3)vin.worldInvTranspose));

    // calculate the binormal and normalize it
    vout.binormalW = normalize(mul(vin.binormalL, (float3x3)vin.worldInvTranspose));

    // output vertex texture attributes for interpolation across triangle
    vout.tex = mul(float4(vin.tex, 0.0f, 1.0f), vin.texTransform).xy;

    vout.instanceID = vin.instanceID;

    return vout;
}
