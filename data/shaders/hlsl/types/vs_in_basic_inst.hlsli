struct VS_IN
{
    // data per instance
    row_major matrix   world             : WORLD;
    row_major matrix   worldInvTranspose : WORLD_INV_TRANSPOSE;
    row_major float4x4 material          : MATERIAL;
    uint               instanceID        : SV_InstanceID;

    // data per vertex
    float3   posL                        : POSITION;       // vertex position in local space
    float2   tex                         : TEXCOORD;
    float3   normalL                     : NORMAL;         // vertex normal in local space
    float4   tangentL                    : TANGENT;        // tangent in local space
};
