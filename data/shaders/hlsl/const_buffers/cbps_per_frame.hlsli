//==================================================================================
// const buffer for pixel shaders: container for params changed each frame
//==================================================================================
cbuffer cbPerFrame    : register(b0)
{
    // light sources data
    DirectionalLight  gDirLights[3];
    PointLight        gPointLights[25];
    SpotLight         gSpotLights[25];
    int               gCurrNumDirLights;
    int               gCurrNumPointLights;
    int               gCurrNumSpotLights;
};
