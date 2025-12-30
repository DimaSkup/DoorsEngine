#include <CoreCommon/pch.h>
#include "animation_loader.h"
#include "animation_helper.h"
#include "animation_mgr.h"

#pragma warning (disable : 4996)
#include <parse_helpers.h>      // helpers for parsing string buffers, or reading data from file

#include <DirectXMath.h>


namespace Core
{

// some typedefs
using VECTOR = DirectX::XMVECTOR;
using MATRIX = DirectX::XMMATRIX;

//---------------------------------------------------------
// forward declarations of helper functions
//---------------------------------------------------------
bool LoadCommonInfo     (FILE* pFile, AnimSkeleton* pSkeleton);
bool LoadBonesHierarchy (FILE* pFile, AnimSkeleton* pSkeleton);
void LoadBonesBaseframe (FILE* pFile, AnimSkeleton* pSkeleton);
void LoadBonesOffsets   (FILE* pFile, AnimSkeleton* pSkeleton);
void LoadBonesWeights   (FILE* pFile, AnimSkeleton* pSkeleton, const char* buf);
void LoadAnimation      (FILE* pFile, AnimSkeleton* pSkeleton, const char* buf, const index animIdx);

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


    LogMsg(LOG, "load skeleton and animations from file: %s", filename);

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
            // alloc memory for bones weights
            assert(numVertices > 0);
            skeleton.vertexToBones.resize(numVertices);
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
        }
    }

    LogMsg("%sis loaded%s", YELLOW, RESET);
    fclose(pFile);
    return true;
}

//---------------------------------------------------------
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

        count = sscanf(buf, "%s  %d", boneName, &hierarchy[i]);
        assert(count == 2);

        // remove last quote (") symbol from the name
        boneName[strlen(boneName) - 1] = '\0';
    }

    // skip "\n" after block
    fscanf(pFile, "\n");

    return true;
}

//---------------------------------------------------------
// Desc:  read base frame joints (bind-pose / T-pose tranformations)
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

    int count = 0;
    int numFrames = 0;
    int frameRate = 0;
    char* animName = pSkeleton->animNames_[animationIdx].name;
    AnimationClip& anim = anims[animationIdx];

    count = sscanf(buf, "animation %s", animName);
    assert(count == 1);

    count = fscanf(pFile, "numFrames %d\n", &numFrames);
    assert(count == 1);

    count = fscanf(pFile, "frameRate %d\n", &frameRate);
    assert(count == 1);

    printf("animation %-48s frames %-10d framerate %d\n", animName, numFrames, frameRate);
}

} // namespace

