//==================================================================================
// Desc:  a vertex shader for grass rendering
//==================================================================================
#include "helpers/fog.hlsli"
#include "helpers/noise_rand.hlsli"

#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cb_grass.hlsli"


//--------------------------------
// TYPEDEFS
//--------------------------------
struct VS_IN
{
    // data per instance
    float3 worldPos   : WORLD_POS;
    float scale       : SCALE;
    uint texColumn    : TEX_COLUMN;
    uint texRow       : TEX_ROW;
    uint instanceID   : SV_InstanceID;

    // data per vertex
    float3 posL       : POSITION;  // vertex position in local space
    float2 tex        : TEXCOORD;
};


struct VS_OUT
{
    float4 posH       : SV_POSITION;
    float3 posW       : POSITION;
    float2 tex        : TEXCOORD;
    float3 normal     : NORMAL;
    float3 fogColor   : COLOR;
	float lightFactor : LIGHT;
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
// Desc:  wind for grass at point posW
// Ret:   displacement factor for a vertex
//--------------------------------
void CalcWind(const float3 posW, const float sizeFactor, out float3 disp)
{
    const float3 eyeToGrass    = posW - gCamPosW;
    const float sqrDistToGrass = dot(eyeToGrass, eyeToGrass);

	// no displacement by default
	disp = 0;
	
    if (sqrDistToGrass < (gSwayDistance * gSwayDistance))
    {
        // compute height-based falloff
        float hf = HeightFactor(float3(0, posW.y, 0));

        // global time (seconds)
        float t = gGameTime * gWindSpeed;

        // --- larget smooth waving (sine waves along wind direction + cross waves) ---
        // position along wind direction to phase waves
        const float phasePos = dot(posW.xz, gWindDir.xz) * gWaveFrequency;

        // primary long-wave
        const float longWave = sin(phasePos + t) * gWaveAmplitude;

        // cross wave for variety (perpendicular)
        const float2 perp = float2(-gWindDir.z, gWindDir.x);
        const float crossPhase = dot(posW.xz, perp) * (gWaveFrequency * 0.7);
        const float crossWave = sin(crossPhase + t * 1.3) * (gWaveAmplitude * 0.4);


        // --- turbulence / small scale noise ---
        const float n = noise(float3(posW.xz * 0.5, t * 0.5));  // [-1,1]
        const float smallTurb = n * gTurbulence;


        // --- gust (a simple envelope) ---
        // gustPower is expected 0...1;  amplify waves momentarily
        const float gust = gGustPower * 2.0;  // scale impact
        const float gustSpeedMod = 1.0 + gust;


        // combine motions
        float sway = (longWave + crossWave) * (1.0 + gust * 1.5) + smallTurb * (1.0 + gust);
        sway *= (0.5 + 0.5 * (sway >= 0.0));

        // farther grass == smaller grass == less swaying
        sway *= sizeFactor;  

        // final horizontal displacement vector (world space)
        const float3 horiz = float3(gWindDir.x, 0, gWindDir.y) * 
		                     sway * gWindStrength * hf * gBendScale;

        // vertical list component (optional subtle lift)
        const float vertical = 
			(sin(phasePos * 0.5 + t * 0.8) * 0.05 +
		    noise(float3(posW.xy, t * 0.2)) * 0.02) * hf;
		
		// return final displacement
        disp = horiz + float3(0, vertical, 0);
    }	
}

//--------------------------------
// VERTEX SHADER
//--------------------------------
VS_OUT VS(VS_IN vin)
{
	VS_OUT vout;

	// vector from vertex to camera (both in world space)
    float3 toEyeW = gCamPosW - vin.worldPos;
	const float distSqr = dot(toEyeW, toEyeW);
	
    const float sqrDistMaxVisible = gDistGrassVisible * gDistGrassVisible;
    
	// if grass is out of visibility...
	if (distSqr > sqrDistMaxVisible)
	{
		// ... set all the output values to zero
		// and skip all the calculations
		vout.posH = 0;
		vout.posW = 0;
		vout.tex = 0;
		vout.normal = 0;
		vout.fogColor = 0;
		vout.lightFactor = 0;
		
		return vout;
	}
	
	
	float sizeFactor = 1.0;
	const float sqrDistFullSize = gDistGrassFullSize * gDistGrassFullSize;
	
	// farther grass becomes smaller...
	if (distSqr > sqrDistFullSize)
	{
		sizeFactor -= (distSqr-sqrDistFullSize) / (sqrDistMaxVisible-sqrDistFullSize);
	}
	
	vout.fogColor = GetFogColor();
	
	// scale grass instance
	const float3 posL = vin.posL * (sizeFactor * vin.scale);
	
	// transform to world space
	vout.posW = posL + vin.worldPos;
	
	// wind displacement
	float3 displacement = float3(0,0,0);
	
	// if this vertex isn't part of grass root then it is affected by wind
	if (posL.y > 0 && (sizeFactor > 0))
		CalcWind(vout.posW, sizeFactor, displacement);
	
	// displace vertex by wind
	vout.posW += displacement;
  
    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0), gViewProj);
	
	// use local position as a normal vector so all the normals looks "outside" of the instance
    vout.normal = posL;

	//
	// setup texture coords
	//
	const float tu = 1.0 / gNumTexColumns;
    const float tv = 1.0 / gNumTexRows;
	
	// if left edge of tex image...
	[flatten]
	if (vin.tex.x == 0)
		vout.tex.x = tu * vin.texColumn;
	
	// right edge...
	else 
		vout.tex.x = tu * vin.texColumn + tu;
	
	// if top edge of tex image...
	[flatten]
	if (vin.tex.y == 0)
		vout.tex.y = tv * vin.texRow;
	
	// bottom edge...
	else
		vout.tex.y = tv * vin.texRow + tv;
	
	//
	// make grass darker when closer to the ground
	//
    float topColor    = 1.0;
    float bottomColor = 0.3;
    vout.lightFactor = lerp(bottomColor, topColor, vin.posL.y);
	
	
    return vout;
}
