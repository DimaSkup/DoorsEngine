// *********************************************************************************
// Filename:    DebugPS.hlsl
// Description: this is a pixel shader for debugging
// 
// Created:     24.11.24
// *********************************************************************************
#include "LightHelper.hlsli"
#include "const_buffers/cbps_per_frame.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cbps_material_colors.hlsli"

//---------------------------
// GLOBALS
//---------------------------
Texture2D    gTextures[22]  : register(t100);
SamplerState gBasicSampler  : register(s0);


// debug states/flags
#define DEFAULT_RENDERING     		 0
#define SHOW_NORMALS_0_TO_1        	 1
#define SHOW_NORMALS 				 2
#define SHOW_NORMALS_FROM_NORMAL_MAP 3
#define SHOW_TANGENTS 				 4
#define SHOW_BINORMALS               5
#define SHOW_BUMPED_NORMALS 		 6
#define SHOW_ONLY_LIGHTING			 7   // together: directed, point and spot lighting
#define SHOW_ONLY_DIRECTED_LIGHTING  8
#define SHOW_ONLY_POINT_LIGHTING 	 9
#define SHOW_ONLY_SPOT_LIGHTING 	 10
#define SHOW_ONLY_DIFFUSE_MAP 		 11
#define SHOW_ONLY_NORMAL_MAP 		 12
#define WIREFRAME 					 13
#define SHOW_MATERIAL_AMBIENT 		 14
#define SHOW_MATERIAL_DIFFUSE		 15
#define SHOW_MATERIAL_SPECULAR		 16
#define SHOW_MATERIAL_REFLECTION 	 17
#define SHOW_AS_ALPHA_CHANNEL        18


//---------------------------
// TYPEDEFS
//---------------------------
struct PS_IN
{
    matrix   world     : WORLD_MATRIX;

    float4   posH      : SV_POSITION;  // homogeneous position
    float3   posW      : POSITION;     // position in world
    float    texU      : TEXCOORD0;
    float3   normalW   : NORMAL;       // normal in world
    float    texV      : TEXCOORD1;
    float4   tangentW  : TANGENT;      // tangent in world
};

struct PS_OUT
{
    float4 color : SV_Target;
    float  depth : SV_Depth;
};


// ====================================================================================
// PIXEL SHADERS (HELPERS, NOT AN ENTRY POINT)
// ====================================================================================

/*
//---------------------------------------------------------
// visualize normals(1) / tangents(2) / binormals(3) / bumped normal(4) as colors
//---------------------------------------------------------
float4 PS_DebugVectors(in PS_IN pin, const in uint debugType)
{
    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

    switch (debugType)
    {
        case SHOW_NORMALS:
        {
            float3 color = 0.5 * normalW + 0.5;
            return float4(color, 1.0);
        }
        case SHOW_TANGENTS:
        {
            float3 color = 0.5 * pin.tangentW + 0.5;
            return float4(color, 1.0);
        }
        case SHOW_BUMPED_NORMALS:
        {
            float2 uv = float2(pin.texU, pin.texV);
            float3 normalMap = gTextures[6].Sample(gBasicSampler, uv).rgb;

            // compute the bumped normal in the world space
            float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);

            float3 color = 0.5 * bumpedNormalW + 0.5;
            return float4(color, 1.0);
        }
    }

    return float4(0.5, 0.5, 0.5, 1.0);
}
*/
/*
//---------------------------------------------------------
// paint geometry only with texture of chosen type
//---------------------------------------------------------
float4 PS_DebugTextures(float2 uv, int debugType)
{
    float4 color;
    
    switch (debugType)
    {
        case SHOW_ONLY_DIFFUSE_MAP:
        {
            color = gTextures[1].Sample(gBasicSampler, uv);
            break;
        } 
        case SHOW_ONLY_NORMAL_MAP:
        {
            color = gTextures[6].Sample(gBasicSampler, uv);
            break;
        }
        default:
        {
            color = float4(1,0,0,1);
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
    float4 A = float4(0,0,0,0);
    float4 D = float4(0,0,0,0);
    float4 S = float4(0,0,0,0);

    // compute a sum the light contribution from each directional light source
    for (int i = 0; i < gCurrNumDirLights; ++i)
    {
        ComputeDirectionalLight(
            mat,
            gDirLights[i],
            normal,
            toEyeW,
            specularPower,
            A, D, S);

        ambient  += A;
        diffuse  += D;
        specular += S;
    }
}

//---------------------------

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
    float4 A = float4(0,0,0,0);
    float4 D = float4(0,0,0,0);
    float4 S = float4(0,0,0,0);

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

//---------------------------

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
    float4 A = float4(0,0,0,0);
    float4 D = float4(0,0,0,0);
    float4 S = float4(0,0,0,0);

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

//---------------------------------------------------------
// Desc:  lit geometry only with light sources of chosen type
//        (or all the light type at once)
//---------------------------------------------------------
float4 PS_DebugLight(PS_IN pin, int debugType)
{
    float specFactor = 0.0f; // specColor.x;


    // -------------------  NORMAL  --------------------

    float2 uv = float2(pin.texU, pin.texV);
    float3 normalMap = gTextures[6].Sample(gBasicSampler, uv).rgb;

    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

    // compute the bumped normal in the world space
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);


    // --------------------  LIGHT   --------------------

    // a vector in the world space from vertex to eye pos
    float3 toEyeW = normalize(gCamPosW - pin.posW);

    // start with a sum of zero
    float4 A = float4(0,0,0,0);      // ambient
    float4 D = float4(0,0,0,0);      // diffuse
    float4 S = float4(0,0,0,0);      // specular

	
	Material mat;
    mat.ambient  = gAmbient;
    mat.diffuse  = gDiffuse;
    mat.specular = gSpecular;
    mat.reflect  = gReflect;

    switch (debugType)
    {
        case SHOW_ONLY_LIGHTING:   // all: directed + point + spot
        {
            ComputeSumDirectionalLights(mat,           bumpedNormalW, toEyeW, specFactor, A, D, S);
            //ComputeSumPointLights      (mat, pin.posW, bumpedNormalW, toEyeW, specFactor, A, D, S);
            //ComputeSumSpotLights       (mat, pin.posW, bumpedNormalW, toEyeW, specFactor, A, D, S);
            break;
        }
        case SHOW_ONLY_DIRECTED_LIGHTING:
        {
            ComputeSumDirectionalLights(mat, bumpedNormalW, toEyeW, specFactor, A, D, S);
            break;
        }
        case SHOW_ONLY_POINT_LIGHTING:
        {
            ComputeSumPointLights(mat, pin.posW, bumpedNormalW, toEyeW, specFactor, A, D, S);
            break;
        }
        case SHOW_ONLY_SPOT_LIGHTING:
        {
            ComputeSumSpotLights(mat, pin.posW, bumpedNormalW, toEyeW, specFactor, A, D, S);
            break;
        }
        default:
        {
            // default color if we have a wrong light debug type
            return float4(1,0,0,1);
        }
    }

    return A + D + S;
}

//---------------------------------------------------------
// Desc:  paint geometry with values of chosen material property;
//        it can be values of ambient/diffuse/specular/reflection;
//---------------------------------------------------------
float4 PS_DebugMaterial(PS_IN pin)
{
    return float4(1,0,0,1);
}
*/


// =================================================================================
// PIXEL SHADER (AN ENTRY POINT)
// =================================================================================
//PS_OUT PS(PS_IN pin)
float4 PS(PS_IN pin) : SV_Target
{
	int i = 0;
	float4 finalColor = float4(1,0,0,1);
	
    float2 uv        = float2(pin.texU, pin.texV);
    float4 diffTex   = gTextures[1].Sample(gBasicSampler, uv);
	float  specPower = gTextures[2].Sample(gBasicSampler, uv).r;
	float3 normalMap = gTextures[6].Sample(gBasicSampler, uv).rgb;

    // execute alpha clipping
    if (gAlphaClipping)
        clip(diffTex.a - 0.1f);
	
	
	// --------------------  NORMAL  --------------------
	  
	// a vector in the world space from vertex to eye pos
    float3 toEyeW = normalize(gCamPosW - pin.posW);
	  
    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

	// for foliage, tree branches, bushes: fix normals (when cull:none)
	if (gAlphaClipping && (dot(toEyeW, normalW) < 0))
		normalW *= -1;

	// compute the bumped normal in the world space
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);

	
	// --------------------  LIGHT   --------------------

	// start with a sum of zero
	float4 ambient = 0;
	float4 diffuse = 0;
	float4 spec    = 0;

	// sum the light contribution from each light source (ambient, diffuse, specular)
	float4 A, D, S;

	Material material;
	material.ambient  = gAmbient;
	material.diffuse  = gDiffuse;
	material.specular = gSpecular;
	material.reflect  = gReflect;
	
	
	switch (gDebugType)
	{
		case DEFAULT_RENDERING:
		{
			// sum the light contribution from the directional light source
			ComputeDirectionalLight(
				material,
				gDirLights[0],
				bumpedNormalW,
				toEyeW,
				specPower,
				A, D, S);

			ambient += A;
			diffuse += D;
			spec += S;
			

			// sum the light contribution from each point light source
			for (i = 0; i < gCurrNumPointLights; ++i)
			{
				ComputePointLight(
					material,
					gPointLights[i],
					pin.posW,
					bumpedNormalW,
					toEyeW,
					specPower,
					A, D, S);

				ambient += A;
				diffuse += D;
				spec += S;
			}

			// sum the light contribution from each spot light source
			for (i = 1; i < gCurrNumSpotLights; ++i)
			{
				ComputeSpotLight(
					material,
					gSpotLights[i],
					pin.posW,
					bumpedNormalW, 
					toEyeW,
					specPower,
					A, D, S);

				ambient += A;
				diffuse += D;
				spec += S;
			}

			// modulate with late add
			float4 litColor = (diffTex * (ambient + diffuse) + spec);
			
			finalColor = litColor;
			break;
		}
		case SHOW_NORMALS_0_TO_1:
		{
			// pack normal into [0,1]
            float3 color = 0.5 * normalW + 0.5;
            finalColor = float4(color, 1.0);
			break;
		}
		case SHOW_NORMALS:
        {
			finalColor = float4(normalW, 1.0);
			break;
        }
		case SHOW_NORMALS_FROM_NORMAL_MAP:
		{
			float3 normL = 2.0 * normalMap - 1.0;
			float3 normW = normalize(mul(normL, (float3x3)pin.world));
			finalColor = float4(normW, 1.0);
			break;
		}
        case SHOW_TANGENTS:
        {
			const float3 N = normalW;
			const float3 T = pin.tangentW.xyz;

			// orthonormalize T
			float3 othonormT = normalize(T - N * dot(N, T));
	
			finalColor = float4(othonormT, 1.0) * pin.tangentW.w;

			break;
        }
		case SHOW_BINORMALS:
		{
			// Build orthonormal basis.
			float3 N = normalW;
			float3 T = normalize(pin.tangentW.xyz - N * dot(pin.tangentW.xyz, N));
			float3 B = cross(N, T) * pin.tangentW.w;
			
			finalColor = float4(B, 1.0);
			
			break;
		}
        case SHOW_BUMPED_NORMALS:
        {
            float2 uv        = float2(pin.texU, pin.texV);
            float3 normalSample = gTextures[6].Sample(gBasicSampler, uv).rgb;

			float3 tsNormal = 2.0 * normalSample - 1.0;
			
			// Build orthonormal basis.
			float3 N = normalW;
			float3 T = normalize(pin.tangentW.xyz - N * dot(pin.tangentW.xyz, N));
			float3 B = cross(N, T) * pin.tangentW.w;
			
			finalColor = float4(mul(tsNormal, float3x3(T, B, N)), 1.0);

			break;
        }
		case SHOW_ONLY_DIFFUSE_MAP:
        {
            finalColor = gTextures[1].Sample(gBasicSampler, uv);
            break;
        } 
        case SHOW_ONLY_NORMAL_MAP:
        {
            finalColor = gTextures[6].Sample(gBasicSampler, uv);
            break;
        }
		case SHOW_AS_ALPHA_CHANNEL:
		{
			finalColor = float4(1,1,1,1);
			break;
		}
		case SHOW_ONLY_LIGHTING:
		{
			const float2 uv        = float2(pin.texU, pin.texV);
			const float3 normalMap = gTextures[6].Sample(gBasicSampler, uv).rgb;

			// compute the bumped normal in the world space
			const float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);


			// --------------------  LIGHT   --------------------

			// start with a sum of zero
			float4 A = 0;      // ambient
			float4 D = 0;      // diffuse
			float4 S = 0;      // specular
		
			Material mat;
			mat.ambient  = gAmbient;
			mat.diffuse  = gDiffuse;
			mat.specular = gSpecular;
			mat.reflect  = gReflect;
			
			  ComputeDirectionalLight(
				mat,
				gDirLights[0],
				bumpedNormalW,
				toEyeW,
				specPower,
				A, D, S);
				
			finalColor = A + D + S;
			break;
		}
		case SHOW_ONLY_DIRECTED_LIGHTING:
		{
			float2 uv = float2(pin.texU, pin.texV);
			float3 normalMap = gTextures[6].Sample(gBasicSampler, uv).rgb;
			
			// compute the bumped normal in the world space
			float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);

			// --------------------  LIGHT   --------------------

			// start with a sum of zero
			float4 A = float4(0,0,0,0);      // ambient
			float4 D = float4(0,0,0,0);      // diffuse
			float4 S = float4(0,0,0,0);      // specular
		
			Material mat;
			mat.ambient  = gAmbient;
			mat.diffuse  = gDiffuse;
			mat.specular = gSpecular;
			mat.reflect  = gReflect;
			
			  ComputeDirectionalLight(
				mat,
				gDirLights[0],
				bumpedNormalW,
				toEyeW,
				specPower,
				A, D, S);
				
			finalColor = A + D + S;
			break;
		}
	}
	
	
	/*
	
	
    // show normals
    switch (debugType)
    {
        case SHOW_NORMALS:
        case SHOW_TANGENTS:
        case SHOW_BUMPED_NORMALS:
        {
            // show normals/tangents/bumped_normals
            finalColor = PS_DebugVectors(pin, debugType);
            break;
        }
        case SHOW_ONLY_LIGHTING:
        case SHOW_ONLY_DIRECTED_LIGHTING:
        case SHOW_ONLY_POINT_LIGHTING:
        case SHOW_ONLY_SPOT_LIGHTING:
        {
            //finalColor = PS_DebugLight(pin, gDebugType);
            break;
        }
        case SHOW_ONLY_DIFFUSE_MAP:
        case SHOW_ONLY_NORMAL_MAP:
        {
            //finalColor = PS_DebugTextures(uv, debugType);
            break;
        }
        case WIREFRAME:
        {
            //finalColor = float4(1, 1, 1, 1);
            break;
        }
    }
*/

    //pout.depth = pin.posH.z;

    return float4(finalColor.rgb, 1.0);
}
