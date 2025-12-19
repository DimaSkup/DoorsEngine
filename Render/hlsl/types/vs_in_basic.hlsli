struct VS_IN
{
    // data per vertex
    float3 posL      : POSITION;        // vertex position in local space
    float2 tex       : TEXCOORD;
    float3 normalL   : NORMAL;          // vertex normal in local space
    float4 tangentL  : TANGENT;         // tangent in local space
};

