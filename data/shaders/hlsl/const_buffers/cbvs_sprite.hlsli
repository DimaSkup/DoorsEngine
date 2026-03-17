//==================================================================================
// Filename:  cbvs_sprite.hlsli
// Desc:      a const buffer for vertex shaders:
//            contains data for generation of 2D sprite
//==================================================================================
cbuffer Sprite : register(b5)
{
    float gSpriteLeft;
    float gSpriteTop;
    float gSpriteWidth;
    float gSpriteHeight;
}
