/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_saver.cpp

    Desc:     implementation of functional for saving
              a loaded skeleton, its bones, and animations
              into file of internal format

    Created:  27.12.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "animation_saver.h"
#include "animation_helper.h"

#pragma warning (disable : 4996)

using namespace DirectX;


namespace Core
{

//---------------------------------------------------------
// forward declarations of helper functions
//---------------------------------------------------------
void StoreCommonInfo(FILE* pFile, const AnimSkeleton* pSkeleton);
void StoreBones     (FILE* pFile, const AnimSkeleton* pSkeleton);
void StoreOffsets   (FILE* pFile, const AnimSkeleton* pSkeleton);
void StoreWeights   (FILE* pFile, const AnimSkeleton* pSkeleton);
void StoreAnimations(FILE* pFile, const AnimSkeleton* pSkeleton);


//---------------------------------------------------------
// Desc:  save input skeleton, its bones, and animations into file by filename
//---------------------------------------------------------
bool AnimationSaver::Save(const AnimSkeleton* pSkeleton, const char* filename)
{
    if (!pSkeleton)
    {
        LogErr(LOG, "ptr to skeleton == nullptr");
        return false;
    }
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "can't save skeleton '%s' into file: path is empty", pSkeleton->GetName());
        return false;
    }


    FILE* pFile = fopen(filename, "w");
    if (!pFile)
    {
        LogErr(LOG, "can't open file for writing: %s", filename);
        return false;
    }

    StoreCommonInfo (pFile, pSkeleton);
    StoreBones      (pFile, pSkeleton);
    StoreOffsets    (pFile, pSkeleton);
    StoreWeights    (pFile, pSkeleton);
    StoreAnimations (pFile, pSkeleton);

    fclose(pFile);

    LogMsg(LOG, "skeleton '%s' is saved into file: %s", pSkeleton->GetName(), filename);
    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void StoreCommonInfo(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    fprintf(pFile, "numAnimations %d\n", (int)pSkeleton->GetNumAnimations());
    fprintf(pFile, "numJoints %d\n", (int)pSkeleton->GetNumBones());
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  a little helper to extract
//        position (vec3) and rotation quaternion (vec4) from input matrix
// Args:  m - matrix to decompose
// Out:   p - position
//        q - rotation quat
//---------------------------------------------------------
inline void GetPosRotQuat(const XMMATRIX& m, XMFLOAT3& p, XMFLOAT4& q)
{
    XMVECTOR S;
    XMVECTOR Q;
    XMVECTOR T;
    XMMatrixDecompose(&S, &Q, &T, m);

    XMStoreFloat4(&q, Q);
    XMStoreFloat3(&p, T);
}

//---------------------------------------------------------
// Desc:  store bones hierarchy and bone info:
//        bone_name, bone_parent_idx, bone_pos, bone_rot_quat
//---------------------------------------------------------
void StoreBones(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    fprintf(pFile, "bones\n");

    const int*      boneHierarchy = pSkeleton->boneHierarchy_.data();
    const BoneName* bonesNames    = pSkeleton->boneNames_.data();

    for (int i = 0; i < pSkeleton->GetNumBones(); ++i)
    {
        // get position and rotation quat from the bind pose matrix of this bone
        XMFLOAT3 p;
        XMFLOAT4 q;
        GetPosRotQuat(pSkeleton->boneTransforms_[i], p, q);
        
        const int parentBoneIdx = boneHierarchy[i];

        fprintf(pFile, "\t\"%s\"\t%d (%f %f %f) (%f %f %f %f)\t\t//",
            bonesNames[i].name,                             // bone_name
            parentBoneIdx, 
            p.x, p.y, p.z,
            q.x, q.y, q.z, q.w); 

        // if current bone has a parent...
        if (parentBoneIdx != -1)
        {
            // ... write a parent bone's name
            fprintf(pFile, "%s", bonesNames[parentBoneIdx].name);
        }

        fprintf(pFile, "\n");
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  store offset matrices of bones
//---------------------------------------------------------
void StoreOffsets(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    fprintf(pFile, "offsets\n");

    for (index i = 0; i < pSkeleton->boneOffsets_.size(); ++i)
    {
        // get position and rotation quat from the offset matrix of this bone
        XMFLOAT3 p;
        XMFLOAT4 q;
        GetPosRotQuat(pSkeleton->boneTransforms_[i], p, q);

        fprintf(pFile, "\t(%f %f %f) (%f %f %f %f)\n", p.x, p.y, p.z, q.x, q.y, q.z, q.w);
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  write into a file bones weights 
//---------------------------------------------------------
void StoreWeights(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    int numWeights = 0;

    // calc the number of non-zero weights
    for (const VertexBoneData& data : pSkeleton->vertexToBones)
    {
        numWeights += (data.weights[0] != 0.0f);
        numWeights += (data.weights[1] != 0.0f);
        numWeights += (data.weights[2] != 0.0f);
        numWeights += (data.weights[3] != 0.0f);
    }

    fprintf(pFile, "numweights %d\n", numWeights);

    int weightIdx = 0;

    for (const VertexBoneData& data : pSkeleton->vertexToBones)
    {
        const float* weights = data.weights;
        const uint*  boneIds = data.boneIds;

        if (weights[0] != 0.0f)
            fprintf(pFile, "\tweight %d %u %f\n", weightIdx++, boneIds[0], weights[0]);

        if (weights[1] != 0.0f)
            fprintf(pFile, "\tweight %d %u %f\n", weightIdx++, boneIds[1], weights[1]);

        if (weights[2] != 0.0f)
            fprintf(pFile, "\tweight %d %u %f\n", weightIdx++, boneIds[2], weights[2]);

        if (weights[3] != 0.0f)
            fprintf(pFile, "\tweight %d %u %f\n", weightIdx++, boneIds[3], weights[3]);
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  write input keyframes into the file
//---------------------------------------------------------
void WriteKeyframes(FILE* pFile, const cvector<Keyframe>& keyframes)
{
    assert(pFile);

    for (index frameIdx = 0; frameIdx < keyframes.size(); ++frameIdx)
    {
        const XMFLOAT3& p = keyframes[frameIdx].translation;
        const XMFLOAT4& q = keyframes[frameIdx].rotQuat;

        // (position) (rotation_quat)
        fprintf(pFile, "\t\t(%f %f %f) (%f %f %f %f)\n", p.x, p.y, p.z, q.x, q.y, q.z, q.w);
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  write all the keyframes data of input animation into the file
//---------------------------------------------------------
void WriteAnimation(FILE* pFile, const AnimationClip& anim)
{
    assert(pFile);

    // save keyframes for each bone
    for (index boneIdx = 0; boneIdx < anim.boneAnimations.size(); ++boneIdx)
    {
        fprintf(pFile, "\tframes for bone %d\n", (int)boneIdx);

        WriteKeyframes(pFile, anim.boneAnimations[boneIdx].keyframes);
    }
}

//---------------------------------------------------------
// Desc:  write all the animations data of this skeleton into the file
//---------------------------------------------------------
void StoreAnimations(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    for (int i = 0; i < (int)pSkeleton->GetNumAnimations(); ++i)
    {
        // for current animation each bone has the same number of keyframes
        const AnimationClip& anim = pSkeleton->animations_[i];

        fprintf(pFile, "animation \"%s\"\n", pSkeleton->GetAnimationName(i));
        fprintf(pFile, "numFrames %d\n",    (int)anim.boneAnimations[0].keyframes.size());
        fprintf(pFile, "frameRate %d\n",    (int)anim.framerate);
        
        WriteAnimation(pFile, anim);
    }
}


} // namespace
