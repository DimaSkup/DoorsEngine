float4 UnpackColor(uint c)
{

    float r = ((c) & 0xFF) * 0.003921f;
    float g = ((c >> 8) & 0xFF) * 0.003921f;
    float b = ((c >> 16) & 0xFF) * 0.003921f;
    float a = ((c >> 24) & 0xFF) * 0.003921f;

    return float4(r, g, b, a);
}
