///////////////////////////////////////////////////////////////////////////////////////////
// Filename:      model_importer_helpers.h
// Description:   contains private helpers for the ModelImporter class;
// 
// Created:       16.02.24
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Texture/texture.h"
#include "animation_mgr.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define PRINT_MESHES_DBG_INFO 0


namespace Core
{


//---------------------------------------------------------

static int s_SpaceCount = 0;

void PrintSpace()
{
    for (int i = 0; i < s_SpaceCount; ++i)
        printf(" ");
}

//---------------------------------------------------------
// determine the type of texture storage by input material and texture type
//---------------------------------------------------------
TexStoreType DetermineTexStoreType(
    const aiScene* pScene,
    const aiMaterial* pMaterial,
    const UINT index,
    const aiTextureType textureType)
{
    if (pMaterial->GetTextureCount(textureType) == 0)
        return TexStoreType::None;

    // get path to the texture
    aiString path;
    pMaterial->GetTexture(textureType, index, &path);

    // ---------------------------------------------

    // check if texture is an embedded indexed texture by seeing if the file path is an index #
    if (path.C_Str()[0] == '*')
    {
        if (pScene->mTextures[0]->mHeight != 0)
        {
            LogErr(LOG, "SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSES TEXTURES");
            return TexStoreType::EmbeddedIndexNonCompressed;
        }

        return TexStoreType::EmbeddedIndexCompressed;
    }

    // ---------------------------------------------

    // check if texture is an embedded texture but not indexed (path will be the texture's name instead of #)
    if (const aiTexture* pTex = pScene->GetEmbeddedTexture(path.C_Str()))
    {
        if (pTex->mHeight != 0)
        {
            LogErr(LOG, "SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSES TEXTURES");
            return TexStoreType::EmbeddedNonCompressed;
        }

        return TexStoreType::EmbeddedCompressed;
        
    }

    // ---------------------------------------------

    // lastly check if texture is stored on the disk
    // (just check for '.' before the extension)
    if (strchr(path.C_Str(), '.') != NULL)
    {
        return TexStoreType::Disk;
    }

    // ---------------------------------------------

    // no texture exists
    return TexStoreType::None;   

} 

//---------------------------------------------------------
// return an index of the embedded compressed texture by path pStr
//---------------------------------------------------------
UINT GetIndexOfEmbeddedCompressedTexture(aiString* pStr)
{
    // assert that path is "*0", "*1", or something like that
    if (!(pStr->length >= 2))
        return 0;

    // return an index
    return (UINT)atoi(&pStr->C_Str()[1]); 
}

//---------------------------------------------------------
// Desc:  print out an ASSIMP matrix values into the console
//---------------------------------------------------------
void PrintAssimpMatrix(const aiMatrix4x4& m)
{
    PrintSpace(); printf("%f %f %f %f\n", m.a1, m.a2, m.a3, m.a4);
    PrintSpace(); printf("%f %f %f %f\n", m.b1, m.b2, m.b3, m.b4);
    PrintSpace(); printf("%f %f %f %f\n", m.c1, m.c2, m.c3, m.c4);
    PrintSpace(); printf("%f %f %f %f\n", m.d1, m.d2, m.d3, m.d4);
}

//---------------------------------------------------------
// Desc:  convert from ASSIMP matrix to DirectXMath matrix
//---------------------------------------------------------
void AiToXmMatrix(const aiMatrix4x4& m, DirectX::XMMATRIX& outMat)
{
    outMat =
    {
        m.a1, m.a2, m.a3, m.a4,
        m.b1, m.b2, m.b3, m.b4,
        m.c1, m.c2, m.c3, m.c4,
        m.d1, m.d2, m.d3, m.d4
    };
}

//---------------------------------------------------------
// Desc:  load offset matrix for input bone as well as
//        weights + bone ids for each vertex related to this particular bone
//---------------------------------------------------------
void LoadSingleBone(const int meshIdx, const aiBone* pBone, AnimSkeleton& skeleton)
{
    // add a new bone or just get id by name
    const int boneId = skeleton.AddBone(pBone->mName.C_Str());

    // debug info
    //printf("Bone_%d %-20s (of mesh %d) num vertices affected by this bone: %d\n", boneId, pBone->mName.C_Str(), meshIdx, pBone->mNumWeights);

    // if this is a new bone then we store its offset matrix
    if (boneId == skeleton.boneOffsets_.size())
    {
        DirectX::XMMATRIX offsetMatrix;
        AiToXmMatrix(pBone->mOffsetMatrix, offsetMatrix);
        skeleton.boneOffsets_.push_back(DirectX::XMMatrixTranspose(offsetMatrix));
    }

    // store weights + bone ids
    for (uint i = 0; i < pBone->mNumWeights; ++i)
    {
        const aiVertexWeight&  vw = pBone->mWeights[i];
        const uint globalVertexId = skeleton.meshBaseVertex[meshIdx] + vw.mVertexId;

        // debug info
        //printf("  vertex_id %d, ", globalVertexId);

        assert(globalVertexId < skeleton.vertexToBones.size());
        skeleton.vertexToBones[globalVertexId].AddBoneData(boneId, vw.mWeight);
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void LoadMeshBones(const int meshIdx, const aiMesh* pMesh, AnimSkeleton& skeleton)
{
    for (uint i = 0; i < pMesh->mNumBones; ++i)
    {
        LoadSingleBone(meshIdx, pMesh->mBones[i], skeleton);
    }
}

//---------------------------------------------------------
// Desc:  find an animation for a node by input name
//---------------------------------------------------------
const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const char* nodeName)
{
    assert(pAnimation);
    assert(nodeName && nodeName[0] != '\0');

    for (uint i = 0; i < pAnimation->mNumChannels; ++i)
    {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (strcmp(pNodeAnim->mNodeName.data, nodeName) == 0)
            return pNodeAnim;
    }

    return nullptr;
}


//**********************************************************************************
// INTERPOLATIONS:
//**********************************************************************************

//---------------------------------------------------------
// Desc:  find index of translation keyframe by input time
//---------------------------------------------------------
uint FindPositionIdx(const float animTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim);
    assert(pNodeAnim->mNumPositionKeys > 0);

    for (uint i = 0; i < pNodeAnim->mNumPositionKeys - 1; ++i)
    {
        const float tNext = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (animTimeTicks < tNext)
            return i;
    }

    return 0;
}

//---------------------------------------------------------
// Desc:  find index of rotation keyframe by input time
//---------------------------------------------------------
uint FindRotationIdx(const float animTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim);
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0; i < pNodeAnim->mNumRotationKeys - 1; ++i)
    {
        const float tNext = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
        if (animTimeTicks < tNext)
            return i;
    }

    return 0;
}

//---------------------------------------------------------
// Desc:  find index of scaling keyframe by input time
//---------------------------------------------------------
uint FindScalingIdx(const float animTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim);
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint i = 0; i < pNodeAnim->mNumScalingKeys - 1; ++i)
    {
        const float tNext = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
        if (animTimeTicks < tNext)
            return i;
    }

    return 0;
}

//---------------------------------------------------------
// Desc:  compute translation value for keyframe at time pos
//---------------------------------------------------------
void CalcInterpolatedTranslation(
    const float animTimeTicks,
    const aiNodeAnim* pNodeAnim,
    aiVector3D& out)
{
    assert(pNodeAnim);

    // we need at two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1)
    {
        out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    const uint idxCurr = FindPositionIdx(animTimeTicks, pNodeAnim);
    const uint idxNext = idxCurr + 1;
    assert(idxNext < pNodeAnim->mNumPositionKeys);

    const float t1     = (float)pNodeAnim->mPositionKeys[idxCurr].mTime;
    const float t2     = (float)pNodeAnim->mPositionKeys[idxNext].mTime;
    const float dt     = t2 - t1;
    const float factor = (animTimeTicks - t1) / dt;
    assert(factor >= 0.0f && factor <= 1.0f);

    const aiVector3D& start = pNodeAnim->mPositionKeys[idxCurr].mValue;
    const aiVector3D& end   = pNodeAnim->mPositionKeys[idxNext].mValue;

    aiVector3D delta = end - start;
    out = start + factor * delta;
}

//---------------------------------------------------------
// Desc:  compute a rotation quaternion for keyframe at time pos
//---------------------------------------------------------
void CalcInterpolatedRotation(
    const float animTimeTicks,
    const aiNodeAnim* pNodeAnim,
    aiQuaternion& out)
{
    assert(pNodeAnim);

    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1)
    {
        out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    const uint idxCurr = FindRotationIdx(animTimeTicks, pNodeAnim);
    const uint idxNext = idxCurr + 1;
    assert(idxNext < pNodeAnim->mNumRotationKeys);

    const float t1     = (float)pNodeAnim->mRotationKeys[idxCurr].mTime;
    const float t2     = (float)pNodeAnim->mRotationKeys[idxNext].mTime;
    const float dt     = t2 - t1;
    const float factor = (animTimeTicks - t1) / dt;
    assert(factor >= 0.0f && factor <= 1.0f);

    const aiQuaternion& startRotQ = pNodeAnim->mRotationKeys[idxCurr].mValue;
    const aiQuaternion& endRotQ   = pNodeAnim->mRotationKeys[idxNext].mValue;
    aiQuaternion::Interpolate(out, startRotQ, endRotQ, factor);
    out.Normalize();
}

//---------------------------------------------------------
// Desc:  compute a scaling value for keyframe at time pos
//---------------------------------------------------------
void CalcInterpolatedScaling(
    const float animTimeTicks,
    const aiNodeAnim* pNodeAnim,
    aiVector3D& out)
{
    assert(pNodeAnim);

    // we need at least two values to interpolate...
    if (pNodeAnim->mNumScalingKeys == 1)
    {
        out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    const uint idxCurr = FindScalingIdx(animTimeTicks, pNodeAnim);
    const uint idxNext = idxCurr + 1;
    assert(idxNext < pNodeAnim->mNumScalingKeys);

    const float t1     = (float)pNodeAnim->mScalingKeys[idxCurr].mTime;
    const float t2     = (float)pNodeAnim->mScalingKeys[idxNext].mTime;
    const float dt     = t2 - t1;
    const float factor = (animTimeTicks - t1) / dt;
    assert(factor >= 0.0f && factor <= 1.0f);

    const aiVector3D& start = pNodeAnim->mScalingKeys[idxCurr].mValue;
    const aiVector3D& end   = pNodeAnim->mScalingKeys[idxNext].mValue;

    aiVector3D delta = end - start;
    out = start + factor * delta;
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadNodeHierarchy(
    const aiScene* pScene,
    const aiNode* pNode,
    const DirectX::XMMATRIX& parentTransform,
    AnimSkeleton& skeleton,
    AnimationClip& animation,
    const float ticksPerSecond,
    const float animDurationInTicks,
    const int parentIdxOfBone)
{
    DirectX::XMMATRIX nodeTransform;
    AiToXmMatrix(pNode->mTransformation, nodeTransform);
    nodeTransform = DirectX::XMMatrixTranspose(nodeTransform);
    const DirectX::XMMATRIX globalTransform = nodeTransform * parentTransform;

    const char* nodeName = pNode->mName.C_Str();
    const int   boneId   = skeleton.GetBoneIdByName(nodeName);

    if (boneId != -1)
    {
        const char* parentNodeName = pNode->mParent->mName.C_Str();
        const int   parentBoneId   = skeleton.GetBoneIdByName(parentNodeName);

        skeleton.boneHierarchy_[boneId] = parentBoneId;
        skeleton.boneTransformations_[boneId] = nodeTransform;
    }

    const aiAnimation* pAnimation = pScene->mAnimations[0];
    const aiNodeAnim*   pNodeAnim = FindNodeAnim(pAnimation, nodeName);

    // if we have a bone and animations for it...
    if (boneId != -1 && pNodeAnim)
    {
        // ... then we load all the keyframes for this particular bone

        // define how many keyframes we have for this bone and this animation
        uint numKeyFrames = 0;
        numKeyFrames = max(numKeyFrames, pNodeAnim->mNumPositionKeys);
        numKeyFrames = max(numKeyFrames, pNodeAnim->mNumRotationKeys);
        numKeyFrames = max(numKeyFrames, pNodeAnim->mNumScalingKeys);

        assert(boneId < animation.boneAnimations.size());
        animation.boneAnimations[boneId].keyframes.resize(numKeyFrames);

        // calc timings 
        float timePos = 0;
        float tick = 0;
        float keyframeDurationSec = (1000.0f / ticksPerSecond) * 0.001f;
        float animDurationSec     = keyframeDurationSec * animDurationInTicks;

        animation.startTime = 0;
        animation.endTime   = animDurationSec;


        // create data for each keyframe
        for (uint i = 0; i < numKeyFrames; ++i)
        {
            assert(timePos <= animDurationSec);
            assert(tick    <= animDurationInTicks);

            aiVector3D   pos;
            aiQuaternion rotQ;
            aiVector3D   scale;

            CalcInterpolatedTranslation(tick, pNodeAnim, pos);
            CalcInterpolatedRotation   (tick, pNodeAnim, rotQ);
            CalcInterpolatedScaling    (tick, pNodeAnim, scale);

            Keyframe& keyframe   = animation.boneAnimations[boneId].keyframes[i];
            keyframe.timePos     = timePos;
            keyframe.scale       = scale.x;                     // uniform scale
            keyframe.translation = { pos.x, pos.y, pos.z };
            keyframe.rotQuat     = { rotQ.x, rotQ.y, rotQ.z, rotQ.w };
#if 0
            // debug
            printf("tick %.2f, timePos %.2f, pos(%.2f %.2f %.2f), quat(%.2f %.2f %.2f %.2f), scale %.2f\n",
                tick,
                timePos,
                pos.x, pos.y, pos.z,
                rotQ.x, rotQ.y, rotQ.z, rotQ.w,
                scale.x);
#endif

            timePos += keyframeDurationSec;
            tick += 1.0f;

            // clamp tick
            if (tick == animDurationInTicks)
                tick = fmodf(tick, animDurationInTicks);
        }
    }


    // recursively go down to the node's children if we have any
    for (uint i = 0; i < pNode->mNumChildren; ++i)
    {
        ReadNodeHierarchy(
            pScene,
            pNode->mChildren[i],
            globalTransform,
            skeleton,
            animation,
            ticksPerSecond,
            animDurationInTicks,
            parentIdxOfBone + 1);
    }
}

//-----------------------------------------------------------------------------
// Desc:  load in all the animations for input skeleton
//-----------------------------------------------------------------------------
void AddAnimationsToSkeleton(const aiScene* pScene, AnimSkeleton& skeleton)
{
    assert(pScene);

    // do we have any animations?
    if (pScene->mNumAnimations == 0)
        return;

    const uint    animId = skeleton.AddAnimation("search");
    AnimationClip&  anim = skeleton.GetAnimation(animId);

    // alloc enough space for this animation
    anim.boneAnimations.resize(skeleton.GetNumBones());

    // animation common stats
    const aiAnimation* pAiAnim = pScene->mAnimations[0];

    float ticksPerSecond = 25.0f;
    if (pAiAnim->mTicksPerSecond != 0)
        ticksPerSecond = (float)pAiAnim->mTicksPerSecond;

    float animDurationInTicks = (float)pAiAnim->mDuration;
    float timePerTick = 1000.0f / ticksPerSecond;

    printf("\n");
    printf("TICKS PER SECOND:      %f\n", ticksPerSecond);
    printf("ANIM DURATION (TICKS): %f\n", animDurationInTicks);
    printf("TIME PER TICK (MS):    %f\n", timePerTick);

    skeleton.boneHierarchy_.resize(skeleton.GetNumBones());
    skeleton.boneTransformations_.resize(skeleton.GetNumBones());
    int parentIdxOfBone = -1;

    // recursively go down to the node's children if we have any and load its data
    ReadNodeHierarchy(
        pScene,
        pScene->mRootNode,
        DirectX::XMMatrixIdentity(),   // parent transform
        skeleton,
        anim,
        ticksPerSecond,
        animDurationInTicks,
        parentIdxOfBone);
}

//---------------------------------------------------------
// Desc:  gather weights, bone ids for vertices, and
//        offset matrices for bones
//---------------------------------------------------------
void GatherBonesData(
    const aiScene* pScene,
    AnimSkeleton& skeleton,
    const int totalVertices,
    const int totalBones)
{
    skeleton.vertexToBones.resize(totalVertices);
    skeleton.boneOffsets_.reserve(totalBones);

    for (uint i = 0; i < pScene->mNumMeshes; ++i)
    {
        if (pScene->mMeshes[i]->HasBones())
            LoadMeshBones(i, pScene->mMeshes[i], skeleton);
    }

    skeleton.vertexToBones.shrink_to_fit();
    skeleton.boneOffsets_.shrink_to_fit();
}

//---------------------------------------------------------
// Desc:  init vertex buffer (weights, bone ids)
//        related to input skeleton
//---------------------------------------------------------
void InitSkeletonBonesVB(ID3D11Device* pDevice, AnimSkeleton& skeleton)
{
    const VertexBoneData* bonesWeights    = skeleton.vertexToBones.data();
    const int             numBonesWeights = (int)skeleton.vertexToBones.size();
    const bool            isDynamicVB     = false;

    if (!skeleton.bonesVB_.Initialize(pDevice, bonesWeights, numBonesWeights, isDynamicVB))
    {
        LogErr(LOG, "can't init bones VB for skeleton: %s", skeleton.GetName());
        return;
    }
}

//---------------------------------------------------------
// Desc:  load in all the skeleton (model skinning) related data
//---------------------------------------------------------
void ParseMeshes(
    ID3D11Device* pDevice,
    const aiScene* pScene,
    const char* skeletonName)
{
    int totalVertices = 0;
    int totalIndices = 0;
    int totalBones = 0;

    // TODO:
    // implement some check if there is any animation if no just get out of here


    const uint skeletonId  = g_AnimationMgr.AddSkeleton(skeletonName);
    AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton(skeletonId);

    // calc and store global inverse transformation for the root of this skeleton
    DirectX::XMMATRIX rootTransform;
    AiToXmMatrix(pScene->mRootNode->mTransformation, rootTransform);
    skeleton.globalInvTransform_ = DirectX::XMMatrixInverse(nullptr, rootTransform);


    // calc mesh data
    printf("\n\n*****************************************************\n");
    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

    skeleton.meshBaseVertex.resize(pScene->mNumMeshes);

    for (uint i = 0; i < pScene->mNumMeshes; ++i)
    {
        const aiMesh* pMesh   = pScene->mMeshes[i];
        const int numVertices = pMesh->mNumVertices;
        const int numIndices  = pMesh->mNumFaces * 3;
        const int numBones    = pMesh->mNumBones;
        skeleton.meshBaseVertex[i] = totalVertices;

        printf("  Mesh_%d: %-32s vertices %d, indices %d, bones %d\n",
            i, pMesh->mName.C_Str(), numVertices, numIndices, numBones);

        totalVertices += numVertices;
        totalIndices  += numIndices;
        totalBones    += numBones;
    }
    printf("\nTotal vertices %d, indices %d, bones %d\n\n", totalVertices, totalIndices, totalBones);

    GatherBonesData        (pScene,  skeleton, totalVertices, totalBones);
    InitSkeletonBonesVB    (pDevice, skeleton);
    AddAnimationsToSkeleton(pScene,  skeleton);
}

//---------------------------------------------------------
// DEBUG:  recursively print into console a tree of the model's nodes
//---------------------------------------------------------
void ParseNode(const aiNode* pNode)
{
    PrintSpace();  printf("Node name : '%s', num children %d, num meshes %d\n", pNode->mName.C_Str(), pNode->mNumChildren, pNode->mNumMeshes);
    PrintSpace();  printf("Node transformation:\n");
    PrintAssimpMatrix(pNode->mTransformation);

    s_SpaceCount += 4;

    for (int i = 0; i < (int)pNode->mNumChildren; ++i)
    {
        printf("\n");
        PrintSpace();
        printf("--- %d ---\n", i);
        ParseNode(pNode->mChildren[i]);
    }

    s_SpaceCount -= 4;
}

//---------------------------------------------------------
// DEBUG:  recursively print into console a tree of the model's nodes
//         starting from the root node
//---------------------------------------------------------
void ParseHierarchy(const aiScene* pScene)
{
    printf("\n\n\n********************************************\n");
    printf("Parsing the node hierarchy\n");

    ParseNode(pScene->mRootNode);
    printf("\n");
}

//---------------------------------------------------------
// load in skeleton and related animations from aiScene
//        1. load bones of a model
//        2. create bones VB
//        3. load animations (model skinning)
//---------------------------------------------------------
void LoadSkeletonAndAnimations(
    ID3D11Device* pDevice,
    const aiScene* pScene,
    const char* skeletonName)
{
    ParseMeshes(pDevice, pScene, skeletonName);

    // for debug
    //ParseHierarchy(pScene);
}

} // namespace
