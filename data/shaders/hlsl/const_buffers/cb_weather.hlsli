//==================================================================================
// Filename:  cb_weather.hlsli
// Desc:      COMMON const buffer, container for wather param
//==================================================================================

cbuffer cbWeather : register(b12)
{
    float3 gWindDir;
    float  gWindSpeed;
    float  gWaveAmplitude;   // control how far blades bend
    float  gWindStrength;    // control how far blades bend
    float  gTurbulence;      // adds jitter / small ripples (noise)
    float  gGustDecay;
    float  gGustPower;
    float  gWaveFrequency;   // controls wavelength; higher = faster small waves
    float  gBendScale;
    float  gSwayDistance;    // from us (camera) and to this distance the swaying is linearly goes down
                             // after this distance we see no swaying
							 
							 
    float3 gFixedFogColor;       // what is the color of fog?
    float  gFogStart;            // how far from camera the fog starts?

    float3 gSkyColorCenter;      // near horizon
    float  gFogRange;            // how far from camera the object is fully fogged?

    float3 gSkyColorApex;        // top color
}
