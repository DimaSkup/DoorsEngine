//==================================================================================
// Desc:  a vertex shader for grass rendering
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cb_weather.hlsli"


//--------------------------------
// GLOBALS
//--------------------------------
TextureCube gCubeMap : register(t0);
SamplerState gSkySampler : register(s1);

//--------------------------------
// TYPEDEFS
//--------------------------------
struct VS_IN
{
    // data per instance
    float3 worldPos : WORLD_POS;
    uint texColumn : TEX_COLUMN;
    uint texRow : TEX_ROW;
    uint instanceID : SV_InstanceID;

    // data per vertex
    float3 posL : POSITION; // vertex position in local space
    float2 tex : TEXCOORD;
    float3 normalL : NORMAL; // vertex normal in local space
    float4 tangentL : TANGENT; // tangent in local space
};


struct VS_OUT
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
    float3 fogColor : COLOR;
};

//--------------------------------
// VERTEX SHADER
//--------------------------------
VS_OUT VS(VS_IN vin)
{
	//
	// CONSTANTS
	//
	const float numCols = 4.0;
	const float numRows = 4.0;
	const float sqrDistFullSize   = gDistGrassFullSize * gDistGrassFullSize;
	const float sqrDistMaxVisible = gDistGrassVisible * gDistGrassVisible;
	
	
	VS_OUT vout;

    vout.fogColor = gCubeMap.SampleLevel(gSkySampler, float3(0, -490, 0), 0).rgb;

	// vector from vertex to camera (both in world space)
    float3 toEyeW = gCamPosW - vin.worldPos;
	const float distSqr = dot(toEyeW, toEyeW);
	
	
	float sizeFactor = 1.0;
	
	// if grass is out of visibility
	if (distSqr > sqrDistMaxVisible)
	{
		sizeFactor = 0;
	}
	
	// farther grass becomes smaller...
	else if (distSqr > sqrDistFullSize)
	{
		sizeFactor -= (distSqr-sqrDistFullSize) / (sqrDistMaxVisible-sqrDistFullSize);
	}
	
	// scale grass instance
	const float3 posL = vin.posL * sizeFactor;
	
	// transform to world space
	vout.posW = posL + vin.worldPos;
  
    // transform to homogeneous clip space
    vout.posH = mul(float4(vout.posW, 1.0), gViewProj);

    vout.normal = vin.normalL;
	
	float3 vertToEyeW = gCamPosW - vout.posW;
	
    //if (dot(vertToEyeW, vout.normal) <= 0)
    //    vout.normal *= -1;
	
	// setup texture coords
	const float tu = 1.0 / numCols;
	const float tv = 1.0 / numRows;
	
	// if left edge of tex image...
	if (vin.tex.x == 0)
		vout.tex.x = tu * vin.texColumn;
	
	// right edge...
	else 
		vout.tex.x = tu * vin.texColumn + tu;
	
	// if top edge of tex image...
	if (vin.tex.y == 0)
		vout.tex.y = tv * vin.texRow;
	
	// bottom edge...
	else
		vout.tex.y = tv * vin.texRow + tv;
		
	
    return vout;
}
