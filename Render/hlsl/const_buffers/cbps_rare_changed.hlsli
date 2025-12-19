//==================================================================================
// const buffer for pixel shaders: container for rare changed params
//==================================================================================

cbuffer cbRareChanged : register(b1)
{
    // some flags for controlling the rendering process and
    // params which are changed very rarely

	int    gDebugType;
    int    gFogEnabled;          // turn on/off the fog effect
    int    gTurnOnFlashLight;    // turn on/off the flashlight
	int    gAlphaClipping;       // turn on/off alpha clipping
};
