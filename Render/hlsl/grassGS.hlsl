// =================================================================================
// Filename:     grassGS.hlsl
// Description:  a geometry shader (GS) to expand a grass point into a 3 grass planes
// =================================================================================

//--------------------------------
// GLOBALS
//--------------------------------
Texture2D    gPerlinNoise   : register(t1);
SamplerState gBasicSampler  : register(s0);

//--------------------------------
// CONSTANT BUFFERS
//--------------------------------
cbuffer cbPerFrame : register(b0)
{
    matrix gViewProj;
    float3 gEyePosW;                // eye position in world space
    float  gTime;
};

cbuffer cbGrassRareChanged : register(b1)
{
    float gDistGrassFullSize;        // from camera and to this distance grass planes are in full size
    float gDistGrassVisible;         // after this distance we don't see any grass planes
}

//--------------------------------
// TYPEDEFS
//--------------------------------
struct GS_IN
{
    float3 posW : POSITION;
    float2 size : SIZE;
};

struct GS_OUT
{
    float4 posH     : SV_POSITION;
    float3 posW     : POSITION;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD;
    uint   primID   : SV_PrimitiveID;
};

//--------------------------------
// GEOMETRY SHADER
//--------------------------------
[maxvertexcount(18)]
void GS(
    point GS_IN gin[1],
    uint primID : SV_PrimitiveID,
    inout TriangleStream<GS_OUT> triStream)
{
    // we expand each point into 3 quads (12 vertices), so the maximum number
    // of vertices we output per geometry shader invocation is 12

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gEyePosW - gin[0].posW;
    float distSqr = dot(toEyeW, toEyeW);

    // squared values
    const float sqrDistMaxVisible = gDistGrassVisible * gDistGrassVisible;
    const float sqrDistFullSize   = gDistGrassFullSize * gDistGrassFullSize;

    if (distSqr > sqrDistMaxVisible)
        return;

    float sizeX = gin[0].size.x;
    float sizeY = gin[0].size.y;

    if (distSqr > sqrDistFullSize)
    {
        const float sizeFactor = 1.0f - (distSqr-sqrDistFullSize) / (sqrDistMaxVisible-sqrDistFullSize);

        sizeX *= sizeFactor;
        sizeY *= sizeFactor;
    }


    float3 vBaseDir[3] =
    {
        float3(1.0f, 0.0f, 0.0f),
        float3(+0.707f, 0.0f, +0.707f),
        float3(+0.707f, 0.0f, -0.707f)
    };

    float3 vNorm[3] =
    {
        float3(0, 0, -1),
        float3(+0.707, 0.000, -0.707),
        float3(-0.707, 0.000, -0.707)
    };

    for (int idx = 0; idx < 3; idx++)
        if (dot(toEyeW, vNorm[idx]) < 0)
            vNorm[idx] = -vNorm[idx];


    float  grassPatchSize = 5.0f;
    float  windStrength   = 2.0f;
    float3 windDirection  = float3(0.707f, 0.0f, 0.707f);   // normalized vec<1,0,1>


    float2 gTexC[4];
    
    if (primID % 2 == 0)
    {
        gTexC[0] = float2(0.5f, 1.0f);
        gTexC[1] = float2(0.5f, 0.0f);
        gTexC[2] = float2(0.75f, 1.0f);
        gTexC[3] = float2(0.75f, 0.0f);
    }
    else if (primID % 3 == 0)
    {
        gTexC[0] = float2(0.75f, 1.0f);
        gTexC[1] = float2(0.75f, 0.0f);
        gTexC[2] = float2(1.0f, 1.0f);
        gTexC[3] = float2(1.0f, 0.0f);
    }
    else if (primID % 5 == 0)
    {
        gTexC[0] = float2(0.25f, 1.0f);
        gTexC[1] = float2(0.25f, 0.0f);
        gTexC[2] = float2(0.5f, 1.0f);
        gTexC[3] = float2(0.5f, 0.0f);
    }
    else
    {
        gTexC[0] = float2(0.0f, 1.0f);
        gTexC[1] = float2(0.0f, 0.0f);
        gTexC[2] = float2(0.25f, 1.0f);
        gTexC[3] = float2(0.25f, 0.0f);
    }

    // 1/30 == 0.0333
    // 1/20 == 0.05
    float windPower = 0.5 + sin(
        gin[0].posW.x*0.0333 + gin[0].posW.z*0.0333 +  // different instances will oscillate by wind in a different way
        gTime * (1.2 + windStrength * 0.05)            // according to time and wind strength
        + sizeY);                                      // and height of the grass (higher will oscillate bigger)


    if (windPower < 0.0)
        windPower *= 0.02;
    else
        windPower *= 0.05;

    float3 vWindFactor = windDirection * (windPower * windStrength);
    float halfHeight = sizeY * 0.5f;
    float heightOffset = sizeY * 0.4f;
    

    [unroll]
    for (int i = 0; i < 3; i++)
    {       
        
        float offsetX     = vBaseDir[i].x * sizeX * 0.5f;
        float offsetZ     = vBaseDir[i].z * sizeX * 0.5f;

        // compute triangle strip vertices (quad) in world space
        float3 v[4];

        // BR
        v[0] = gin[0].posW + float3(offsetX, -halfHeight, offsetZ);

        // TR
        v[1] = gin[0].posW + float3(offsetX, +halfHeight, offsetZ) + vWindFactor;

        // BL
        v[2] = gin[0].posW + float3(-offsetX, -halfHeight, -offsetZ);

        // TL
        v[3] = gin[0].posW + float3(-offsetX, halfHeight, -offsetZ) + vWindFactor;
     
 
        v[0].y += heightOffset;
        v[1].y += heightOffset;
        v[2].y += heightOffset;
        v[3].y += heightOffset;

        // transform quad vertices to world space and output them as a triangle strip


        //[unroll]
        //for (int vIdx = 0; vIdx < 4; ++vIdx)
        //{

            GS_OUT p0;
            GS_OUT p1;
            GS_OUT p2;
            GS_OUT p3;

            p0.posH = mul(float4(v[0], 1.0f), gViewProj);
            p1.posH = mul(float4(v[1], 1.0f), gViewProj);
            p2.posH = mul(float4(v[2], 1.0f), gViewProj);
            p3.posH = mul(float4(v[3], 1.0f), gViewProj);

            p0.posW = v[0];
            p1.posW = v[1];
            p2.posW = v[2];
            p3.posW = v[3];

            p0.normal = vNorm[i];
            p1.normal = vNorm[i];
            p2.normal = vNorm[i];
            p3.normal = vNorm[i];

            p0.tex = gTexC[0];
            p1.tex = gTexC[1];
            p2.tex = gTexC[2];
            p3.tex = gTexC[3];

            p0.primID = primID;
            p1.primID = primID;
            p2.primID = primID;
            p3.primID = primID;

            /*
            gout.posH   = mul(float4(v[vIdx], 1.0f), gViewProj);
            gout.posW   = v[vIdx];
            gout.normal = vNorm[i];// vNorm[i];
            gout.tex    = gTexC[vIdx];
            gout.primID = primID;
            */
            triStream.Append(p0);
            triStream.Append(p1);
            triStream.Append(p2);
            triStream.RestartStrip();

            triStream.Append(p2);
            triStream.Append(p1);
            triStream.Append(p3);
            triStream.RestartStrip();


            
        //}
    }
  
}
