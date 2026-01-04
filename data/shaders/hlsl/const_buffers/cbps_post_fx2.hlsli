//==================================================================================
// const buffer for pixel shaders: container of post effects params
//==================================================================================

//---------------------------------------------------------
// Const buffer for post effects parameters
//---------------------------------------------------------
cbuffer cbPostFx : register(b7)
{
    float4 gPostFxParams[64];
}

//---------------------------------------------------------
// Desc:  since floats are grouped in float4 registers
//        we have to calc index to each separate float
//        and return a value by this index
// Args:  a value from ePostFxParam (look at the included header)
//---------------------------------------------------------
inline float GetPostFxParam(const int paramType)
{
    // arr[paramType / 4][paramType % 4]
    return gPostFxParams[paramType >> 2][paramType & 3];
}
