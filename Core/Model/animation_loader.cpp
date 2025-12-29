#include <CoreCommon/pch.h>
#include "animation_loader.h"

#pragma warning (disable : 4996)
#include <parse_helpers.h>      // helpers for parsing string buffers, or reading data from file


namespace Core
{

//---------------------------------------------------------
// Desc:  load a skeleton, its bones data, weights, and animations from file
// Args:  pSkeleton - skeleton to init
//        filename  - path to file
// Ret:   true      - if everything is ok
//---------------------------------------------------------
bool AnimationLoader::Load(const AnimSkeleton* pSkeleton, const char* filename)
{
    if (!pSkeleton)
    {
        LogErr(LOG, "ptr to skeleton == nullptr");
        return false;
    }
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

    int numAnimations = 0;
    int numBones = 0;

    ReadFileInt(pFile, "numAnimations", &numAnimations);
    ReadFileInt(pFile, "numJoints",     &numBones);

    fclose(pFile);
    return true;
}


} // namespace

