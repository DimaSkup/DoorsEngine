/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_helper.h
    Desc:     contains structures for defining keyframes and animation

    Created:  02.12.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "animation_helper.h"
#include <DirectXMath.h>
#pragma warning (disable:4996)


namespace Core
{

using namespace DirectX;

// static arrays for internal purposes
cvector<XMMATRIX> s_ToParentTransforms;
cvector<XMMATRIX> s_ToRootTransforms;



//**********************************************************************************
// SKELETON DEBUG TOOLS
//**********************************************************************************

void PrintXmMatrix(const DirectX::XMMATRIX matrix)
{
    const float* m = matrix.r[0].m128_f32;

    printf("\t%f %f %f %f\n", m[0],  m[1],  m[2],  m[3]);
    printf("\t%f %f %f %f\n", m[4],  m[5],  m[6],  m[7]);
    printf("\t%f %f %f %f\n", m[8],  m[9],  m[10], m[11]);
    printf("\t%f %f %f %f\n", m[12], m[13], m[14], m[15]);
}

//---------------------------------------------------------

void DumpXmMatrices(const cvector<DirectX::XMMATRIX>& matrices, const size numMatrices)
{
    size numToDump = numMatrices;

    if (numToDump > matrices.size())
        numToDump = matrices.size();

    for (index i = 0; i < numToDump; ++i)
    {
        printf("matrix_%d:\n", (int)i);
        PrintXmMatrix(matrices[i]);
    }
    printf("\n");
}

//---------------------------------------------------------
// Desc:  dump (print into console) info about animations
//        which are related to this skeleton
//---------------------------------------------------------
void AnimSkeleton::DumpAnimations() const
{
    // check to prevent fuck up
    assert(animNames_.size() == animations_.size());
    const int numAnims = (int)animations_.size();

    printf("\nDump %d animations of skeleton '%s':\n", numAnims, name_);
    printf(" - num bones %ti\n", GetNumBones());

    // print: [idx] anim_name, max_num_keyframes, time [start, end]
    for (int i = 0; i < numAnims; ++i)
    {
        printf("\t[%d] %-50s  max_keyframes: %-10ti  time [%f, %f]\n",
            i,
            animNames_[i].name,
            animations_[i].GetNumKeyframes(),
            animations_[i].GetStartTime(),
            animations_[i].GetEndTime());
    }
}

//---------------------------------------------------------

void AnimSkeleton::DumpBoneOffsets() const
{
    printf("Dump bones (offset matrix) of skeleton '%s':\n", name_);
    printf(" - num bones %d\n", (int)GetNumBones());

    for (int i = 0; i < (int)GetNumBones(); ++i)
    {
        printf("\tbone_%d, name: %s\n", i, boneNames_[i].name);
        PrintXmMatrix(boneOffsets_[i]);
        printf("\n");
    }
    printf("\n");
}

//---------------------------------------------------------

void AnimSkeleton::DumpBoneParents() const
{
    printf("Dump bones (its parents) of skeleton '%s':\n", name_);
    printf(" - num bones %d\n", (int)GetNumBones());

    constexpr int ROOT_PARENT_IDX = -1;

    for (int boneIdx = 0; boneIdx < (int)GetNumBones(); ++boneIdx)
    {
        const char* boneName       = boneNames_[boneIdx].name;
        const int   parentBoneIdx  = boneHierarchy_[boneIdx];
        
        if (parentBoneIdx == ROOT_PARENT_IDX)
        {
            printf("\tbone_%d: %-40s parent_idx: %-5d parent_name: ''\n",
                boneIdx,
                boneName,
                parentBoneIdx);
        }
        else
        {
            printf("\tbone_%d: %-40s parent_idx: %-5d parent_name: '%s'\n",
                boneIdx,
                boneName,
                parentBoneIdx,
                boneNames_[parentBoneIdx].name);   // parent bone name
        }
    }
    printf("\n");
}

//---------------------------------------------------------
// Desc:  dump all the keyframes for animation (by name) and bone (by id)
//---------------------------------------------------------
void AnimSkeleton::DumpKeyframes(const char* animName, const int boneId) const
{
    assert(animName && animName[0] != '\0');

    const int animIdx = GetAnimationIdx(animName);
    if (animIdx == -1)
    {
        LogErr(LOG, "there is no animation by name: %s", animName);
        return;
    }


    const AnimationClip& anim = animations_[animIdx];
    assert(boneId < anim.boneAnimations.size());

    const cvector<Keyframe>& keyframes    = anim.boneAnimations[boneId].keyframes;
    const int                numKeyframes = (int)keyframes.size();
    const float              frameTime    = 1.0f / anim.framerate;

    SetConsoleColor(CYAN);
    printf("\nDump keyframes (skeleton '%s', animation '%s', bone_id %d):\n", name_, animName, boneId);
    printf(" - num keyframes %d\n", numKeyframes);
    SetConsoleColor(RESET);

    // print data of each keyframe
    for (int i = 0; i < numKeyframes; ++i)
    {
        const float frameTimePos = (float)i * frameTime;

        printf("\t[%d] timePos %.2f, pos(%.2f %.2f %.2f), quat(%.2f %.2f %.2f %.2f)\n",
            i,
            frameTimePos,
            keyframes[i].translation.x,
            keyframes[i].translation.y,
            keyframes[i].translation.z,
            keyframes[i].rotQuat.x,
            keyframes[i].rotQuat.y,
            keyframes[i].rotQuat.z,
            keyframes[i].rotQuat.w);
    }
    printf("\n\n");
}


//**********************************************************************************
// ANIMATIONS STUFF
//**********************************************************************************

//---------------------------------------------------------
// constructor for a single keyframe
//---------------------------------------------------------
Keyframe::Keyframe() :
    translation(0,0,0),
    rotQuat(0,0,0,1)
{
}

//---------------------------------------------------------
// Desc:  interpolated bone animation between two keyframes nearest to input time
//---------------------------------------------------------
void BoneAnimation::Interpolate(
    const float framerate,
    const float t,
    XMMATRIX& M) const
{
    if (t <= 0)
    {
        const Keyframe& frame = keyframes[0];

        const XMVECTOR S = { 1,1,1 };
        const XMVECTOR P = XMLoadFloat3(&frame.translation);
        const XMVECTOR Q = XMLoadFloat4(&frame.rotQuat);

        const XMVECTOR zero = { 0,0,0,1 };
        M = XMMatrixAffineTransformation(S, zero, Q, P);
        return;
    }


    const float frametime = 1.0f / framerate;
    const float endTime = (float)keyframes.size() * frametime;

    if (t >= endTime)
    {
        const Keyframe& frame = keyframes.back();

        const XMVECTOR S = { 1,1,1 };
        const XMVECTOR P = XMLoadFloat3(&frame.translation);
        const XMVECTOR Q = XMLoadFloat4(&frame.rotQuat);

        const XMVECTOR zero = { 0,0,0,1 };
        M = XMMatrixAffineTransformation(S, zero, Q, P);
    }

    else
    {
        // calc 2 current frames and ...
        const float animFrame = t * framerate;
        const int   numFrames = (int)keyframes.size();
        const int   frameIdxA = (int)floorf(animFrame);
        const int   frameIdxB = (frameIdxA + 1) % numFrames;

        assert(frameIdxA < numFrames&& frameIdxB < numFrames);

        const Keyframe& frame0 = keyframes[frameIdxA];
        const Keyframe& frame1 = keyframes[frameIdxB];

#if 1
        float t0 = frameIdxA * frametime;
        float t1 = frameIdxB * frametime;
#else
        float t0 = frame0.timePos;
        float t1 = frame1.timePos;
#endif


        // ... lerp time between them
        //float lerpPercent = (t - frame0.timePos) / (frame1.timePos - frame0.timePos);
        float lerpPercent = (t - t0) / (t1 - t0);
        lerpPercent = clampf(lerpPercent, 0, 1);


        // calc interpolated values
        const XMVECTOR p0 = XMLoadFloat3(&frame0.translation);
        const XMVECTOR p1 = XMLoadFloat3(&frame1.translation);

        const XMVECTOR q0 = XMLoadFloat4(&frame0.rotQuat);
        const XMVECTOR q1 = XMLoadFloat4(&frame1.rotQuat);

        const XMVECTOR S = { 1,1,1 };
        const XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
        const XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

        const XMVECTOR zero = { 0,0,0,1 };
        M = XMMatrixAffineTransformation(S, zero, Q, P);
    }
}

//---------------------------------------------------------
// Desc:  find smallest start time over all bones in this clip
//---------------------------------------------------------
float AnimationClip::GetStartTime() const
{
    return 0.0f;
}

//---------------------------------------------------------
// Desc:  find largets end time over all bones in this clip
//---------------------------------------------------------
float AnimationClip::GetEndTime() const
{
    const float numKeyframes = (float)GetNumKeyframes();
    const float frameTime    = 1.0f / framerate;

    return numKeyframes * frameTime;
}

//---------------------------------------------------------
// Desc:  get maximal number of keyframes (per bone) for this animation
//---------------------------------------------------------
size AnimationClip::GetNumKeyframes() const
{
    size maxKeys = 0;

    for (index i = 0; i < boneAnimations.size(); ++i)
        maxKeys = max(maxKeys, boneAnimations[i].keyframes.size());

    return maxKeys;

    //assert(boneAnimations[0].keyframes.size() > 0);
    //return boneAnimations[0].keyframes.size();
}

//---------------------------------------------------------
// Desc:  interpolate animation for each bone of this animation clip
//---------------------------------------------------------
void AnimationClip::Interpolate(const float t, cvector<XMMATRIX>& outBoneTransforms) const
{
    for (index i = 0; i < boneAnimations.size(); ++i)
    {
        if (boneAnimations[i].keyframes.size() == 0)
            continue;

        boneAnimations[i].Interpolate(framerate, t, outBoneTransforms[i]);
    }
}

//---------------------------------------------------------
// Desc:  find an index of animation clip by input name
//---------------------------------------------------------
int AnimSkeleton::GetAnimationIdx(const char* animName) const
{
    if (StrHelper::IsEmpty(animName))
    {
        LogErr(LOG, "input name is empty");
        return -1;
    }

    for (index i = 0; i < animNames_.size(); ++i)
    {
        if (strcmp(animName, animNames_[i].name) == 0)
            return (int)i;
    }

    return -1;
}

//---------------------------------------------------------
// Desc:  get start time of animation clip by its idx (id)
//---------------------------------------------------------
float AnimSkeleton::GetAnimationStartTime(const int animIdx) const
{
    if (animIdx < 0 || animIdx >= animations_.size())
    {
        LogErr("input animation idx is invalid (%d)", animIdx);
        return 0.0f;
    }

    return animations_[animIdx].GetStartTime();
}

//---------------------------------------------------------
// Desc:  get end time of animation clip by its idx (id)
//---------------------------------------------------------
float AnimSkeleton::GetAnimationEndTime(const int animIdx) const
{
    if (animIdx < 0 || animIdx >= animations_.size())
    {
        LogErr("input animation idx is invalid (%d)", animIdx);
        return 0.0f;
    }

    return animations_[animIdx].GetEndTime();
}

//---------------------------------------------------------
// Desc:  return arr of animations related to this skeleton
//---------------------------------------------------------
const cvector<AnimationName>& AnimSkeleton::GetAnimNames() const
{
    return animNames_;
}

//---------------------------------------------------------
// Desc:  return number of animations related to this skeleton
//---------------------------------------------------------
size AnimSkeleton::GetNumAnimations() const
{
    return animations_.size();
}

//---------------------------------------------------------
// Desc:  get the number of bones for this skinned data
//---------------------------------------------------------
size AnimSkeleton::GetNumBones() const
{
    return boneOffsets_.size();
}

//---------------------------------------------------------
// Desc:  return a name of this skeleton
//---------------------------------------------------------
const char* AnimSkeleton::GetName() const
{
    return name_;
}

//---------------------------------------------------------
// Desc:  return a ptr to the vertex buffer of bones
//---------------------------------------------------------
ID3D11Buffer* AnimSkeleton::GetBonesVB() const
{
    return bonesVB_.Get();
}

//---------------------------------------------------------
// Desc:  add a new animation for this skeleton
//---------------------------------------------------------
int AnimSkeleton::AddAnimation(const char* animName)
{
    if (StrHelper::IsEmpty(animName))
    {
        LogErr(LOG, "input name is empty");
        return -1;
    }


    int animId = GetAnimationIdx(animName);

    // if there is already animation clip by input name we just return its id
    if (animId != -1)
        return animId;


    // add a new animation
    animId = (int)animations_.size();
    animations_.push_back(AnimationClip());
    animNames_.push_back(AnimationName());

    animations_[animId].id = animId;
    SetAnimationName(animId, animName);

    return animId;
}


//---------------------------------------------------------
// Desc:  return a name of animation by input index
//---------------------------------------------------------
const char* AnimSkeleton::GetAnimationName(const int animIdx) const
{
    if (animIdx < 0 || animIdx >= animations_.size())
    {
        LogErr("input animation idx is invalid (%d), so return nullptr", animIdx);
        return nullptr;
    }

    return animNames_[animIdx].name;
}

//---------------------------------------------------------
// Desc:  setup a name for animation clip by its index
//---------------------------------------------------------
void AnimSkeleton::SetAnimationName(const int animIdx, const char* newName)
{
    if (animIdx < 0 || animIdx >= animations_.size())
    {
        LogErr("input animation idx is invalid (%d)", animIdx);
        return;
    }

    strncpy(animNames_[animIdx].name, newName, MAX_LEN_ANIMATION_NAME);
}

//---------------------------------------------------------
// Desc:  return an animation clip by its id
//---------------------------------------------------------
AnimationClip& AnimSkeleton::GetAnimation(const int animIdx)
{
    assert(animIdx >= 0 && animIdx < animations_.size());
    return animations_[animIdx];
}

const AnimationClip& AnimSkeleton::GetAnimation(const int animIdx) const
{
    assert(animIdx >= 0 && animIdx < animations_.size());
    return animations_[animIdx];
}

#if 0
//---------------------------------------------------------
// Desc:  setup animations for this skinned object
//---------------------------------------------------------
void AnimSkeleton::Set(
    const cvector<int>& boneHierarchy,
    const cvector<XMMATRIX>& boneOffsets,
    const cvector<AnimationName>& names,
    const cvector<AnimationClip>& animations)
{
    assert(0 && "FIXME");
    boneHierarchy_ = boneHierarchy;
    animNames_     = names;
    animations_    = animations;
}
#endif

//---------------------------------------------------------
// Desc:  calculate all the bones transformation for animation clip
//        by input name at particular time position
//---------------------------------------------------------
void AnimSkeleton::GetFinalTransforms(
    const AnimationID animId,
    const float timePos,
    cvector<XMMATRIX>& outFinalTransforms)
{
    if (animId >= animations_.size())
    {
        LogErr(LOG, "input animation id (%d) is invalid (must be lower than %d)", (int)animId, (int)animations_.size());
        return;
    }
    const AnimationClip& animation = animations_[animId];

    // interpolate all the bones of this animation clip at the given time instance
    const size numBones = GetNumBones();
    s_ToParentTransforms = boneTransforms_;
    animation.Interpolate(timePos, s_ToParentTransforms);


    //
    // traverse the hierarchy and transform all the bones to the root space
    //
    s_ToRootTransforms.resize(numBones, DirectX::XMMatrixIdentity());
    assert(numBones > 0);

    for (index i = 0; i < numBones; ++i)
    {
        const int parentIdx = boneHierarchy_[i];
        XMMATRIX& toRoot    = s_ToRootTransforms[i];
        XMMATRIX& toParent  = s_ToParentTransforms[i];

        if (parentIdx >= 0)
            toRoot = toParent * s_ToRootTransforms[parentIdx];
        else
            toRoot = toParent;

        outFinalTransforms[i] = boneOffsets_[i] * toRoot;
    }
}

//---------------------------------------------------------
// Desc:  find bone id by its name
//---------------------------------------------------------
int AnimSkeleton::GetBoneIdByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return -1;
    }

    for (index i = 0; i < boneNames_.size(); ++i)
    {
        if (strcmp(boneNames_[i].name, name) == 0)
            return (int)i;
    }

    return -1;
}

//---------------------------------------------------------
// Desc:  add a new bone
// Ret:   id of created or already existed bone
//---------------------------------------------------------
int AnimSkeleton::AddBone(const char* boneName)
{
    assert(boneName && boneName[0] != '\0');

    int boneId = GetBoneIdByName(boneName);
    if (boneId == -1)
    {
        // add a new bone
        boneId = (int)boneNames_.size();
        boneNames_.push_back(BoneName());
        SetBoneNameById(boneId, boneName);
    }

    return boneId;
}

//---------------------------------------------------------
// Desc:  setup a name for bone by input id
//---------------------------------------------------------
void AnimSkeleton::SetBoneNameById(const int id, const char* name)
{
    assert(id >= 0 && id <= (int)vertexToBones.size());
    assert(!StrHelper::IsEmpty(name));

    strncpy(boneNames_[id].name, name, MAX_LEN_BONE_NAME);
    boneNames_[id].name[MAX_LEN_BONE_NAME - 1] = '\0';
}



} // namespace
