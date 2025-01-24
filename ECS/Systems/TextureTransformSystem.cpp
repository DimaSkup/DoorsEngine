// ********************************************************************************
// Filename:      TextureTransformSystem.cpp
// Description:   implementation of the TextureTransformSystem functional
// 
// Created:       29.06.24
// ********************************************************************************
#include "TextureTransformSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"
#include "../Common/Utils.h"

#include <fstream>    // for serialization / deserialization of data

using namespace Utils;
using namespace DirectX;

namespace ECS
{


// *********************************************************************************
// 
//                               PUBLIC METHODS
// 
// *********************************************************************************


TextureTransformSystem::TextureTransformSystem(TextureTransform* pTexTransformComp)
{
	Assert::NotNullptr(pTexTransformComp, "input ptr to the texture transform component == nullptr");
	pTexTransformComponent_ = pTexTransformComp;
}

// --------------------------------------------------------

void TextureTransformSystem::AddTexTransformation(
	const TexTransformType type, 
	const std::vector<EntityID>& ids, 
	const TexTransformInitParams& inParams)
{
	Assert::NotEmpty(ids.empty(), "the array of entities IDs is empty");

	if (type == STATIC)
	{
		AddStaticTexTransform(ids, inParams);
	}
	else if (type == ATLAS_ANIMATION)
	{
		AddAtlasTextureAnimation(ids, inParams);
	}
	else if (type == ROTATION_AROUND_TEX_COORD)
	{
		AddRotationAroundTexCoord(ids, inParams);
	}
}

// --------------------------------------------------------

void TextureTransformSystem::GetTexTransformsForEntts(
	const std::vector<EntityID>& ids,
	std::vector<XMMATRIX>& outTexTransforms)
{
	// what we do here:
	// go through each input entity and define if it has some tex transformation 
	// if so we store this transformation or in another case we store an identity matrix;
	//
	// in:    arr of entities IDs
	// out:   arr of texture transformations for these entities

	

	const TextureTransform& comp = *pTexTransformComponent_;
	std::vector<bool> flags;
	std::vector<ptrdiff_t> idxs;

	// get arr of flags which will be used to define if entt has texture transformation
	GetExistingFlags(comp.ids_, ids, flags);

	// fill in the output arr with default values
	outTexTransforms.resize(std::ssize(ids), DirectX::XMMatrixIdentity());

	GetIdxsInSortedArr(comp.ids_, ids, idxs);

	for (u32 idx = 0; idx < std::ssize(ids); ++idx)
		outTexTransforms[idx] = (flags[idx]) ? comp.texTransforms_[idxs[idx]] : DirectX::XMMatrixIdentity();
}

// --------------------------------------------------------

void TextureTransformSystem::UpdateAllTextrureAnimations(
	const float totalGameTime,
	const float deltaTime)
{
	UpdateTextureStaticTransformation(deltaTime);
	UpdateTextureAtlasAnimations(deltaTime);
	UpdateTextureRotations(totalGameTime);
}




// *********************************************************************************
// 
//                               PRIVATE METHODS
// 
// *********************************************************************************

bool TextureTransformSystem::CheckCanAddRecords(const std::vector<EntityID>& ids)
{
	// check if we can add records by IDs
	bool canAddRecords = true;
	std::vector<bool> flags;

	GetExistingFlags(pTexTransformComponent_->ids_, ids, flags);

	for (bool exist : flags)
		canAddRecords &= (!exist);

	return canAddRecords;
}

// --------------------------------------------------------

void TextureTransformSystem::AddStaticTexTransform(
	const std::vector<EntityID>& ids,
	const TexTransformInitParams& inParams)
{
	// add a STATIC (it won't move) texture transformation to each input entity

	Assert::True(CheckCanAddRecords(ids), "there is already a record with some input entity ID");

	TextureTransform& comp = *pTexTransformComponent_;
	TexStaticTransformations& staticTransf = comp.texStaticTrans_;
	const StaticTexTransParams& params = static_cast<const StaticTexTransParams&>(inParams);

	// execute sorted insertion of new records into the data arrays
	for (u32 idx = 0; const EntityID& id : ids)
	{
		const ptrdiff_t pos = GetPosForID(comp.ids_, id);

		InsertAtPos(comp.ids_, pos, id);
		InsertAtPos(comp.transformTypes_, pos, TexTransformType::STATIC);
		InsertAtPos(comp.texTransforms_, pos, params.initTransform_[idx]);
	
		// setup specific data for this kind of texture transformation
		const ptrdiff_t data_idx = GetPosForID(staticTransf.ids_, id);

		InsertAtPos(staticTransf.ids_, data_idx, id);
		InsertAtPos(staticTransf.transformations_, data_idx, params.texTransforms_[idx]);

		++idx;
	}
}

// --------------------------------------------------------

void TextureTransformSystem::AddAtlasTextureAnimation(
	const std::vector<EntityID>& ids,
	const TexTransformInitParams& inParams)
{
	// make a texture animation for entity by ID;
	//
	// if texRows or texColumns > 0 (or both) we separate a texture into frames 
	// (it's supposed that we use an atlas texture getting it from the Textured component);
	// and go through these frames during the time so we make a texture animation;

	Assert::True(CheckCanAddRecords(ids), "there is already a record with some input entity ID");

	TextureTransform& comp = *pTexTransformComponent_;
	const AtlasAnimParams& params = static_cast<const AtlasAnimParams&>(inParams);

	// execute sorted insertion into the data arrays
	for (u32 idx = 0; const EntityID & id : ids)
	{
		const ptrdiff_t pos = GetPosForID(comp.ids_, id);

		// add common data
		InsertAtPos(comp.ids_, pos, id);
		InsertAtPos(comp.transformTypes_, pos, TexTransformType::ATLAS_ANIMATION);

		// add specific data according to this texture transformation type
		const ptrdiff_t animIdx = AddAtlasAnimationData(
			id, 
			params.texRows_[idx],
			params.texColumns_[idx],
			params.animDurations_[idx]);

		// setup the texture transformation to the first animation frame
		// (we just scale a texture and setup position to the top left corner)
		TexAtlasAnimationData& animData = comp.texAtlasAnim_.data_[animIdx];

		InsertAtPos(comp.texTransforms_, pos, XMMatrixTranspose(
			XMMatrixScaling(animData.texCellWidth_, animData.texCellHeight_, 0)));

		++idx;
	}
}

// --------------------------------------------------------

void TextureTransformSystem::AddRotationAroundTexCoord(
	const std::vector<EntityID>& ids,
	const TexTransformInitParams& inParams)
{
	// add rotation around particular texture coordinate for input entity
	// (for instance: p(0.5, 0.5) - rotation arount its center)
	// 
	// input: tu,tv         -- texture coords
	//        rotationSpeed -- how fast the texture will rotate

	Assert::True(CheckCanAddRecords(ids), "there is already a record with some input entity ID");

	TextureTransform& comp = *pTexTransformComponent_;
	TexRotationsAroundCoords& rotations = comp.texRotations_;
	const RotationAroundCoordParams& params = static_cast<const RotationAroundCoordParams&>(inParams);

	// if there is no records with this ID yet
	for (u32 idx = 0; const EntityID & id : ids)
	{
		// execute sorted insertion into the common data arrays
		
		const ptrdiff_t comp_idx = GetPosForID(comp.ids_, id);

		InsertAtPos(comp.ids_, comp_idx, id);
		InsertAtPos(comp.transformTypes_, comp_idx, TexTransformType::ROTATION_AROUND_TEX_COORD);
		InsertAtPos(comp.texTransforms_, comp_idx, DirectX::XMMatrixIdentity());  // current texture transformation

		// setup specific data
		const ptrdiff_t data_idx = GetPosForID(rotations.ids_, id);

		InsertAtPos(rotations.ids_, data_idx, id);
		InsertAtPos(rotations.texCoords_, data_idx, params.rotationsCenter_[idx]);
		InsertAtPos(rotations.rotationsSpeed_, data_idx, params.rotationsSpeed_[idx]);

		++idx;
	}
}

// --------------------------------------------------------

const ptrdiff_t TextureTransformSystem::AddAtlasAnimationData(
	const EntityID id,
	const u32 texRows,
	const u32 texColumns,
	const float animDuration)
{
	// add new texture atlas animation for entity by ID;
	// in:      params.animDuration -- duration of the whole animation from the first to the last frame
	//          params.texRows      -- count of animation frames in one texture row
	//          params.texColumns   -- count of animation frames in one texture column
	// return:  idx into array of animations in the TextureTransform component

	assert(((bool)texRows & (bool)texColumns) && (animDuration > 0));

	TexAtlasAnimations& anim = pTexTransformComponent_->texAtlasAnim_;

	const ptrdiff_t animIdx = GetPosForID(anim.ids_, id);
	InsertAtPos(anim.ids_, animIdx, id);

	// frame_duration = full_anim_duration / frames_count
	InsertAtPos(anim.timeSteps_, animIdx, animDuration / (texRows * texColumns));
	InsertAtPos(anim.currAnimTime_, animIdx, 0.0f);

	// compute and store animation frames data
	InsertAtPos(anim.data_, animIdx, TexAtlasAnimationData(texRows, texColumns, animDuration));

	return animIdx;
}

// --------------------------------------------------------

void TextureTransformSystem::UpdateTextureStaticTransformation(const float deltaTime)
{
	TextureTransform& comp = *pTexTransformComponent_;
	std::vector<index> idxs;

	GetDataIdxsOfIDs(comp.ids_, comp.texStaticTrans_.ids_, idxs);

	for (u32 idx = 0; const index transIdx : idxs)
	{
		XMMATRIX& curTransformation = comp.texTransforms_[transIdx];
		XMMATRIX& transformation = comp.texStaticTrans_.transformations_[idx++];

		// translate the texture
		curTransformation.r[3] += DirectX::XMVectorScale(transformation.r[3], deltaTime);
	}
}

// --------------------------------------------------------

void TextureTransformSystem::UpdateTextureAtlasAnimations(const float deltaTime)
{
	// here we update animation time for each texture atlas animation;
	// if some animation frame must be changed for particular animation we compute
	// a new texture transformation matrix for it

	std::vector<u32> idxsOfAnimToUpdate;             // idxs of animations where a frame must be changed
	std::vector<EntityID> enttsToUpdate;                // entts IDs whose texture animation was updated
	std::vector<XMMATRIX> texTransToUpdate;           // computed tex transformations for new animation frames
	std::vector<ptrdiff_t> transformsIdxs;              // idxs of transformations which will be updated

	TextureTransform& comp = *pTexTransformComponent_;
	TexAtlasAnimations& anim = comp.texAtlasAnim_;

	// update animation time of each animation and define if its frame must be changed
	for (u32 idx = 0; idx < anim.ids_.size(); ++idx)
	{
		anim.currAnimTime_[idx] += deltaTime;

		if (anim.currAnimTime_[idx] > anim.timeSteps_[idx])
		{
			anim.currAnimTime_[idx] = 0;          // reset the current animation time
			idxsOfAnimToUpdate.push_back(idx);    // and store its index
		}
	}

	// prepare memory for some data
	const size animsToUpdateCount = std::ssize(idxsOfAnimToUpdate);
	texTransToUpdate.reserve(animsToUpdateCount);
	enttsToUpdate.reserve(animsToUpdateCount);

	// update each chosen animation
	for (const u32 animIdx : idxsOfAnimToUpdate)
	{
		TexAtlasAnimationData& data = anim.data_[animIdx];

		// change animation frame
		++data.currTexFrameIdx_;

		// do we need to restart animation?
		if (data.currTexFrameIdx_ >= data.texFramesCount_)
			data.currTexFrameIdx_ = 0;

		// compute frame row and column at the texture (atlas texture)
		const u32 col_idx = data.currTexFrameIdx_ % data.texColumns_;
		const u32 row_idx = data.currTexFrameIdx_ / data.texColumns_;

		const float m00 = data.texCellWidth_;
		const float m11 = data.texCellHeight_;

		// store transformation matrix to use it later for update
		texTransToUpdate.emplace_back(
			m00, 0.0f, 0.0f, 0.0f,                     // m00, m01, m02, m03
			0.0f, m11, 0.0f, 0.0f,                     // m10, m11, m12, m13
			0.0f, 0.0f, 0.0f, 0.0f,                    // m20, m21, m22, m23
			m00 * col_idx, m11 * row_idx, 0.0f, 0.0f); // m30, m31, m32, m33
	}

	// get entities IDs whose animations will be updated
	for (const u32 animIdx : idxsOfAnimToUpdate)
		enttsToUpdate.push_back(anim.ids_[animIdx]);

	// get data idxs of transformations to update and apply new values by these idxs
	GetDataIdxsOfIDs(comp.ids_, enttsToUpdate, transformsIdxs);
	ApplyTexTransformsByIdxs(transformsIdxs, texTransToUpdate);
}

// --------------------------------------------------------

void TextureTransformSystem::UpdateTextureRotations(const float totalGameTime)
{
	// update all the texture rotations around responsible texture coords 
	// according to the time

	TextureTransform& comp = *pTexTransformComponent_;
	TexRotationsAroundCoords& rotations = comp.texRotations_;
	std::vector<ptrdiff_t> transformsIdxs;                    // apply new data by these idxs
	std::vector<XMMATRIX> texTransToUpdate;                   // new texture transformations
	const size texRotCount = std::ssize(rotations.ids_);

	texTransToUpdate.reserve(rotations.ids_.size());

	// compute and store new rotation transformations
	for (u32 idx = 0; idx < texRotCount; ++idx)
	{
		const XMFLOAT2& texCoord = rotations.texCoords_[idx];

		texTransToUpdate.emplace_back(
			DirectX::XMMatrixTranslation(-texCoord.x, -texCoord.y, 0) *    // translate to the rotation center
			DirectX::XMMatrixRotationZ(rotations.rotationsSpeed_[idx] * totalGameTime) *    // rotate by this angle
			DirectX::XMMatrixTranslation(texCoord.x, texCoord.y, 0)        // translate back to the origin pos
		);
	}

	// get data idxs of transformations to update and apply new values by these idxs
	GetDataIdxsOfIDs(comp.ids_, rotations.ids_, transformsIdxs);
	ApplyTexTransformsByIdxs(transformsIdxs, texTransToUpdate);
}

// --------------------------------------------------------

void TextureTransformSystem::GetDataIdxsOfIDs(
	const std::vector<EntityID>& allEnttsIDs,
	const std::vector<EntityID>& searchedEnttsIDs,
	std::vector<ptrdiff_t>& outDataIdxs)
{
	// here we get data idx of ID in the examined array;
	//
	// input:   1. examined array of IDs
	//          2. array of searched IDs 
	// output:     array of data idxs

	outDataIdxs.reserve(searchedEnttsIDs.size());

	for (const EntityID id : searchedEnttsIDs)
		outDataIdxs.push_back(GetIdxInSortedArr(allEnttsIDs, id));
}

// --------------------------------------------------------

void TextureTransformSystem::ApplyTexTransformsByIdxs(
	const std::vector<ptrdiff_t>& dataIdxs,
	const std::vector<XMMATRIX>& texTransToUpdate)
{
	// here apply new texture transformations by input data indices

	TextureTransform& comp = *pTexTransformComponent_;

	// pay attention that we transpose each matrix so we don't need to do it
	// each frame within the rendering shaders
	for (ptrdiff_t data_idx = 0; const ptrdiff_t idx : dataIdxs)
		comp.texTransforms_[idx] = texTransToUpdate[data_idx++];
}

}