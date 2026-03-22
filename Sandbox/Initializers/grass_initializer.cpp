/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: grass_initializer.cpp
    Desc:     implementation of the GrassInitializer class

    Created:  25.01.2025  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "grass_initializer.h"
#include <Model/grass_mgr.h>
#include <Model/model_mgr.h>
#include <Timers/game_timer.h>
#pragma warning (disable : 4996)


namespace Game
{

//---------------------------------------------------------
// helper structures for initialization
//---------------------------------------------------------
struct GrassCommonParams
{
    int grassDistFullSize = 0;
    int grassDistVisible = 40;
};


//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void ReadCommonParams    (FILE* pFile, GrassCommonParams& outParams);
void ReadGrassFieldParams(FILE* pFile, Core::GrassFieldInitParams& outData, const char* buf);
void CreateGrassField    (const Core::GrassField& data);

//---------------------------------------------------------
// Desc:  read in configs from file and initialize all the grass fields
// Args:  - filepath:  a path to grass config file
//---------------------------------------------------------
bool GrassInitializer::Init(const char* filepath, ECS::EntityMgr& enttMgr)
{
    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "empty path");
        return false;
    }

    SetConsoleColor(YELLOW);
    LogMsg("---------------------------------------------------------");
    LogMsg("            INITIALIZATION: GRASS                        ");
    LogMsg("---------------------------------------------------------");
    LogMsg(LOG, "initialize grass from file: %s", filepath);


    Core::GrassMgr& grassMgr = Core::g_GrassMgr;
    FILE* pFile = nullptr;
    char buf[128]{'\0'};
    int count = 0;
    bool bInitCommonParams = false;
    const TimePoint start = Core::GameTimer::GetTimePoint();

    // open config file
    pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", filepath);
        return false;
    }

    // skip comments section
    do {
        fgets(buf, sizeof(buf), pFile);
    } while (buf[0] == ';');


    // read in configs
    while (fgets(buf, sizeof(buf), pFile))
    {
        if (strncmp(buf, "common_params", 13) == 0)
        {
            // read in common params
            GrassCommonParams params;
            ReadCommonParams(pFile, params);
            bInitCommonParams = true;

            assert(params.grassDistFullSize >= 0);
            assert(params.grassDistVisible >= params.grassDistFullSize);

            Core::g_GrassMgr.SetGrassVisibilityRange((float)params.grassDistVisible);
        }
        else if (strncmp(buf, "grass_field", 11) == 0)
        {
            // read in params of grass field and create it
            Core::GrassFieldInitParams initData;
            memset(&initData, 0, sizeof(initData));

            ReadGrassFieldParams(pFile, initData, buf);

            grassMgr.AddGrassField(initData);
        }
    }

    // do some additional check
    assert(bInitCommonParams && "did you forget to init grass common params?");


    const TimeDurationMs dur = Core::GameTimer::GetTimePoint() - start;

    LogMsg(LOG, "all the GRASS instances are initialized");
    SetConsoleColor(MAGENTA);
    LogMsg("--------------------------------------");
    LogMsg("Init of grass took: %.3f ms", dur.count());
    LogMsg("--------------------------------------\n");
    SetConsoleColor(RESET);

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadCommonParams(FILE* pFile, GrassCommonParams& outParams)
{
    assert(pFile);

    char paramType[64];
    int count = 0;

    count = fscanf(pFile, "%s %d\n", paramType, &outParams.grassDistFullSize);
    assert(count == 2);
    assert(strcmp(paramType, "grass_dist_full_size") == 0);

    count = fscanf(pFile, "%s %d\n", paramType, &outParams.grassDistVisible);
    assert(count == 2);
    assert(strcmp(paramType, "grass_dist_visible") == 0);

    printf("\n");
    LogMsg("Read grass common params:");
    LogMsg("grass dist full size:  %d", outParams.grassDistFullSize);
    LogMsg("grass dist visible:    %d", outParams.grassDistVisible);
    printf("\n");
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadStrParam(const char* buf, char* outValue)
{
    // check input args
    assert(buf && buf[0] != '\0');
    assert(outValue);

    char paramType[64];
    int count = sscanf(buf, "%s %s", paramType, outValue);

    assert(count == 2);                         // did we read proper data?
    assert(outValue && outValue[0] != '\0');    // did we read a non-empty value?
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadIntParam(const char* buf, int& outInt)
{
    // check input args
    assert(buf && buf[0] != '\0');

    char paramType[64];
    int count = sscanf(buf, "%s %d", paramType, &outInt);

    assert(count == 2);                         // did we read proper data?
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadFloatParam(const char* buf, float& outFloat)
{
    // check input args
    assert(buf && buf[0] != '\0');

    char paramType[64];
    int count = sscanf(buf, "%s %f", paramType, &outFloat);

    assert(count == 2);                         // did we read proper data?
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadGrassFieldParams(
    FILE* pFile,
    Core::GrassFieldInitParams& outData,
    const char* header)
{
    assert(pFile);
    assert(header);

    int count = 0;
    char key[64];
    char buf[128];

    memset(buf, 0, sizeof(buf));

    // extract a name of grass field from the input header
    count = sscanf(header, "%s \"%s", key, outData.name);
    assert(count == 2);
    assert(strcmp(key, "grass_field") == 0);

    // remove last quote (") symbol from the name
    outData.name[strlen(outData.name) - 1] = '\0';


    // read in each parameter for the grass field
    while (fgets(buf, sizeof(buf), pFile))
    {
        // if we finished reading params 
        if (buf[0] == '}')
            break;

        count = sscanf(buf, "%s", key);
        assert(count == 1);

        if (strcmp(key, "channel_0_model") == 0)
            ReadStrParam(buf, outData.modelNames[0]);

        else if (strcmp(key, "channel_1_model") == 0)
            ReadStrParam(buf, outData.modelNames[1]);

        else if (strcmp(key, "channel_2_model") == 0)
            ReadStrParam(buf, outData.modelNames[2]);

        else if (strcmp(key, "channel_3_model") == 0)
            ReadStrParam(buf, outData.modelNames[3]);

        else if (strcmp(key, "material") == 0)
            ReadStrParam(buf, outData.materialName);

        else if (strcmp(key, "grass_field_center_x") == 0)
            ReadIntParam(buf, outData.centerX);

        else if (strcmp(key, "grass_field_center_z") == 0)
            ReadIntParam(buf, outData.centerZ);

        else if (strcmp(key, "grass_field_size_x") == 0)
            ReadIntParam(buf, outData.sizeX);

        else if (strcmp(key, "grass_field_size_z") == 0)
            ReadIntParam(buf, outData.sizeZ);

        else if (strcmp(key, "cells_by_x") == 0)
            ReadIntParam(buf, outData.cellsByX);

        else if (strcmp(key, "cells_by_z") == 0)
            ReadIntParam(buf, outData.cellsByZ);

        else if (strcmp(key, "texture_slots") == 0)
            ReadIntParam(buf, outData.texSlots);

        else if (strcmp(key, "texture_rows") == 0)
            ReadIntParam(buf, outData.texRows);

        else if (strcmp(key, "grass_count") == 0)
            ReadIntParam(buf, outData.grassCount);

        else if (strcmp(key, "density_mask") == 0)
            ReadStrParam(buf, outData.densityMask);

        else if (strcmp(key, "grass_min_height") == 0)
            ReadFloatParam(buf, outData.grassMinHeight);

        else if (strcmp(key, "grass_max_height") == 0)
            ReadFloatParam(buf, outData.grassMaxHeight);

        else
            LogErr(LOG, "wtf? there is a wrong key: %s", key);
    }

    LogMsg("Read grass field params:");
    LogMsg("\tname:          %s", outData.name);
    LogMsg("\tcenter X:      %d", outData.centerX);
    LogMsg("\tcenter Z:      %d", outData.centerZ);
    LogMsg("\tsize X:        %d", outData.sizeX);
    LogMsg("\tsize Z:        %d", outData.sizeZ);
    LogMsg("\tcells by X:    %d", outData.cellsByX);
    LogMsg("\tcells by Z:    %d", outData.cellsByZ);
    LogMsg("\ttex slots:     %d", outData.texSlots);
    LogMsg("\ttex rows:      %d", outData.texRows);
    LogMsg("\tgrass count:   %d", outData.grassCount);
    LogMsg("\tdensity mask:  %s", outData.densityMask);
}

} // namespace
