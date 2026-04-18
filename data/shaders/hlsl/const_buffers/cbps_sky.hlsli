//==================================================================================
// const buffer for pixel shaders: container of sky/clouds params
//==================================================================================

cbuffer cbSkyBuffer : register(b6)
{
    // cloud translations
    float gFirstTranslationX;
    float gFirstTranslationZ;
    float gSecondTranslationX;
    float gSecondTranslationZ;

    float gBrightness;
    float3 padding;
}
