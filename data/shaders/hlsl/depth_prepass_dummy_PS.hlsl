//==================================================================================
// dummy pixel shader for depth pre-pass
// (we don't use it, but I don't want to fix shaders loader)
//==================================================================================
float4 PS() : SV_Target
{
    return float4(1,0,0,1);
}
