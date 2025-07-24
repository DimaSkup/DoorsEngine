// =================================================================================
// Filename:     billboardGS.hlsl
// Description:  a geometry shader (GS) to expand a point sprite into a y-axis
//               aligned billboard that faces the camera
// =================================================================================


// ==========================
// CONSTANT BUFFERS
// ==========================
cbuffer cbPerFrame : register(b0)
{
	matrix gViewProj;
	float3 gEyePosW;                // eye position in world space  
};


// ==========================
// TYPEDEFS
// ==========================
struct GS_IN
{
	float3 centerW  : POSITION;       // billboard center pos in a world space
    float   translucency : TRANSLUCENCY;
    float3 color    : COLOR;
	float2 sizeW    : SIZE;           // width and height of the billboard
};

struct GS_OUT
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
    float   translucency : TRANSLUCENCY;
    float3 color   : COLOR;
	float3 normalW : NORMAL;
	float2 tex     : TEXCOORD;
	uint   primID  : SV_PrimitiveID;
};


// ==========================
// GEOMETRY SHADER
// ==========================

[maxvertexcount(4)]
void GS(
	point GS_IN gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GS_OUT> triStream)
{
	// we expand each point into a quad (4 vertices), so the maximum number
	// of vertices we output per geometry shader invocation is 4

	float2 gTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	// compute the local coordinate system of the sprite relative to the world space
	// such that the billboard is aligned to the y-axis and faces the eye
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = gEyePosW - gin[0].centerW;

	look.y = 0.0f;
	look = normalize(look);

	// y-axis aligned, so project to xz-plane
	//look = normalize(float3(look.x, 0.0f, look.z));

	float3 right = cross(up, look);

	// compute triangle strip vertices (quad) in world space
	float halfWidth  = 0.5f * gin[0].sizeW.x;
	float halfHeight = 0.5f * gin[0].sizeW.y;

	//float halfWidth = 10;
	//float halfHeight = 10;

	float4 v[4];
	v[0] = float4(gin[0].centerW + (halfWidth * right) - (halfHeight * up), 1.0f);
	v[1] = float4(gin[0].centerW + (halfWidth * right) + (halfHeight * up), 1.0f);
	v[2] = float4(gin[0].centerW - (halfWidth * right) - (halfHeight * up), 1.0f);
	v[3] = float4(gin[0].centerW - (halfWidth * right) + (halfHeight * up), 1.0f);

	// transform quad vertices to world space and output them as a triangle strip
	GS_OUT gout;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		gout.posH    = mul(v[i], gViewProj);
		gout.posW    = v[i].xyz;
        gout.translucency = gin[0].translucency;
        gout.color   = gin[0].color;
		gout.normalW = look;
		gout.tex     = gTexC[i];
		gout.primID  = primID;
		triStream.Append(gout);
	}
}
