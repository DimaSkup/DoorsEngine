// *********************************************************************************
// Filename:     TextureTransformHelper.h
// Description:  contains different helper stuff for the TextureTransform component
// 
// Created:      20.08.24
// *********************************************************************************
#pragma once


#include "../Common/Types.h"
#include "../Common/Utils.h"
#include <vector>

namespace ECS
{


// ************************************************************************************
// 
//              DATA STRUCTURES FOR INITIALIZATION OF TEXTURE TRANSFORMATIONS 
// 
// ************************************************************************************

struct TexTransformInitParams {};

///////////////////////////////////////////////////////////

struct StaticTexTransParams : public TexTransformInitParams
{
	// contains array of init params for the texture static transformation 
	// (for example: scaling and then moving in one direction)


	StaticTexTransParams() {};

	StaticTexTransParams(
		const u32 transformsCount,
		const XMMATRIX& initTransform,
		const XMMATRIX& transformToUpdate = DirectX::XMMatrixIdentity())  // by default we have no changes of transformation during runtime
	{
		// a constructor to init the input number of transformations with the same value
		assert(transformsCount > 0);
		initTransform_.resize(transformsCount, initTransform);
		texTransforms_.resize(transformsCount, transformToUpdate);
	}

	StaticTexTransParams(
		const std::vector<XMMATRIX>& initTransformations,
		const std::vector<XMMATRIX>& transformationsToUpdate)
	{
		// a constructor to init data with input arrays
		assert(initTransformations.size() == transformationsToUpdate.size());
		initTransform_ = initTransformations;
		texTransforms_ = transformationsToUpdate;
	}

	void Push(
		const std::vector<XMMATRIX>& initTransformations,
		const std::vector<XMMATRIX>& transformationsToUpdate)
	{
		assert(initTransformations.size() == transformationsToUpdate.size());
		Utils::AppendToArray(initTransform_, initTransformations);
		Utils::AppendToArray(texTransforms_, transformationsToUpdate);
	}

	void Push(
		const XMMATRIX& initTransform,
		const XMMATRIX& transformToUpdate = DirectX::XMMatrixIdentity()) // by default we have no changes of transformation during runtime	
	{
		initTransform_.push_back(initTransform);
		texTransforms_.push_back(transformToUpdate);
	}


	// how the texture is transformated in the beginning
	std::vector<XMMATRIX> initTransform_;

	// then we maybe want to move the texture in some direction so we use this matrix 
	// (or use the identity matrix for no changes)
	std::vector<XMMATRIX> texTransforms_;
};

///////////////////////////////////////////////////////////

struct AtlasAnimParams : public TexTransformInitParams
{
	// contains arrays of init params for the texture atlas animations

	void Push(
		const std::vector<u32>& inTexRows,
		const std::vector<u32>& inTexColumns,
		const std::vector<float>& inAnimDurations)
	{
		// check input data
		bool areTexRowsValid = true;
		bool areTexColumnsValid = true;
		bool areAnimDurationsValid = true;

		for (const u32 rowsCount : inTexRows)
			areTexRowsValid &= (bool)rowsCount;

		for (const u32 colsCount : inTexColumns)
			areTexColumnsValid &= (bool)colsCount;

		for (const float duration : inAnimDurations)
			areAnimDurationsValid &= (duration > 0);

		assert((inTexRows.size() == inTexColumns.size()) && (inTexRows.size() == inAnimDurations.size()) && "input data is invalid");
		assert((areTexRowsValid & areTexColumnsValid & areAnimDurationsValid) && "input data is invalid");

		// append data arrays
		Utils::AppendToArray(texRows_, inTexRows);
		Utils::AppendToArray(texColumns_, inTexColumns);
		Utils::AppendToArray(animDurations_, inAnimDurations);
	}

	void Push(const u32 rowsCount, const u32 columnsCount, const float animDuration)
	{
		assert((rowsCount & columnsCount) && (animDuration > 0));

		texRows_.push_back(rowsCount);
		texColumns_.push_back(columnsCount);
		animDurations_.push_back(animDuration);
	}

	std::vector<u32> texRows_;
	std::vector<u32> texColumns_;
	std::vector<float> animDurations_;
};

///////////////////////////////////////////////////////////

struct RotationAroundCoordParams : public TexTransformInitParams
{
	// contains init params for the texture rotation around coordinates
	// (for instance: p(0.5, 0.5) - rotation arount its center)

	void Push(const float rotCenterX, const float rotCenterY, const float rotSpeed)
	{
		rotationsCenter_.emplace_back(rotCenterX, rotCenterY);
		rotationsSpeed_.push_back(rotSpeed);
	}

	void Push(const XMFLOAT2 rotCenter, const float rotSpeed)
	{
		rotationsCenter_.push_back(rotCenter);
		rotationsSpeed_.push_back(rotSpeed);
	}

	std::vector<XMFLOAT2> rotationsCenter_;      // around this coords a texture will rotate
	std::vector<float>    rotationsSpeed_;       // how fast will a texture rotate
};




// ***********************************************************************************
//
//                  HELPER DATA STRUCTURES FOR STORING DATA
// 
// ***********************************************************************************

struct TexStaticTransformations
{
	std::vector<EntityID> ids_;
	std::vector<XMMATRIX> transformations_;   // these matrices are used to update the current static transformation
};

///////////////////////////////////////////////////////////

struct TexAtlasAnimationData
{
	// contains animation data for an atlas texture 

	TexAtlasAnimationData() {}

	TexAtlasAnimationData(
		const u32 texRows,
		const u32 texColumns,
		const float duration) :
		texRows_(texRows),
		texColumns_(texColumns),
		texFramesCount_(texRows* texColumns),
		animDuration_(duration),
		texCellWidth_(1.0f / (float)texColumns),
		texCellHeight_(1.0f / (float)texRows) {}


	u32 currTexFrameIdx_ = 0;       // index of the current animation frame
	u32 texFramesCount_ = 0;        // how many animation frames we have
	u32 texColumns_ = 0;            // vertical cells (frames) count
	u32 texRows_ = 0;               // horizontal cells (frames) count

	float animDuration_ = 0;        // duration of the whole animation
	float texCellWidth_ = 0;        // width of one animation frame
	float texCellHeight_ = 0;       // height of one animation frame
};

///////////////////////////////////////////////////////////

struct TexAtlasAnimations
{
	// TYPE: ATLAS_ANIMATION
	std::vector<EntityID> ids_;
	std::vector<float> timeSteps_;               // duration of one animation frame; after this time point we change an animation frame
	std::vector<float> currAnimTime_;           // frame time value in [0, timeStep]; when this val >= timeStep we change a frame
	std::vector<TexAtlasAnimationData> data_;
};

///////////////////////////////////////////////////////////

struct TexRotationsAroundCoords
{
	// TYPE: ROTATION_AROUND_TEX_COORD
	std::vector<EntityID> ids_;
	std::vector<XMFLOAT2> texCoords_;
	std::vector<float>    rotationsSpeed_;
};

}