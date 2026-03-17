//==================================================================================
// Filename:  cbps_material_colors.hlsli
// Desc:      pixel shader constant buffer for material colors
//==================================================================================

cbuffer cbMaterialColors : register(b5)
{
    float4 gAmbient;
    float4 gDiffuse;
    float4 gSpecular;
    float4 gReflect;
};