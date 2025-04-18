/////////////////////////////////////////////////////////////////////
// Filename: color.ps
/////////////////////////////////////////////////////////////////////


cbuffer cbRareChanged : register(b1)
{
	// some flags for controlling the rendering process and
	// params which are changed very rarely

	float3 gFogColor;            // what is the color of fog?
	float  gFogStart;            // how far from camera the fog starts?
	float  gFogRange;            // how far from camera the object is fully fogged?

	int    gNumOfDirLights;      // current number of directional light sources

    int   gFogEnabled;          // turn on/off the fog effect
    int   gTurnOnFlashLight;    // turn on/off the flashlight
    int   gAlphaClipping;       // turn on/off alpha clipping
}

/////////////////////////////
// TYPEDEFS
/////////////////////////////
struct PS_INPUT
{
	float4 posH   : SV_POSITION;    // homogeneous position
	float3 posW   : POSITION;       // position in world
	float4 color  : COLOR;          // color of the vertex
};


/////////////////////////////
// PIXEL SHADER
/////////////////////////////
float4 PS(PS_INPUT pin) : SV_TARGET
{
	return float4(gFogColor, 1.0f);
};
