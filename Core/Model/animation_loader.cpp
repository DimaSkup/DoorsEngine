#include <CoreCommon/pch.h>
#include "animation_loader.h"
#include "animation_helper.h"
#include "animation_mgr.h"

#pragma warning (disable : 4996)
#include <parse_helpers.h>      // helpers for parsing string buffers, or reading data from file

#include <Render/d3dclass.h>
#include <DirectXMath.h>


namespace Core
{

// some typedefs
using VECTOR = DirectX::XMVECTOR;
using MATRIX = DirectX::XMMATRIX;

//---------------------------------------------------------
// forward declarations of helper functions
//---------------------------------------------------------
bool LoadAnimationsNames(FILE* pFile, AnimSkeleton* pSkeleton);
bool LoadBonesHierarchy (FILE* pFile, AnimSkeleton* pSkeleton);
void LoadBonesBaseframe (FILE* pFile, AnimSkeleton* pSkeleton);
void LoadBonesOffsets   (FILE* pFile, AnimSkeleton* pSkeleton);
void LoadBonesWeights   (FILE* pFile, AnimSkeleton* pSkeleton, const char* buf);
void LoadAnimation      (FILE* pFile, AnimSkeleton* pSkeleton, const char* buf, const index animIdx);
void LoadKeyframes      (FILE* pFile, Keyframe* keyframes, const int numFrames);
void InitSkeletonBonesVB(AnimSkeleton* pSkeleton);


//---------------------------------------------------------
// Desc:  load a skeleton, its bones data, weights, and animations from file
// Args:  - pSkeleton: skeleton to init
//        - filename:  path to file
// Ret:   id of created skeleton (if zero it means that we failed for some reason)
//---------------------------------------------------------
SkeletonID AnimationLoader::Load(const char* filename)
{
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "empty filename");
        return false;
    }

    FILE* pFile = fopen(filename, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", filename);
        return false;
    }


    LogMsg(LOG, "\n\n\nload skeleton and animations from file: %s", filename);

    char skeletonName[MAX_LEN_SKELETON_NAME]{'\0'};
    FileSys::GetFileStem(filename, skeletonName);

    // skeleton has the same name as its file
    SkeletonID    skeletonId = g_AnimationMgr.AddSkeleton(skeletonName);
    AnimSkeleton& skeleton   = g_AnimationMgr.GetSkeleton(skeletonId);

    char buf[512];
    int numAnimations = 0;
    int numBones = 0;
    int numVertices = 0;
    index animationIdx = 0;


    while (!feof(pFile))
    {
        // read whole line
        fgets(buf, sizeof(buf), pFile);

        if (sscanf(buf, "numAnimations %d", &numAnimations) == 1)
        {
            // alloc memory for animations
            assert(numAnimations > 0);
            skeleton.animNames_.resize(numAnimations);
            skeleton.animations_.resize(numAnimations);
        }
        else if (sscanf(buf, "numJoints %d", &numBones) == 1)
        {
            // alloc memory for bones stuff
            assert(numBones > 0);
            skeleton.boneNames_.resize(numBones);
            skeleton.boneHierarchy_.resize(numBones);
            skeleton.boneTransforms_.resize(numBones);
            skeleton.boneOffsets_.resize(numBones);
        }
        else if (sscanf(buf, "numVertices %d", &numVertices) == 1)
        {
            // alloc memory for bones weights and ids (per each vertex)
            assert(numVertices > 0);
            skeleton.vertexToBones.resize(numVertices);
        }
        else if (strncmp(buf, "animations", 10) == 0)
        {
            LoadAnimationsNames(pFile, &skeleton);
        }
        else if (strncmp(buf, "hierarchy", 9) == 0)
        {
            LoadBonesHierarchy(pFile, &skeleton);
        }
        else if (strncmp(buf, "baseframe", 9) == 0)
        {
            LoadBonesBaseframe(pFile, &skeleton);
        }
        else if (strncmp(buf, "offsets", 7) == 0)
        {
            LoadBonesOffsets(pFile, &skeleton);
        }
        else if (strncmp(buf, "numweights", 10) == 0)
        {
            LoadBonesWeights(pFile, &skeleton, buf);
        }
        else if (strncmp(buf, "animation", 9) == 0)
        {
            LoadAnimation(pFile, &skeleton, buf, animationIdx);
            animationIdx++;
        }
    }

    // init vertex buffer which contains bones weights and ids
    InitSkeletonBonesVB(&skeleton);

    LogMsg("%sis loaded%s", YELLOW, RESET);
    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:  load a name for each animation
//---------------------------------------------------------
bool LoadAnimationsNames(FILE* pFile, AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    AnimationName* animNames = pSkeleton->animNames_.data();
    const int      numAnims  = (int)pSkeleton->animations_.size();
    int            animIdx   = 0;
    int            count     = 0;

    for (int i = 0; i < numAnims; ++i)
    {
        char* animName = animNames[i].name;

        count = fscanf(pFile, " %d \"%s", &animIdx, animName);
        assert(count == 2);
        assert(animIdx == i);

        // skip the last quote (") symbol in the name
        animName[strlen(animName) - 1] = '\0';
    }
    fscanf(pFile, "\n");

    return true;
}

//---------------------------------------------------------
// Desc:  calculate a transformation matrix based on position and rotation quaternion
//---------------------------------------------------------
inline void CalcTransform(const Vec4& pos, const Vec4& quat, MATRIX& m)
{
    const VECTOR S      = { 1,1,1 };
    const VECTOR Q      = { quat.x, quat.y, quat.z, quat.w };
    const VECTOR T      = { pos.x, pos.y, pos.z };
    const VECTOR zero   = { 0,0,0,1 };

    m = DirectX::XMMatrixAffineTransformation(S, zero, Q, T);
}

//---------------------------------------------------------
// Desc:  read in bones names and its parent bone's idxs
//---------------------------------------------------------
bool LoadBonesHierarchy(FILE* pFile, AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    char buf[512]{'\0'};

    BoneName* names     = pSkeleton->boneNames_.data();
    int*      hierarchy = pSkeleton->boneHierarchy_.data();
    int       count     = 0;

    for (index i = 0; i < pSkeleton->GetNumBones(); ++i)
    {
        // read whole line
        fgets(buf, sizeof(buf), pFile);

        char* boneName = names[i].name;

        // +2 because we skip tabulation ('\t') and quote symbol (")
        count = sscanf(buf+2, "%s %d", boneName, &hierarchy[i]);
        assert(count == 2);

        // remove last quote (") symbol from the name
        boneName[strlen(boneName) - 1] = '\0';
    }

    // skip "\n" after block
    fscanf(pFile, "\n");

    return true;
}

//---------------------------------------------------------
// Desc:  read base frame joints matrices (bind-pose / T-pose tranformations)
//---------------------------------------------------------
void LoadBonesBaseframe(FILE* pFile, AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    char    buf[512]{ '\0' };
    MATRIX* transforms = pSkeleton->boneTransforms_.data();
    Vec4    p, q;
    int     count = 0;

    for (index i = 0; i < pSkeleton->GetNumBones(); ++i)
    {
        // read whole line
        fgets(buf, sizeof(buf), pFile);

        // read (position) (rot_quat)
        count = sscanf(buf, " ( %f %f %f ) ( %f %f %f %f )", &p.x, &p.y, &p.z, &q.x, &q.y, &q.z, &q.w);
        assert(count == 7);

        CalcTransform(p, q, transforms[i]);
    }

    // skip "\n" after block
    fscanf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  read offset matrix for each bone
//---------------------------------------------------------
void LoadBonesOffsets(FILE* pFile, AnimSkeleton* pSkeleton)
{
    assert(pFile);
    assert(pSkeleton);

    char    buf[512]{ '\0' };
    MATRIX* offsets = pSkeleton->boneOffsets_.data();
    Vec4    p, q;
    int     count = 0;

    for (index i = 0; i < pSkeleton->GetNumBones(); ++i)
    {
        // read whole line
        fgets(buf, sizeof(buf), pFile);

        // read (position) (rot_quat)
        count = sscanf(buf, " ( %f %f %f ) ( %f %f %f %f )", &p.x, &p.y, &p.z, &q.x, &q.y, &q.z, &q.w);
        assert(count == 7);

        CalcTransform(p, q, offsets[i]);
    }

    // skip "\n" after the block
    fscanf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  load bones weights for each vertex
//---------------------------------------------------------
void LoadBonesWeights(FILE* pFile, AnimSkeleton* pSkeleton, const char* buf)
{
    assert(pFile);
    assert(pSkeleton);
    assert(buf && buf[0] != '\0');


    int   count = 0;
    int   weightIdx  = 0;
    int   vertIdx    = 0;      // vertex number
    int   offset     = 0;
    int   boneId     = 0;
    float weight     = 0;
    int   numWeights = 0;

    int   numBones            = (int)pSkeleton->GetNumBones();
    int   numVertices         = (int)pSkeleton->vertexToBones.size();
    int   numWeightsPerVertex = 4;
    int   maxNumWeights       = numVertices * numWeightsPerVertex;

    count = sscanf(buf, "numweights %d", &numWeights);
    assert(count == 1);
    assert(numWeights <= maxNumWeights);


    // read in each weight
    for (int i = 0; i < numWeights; ++i)
    {
        count = fscanf(pFile, " w %d %d %d %d %f\n",
            &weightIdx,
            &vertIdx,
            &offset,
            &boneId,
            &weight);

        // check to prevent fuck up
        assert(weightIdx < numWeights);
        assert(vertIdx <= numVertices);
        assert(offset < numWeightsPerVertex);
        assert(boneId < numBones);
        assert(weight <= 1.0f);

        pSkeleton->vertexToBones[vertIdx].weights[offset] = weight;
        pSkeleton->vertexToBones[vertIdx].boneIds[offset] = boneId;
    }

    // skip "\n" after the block
    fscanf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  load in data for animation by animationIdx
//---------------------------------------------------------
void LoadAnimation(
    FILE* pFile,
    AnimSkeleton* pSkeleton,
    const char* buf,
    const index animationIdx)
{
    assert(pFile);
    assert(pSkeleton);

    cvector<AnimationClip>& anims = pSkeleton->animations_;
    assert(animationIdx >= 0  &&  animationIdx < anims.size());

    int boneIdx   = 0;
    int count     = 0;
    int numFrames = 0;
    int frameRate = 0;
    int animIdx   = 0;

    const int  numBones = (int)pSkeleton->GetNumBones();
    AnimationClip& anim = anims[animationIdx];


    count = sscanf(buf, "animation %d", &animIdx);
    assert(count == 1);

    count = fscanf(pFile, "numFrames %d\n", &numFrames);
    assert(count == 1);

    count = fscanf(pFile, "frameRate %d\n", &frameRate);
    assert(count == 1);

    assert(frameRate > 0);
    assert(animIdx >= 0);
    assert(animIdx == animationIdx);

    anim.framerate = (float)frameRate;
    anim.id = animIdx;


    // alloc memory for each bone and each keyframe for this bone
    pSkeleton->animations_[animIdx].boneAnimations.resize(numBones);

    for (int i = 0; i < numBones; ++i)
        anim.boneAnimations[i].keyframes.resize(numFrames);


    // read each keyframe for each bone
    for (int i = 0; i < numBones; ++i)
    {
        count = fscanf(pFile, " frames for bone %d\n", &boneIdx);
        assert(count == 1);
        assert(boneIdx == i);

        LoadKeyframes(pFile, anim.boneAnimations[boneIdx].keyframes.data(), numFrames);
    }
}

//---------------------------------------------------------
// Desc:  read in a set of keyframes for bone
//---------------------------------------------------------
void LoadKeyframes(FILE* pFile, Keyframe* keyframes, const int numFrames)
{
    assert(pFile);
    assert(keyframes);
    assert(numFrames > 0);

    // read in each keyframe
    for (int i = 0; i < numFrames; ++i)
    {
        DirectX::XMFLOAT3& p = keyframes[i].translation;
        DirectX::XMFLOAT4& q = keyframes[i].rotQuat;

        int count = fscanf(pFile, "(%f %f %f) (%f %f %f %f)\n", &p.x, &p.y, &p.z, &q.x, &q.y, &q.z, &q.w);
        assert(count == 7);
    }
    fscanf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:  init a vertex buffer (which contains bones weights and indices)
//        related to the input skeleton
//---------------------------------------------------------
void InitSkeletonBonesVB(AnimSkeleton* pSkeleton)
{
    assert(pSkeleton);

    ID3D11Device*         pDevice         = Render::g_pDevice;
    const VertexBoneData* bonesWeights    = pSkeleton->vertexToBones.data();
    const int             numBonesWeights = (int)pSkeleton->vertexToBones.size();
    const bool            isDynamicVB     = false;

    if (!pSkeleton->bonesVB_.Initialize(pDevice, bonesWeights, numBonesWeights, isDynamicVB))
    {
        LogErr(LOG, "can't init bones VB for skeleton: %s", pSkeleton->GetName());
    }
}

} // namespace

