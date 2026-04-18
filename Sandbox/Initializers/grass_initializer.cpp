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

    Created:  25.01.2026  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "grass_initializer.h"
#include <Model/grass_mgr.h>
#include <Model/model_mgr.h>
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

    LogMsg(LOG, "initialize grass from file: %s", filepath);


    Core::GrassMgr& grassMgr = Core::g_GrassMgr;
    FILE* pFile = nullptr;
    char buf[128]{'\0'};
    int count = 0;
    bool bInitCommonParams = false;
    

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

            grassMgr.SetGrassVisibilityRange((float)params.grassDistVisible);
            grassMgr.SetGrassDistFullSize((float)params.grassDistFullSize);
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

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadCommonParams(FILE* pFile, GrassCommonParams& outParams)
{
    assert(pFile);

    char key[64];
    int count = 0;

    count = fscanf(pFile, "%s %d\n", key, &outParams.grassDistFullSize);
    assert(count == 2);
    assert(strcmp(key, "grass_dist_full_size") == 0);

    count = fscanf(pFile, "%s %d\n", key, &outParams.grassDistVisible);
    assert(count == 2);
    assert(strcmp(key, "grass_dist_visible") == 0);

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

    char key[64];
    int count = sscanf(buf, "%s %s", key, outValue);

    assert(count == 2);                         // did we read proper data?
    assert(outValue && outValue[0] != '\0');    // did we read a non-empty value?
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadIntParam(const char* buf, int& outInt)
{
    assert(buf && buf[0] != '\0');

    char key[64];
    int count = sscanf(buf, "%s %d", key, &outInt);

    assert(count == 2);                         // did we read proper data?
}

//---------------------------------------------------------
//---------------------------------------------------------
void ReadFloatParam(const char* buf, float& outFloat)
{
    assert(buf && buf[0] != '\0');

    char key[64];
    int count = sscanf(buf, "%s %f", key, &outFloat);

    assert(count == 2);                         // did we read proper data?
}

//---------------------------------------------------------
// read in two float params after key from the input buffer
//---------------------------------------------------------
void ReadTwoFloatParam(const char* buf, float& f1, float& f2)
{
    assert(buf && buf[0] != '\0');

    char key[64];
    int count = sscanf(buf, "%s %f %f", key, &f1, &f2);

    assert(count == 3);                         // did we read proper data?
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


        // read in models per each grass channel
        if (strcmp(key, "channel_0_model") == 0)
            ReadStrParam(buf, outData.modelNames[0]);

        else if (strcmp(key, "channel_1_model") == 0)
            ReadStrParam(buf, outData.modelNames[1]);

        else if (strcmp(key, "channel_2_model") == 0)
            ReadStrParam(buf, outData.modelNames[2]);

        else if (strcmp(key, "channel_3_model") == 0)
            ReadStrParam(buf, outData.modelNames[3]);

        // read in appearing probability factor per each grass channel
        else if (strcmp(key, "channel_0_probability") == 0)
            ReadFloatParam(buf, outData.channelProbability[0]);

        else if (strcmp(key, "channel_1_probability") == 0)
            ReadFloatParam(buf, outData.channelProbability[1]);

        else if (strcmp(key, "channel_2_probability") == 0)
            ReadFloatParam(buf, outData.channelProbability[2]);

        else if (strcmp(key, "channel_3_probability") == 0)
            ReadFloatParam(buf, outData.channelProbability[3]);


        // read in grass height ranges per channel
        else if (strcmp(key, "channel_0_grass_scale_range") == 0)
            ReadTwoFloatParam(buf, outData.channelGrassScaleMin[0], outData.channelGrassScaleMax[0]);

        else if (strcmp(key, "channel_1_grass_scale_range") == 0)
            ReadTwoFloatParam(buf, outData.channelGrassScaleMin[1], outData.channelGrassScaleMax[1]);

        else if (strcmp(key, "channel_2_grass_scale_range") == 0)
            ReadTwoFloatParam(buf, outData.channelGrassScaleMin[2], outData.channelGrassScaleMax[2]);

        else if (strcmp(key, "channel_3_grass_scale_range") == 0)
            ReadTwoFloatParam(buf, outData.channelGrassScaleMin[3], outData.channelGrassScaleMax[3]);


        // read in params of the whole field
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

        else if (strcmp(key, "density_mask_rgb") == 0)
            ReadStrParam(buf, outData.densityMaskRGB);

        else if (strcmp(key, "density_mask_alpha") == 0)
            ReadStrParam(buf, outData.densityMaskAlpha);

        else
            LogFatal(LOG, "wtf? there is a wrong key: %s", key);
    }

    LogMsg("Read grass field params:");

    LogMsg("\tfield name:                    %s", outData.name);

    LogMsg("\tchannel_0_model:               %s", outData.modelNames[0]);
    LogMsg("\tchannel_1_model:               %s", outData.modelNames[1]);
    LogMsg("\tchannel_2_model:               %s", outData.modelNames[2]);
    LogMsg("\tchannel_3_model:               %s", outData.modelNames[3]);

    LogMsg("\tchannel_0_probability:         %.2f", outData.channelProbability[0]);
    LogMsg("\tchannel_1_probability:         %.2f", outData.channelProbability[1]);
    LogMsg("\tchannel_2_probability:         %.2f", outData.channelProbability[2]);
    LogMsg("\tchannel_3_probability:         %.2f", outData.channelProbability[3]);

    for (int i = 0; i < 4; ++i)
    {
        const float minScale = outData.channelGrassScaleMin[i];
        const float maxScale = outData.channelGrassScaleMax[i];

        LogMsg("\tchannel_%d_grass_scale_range:  %.2f %.2f", i, minScale, maxScale);
    }

    LogMsg("\tmaterial:                      %s", outData.materialName);

    LogMsg("\tcenter X:                      %d", outData.centerX);
    LogMsg("\tcenter Z:                      %d", outData.centerZ);
    LogMsg("\tsize X:                        %d", outData.sizeX);
    LogMsg("\tsize Z:                        %d", outData.sizeZ);
    LogMsg("\tcells by X:                    %d", outData.cellsByX);
    LogMsg("\tcells by Z:                    %d", outData.cellsByZ);
    LogMsg("\ttex slots:                     %d", outData.texSlots);
    LogMsg("\ttex rows:                      %d", outData.texRows);
    LogMsg("\tgrass count:                   %d", outData.grassCount);
    LogMsg("\tdensity mask RGB:              %s", outData.densityMaskRGB);
    LogMsg("\tdensity mask Alpha:            %s", outData.densityMaskAlpha);
}

} // namespace
