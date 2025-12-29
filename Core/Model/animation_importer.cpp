/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_importer.cpp
    Desc:     impelementation of helper class for importing animations using ASSIMP

    Created:  27.12.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "animation_importer.h"
#include "animation_mgr.h"
#include "animation_helper.h"
#include <Render/d3dclass.h>

#pragma warning (disable : 4996)


namespace Core
{

//---------------------------------------------------------
// forward declarations of helper functions
//---------------------------------------------------------
void CalculateMeshData(
    const aiScene* pScene,
    AnimSkeleton& skeleton,
    int& outTotalVerts,
    int& outTotalIdxs,
    int& outTotalBones);

void GatherBonesData(
    const aiScene* pScene,
    AnimSkeleton& skeleton,
    const int totalVertices,
    const int totalBones);

void LoadMeshBones(
    const int meshIdx,
    const aiMesh* pMesh,
    AnimSkeleton& skeleton);

void LoadSingleBone(
    const int meshIdx,
    const aiBone* pBone,
    AnimSkeleton& skeleton);

void InitSkeletonBonesVB(AnimSkeleton& skeleton);

void AddAnimationsToSkeleton(
    const aiScene* pScene,
    AnimSkeleton& skeleton);

void LoadAnimation(
    const aiScene* pScene,
    const aiAnimation* pAiAnim,
    const uint animIdx,
    AnimSkeleton& skeleton);

void ReadNodeHierarchyAndAnimation(
    const aiScene* pScene,
    const aiNode* pNode,
    const DirectX::XMMATRIX& parentTransform,
    AnimSkeleton& skeleton,
    AnimationClip& animation,
    const uint animIdx,
    const float ticksPerSecond,
    const float animDurationInTicks,  // aka "keyframes number"
    const int parentBoneIdx);

void LoadBoneAnimationKeyframes(
    const int boneId,
    const float animDurationInTicks,
    const float ticksPerSecond,
    const aiNodeAnim* pNodeAnim,
    AnimationClip& animation);

const aiNodeAnim* FindNodeAnim(
    const aiAnimation* pAnimation,
    const char* nodeName);

void LoadBonesHierarchy(
    const aiScene* pScene,
    AnimSkeleton& skeleton);

void ReadNodeHierarchy(
    const aiScene* pScene,
    const aiNode* pNode,
    const int parentBoneIdx,
    AnimSkeleton& skeleton);

void CalcInterpolatedTranslation(
    const float animTimeTicks,
    const aiNodeAnim* pNodeAnim,
    aiVector3D& out);

void CalcInterpolatedRotation(
    const float animTimeTicks,
    const aiNodeAnim* pNodeAnim,
    aiQuaternion& out);

void CalcInterpolatedScaling(
    const float animTimeTicks,
    const aiNodeAnim* pNodeAnim,
    aiVector3D& out);


//---------------------------------------------------------
// Desc:  load skeleton and its animations from the ASSIMP scene
//        and store it into the animation manager
// Args:  pScene       - a ptr to the assimp scene
//        skeletonName - a name for a new skeleton
//---------------------------------------------------------
void AnimationImporter::LoadSkeletonAnimations(
    const aiScene* pScene,
    const char* skeletonName)
{
    if (StrHelper::IsEmpty(skeletonName))
    {
        LogErr(LOG, "input name for a skeleton is empty");
        return;
    }
    if (!pScene)
    {
        LogErr(LOG, "scene ptr == nullptr (for skeleton: %s)", skeletonName);
        return;
    }


    printf("\n\n*****************************************************\n");
    LogMsg("import skeleton: %s", skeletonName);


    int totalVertices = 0;
    int totalIndices = 0;
    int totalBones = 0;

    const uint skeletonId  = g_AnimationMgr.AddSkeleton(skeletonName);
    AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton(skeletonId);

    CalculateMeshData  (pScene,  skeleton, totalVertices, totalIndices, totalBones);
    GatherBonesData    (pScene,  skeleton, totalVertices, totalBones);
    InitSkeletonBonesVB(skeleton);

    // if we have any animations...
    if (pScene->mNumAnimations > 0)
    {
        printf("Load bone hierarhy and animations for this skeleton\n");
        AddAnimationsToSkeleton(pScene, skeleton);
    }

    // ... or just load only bones hierarchy
    else
    {
        printf("We have no animations for this skeleton so load only bone hierarhy\n");
        LoadBonesHierarchy(pScene, skeleton);
    }

    skeleton.DumpBoneParents();
}

//---------------------------------------------------------

static int s_SpaceCount = 0;

void PrintSpace()
{
    for (int i = 0; i < s_SpaceCount; ++i)
        printf(" ");
}

//---------------------------------------------------------
// Desc:  print out an ASSIMP matrix values into the console
//---------------------------------------------------------
void PrintAssimpMatrix(const aiMatrix4x4& m)
{
    PrintSpace(); printf("%-8.3f %-8.3f %-8.3f %-8.3f\n", m.a1, m.a2, m.a3, m.a4);
    PrintSpace(); printf("%-8.3f %-8.3f %-8.3f %-8.3f\n", m.b1, m.b2, m.b3, m.b4);
    PrintSpace(); printf("%-8.3f %-8.3f %-8.3f %-8.3f\n", m.c1, m.c2, m.c3, m.c4);
    PrintSpace(); printf("%-8.3f %-8.3f %-8.3f %-8.3f\n", m.d1, m.d2, m.d3, m.d4);
}

//---------------------------------------------------------
// Desc:  convert from ASSIMP matrix to DirectXMath matrix
// NOTE:  we also execute matrix transpose here to get proper row-major matrix
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
// Desc:  calculate model/meshes stats
// Args:  pScene        - ptr to assimp scene
//        skeleton      - animation skeleton
//        outTotalVerts - total number of vertices
//        outTotalIdxs  - total number of indices
//        outTotalBones - total number of bones
//---------------------------------------------------------
void CalculateMeshData(
    const aiScene* pScene,
    AnimSkeleton& skeleton,
    int& outTotalVerts,
    int& outTotalIdxs,
    int& outTotalBones)
{
    // reset output agrs
    outTotalVerts = 0;
    outTotalIdxs  = 0;
    outTotalBones = 0;

    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

    skeleton.meshBaseVertex.resize(pScene->mNumMeshes);

    for (uint i = 0; i < pScene->mNumMeshes; ++i)
    {
        const aiMesh* pMesh         = pScene->mMeshes[i];
        const int numVertices       = pMesh->mNumVertices;
        const int numIndices        = pMesh->mNumFaces * 3;
        const int numBones          = pMesh->mNumBones;
        skeleton.meshBaseVertex[i]  = outTotalVerts;

        printf("  Mesh_%d: %-32s vertices %d, indices %d, bones %d\n",
            i, pMesh->mName.C_Str(), numVertices, numIndices, numBones);

        outTotalVerts += numVertices;
        outTotalIdxs  += numIndices;
        outTotalBones += numBones;
    }

    printf("\nTotal vertices %d, indices %d, bones %d\n\n",
            outTotalVerts,
            outTotalIdxs,
            outTotalBones);
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
    assert(pScene->mNumMeshes > 0);

    skeleton.vertexToBones.resize(totalVertices);
    skeleton.boneOffsets_.reserve(totalBones / pScene->mNumMeshes);

    for (uint i = 0; i < pScene->mNumMeshes; ++i)
    {
        if (pScene->mMeshes[i]->HasBones())
            LoadMeshBones(i, pScene->mMeshes[i], skeleton);
    }

    skeleton.vertexToBones.shrink_to_fit();
    skeleton.boneOffsets_.shrink_to_fit();
}

//---------------------------------------------------------
//---------------------------------------------------------
void LoadMeshBones(
    const int meshIdx,
    const aiMesh* pMesh,
    AnimSkeleton& skeleton)
{
    assert(meshIdx >= 0);
    assert(pMesh);

    for (uint i = 0; i < pMesh->mNumBones; ++i)
    {
        LoadSingleBone(meshIdx, pMesh->mBones[i], skeleton);
    }
}

//---------------------------------------------------------
// Desc:  load offset matrix for input bone as well as
//        weights + bone ids for each vertex related to this particular bone
//---------------------------------------------------------
void LoadSingleBone(
    const int meshIdx,
    const aiBone* pBone,
    AnimSkeleton& skeleton)
{
    assert(meshIdx >= 0);
    assert(pBone);

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
// Desc:  init a vertex buffer (which contains bones weights and indices)
//        related to the input skeleton
//---------------------------------------------------------
void InitSkeletonBonesVB(AnimSkeleton& skeleton)
{
    const VertexBoneData* bonesWeights = skeleton.vertexToBones.data();
    const int             numBonesWeights = (int)skeleton.vertexToBones.size();
    const bool            isDynamicVB = false;

    ID3D11Device* pDevice = Render::g_pDevice;

    if (!skeleton.bonesVB_.Initialize(pDevice, bonesWeights, numBonesWeights, isDynamicVB))
    {
        LogErr(LOG, "can't init bones VB for skeleton: %s", skeleton.GetName());
        return;
    }
}


//**********************************************************************************
// LOADING OF BONES HIERARCHY AND ANIMATIONS
//**********************************************************************************

//-----------------------------------------------------------------------------
// Desc:  load in all the animations for input skeleton (if we have any)
//-----------------------------------------------------------------------------
void AddAnimationsToSkeleton(
    const aiScene* pScene,
    AnimSkeleton& skeleton)
{
    assert(pScene);

    // do we have any animations?
    if (pScene->mNumAnimations == 0)
        return;

    skeleton.boneHierarchy_.resize(skeleton.GetNumBones());
    skeleton.boneTransforms_.resize(skeleton.GetNumBones());

    for (uint animIdx = 0; animIdx < pScene->mNumAnimations; ++animIdx)
    {
        LoadAnimation(pScene, pScene->mAnimations[animIdx], animIdx, skeleton);
    }
}

//---------------------------------------------------------
// Desc:  create a proper name for animation
//---------------------------------------------------------
void HandleAnimationName(
    const aiAnimation* pAiAnim,
    const AnimSkeleton& skeleton,
    const uint animIdx,
    char* outName)
{
    assert(pAiAnim);
    assert(outName);

    char animName[MAX_LEN_ANIMATION_NAME]{'\0'};
    const char* aiAnimName = pAiAnim->mName.C_Str();

    // if for some reason original name of animation is empty...
    if (StrHelper::IsEmpty(aiAnimName))
    {
        snprintf(animName, MAX_LEN_ANIMATION_NAME, "%s_%u", skeleton.GetName(), animIdx);
    }
    else
    {
        strncpy(animName, aiAnimName, MAX_LEN_ANIMATION_NAME);
        animName[MAX_LEN_ANIMATION_NAME - 1] = '\0';
    }

    // prevent animation name to have non digit and non alphabet symbols
    for (size_t i = 0; i < strlen(animName); ++i)
    {
        if (!isdigit(animName[i]) && !isalpha(animName[i]))
            animName[i] = '_';
    }

    // copy generated name into output
    strcpy(outName, animName);
}

//---------------------------------------------------------
// Desc:  
//---------------------------------------------------------
void LoadAnimation(
    const aiScene* pScene,
    const aiAnimation* pAiAnim,
    const uint animIdx,
    AnimSkeleton& skeleton)
{
    assert(pScene);
    assert(pAiAnim);
   
    char animName[MAX_LEN_ANIMATION_NAME]{ '\0' };
    HandleAnimationName(pAiAnim, skeleton, animIdx, (char*)&animName);

    const uint        animId = skeleton.AddAnimation(animName);
    AnimationClip& animation = skeleton.GetAnimation(animId);
    animation.boneAnimations.resize(skeleton.GetNumBones());
    
    // animation common stats
    float ticksPerSecond = 25.0f;
    if (pAiAnim->mTicksPerSecond != 0)
        ticksPerSecond = (float)pAiAnim->mTicksPerSecond;

    float animDurationInTicks = (float)pAiAnim->mDuration;
    float timeMsPerTick       = 1000.0f / ticksPerSecond;
    float animDurationMs      = timeMsPerTick * animDurationInTicks;

    printf("\n\n");
    printf("*** load animation:     %s\n", animName);
    printf("    ticks per sec:      %f\n", ticksPerSecond);
    printf("    time per tick (ms): %f\n", timeMsPerTick);
    printf("    duration (ticks):   %f\n", animDurationInTicks);
    printf("    duration (sec):     %f\n", animDurationMs * 0.001f);
    printf("\n");

    animation.framerate = ticksPerSecond;

    // recursively go down to the node's children if we have any and load its data
    int parentBoneIdx = -1;

    ReadNodeHierarchyAndAnimation(
        pScene,
        pScene->mRootNode,
        DirectX::XMMatrixIdentity(),   // parent transform
        skeleton,
        animation,
        animIdx,
        ticksPerSecond,
        animDurationInTicks,
        parentBoneIdx);
}


//---------------------------------------------------------
// Desc:  recursively traverse the tree, load bones hierarchy and animation
//---------------------------------------------------------
void ReadNodeHierarchyAndAnimation(
    const aiScene* pScene,
    const aiNode* pNode,
    const DirectX::XMMATRIX& parentTransform,
    AnimSkeleton& skeleton,
    AnimationClip& animation,
    const uint animIdx,
    const float ticksPerSecond,
    const float animDurationInTicks,  // aka "keyframes number"
    const int parentBoneIdx)
{
    DirectX::XMMATRIX nodeTransform;
    AiToXmMatrix(pNode->mTransformation, nodeTransform);

    // bone has the same name as the related node
    const char* nodeName = pNode->mName.C_Str();
    const int   boneId   = skeleton.GetBoneIdByName(nodeName);

    // if we have a bone...
    if (boneId != -1)
    {
        const char* parentNodeName = pNode->mParent->mName.C_Str();
        const int   parentBoneId   = skeleton.GetBoneIdByName(parentNodeName);

        skeleton.boneHierarchy_[boneId]       = parentBoneId;
        skeleton.boneTransforms_[boneId] = DirectX::XMMatrixTranspose(nodeTransform);
    }


    const aiAnimation* pAnimation = pScene->mAnimations[animIdx];
    const aiNodeAnim*  pNodeAnim  = FindNodeAnim(pAnimation, nodeName);

    // if we have a bone and animation for it we load its keyframes
    if (boneId != -1 && pNodeAnim)
    {
        LoadBoneAnimationKeyframes(
            boneId,
            animDurationInTicks,
            ticksPerSecond,
            pNodeAnim,
            animation);
    }

    // recursively go down to the node's children
    for (uint i = 0; i < pNode->mNumChildren; ++i)
    {
        ReadNodeHierarchyAndAnimation(
            pScene,
            pNode->mChildren[i],
            nodeTransform * parentTransform,
            skeleton,
            animation,
            animIdx,
            ticksPerSecond,
            animDurationInTicks,
            parentBoneIdx + 1);
    }
}

//---------------------------------------------------------
// Desc:  load all the keyframes related to bone by boneId
//        for this particular animation type
//---------------------------------------------------------
void LoadBoneAnimationKeyframes(
    const int boneId,
    const float animDurationInTicks,
    const float ticksPerSecond,
    const aiNodeAnim* pNodeAnim,
    AnimationClip& animation)
{
    assert(boneId > -1);
    assert(boneId < animation.boneAnimations.size());
    assert(pNodeAnim);
    assert(ticksPerSecond > 0);
    assert(animDurationInTicks > 0);


    const uint numKeyframes      = (uint)animDurationInTicks;
    cvector<Keyframe>& keyframes = animation.boneAnimations[boneId].keyframes;
    keyframes.resize(numKeyframes);

#if 0
    // calc timings (us - micro seconds)
    uint usInSec                = 1000 * 1000;
    uint usFrameDuration        = usInSec / (uint)ticksPerSecond;
    uint usTimePos              = 0;
    uint usAnimDuration         = usFrameDuration * numKeyframes;

    float keyframeDurationSec = (1000.0f / ticksPerSecond) * 0.001f;
    float animDurationSec = keyframeDurationSec * numKeyframes;
#endif

    // for debug
    // printf("   load %u keyframes for bone [%d] '%s'       time[%.5f, %.5f]\n", (uint)animDurationInTicks, boneId, nodeName, animation.startTime, animation.endTime);

    float tick = 0;

    // create data for each keyframe
    for (uint i = 0; i < numKeyframes; ++i)
    {
        assert(tick <= animDurationInTicks);

        aiVector3D   pos;
        aiQuaternion rotQ;
        //aiVector3D   scale;

        CalcInterpolatedTranslation(tick, pNodeAnim, pos);
        CalcInterpolatedRotation   (tick, pNodeAnim, rotQ);
        //CalcInterpolatedScaling    (tick, pNodeAnim, scale);

        Keyframe& keyframe   = animation.boneAnimations[boneId].keyframes[i];
        //keyframe.scale       = scale.x;                     // uniform scale
        keyframe.translation = { pos.x, pos.y, pos.z };
        keyframe.rotQuat     = { rotQ.x, rotQ.y, rotQ.z, rotQ.w };

#if 0
        // debug
        printf("    tick %.1f     timePos %.6f     pos(%.3f %.3f %.3f)    quat(%.3f %.3f %.3f %.3f)     scale %.3f\n",
            tick,
            timePos,
            pos.x, pos.y, pos.z,
            rotQ.x, rotQ.y, rotQ.z, rotQ.w,
            scale.x);
#endif

        //usTimePos += usFrameDuration;
        //timePos += keyframeDurationSec;
        //timePos = (((float)(usTimePos) * 0.001f) * 0.001f);

        tick += 1.0f;

        // clamp tick
        if (tick == animDurationInTicks)
            tick = fmodf(tick, animDurationInTicks);
    }
}

//---------------------------------------------------------
// Desc:  find an animation for a node by input name
//---------------------------------------------------------
const aiNodeAnim* FindNodeAnim(
    const aiAnimation* pAnimation,
    const char* nodeName)
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
// LOADING OF ONLY BONES HIERARCHY (WITHOUT NO ANIMATIONS)
//**********************************************************************************

//---------------------------------------------------------
// Desc:  load only bones hierarchy and bind-pose (T-pose) matrices
//---------------------------------------------------------
void LoadBonesHierarchy(const aiScene* pScene, AnimSkeleton& skeleton)
{
    skeleton.boneHierarchy_.resize(skeleton.GetNumBones());
    skeleton.boneTransforms_.resize(skeleton.GetNumBones());

    int parentBoneIdx = -1;
    ReadNodeHierarchy(pScene, pScene->mRootNode, parentBoneIdx, skeleton);
}

//---------------------------------------------------------
// Desc:  load only hierarchy data of node and bind-pose (T-pose)
//        transformation of related bone
//---------------------------------------------------------
void ReadNodeHierarchy(
    const aiScene* pScene,
    const aiNode* pNode,
    const int parentBoneIdx,
    AnimSkeleton& skeleton)
{
    assert(pScene);
    assert(pNode);

    DirectX::XMMATRIX nodeTransform;
    AiToXmMatrix(pNode->mTransformation, nodeTransform);

    const char* nodeName = pNode->mName.C_Str();
    const int   boneId   = skeleton.GetBoneIdByName(nodeName);

    // if we have a bone...
    if (boneId != -1)
    {
        const char* parentNodeName = pNode->mParent->mName.C_Str();
        const int   parentBoneId   = skeleton.GetBoneIdByName(parentNodeName);

        skeleton.boneHierarchy_[boneId]       = parentBoneId;
        skeleton.boneTransforms_[boneId] = DirectX::XMMatrixTranspose(nodeTransform);
    }

    // recursively go down to the node's children
    for (uint i = 0; i < pNode->mNumChildren; ++i)
    {
        ReadNodeHierarchy(pScene, pNode->mChildren[i], parentBoneIdx + 1, skeleton);
    }
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


//**********************************************************************************
// DEBUG NODES TREE OUTPUT:
//**********************************************************************************

//---------------------------------------------------------
// DEBUG:  recursively print into console a tree of the model's nodes
//---------------------------------------------------------
void ParseNode(const aiNode* pNode, const bool printNodeTransformMatrix)
{
    PrintSpace();
    printf("Node name : '%s', num children %d, num meshes %d\n",
        pNode->mName.C_Str(),
        pNode->mNumChildren,
        pNode->mNumMeshes);

    if (printNodeTransformMatrix)
    {
        PrintSpace();
        printf("Node transformation:\n");
        PrintAssimpMatrix(pNode->mTransformation);
    }

    s_SpaceCount += 4;

    for (int i = 0; i < (int)pNode->mNumChildren; ++i)
    {
        printf("\n");
        PrintSpace();
        printf("--- %d ---\n", i);
        ParseNode(pNode->mChildren[i], printNodeTransformMatrix);
    }

    s_SpaceCount -= 4;
}

//---------------------------------------------------------
// DEBUG:  recursively print into console a tree of the model's nodes
//         starting from the root node
//---------------------------------------------------------
void AnimationImporter::PrintNodesHierarchy(
    const aiScene* pScene,
    const bool printNodeTransformMatrix)
{
    assert(pScene);

    printf("\n\n\n********************************************\n");
    printf("Parsing (debug print) the node hierarchy for scene '%s'\n", pScene->mName.C_Str());

    ParseNode(pScene->mRootNode, printNodeTransformMatrix);
    printf("\n");
}

} // namespace
