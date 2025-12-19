// =================================================================================
// Filename:     grassGS.hlsl
// Description:  a geometry shader (GS) to expand a grass point into a 3 grass planes
// =================================================================================
#include "const_buffers/cb_camera.hlsli"
#include "types/gs_in_basic_inst.hlsli"
#include "types/gs_out_basic_inst.hlsli"


//--------------------------------
// GEOMETRY SHADER
//--------------------------------
[maxvertexcount(3)]
void GS(
    triangle GS_IN gin[3],
    uint primID : SV_PrimitiveID,
    inout TriangleStream<GS_OUT> triStream)
{


    GS_OUT p0 = gin[0];
    GS_OUT p1 = gin[1];
    GS_OUT p2 = gin[2];
	
	float4 posH0 = p0.posH;
	float4 posH1 = p1.posH;
	float4 posH2 = p2.posH;
	
	float3 posW0 = p0.posW;
	float3 posW1 = p1.posW;
	float3 posW2 = p2.posW;
	
	// calc centroid
	float4 cH = (posH0 + posH1 + posH2) * 0.3333;
	float3 cW = (posW0 + posW1 + posW2) * 0.3333;

	
	// scale outward from centroid
	const float scaleFactor = 1.0;
	
	posH0 = cH + (posH0 - cH) * scaleFactor;
	posH1 = cH + (posH1 - cH) * scaleFactor;
	posH2 = cH + (posH2 - cH) * scaleFactor;
	
	posW0 = cW + (posW0 - cW) * scaleFactor;
	posW1 = cW + (posW1 - cW) * scaleFactor;
	posW2 = cW + (posW2 - cW) * scaleFactor;
	
	p0.posH = posH0;
	p1.posH = posH1;
	p2.posH = posH2;
	
	p0.posW = posW0;
	p1.posW = posW1;
	p2.posW = posW2;

    triStream.Append(p0);
    triStream.Append(p1);
    triStream.Append(p2);
    triStream.RestartStrip();
	
}
