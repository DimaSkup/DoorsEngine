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
void WriteCommonInfo     (FILE* pFile, const AnimSkeleton* pSkeleton);
void WriteAnimationsNames(FILE* pFile, const AnimSkeleton* pSkeleton);
void WriteBonesHierarchy (FILE* pFile, const AnimSkeleton* pSkeleton);
void WriteBaseframe      (FILE* pFile, const AnimSkeleton* pSkeleton);
void WriteOffsets        (FILE* pFile, const AnimSkeleton* pSkeleton);
void WriteWeights        (FILE* pFile, const AnimSkeleton* pSkeleton);
void WriteAnimations     (FILE* pFile, const AnimSkeleton* pSkeleton);


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

    WriteCommonInfo     (pFile, pSkeleton);
    WriteAnimationsNames(pFile, pSkeleton);
    WriteBonesHierarchy (pFile, pSkeleton);
    WriteBaseframe      (pFile, pSkeleton);
    WriteOffsets        (pFile, pSkeleton);
    WriteWeights        (pFile, pSkeleton);
    WriteAnimations     (pFile, pSkeleton);

    fclose(pFile);

    LogMsg(LOG, "skeleton '%s' is saved into file: %s", pSkeleton->GetName(), filename);
    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void WriteCommonInfo(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    fprintf(pFile, "numAnimations %d\n", (int)pSkeleton->GetNumAnimations());
    fprintf(pFile, "numJoints %d\n",     (int)pSkeleton->GetNumBones());
    fprintf(pFile, "numVertices %d\n",   (int)pSkeleton->vertexToBones.size());
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  write a list of [animation_idx => animation name]
//---------------------------------------------------------
void WriteAnimationsNames(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    fprintf(pFile, "animations\n");

    const AnimationName* names = pSkeleton->animNames_.data();
    const size        numAnims = pSkeleton->animNames_.size();

    for (index i = 0; i < numAnims; ++i)
    {
        // animation_idx "animation_name"
        fprintf(pFile, "\t%d\t\"%s\"\n", (int)i, names[i].name);
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  a little helper to extract
//        position (vec3) and rotation quaternion (vec4) from input matrix
// Args:  m - matrix to decompose
// Out:   p - position
//        q - rotation quat
//---------------------------------------------------------
inline void GetPosRotQuat(const XMMATRIX& m, XMFLOAT4& p, XMFLOAT4& q)
{
    XMVECTOR S;
    XMVECTOR Q;
    XMVECTOR T;
    XMMatrixDecompose(&S, &Q, &T, m);

    Q = XMQuaternionNormalize(Q);

    XMStoreFloat4(&p, T);
    XMStoreFloat4(&q, Q);
}

//---------------------------------------------------------
// Desc:  store bones hierarchy and bone info:
//        bone_name, bone_parent_idx
//---------------------------------------------------------
void WriteBonesHierarchy(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    // print block header
    fprintf(pFile, "hierarchy\n");

    const int*      boneHierarchy   = pSkeleton->boneHierarchy_.data();
    const BoneName* bonesNames      = pSkeleton->boneNames_.data();

    for (index i = 0; i < pSkeleton->GetNumBones(); ++i)
    {
        fputc('\t', pFile);
        fputc('"',  pFile);
        fprintf(pFile, "%s", bonesNames[i].name);   // write name of bone

        fputc('"', pFile);
        fputc('\t', pFile);
        fprintf(pFile, "%d", boneHierarchy[i]);     // write parent bone idx

        fprintf(pFile, "\t\t//");

        // if current bone has a parent we write a parent bone's name
        if (boneHierarchy[i] != -1)
            fprintf(pFile, "%s", bonesNames[boneHierarchy[i]].name);

        fprintf(pFile, "\n");
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  write base frame joints (bind-pose / T-pose tranformations)
//---------------------------------------------------------
void WriteBaseframe(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    fprintf(pFile, "baseframe\n");

    const XMMATRIX* transforms = pSkeleton->boneTransforms_.data();

    for (index i = 0; i < pSkeleton->GetNumBones(); ++i)
    {
        XMFLOAT4 p, q;
        GetPosRotQuat(transforms[i], p, q);

        // (position) (rotation_quat)
        fprintf(pFile, "\t(%f %f %f)\t(%f %f %f %f)\n", p.x, p.y, p.z, q.x, q.y, q.z, q.w);
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  store offset matrices of bones
//---------------------------------------------------------
void WriteOffsets(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    const cvector<XMMATRIX>& offsets = pSkeleton->boneOffsets_;

    fprintf(pFile, "offsets\n");

    for (index i = 0; i < offsets.size(); ++i)
    {
        XMFLOAT4 p, q;
        GetPosRotQuat(offsets[i], p, q);

        // (position) (rotation_quat)
        fprintf(pFile, "\t(%f %f %f)\t(%f %f %f %f)\n", p.x, p.y, p.z, q.x, q.y, q.z, q.w);
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  write into a file bones weights 
//---------------------------------------------------------
void WriteWeights(FILE* pFile, const AnimSkeleton* pSkeleton)
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

    const VertexBoneData* data      = pSkeleton->vertexToBones.data();
    const size            numBlocks = pSkeleton->vertexToBones.size();


    fprintf(pFile, "numweights %d\n", numWeights);

    for (int i = 0, weightIdx = 0; i < (int)numBlocks; ++i)
    {
        const float* weights = data[i].weights;
        const uint*  boneIds = data[i].boneIds;

        // write: w weight_idx vertex_idx offset_in_block bone_id weight
        if (weights[0] != 0.0f)
            fprintf(pFile, "\tw %d %d %d %u %f\n", weightIdx++, i, 0, boneIds[0], weights[0]);

        if (weights[1] != 0.0f)
            fprintf(pFile, "\tw %d %d %d %u %f\n", weightIdx++, i, 1, boneIds[1], weights[1]);

        if (weights[2] != 0.0f)
            fprintf(pFile, "\tw %d %d %d %u %f\n", weightIdx++, i, 2, boneIds[2], weights[2]);

        if (weights[3] != 0.0f)
            fprintf(pFile, "\tw %d %d %d %u %f\n", weightIdx++, i, 3, boneIds[3], weights[3]);
    }
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  write input keyframes into the file
//---------------------------------------------------------
void WriteKeyframes(FILE* pFile, const cvector<Keyframe>& keyframes)
{
    assert(pFile);

    for (const Keyframe& frame : keyframes)
    {
        const XMFLOAT3& p = frame.translation;
        const XMFLOAT4& q = frame.rotQuat;

        // (position) (rotation_quat)
        fprintf(pFile, "\t\t(%f %f %f)\t(%f %f %f %f)\n", p.x, p.y, p.z, q.x, q.y, q.z, q.w);
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
void WriteAnimations(FILE* pFile, const AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    for (int i = 0; i < (int)pSkeleton->GetNumAnimations(); ++i)
    {
        // for current animation each bone has the same number of keyframes
        const AnimationClip& anim = pSkeleton->animations_[i];

        fprintf(pFile, "animation %d\n", i);
        fprintf(pFile, "numFrames %d\n", (int)anim.boneAnimations[0].keyframes.size());
        fprintf(pFile, "frameRate %d\n", (int)anim.framerate);
        
        WriteAnimation(pFile, anim);
    }
}


} // namespace
