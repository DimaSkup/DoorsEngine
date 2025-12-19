//==================================================================================
// Filename:  cbvs_skinned.hlsli
// Desc:      a const buffer for vertex shaders:
//            contains matrices for bones transformations (model skinning/animation)
//==================================================================================

cbuffer cbSkinned : register(b4)
{
    // max support of 96 bones per character
    matrix gBoneTransforms[96];
}
