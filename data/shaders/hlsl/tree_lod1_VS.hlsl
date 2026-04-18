//==================================================================================
// Desc:  a vertex shader which is used as default (texturing + lighting)
//==================================================================================
#include "helpers/fog.hlsli"
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "const_buffers/cb_camera.hlsli"

// how many different billboard images we have for this lod
#define NUM_BILLBOARD_VIEWS 16


//---------------------------
// TYPEDEFS
//---------------------------
struct VS_IN
{
    // data per instance
    row_major matrix   world             : WORLD;
    row_major float4x4 material          : MATERIAL;
    uint               instanceID        : SV_InstanceID;

    // data per vertex
    float3   posL : POSITION;            // stores width and height (in X and Y)          
};

struct VS_OUT
{
    float4x4 material  : MATERIAL;
    matrix   world     : WORLD;

    float3   posW      : POSITION;       // origin position in world
	float3   fogColor  : COLOR;
	float    blend     : BLEND;         // blend factor between two billboards
    float2   sizeW     : TEXCOORD0;
	float2   stage     : TEXCOORD1;     // indices of left and right billboards
};

//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;
	
    vout.fogColor = GetFogColor();
    vout.material = vin.material;
    vout.world    = vin.world;
	
    // use only world translation
    vout.posW = float3(vin.world[3][0], vin.world[3][1], vin.world[3][2]);
	
	// calc camera local position relatively to the lod instance
	float2 toEye = normalize(gCamPosW.xz - vout.posW.xz);
	
	// transform the vector using a world matrix of the tree
	toEye = mul(float3(toEye.x, 0, toEye.y), (float3x3)vin.world).xz;
	
	const float PI = 3.14159;
	float angle = (atan2(toEye.y, toEye.x) + PI) / (2.0*PI) - (0.25*PI);
	
	// index of the left and right billboard image for this lod
	vout.stage.x = floor(angle * NUM_BILLBOARD_VIEWS);
	vout.stage.y = ceil(angle * NUM_BILLBOARD_VIEWS);

    // width and height are stored in x and y component of input local position
	const float scale = length(vin.world[0].xyz);
	vout.sizeW.x = vin.posL.x * scale;
	vout.sizeW.y = vin.posL.y * scale;
	
	// pass blend factor between 2 billboard images
	vout.blend = angle * NUM_BILLBOARD_VIEWS - vout.stage.x;
	
    return vout;
}
