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
Texture2D    gTextures[128] : register(t0);
SamplerState gSampleType   : register(s0);

// debug states/flags
static const int SHOW_NORMALS = 1;
static const int SHOW_TANGENTS = 2;
static const int SHOW_BUMPED_NORMALS = 3;
static const int SHOW_ONLY_LIGHTING = 4;   // together: directed, point and spot lighting
static const int SHOW_ONLY_DIRECTED_LIGHTING = 5;
static const int SHOW_ONLY_POINT_LIGHTING = 6;
static const int SHOW_ONLY_SPOT_LIGHTING = 7;
static const int SHOW_ONLY_DIFFUSE_MAP = 8;
static const int SHOW_ONLY_NORMAL_MAP = 9;
static const int WIREFRAME = 10;


//
// CONSTANT BUFFERS
//
cbuffer cbPerFrame : register(b0)
{
	DirectionalLight  gDirLights[3];
	PointLight        gPointLights[25];
	SpotLight         gSpotLights[25];
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

	int    gNumDirLights;       // current number of directional light sources

    int   gFogEnabled;          // turn on/off the fog effect
    int   gTurnOnFlashLight;    // turn on/off the flashlight
    int   gAlphaClipping;       // turn on/off alpha clipping
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

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

	switch (debugType)
	{
		case SHOW_NORMALS:
		{
            float3 color = 0.5f * normalW + 0.5f;
			return float4(color, 1.0f);
		}
		case SHOW_TANGENTS:
		{
            float3 color = 0.5f * pin.tangentW + 0.5f;
            return float4(color, 1.0f);
		}
		case SHOW_BUMPED_NORMALS:
		{
			float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;

            // compute the bumped normal in the world space
            float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);

            float3 color = 0.5f * bumpedNormalW + 0.5f;
			return float4(color, 1.0f);
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
		case SHOW_ONLY_DIFFUSE_MAP:   // diffuse map color
		{
			color = gTextures[1].Sample(gSampleType, pin.tex);

            // execute alpha clipping
            if (gAlphaClipping)
                clip(color.a - 0.1f);

			break;
		} 
		case SHOW_ONLY_NORMAL_MAP:    // normal map color
		{
			color = gTextures[6].Sample(gSampleType, pin.tex);
			break;
		}
		default:
		{
			color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		}
	}

	return color;
}



// =================================================================================
// light related debug functions
// =================================================================================

void ComputeSumDirectionalLights(
	Material mat,
	float3 normal,                   // normal vector of the pixel
	float3 toEyeW,                   // a vector from a vertex to the camera (eye) position
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
	for (int i = 0; i < gNumDirLights; ++i)
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
	float3 normal,       // normal vector of the pixel
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
	float3 normal,        // normal vector of the pixel
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

    // it is supposed that player's flashlight is a spot light by idx 0
	if (gTurnOnFlashLight)
	{
        ComputeSpotLight(mat, gSpotLights[0], pos, normal, toEye, specPower, A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
	}

    // ...compute the rest of spotlights
    for (int i = 1; i < gCurrNumSpotLights; ++i)
    {
        ComputeSpotLight(mat, gSpotLights[i], pos, normal, toEye, specPower, A, D, S);

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

	//float4 specColor = gTextures[2].Sample(gSampleType, pin.tex); // specular map
	float3 normalMap = gTextures[6].Sample(gSampleType, pin.tex).rgb;
	//float4 roughnessMap = gTextures[16].Sample(gSampleType, pin.tex);

    float specFactor = 0.0f; // specColor.x;

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

    // compute the bumped normal in the world space
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);

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
		case SHOW_ONLY_LIGHTING:   // all: directed + point + spot
		{
			ComputeSumDirectionalLights(mat,           bumpedNormalW, toEyeW, specFactor, ambient, diffuse, spec);
			ComputeSumPointLights      (mat, pin.posW, bumpedNormalW, toEyeW, specFactor, ambient, diffuse, spec);
            ComputeSumSpotLights       (mat, pin.posW, bumpedNormalW, toEyeW, specFactor, ambient, diffuse, spec);
			break;
		}
		case SHOW_ONLY_DIRECTED_LIGHTING:
		{
			ComputeSumDirectionalLights(mat, bumpedNormalW, toEyeW, specFactor, ambient, diffuse, spec);
			break;
		}
		case SHOW_ONLY_POINT_LIGHTING:
		{
			ComputeSumPointLights(mat, pin.posW, bumpedNormalW, toEyeW, specFactor, ambient, diffuse, spec);
			break;
		}
		case SHOW_ONLY_SPOT_LIGHTING:
		{
			ComputeSumSpotLights(mat, pin.posW, bumpedNormalW, toEyeW, specFactor, ambient, diffuse, spec);
			break;
		}
		default:
		{
			// default color if we have a wrong light debug type
			return float4(1.0f, 0.0f, 1.0f, 1.0f);
		}
	}

	return ambient + diffuse + spec;
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
PS_OUT PS(PS_IN pin)
{
	PS_OUT pout;

	// show normals
	switch (gDebugType)
	{
		case SHOW_NORMALS:
		case SHOW_TANGENTS:
		case SHOW_BUMPED_NORMALS:
		{
			// show normals/tangents/bumped_normals
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
        case WIREFRAME:
        {
            pout.color = float4(1, 1, 1, 1);
            break;
        }
	}

	pout.depth = pin.posH.z;

	return pout;
}
