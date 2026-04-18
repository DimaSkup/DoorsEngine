//==================================================================================
// Filename:  cb_grass.hlsli
// Desc:      COMMON const buffer, container for grass param
//==================================================================================
#ifndef CB_GRASS_HLSLI
#define CB_GRASS_HLSLI

cbuffer cbGrass : register(b7)
{
	 // range from camera where grass is full sized
	float gDistGrassFullSize;    

	// grass visibility range
    float gDistGrassVisible;

	// how many texture columns/rows we have on a grass atlas
	float gNumTexColumns;        
	float gNumTexRows;        
}

#endif
