//==================================================================================
// Filename:  cb_time.hlsli
// Desc:      COMMON const buffer, container for time values
//==================================================================================

cbuffer cbTime : register(b10)
{
    float gDeltaTime;
    float gGameTime;         // game/application time
}
