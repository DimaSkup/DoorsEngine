// *********************************************************************************
// Filename:    DebugPS.hlsl
// Description: this is a pixel shader for debugging
// 
// Created:     24.11.24
// *********************************************************************************
#include "LightHelper.hlsli"


//
// GLOBALS
//
Texture2D    gTextures[22] : register(t0);
SamplerState gSampleType   : register(s0);

// debug states/flags
static const int SHOW_NORMALS = 1;
static const int SHOW_TANGENTS = 2;
static const int SHOW_BINORMALS = 3;
static const int SHOW_BUMPED_NORMALS = 4;
static const int SHOW_ONLY_LIGHTING = 5;   // together: directed, point and spot lighting
static const int SHOW_ONLY_DIRECTED_LIGHTING = 6;
static const int SHOW_ONLY_POINT_LIGHTING = 7;
static const int SHOW_ONLY_SPOT_LIGHTING = 8;
static const int SHOW_ONLY_DIFFUSE_MAP = 9;
static const int SHOW_ONLY_NORMAL_MAP = 10;


//
// CONSTANT BUFFERS
//
cbuffer cbPerFrame : register(b0)
{
	DirectionalLight  gDirLights[3];
	PointLight        gPointLights[25];
	SpotLight         gSpotLights;
	float3            gEyePosW;             // eye position in world space
	int               gCurrNumPointLights;
	int               gCurrNumSpotLights;
};

cbuffer cbRareChanged : register(b1)
{
	// some flags for controlling the rendering process and
	// params which are changed very rarely

	float3 gFogColor;            // what is the color of fog?
	float  gFogStart;            // how far from camera the fog starts?
	float  gFogRange;            // how far from camera the object is fully fogged?

	int    gNumOfDirLights;      // current number of directional light sources

	bool   gFogEnabled;          // turn on/off the fog effect
	bool   gTurnOnFlashLight;    // turn on/off the flashlight
	bool   gAlphaClipping;       // turn on/off alpha clipping
}

cbuffer cbRareChangedDebug : register(b4)
{
	int    gDebugType;           // current debug state/flags
}


//
// TYPEDEFS
//
struct PS_IN
{
	float4x4 material  : MATERIAL;
	float4   posH      : SV_POSITION;  // homogeneous position
	float3   posW      : POSITION;     // position in world
	float3   normalW   : NORMAL;       // normal in world
	float3   tangentW  : TANGENT;      // tangent in world
	float3   binormalW : BINORMAL;     // binormal in world
	float2   tex       : TEXCOORD;
};

struct PS_OUT
{
	float4 color : SV_Target;
	float  depth : SV_Depth;
};



// ====================================================================================
// PIXEL SHADERS (HELPERS, NOT AN ENTRY POINT)
// ====================================================================================

float4 PS_DebugVectors(PS_IN pin, int debugType) : SV_Target
{
	// visualize normals(1) / tangents(2) / binormals(3) / bumped normal(4) as colors
	switch (debugType)
	{
		case SHOW_NORMALS:
		{
			return float4(pin.normalW, 1.0f);
		}
		case SHOW_TANGENTS:
		{
			return float4(pin.tangentW, 1.0f);
		}
		case SHOW_BINORMALS:
		{
			return float4(pin.binormalW, 1.0f);
		}
		case SHOW_BUMPED_NORMALS:
		{
			float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;
			normalMap = (normalMap * 2.0f) - 1.0f;
			float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, pin.normalW, pin.tangentW);

			return float4(bumpedNormalW, 1.0f);
		}
	}

	return float4(0.5, 0.5f, 0.5f, 1.0f);
}

///////////////////////////////////////////////////////////

float4 PS_DebugTextures(PS_IN pin, int debugType) : SV_Target
{
	float4 color;

	// paint geometry only with texture of chosen type
	switch (debugType)
	{
		case SHOW_ONLY_DIFFUSE_MAP:
		{
			// diffuse map color
			color = gTextures[1].Sample(gSampleType, pin.tex);
			break;
		}
		case SHOW_ONLY_NORMAL_MAP:
		{
			// normal map color
			color = gTextures[6].Sample(gSampleType, pin.tex);
			break;
		}
		default:
		{
			color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		}
	}

	// execute alpha clipping
	if (gAlphaClipping)
		clip(color.a - 0.1f);

	return color;
}



// =================================================================================
// light related debug functions
// =================================================================================

void ComputeSumDirectionalLights(
	Material mat,
	float3 normal,
	float3 toEyeW,
	float specularPower,
	inout float4 ambient,            // sum ambient light contribution for this pixel
	inout float4 diffuse,            // sum diffuse light contribution
	inout float4 specular)           // sum specular light contribution
{
	// start with a sum of zero
	float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// compute a sum the light contribution from each directional light source
	for (int i = 0; i < gNumOfDirLights; ++i)
	{
		ComputeDirectionalLight(
			mat,
			gDirLights[i],
			normal,
			toEyeW,
			specularPower,
			A, D, S);

		ambient += A;
		diffuse += D;
		specular += S;
	}
}

///////////////////////////////////////////////////////////

void ComputeSumPointLights(
	Material mat,
	float3 pos,          // a position of the vertex
	float3 normal,
	float3 toEye,        // a vector from a vertex to the camera (eye) position
	float specPower,     // specular power
	inout float4 ambient,
	inout float4 diffuse,
	inout float4 spec)
{
	// sum the light contribution from each point light source

	// start with a sum of zero
	float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < gCurrNumPointLights; ++i)
	{
		ComputePointLight(
			mat,
			gPointLights[i],
			pos,                 
			normal,
			toEye,
			specPower,
			A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;
	}
}

///////////////////////////////////////////////////////////

void ComputeSumSpotLights(
	Material mat,
	float3 pos,           // a position of the vertex
	float3 normal,
	float3 toEye,         // a vector from a vertex to the camera (eye) position
	float specPower,      // specular power
	inout float4 ambient,
	inout float4 diffuse,
	inout float4 spec)
{
	// sum the light contribution from each spot light source

	// start with a sum of zero
	float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (gTurnOnFlashLight)
	{
		ComputeSpotLight(mat, gSpotLights, pos, normal, toEye, specPower, A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;
	}
}

///////////////////////////////////////////////////////////

float4 PS_DebugLight(PS_IN pin, int debugType) : SV_Target
{
	// lit geometry only with light sources of chosen type

	float4 textureColor = gTextures[1].Sample(gSampleType, pin.tex);

	// execute alpha clipping
	if (gAlphaClipping)
		clip(textureColor.a - 0.1f);

	float4 specColor = gTextures[2].Sample(gSampleType, pin.tex); // specular map
	float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;
	float4 roughnessMap = gTextures[16].Sample(gSampleType, pin.tex);


	normalMap = (normalMap * 2.0f) - 1.0f;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, pin.normalW, pin.tangentW);

	// normal vector for this pixel
	float3 normalW = pin.normalW; // old: bumbedNormalW;

	// --------------------  LIGHT   --------------------

// a vector in the world space from vertex to eye pos
	float3 toEyeW = normalize(gEyePosW - pin.posW);

	// start with a sum of zero
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	Material mat = (Material)pin.material;



	switch (debugType)
	{
		case SHOW_ONLY_LIGHTING:                // all: directed + point + spot
		{

			ComputeSumDirectionalLights(mat, normalW, toEyeW, specColor.x, ambient, diffuse, spec);
			ComputeSumPointLights(mat, pin.posW, normalW, toEyeW, specColor.x, ambient, diffuse, spec);
			break;
		}
		case SHOW_ONLY_DIRECTED_LIGHTING:
		{
			ComputeSumDirectionalLights(mat, normalW, toEyeW, specColor.x, ambient, diffuse, spec);
			break;
		}
		case SHOW_ONLY_POINT_LIGHTING:
		{
			ComputeSumPointLights(mat, pin.posW, normalW, toEyeW, specColor.x, ambient, diffuse, spec);
			break;
		}
		case SHOW_ONLY_SPOT_LIGHTING:
		{
			ComputeSumSpotLights(mat, pin.posW, normalW, toEyeW, specColor.x, ambient, diffuse, spec);
			break;
		}
		default:
		{
			// default color if we have a wrong light debug type
			return float4(1.0f, 0.0f, 1.0f, 1.0f);
		}
	}

/*

	if (debugType == )
	{

	}
	else if (debugType == )
	{

	}
	else if (debugType == )
	{

	}
	else if (debugType == )
	{

	}
	else
	{

	}

*/

	// modulate with late add
	float4 litColor = ambient + diffuse + spec;

	litColor *= roughnessMap;

	// ---------------------  FOG  ----------------------

	if (gFogEnabled)
	{
		float distToEye = length(gEyePosW - pin.posW);
		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		// blend the fog color and the lit color
		litColor = lerp(litColor, float4(gFogColor, 1.0f), fogLerp);
	}

	return litColor;
}

///////////////////////////////////////////////////////////

float4 PS_DebugMaterial(PS_IN pin) : SV_Target
{
	// paint geometry with values of chosen material property;
	// it can be values of ambient/diffuse/specular/reflection;
	return float4(1.0f, 1.0f, 0.0f, 1.0f);
}



// =================================================================================
// PIXEL SHADERS (AN ENTRY POINT)
// =================================================================================
//float4 PS(PS_IN pin) : SV_Target
PS_OUT PS(PS_IN pin)
{
	PS_OUT pout;

	// show normals
	switch (gDebugType)
	{
		case SHOW_NORMALS:
		case SHOW_TANGENTS:
		case SHOW_BINORMALS:
		case SHOW_BUMPED_NORMALS:
		{
			// show normals/tangents/binormals
			pout.color = PS_DebugVectors(pin, gDebugType);
			break;
		}
		case SHOW_ONLY_LIGHTING:
		case SHOW_ONLY_DIRECTED_LIGHTING:
		case SHOW_ONLY_POINT_LIGHTING:
		case SHOW_ONLY_SPOT_LIGHTING:
		{
			pout.color = PS_DebugLight(pin, gDebugType);
			break;
		}
		case SHOW_ONLY_DIFFUSE_MAP:
		case SHOW_ONLY_NORMAL_MAP:
		{
			pout.color = PS_DebugTextures(pin, gDebugType);
			break;
		}
	}

	//pout.color = float4(pin.posH.z - 0.3f, pin.posH.z - 0.3f, pin.posH.z - 0.3f, 1.0f);
	pout.depth = pin.posH.z;// -0.05f;

	return pout;
}