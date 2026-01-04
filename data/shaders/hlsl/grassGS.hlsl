// =================================================================================
// Filename:     grassGS.hlsl
// Description:  a geometry shader (GS) to expand a grass point into a 3 grass planes
// =================================================================================
#include "helpers/noise_rand.hlsli"
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cb_weather.hlsli"


//--------------------------------
// CONSTANT BUFFERS
//--------------------------------
cbuffer cbGrass: register(b0)
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
    float  texU     : TEXCOORD0;
    float3 normal   : NORMAL;
    float  texV     : TEXCOORD1;
};



//---------------------------------------------------------
// compute a per-vertex "blade height" factor in [0,1]
// We'll use vertex local Y (model space) as height by convention
//---------------------------------------------------------
float HeightFactor(float3 localPos)
{
    // assume model's base at y = 0 and tips at higher y
    // clamp to 0..1 base on bounding expectations; user can scale model appropriately
    float h = saturate(localPos.y);   // if your mesh uses 0..1 height

    // exaggerate with power of smoother root stiffness
    const float heightFalloff = 2.0f;

    return pow(h, heightFalloff);
}


//--------------------------------
// GEOMETRY SHADER
//--------------------------------
[maxvertexcount(18)]
void GS(
    point GS_IN gin[1],
    uint primID : SV_PrimitiveID,
    inout TriangleStream<GS_OUT> triStream)
{
    // we expand each point into 3 quads, so the maximum number
    // of vertices we output per geometry shader invocation is 18

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = gCamPosW - gin[0].posW;
    float distSqr = dot(toEyeW, toEyeW);

    // squared values
    const float sqrDistMaxVisible = gDistGrassVisible * gDistGrassVisible;
    const float sqrDistFullSize   = gDistGrassFullSize * gDistGrassFullSize;

    
    if (distSqr > sqrDistMaxVisible)
        return;
        

    float sizeX = gin[0].size.x;
    float sizeY = gin[0].size.y;


    float sizeFactor = 1.0;
    
    if (distSqr > sqrDistFullSize)
    {
        sizeFactor -= (distSqr-sqrDistFullSize) / (sqrDistMaxVisible-sqrDistFullSize);

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

    // fix normals of planes
    vNorm[0] = sign(dot(toEyeW, vNorm[0])) * vNorm[0];
    vNorm[1] = sign(dot(toEyeW, vNorm[1])) * vNorm[1];
    vNorm[2] = sign(dot(toEyeW, vNorm[2])) * vNorm[2];



    //==============================================================================

    float3 worldPos = gin[0].posW;

    const float3 eyeToGrass    = worldPos - gCamPosW;
    const float sqrDistToGrass = dot(eyeToGrass, eyeToGrass);
    float3 disp = 0;

    if (sqrDistToGrass < (gSwayDistance * gSwayDistance))
    {
        // compute height-based falloff
        float hf = HeightFactor(float3(0, worldPos.y, 0));

        // global time (seconds)
        float t = gGameTime * gWindSpeed;

        // --- larget smooth waving (sine waves along wind direction + cross waves) ---
        // position along wind direction to phase waves
        const float phasePos = dot(worldPos.xz, gWindDir.xz) * gWaveFrequency;

        // primary long-wave
        const float longWave = sin(phasePos + t) * gWaveAmplitude;

        // cross wave for variety (perpendicular)
        const float2 perp = float2(-gWindDir.z, gWindDir.x);
        const float crossPhase = dot(worldPos.xz, perp) * (gWaveFrequency * 0.7);
        const float crossWave = sin(crossPhase + t * 1.3) * (gWaveAmplitude * 0.4);


        // --- turbulence / small scale noise ---
        const float n = noise(float3(worldPos.xz * 0.5, t * 0.5));  // [-1,1]
        const float smallTurb = n * gTurbulence;


        // --- gust (a simple envelope) ---
        // gustPower is expected 0...1;  amplify waves momentarily
        const float gust = gGustPower * 2.0;  // scale impact
        const float gustSpeedMod = 1.0 + gust;


        // combine motions
        float sway = (longWave + crossWave) * (1.0 + gust * 1.5) + smallTurb * (1.0 + gust);
        sway *= (0.5 + 0.5 * (sway >= 0.0));

        // farther grass == less swaying
        sway *= sizeFactor;  

        // final horizontal displacement vector (world space)
        const float3 horiz = float3(gWindDir.x, 0, gWindDir.y) * sway * gWindStrength * hf * gBendScale;

        // vertical list component (optional subtle lift)
        const float vertical = (sin(phasePos * 0.5 + t * 0.8) * 0.05 + noise(float3(worldPos.xy, t * 0.2)) * 0.02) * hf;
        disp = horiz + float3(0, vertical, 0);

    }


    //==============================================================================


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

    float halfHeight   = sizeY * 0.5f;
    float heightOffset = sizeY * 0.4f;
    

    [unroll]
    for (int i = 0; i < 3; i++)
    {       
        
        float offsetX = vBaseDir[i].x * sizeX * 0.5f;
        float offsetZ = vBaseDir[i].z * sizeX * 0.5f;

        // compute triangle strip vertices (quad) in world space
        float3 v[4];

        v[0] = gin[0].posW + float3(offsetX, -halfHeight, offsetZ) ;

        // TR
        v[1] = gin[0].posW + float3(offsetX, +halfHeight, offsetZ) + disp;

        // BL
        v[2] = gin[0].posW + float3(-offsetX, -halfHeight, -offsetZ);

        // TL
        v[3] = gin[0].posW + float3(-offsetX, halfHeight, -offsetZ) + disp;

 
        v[0].y += heightOffset;
        v[1].y += heightOffset;
        v[2].y += heightOffset;
        v[3].y += heightOffset;


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

        p0.texU = gTexC[0].x;
        p1.texU = gTexC[1].x;
        p2.texU = gTexC[2].x;
        p3.texU = gTexC[3].x;

        p0.texV = 1;
        p1.texV = 0;
        p2.texV = 1;
        p3.texV = 0;

        triStream.Append(p0);
        triStream.Append(p1);
        triStream.Append(p2);
        triStream.RestartStrip();

        triStream.Append(p2);
        triStream.Append(p1);
        triStream.Append(p3);
        triStream.RestartStrip();
    }
  
}
