// *********************************************************************************
// Filename:     TextureTransformHelper.h
// Description:  contains different helper stuff for the TextureTransform component
// 
// Created:      20.08.24
// *********************************************************************************
#pragma once


#include "../Common/ECSTypes.h"
#include <Types.h>
#include <cvector.h>

namespace ECS
{

// ---------------------------------------------------------------------------------
//           DATA STRUCTURES FOR INITIALIZATION OF TEXTURE TRANSFORMATIONS 
// ---------------------------------------------------------------------------------

struct TexTransformInitParams
{
    // basic type
};

///////////////////////////////////////////////////////////

struct StaticTexTransInitParams : public TexTransformInitParams
{
    // contains array of init params for the texture static transformation 
    // (for example: scaling and then moving in one direction)


    StaticTexTransInitParams() {};

    StaticTexTransInitParams(
        const uint32 transformsCount,
        const XMMATRIX& inInitTransform,
        const XMMATRIX& transformToUpdate = DirectX::XMMatrixIdentity())  // by default we have no changes of transformation during runtime
    {
        // a constructor to init the input number of transformations with the same value
        assert(transformsCount > 0);
        initTransform.resize(transformsCount, inInitTransform);
        texTransforms.resize(transformsCount, transformToUpdate);
    }

    StaticTexTransInitParams(
        const cvector<XMMATRIX>& initTransformations,
        const cvector<XMMATRIX>& transformationsToUpdate)
    {
        // a constructor to init data with input arrays
        assert(initTransformations.size() == transformationsToUpdate.size());
        initTransform = initTransformations;
        texTransforms = transformationsToUpdate;
    }

    void Push(
        const cvector<XMMATRIX>& initTransformations,
        const cvector<XMMATRIX>& transformationsToUpdate)
    {
        assert(initTransformations.size() == transformationsToUpdate.size());
        initTransform.append_vector(initTransformations);
        texTransforms.append_vector(transformationsToUpdate);
    }

    void Push(
        const XMMATRIX& inInitTransform,
        const XMMATRIX& transformToUpdate = DirectX::XMMatrixIdentity()) // by default we have no changes of transformation during runtime	
    {
        initTransform.push_back(inInitTransform);
        texTransforms.push_back(transformToUpdate);
    }


    // how the texture is transformated in the beginning
    cvector<XMMATRIX> initTransform;

    // then we maybe want to move the texture in some direction so we use this matrix 
    // (or use the identity matrix for no changes)
    cvector<XMMATRIX> texTransforms;
};

///////////////////////////////////////////////////////////

struct AtlasAnimInitParams : public TexTransformInitParams
{
    // contains arrays of init params for the texture atlas animations

    void Push(
        const cvector<uint8_t>& inTexRows,
        const cvector<uint8_t>& inTexColumns,
        const cvector<float>& inAnimDurations)
    {
        // check input data
        bool areTexRowsValid = true;
        bool areTexColumnsValid = true;
        bool areAnimDurationsValid = true;

        // check if we don't have any rows count == 0
        for (const uint8_t rowsCount : inTexRows)
            areTexRowsValid &= (bool)rowsCount;

        // check if we don't have any columns count == 0
        for (const uint8_t colsCount : inTexColumns)
            areTexColumnsValid &= (bool)colsCount;

        // check if we don't have any animation duration <= 0
        for (const float duration : inAnimDurations)
            areAnimDurationsValid &= (duration > 0);

        assert((inTexRows.size() == inTexColumns.size()) && (inTexRows.size() == inAnimDurations.size()) && "input data is invalid");
        assert((areTexRowsValid & areTexColumnsValid & areAnimDurationsValid) && "input data is invalid");

        // append data arrays
        texRows.append_vector(inTexRows);
        texColumns.append_vector(inTexColumns);
        animDurations.append_vector(inAnimDurations);
    }

    void Push(const uint32 rowsCount, const uint32 columnsCount, const float animDuration)
    {
        assert((rowsCount & columnsCount) && (animDuration > 0));

        texRows.push_back(rowsCount);
        texColumns.push_back(columnsCount);
        animDurations.push_back(animDuration);
    }

    cvector<uint8_t> texRows;
    cvector<uint8_t> texColumns;
    cvector<float> animDurations;
};

///////////////////////////////////////////////////////////

struct RotationAroundCoordInitParams : public TexTransformInitParams
{
    // contains init params for the texture rotation around coordinates
    // (for instance: p(0.5, 0.5) - rotation arount its center)

    void Push(const float rotCenterX, const float rotCenterY, const float rotSpeed)
    {
        center.push_back(XMFLOAT2{ rotCenterX, rotCenterY });
        speed.push_back(rotSpeed);
    }

    void Push(const XMFLOAT2 rotCenter, const float rotSpeed)
    {
        center.push_back(rotCenter);
        speed.push_back(rotSpeed);
    }

    cvector<XMFLOAT2> center;      // around this coords a texture will rotate
    cvector<float>    speed;       // how fast texture rotates
};


// =================================================================================
//                  HELPER DATA STRUCTURES FOR STORING DATA
// =================================================================================

struct TexStaticTransformations
{
    cvector<EntityID> ids;
    cvector<XMMATRIX> transformations;   // these matrices are used to update the current static transformation
};

///////////////////////////////////////////////////////////

struct TexAtlasAnimationData
{
    // contains animation data for an atlas texture 

    TexAtlasAnimationData() {}

    TexAtlasAnimationData(
        const uint8_t inTexRows,
        const uint8_t inTexColumns,
        const float duration) :
        texRows(inTexRows),
        texColumns(inTexColumns),
        texFramesCount(inTexRows* inTexColumns),
        animDuration(duration),
        texCellWidth(1.0f / (float)inTexColumns),
        texCellHeight(1.0f / (float)inTexRows) {}


    uint16_t currTexFrameIdx = 0;       // index of the current animation frame
    uint16_t texFramesCount = 0;        // how many animation frames we have
    uint8_t texColumns = 0;            // vertical cells (frames) count
    uint8_t texRows = 0;               // horizontal cells (frames) count

    float animDuration = 0;            // duration of the whole animation
    float texCellWidth = 0;            // width of one animation frame
    float texCellHeight = 0;           // height of one animation frame
};

///////////////////////////////////////////////////////////

struct TexAtlasAnimations
{
    // texture transformation type: ATLAS_ANIMATION
    cvector<EntityID> ids;
    cvector<float> timeSteps;               // duration of one animation frame; after this time point we change an animation frame
    cvector<float> currAnimTime;           // frame time value in [0, timeStep]; when this val >= timeStep we change a frame
    cvector<TexAtlasAnimationData> data;
};

///////////////////////////////////////////////////////////

struct TexRotationsAroundCoords
{
    // texture transformation type: ROTATION_AROUND_TEX_COORD
    cvector<EntityID> ids;
    cvector<XMFLOAT2> texCoords;
    cvector<float>    rotationsSpeed;
};

}
