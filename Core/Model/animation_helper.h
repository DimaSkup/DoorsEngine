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
#pragma once

#include <types.h>
#include <Mesh/vertex_buffer.h>
#include <cvector.h>
#include <DirectXMath.h>



namespace Core
{

//---------------------------------------------------------
// helper defines/structures/enums for skinned data
//---------------------------------------------------------
#define MAX_NUM_BONES_PER_VERTEX 4
#define MAX_LEN_ANIMATION_NAME   16
#define MAX_LEN_BONE_NAME        16
#define MAX_LEN_SKELETON_NAME    16

//---------------------------------------------------------

struct AnimationName
{
    char name[MAX_LEN_ANIMATION_NAME]{'\0'};
};

struct BoneName
{
    char name[MAX_LEN_BONE_NAME]{'\0'};
};

struct SkeletonName
{
    char name[MAX_LEN_SKELETON_NAME]{'\0'};
};


//---------------------------------------------------------
// a keyframe defines the bone transformation at an instant in time
//---------------------------------------------------------
struct Keyframe
{
    Keyframe();

    float             timePos;
    float             scale;          // uniform scale
    DirectX::XMFLOAT3 translation;
    DirectX::XMFLOAT4 rotQuat;        // rotation quaternion
};

//---------------------------------------------------------
// container data structure for MAX_NUM_BONES_PER_VERTEX
// boneIds and weights
//---------------------------------------------------------
struct VertexBoneData
{
    float weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };
    uint  boneIds[MAX_NUM_BONES_PER_VERTEX] = { 0 };

    VertexBoneData()
    {
    }

    void AddBoneData(const uint boneId, const float weight)
    {
        for (uint i = 0; i < MAX_NUM_BONES_PER_VERTEX; ++i)
        {
            if (weights[i] == 0.0f)
            {
                weights[i] = weight;
                boneIds[i] = boneId;
                //printf("bone %d, weight %f, index %u\n", boneId, weight, i);
                return;
            }
        }

        assert(0 && "should never get here - more bones that we have space for");
    }
};

//---------------------------------------------------------
// a BoneAnimation is defined by a list of keyframes. For time
// values inbetween two frames, we interpolate between the two
// nearest keyframes that bound the time
//
// we assume an animation always has two keyframes
//---------------------------------------------------------
struct BoneAnimation
{
    float GetStartTime() const;
    float GetEndTime() const;

    void Interpolate(const float t, DirectX::XMMATRIX& M) const;

    cvector<Keyframe> keyframes;
};

//---------------------------------------------------------
// Examples of AnimationClips are "Walk", "Run, "Attack", "Defend".
// An AnimationClip requires a BoneAnimation for every bone to form
// the animation clip
//---------------------------------------------------------
struct AnimationClip
{
    float GetStartTime() const;
    float GetEndTime() const;

    void Interpolate(const float t, cvector<DirectX::XMMATRIX>& boneTransforms) const;

    uint                   id = 0;
    float                  animTimePos = 0;
    float                  startTime = FLT_MAX;
    float                  endTime = 0;

    cvector<BoneAnimation> boneAnimations;   // set of keyframes per each bone
};


//---------------------------------------------------------
// a character will generally have several animation clips for all the
// animations the character need to perform. All the animation clips work
// on the same skeleton, however, so they use the same number of bones
// (although some bones may be stationary for a particular animation)
// 
// so it is a structure for storing out skeleton animation data
//---------------------------------------------------------
class AnimSkeleton
{
public:
    AnimSkeleton() {};

    size           GetNumBones() const;
    const char*    GetName()     const;
    ID3D11Buffer*  GetBonesVB()  const;

    int            AddAnimation    (const char* animName);
    int            GetAnimationIdx (const char* animName) const;
    AnimationClip& GetAnimation    (const int animIdx);
    void           SetAnimationName(const int animIdx, const char* newName);

    float GetAnimationStartTime(const int animIdx) const;
    float GetAnimationEndTime  (const int animIdx) const;

    int   AddBone              (const char* name);
    int   GetBoneIdByName      (const char* name);
    void  SetBoneNameById      (const int id, const char* name);
    

#if 0
    void Set(
        const cvector<int>& boneHierarchy,
        const cvector<DirectX::XMMATRIX>& boneOffsets,
        const cvector<AnimationName>& names,
        const cvector<AnimationClip>& animations);
#endif

    void GetFinalTransforms(
        const char* animationName,
        const float timePos,
        cvector<DirectX::XMMATRIX>& outFinalTransforms);


    void DumpBoneWeights();
    void DumpBoneOffsets();
    void DumpBoneParents();
    void DumpKeyframes(const char* animName, const int boneId);

public:
    char name_[MAX_LEN_SKELETON_NAME]{'\0'};

    uint id_ = 0;

    // gives parentIndex of ith bone
    cvector<int>                 boneHierarchy_;

    // offset transform of the ith bone
    cvector<DirectX::XMMATRIX>   boneOffsets_;

    // matrices to transform each bone into T-pose, bind pose
    cvector<DirectX::XMMATRIX>   boneTransformations_;

    cvector<AnimationName>       animNames_;
    cvector<AnimationClip>       animations_;

    VertexBuffer<VertexBoneData> bonesVB_;
    cvector<VertexBoneData>      vertexToBones;
    cvector<int>                 meshBaseVertex;

    cvector<BoneName>            boneNameToId;

    DirectX::XMMATRIX            globalInvTransform_ = DirectX::XMMatrixIdentity(); 
};

} // namespace
