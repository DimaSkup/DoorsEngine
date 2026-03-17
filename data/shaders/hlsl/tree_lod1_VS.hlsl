//==================================================================================
// Desc:  a vertex shader which is used as default (texturing + lighting)
//==================================================================================
#include "const_buffers/cb_view_proj.hlsli"
#include "const_buffers/cb_time.hlsli"
#include "const_buffers/cb_weather.hlsli"
#include "const_buffers/cb_camera.hlsli"


//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
SamplerState gSkySampler    : register(s1);

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
    float3   posL                        : POSITION;       // vertex position in local space
    float2   tex : TEXCOORD;
};

struct VS_OUT
{
    float4x4 material           : MATERIAL;
    matrix   worldInvTranspose  : WORLD_INV_TRANSPOSE;

    float3   posW               : POSITION;       // origin position in world
	float4   fogColor           : COLOR;
    float2   sizeW              : TEXCOORD0;
	float2   stage              : TEXCOORD1;     // indices of left and right billboard images
};


float GetAngleFromXZ(const float x, const float z)
{
	float pi = 3.14159;
    float theta = 0.0f;

    // quadrant I or IV
    if (x >= 0.0f)
    {
        // if x == 0, then atanf(y/x) = +pi/2 if y > 0
        //                 atanf(y/x) = -pi/2 if y < 0
        theta = atan(z/x);            // in [-pi/2, +pi/2]

        if (theta < 0.0f)
            theta += pi;  // in [0, 2*pi).
    }

    // quadrant II or III
    else
        theta = atan(z/x) + pi;

    return theta;
}


//---------------------------
// VERTEX SHADER
//---------------------------
VS_OUT VS(VS_IN vin)
{
	const int numBillboardViews = 16;
	
    const float4 skyBottomColor = gCubeMap.SampleLevel(gSkySampler, float3(0, -490, 0), 0);

    VS_OUT vout;
	

    vout.material          = vin.material;
    vout.worldInvTranspose = vin.world;
	
    // use only world translation
    vout.posW = float3(vin.world[3][0], vin.world[3][1], vin.world[3][2]);
	
	// calc camera local position relatively to the lod instance
	float2 hdiff = normalize(vout.posW.xz - gCamPosW.xz);
	
	hdiff = mul(float3(hdiff, 0.0f), (float3x3)vin.world).xy;
	
	float pi = 3.14159;
	float a = (atan2(hdiff.y, hdiff.x) + pi) / (2.0 * pi) - 0.25;
	
	float3 worldRot = mul(float4(1,0,0,0), vin.world).xyz;
	
	a -= GetAngleFromXZ(worldRot.x, worldRot.z);
	a = fmod(a, 1.0);
	
	// store the index of the left and right billboard image for this lod
	vout.stage.x = floor(a * numBillboardViews);
	vout.stage.y = ceil(a * numBillboardViews);
	
		
	//stageL &= 15;
	//stageR &= 15;
	
	
	// blend sky pixel color with fixed fog color
    vout.fogColor = skyBottomColor * float4(gFixedFogColor, 1.0);

    // width and height are stored in x and y component of position
	const float scale = length(vin.world[0].xyz);
	vout.sizeW.x = vin.posL.x * scale * 0.95;
	vout.sizeW.y = vin.posL.y * scale;
	
	// pass blend factor between 2 billboard images in the w-component of fogColor
	vout.fogColor.w = a * numBillboardViews - vout.stage.x;
	
	
    return vout;
}
