struct PS_IN
{
    float4 posH      : SV_POSITION;   // homogeneous position
    float3 posW      : POSITION;      // position in world
    float  texU      : TEXCOORD0;
    float3 normalW   : NORMAL;        // normal in world
    float  texV      : TEXCOORD1;
    float4 tangentW  : TANGENT;       // tangent in world
};
