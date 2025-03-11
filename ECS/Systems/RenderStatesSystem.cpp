// *********************************************************************************
// Filename:     RenderStatesSystem.cpp
// Description:  implementation of the RenderStatesSystem functional
// 
// Created:      29.08.24
// *********************************************************************************
#include "RenderStatesSystem.h"

#include "../Common/Assert.h"
#include "../Common/Utils.h"
#include "../Common/log.h"

namespace ECS
{
	
RenderStatesSystem::RenderStatesSystem(RenderStates* pRenderStatesComponent)
{
	Assert::NotNullptr(pRenderStatesComponent, "a ptr to the component == nullptr");
	pRSComponent_ = pRenderStatesComponent;

	const std::vector<RSTypes> notDefaultRasterStates =
	{
		FILL_WIREFRAME, CULL_FRONT,	CULL_NONE,
	};

	const std::vector<RSTypes> blendingStates =
	{
		ALPHA_ENABLE, ADDING, SUBTRACTING, MULTIPLYING,	TRANSPARENCY,
	};


	// hash masks to filter entts by main states sets
	GetHashByStates(g_DefaultStates, defaultRSMask_);
	GetHashByStates(g_AlphaClipCullNoneStates, alphaClipCullNoneMask_);

	// make a hash mask to get entts: default + blending
	//blendingRSMask_ = defaultRSMask_;
	//blendingRSMask_ &= ~(1 << NO_BLENDING);
	GetHashByStates(blendingStates, blendingRSMask_);

	// hash for all not default raster states
	//GetHashByStates(notDefaultRasterStates, specRasterStates_);

	// we'll use this hash to get entts which must be blended
	//GetHashByStates(blendingStates, blendStatesMask_);

	// by this hash we define if entt has an alpha clipping state
	//alphaClippingMask_ |= (1 << ALPHA_CLIPPING);

	// define a hash which will be used to get entts with specific render states
	//specRenderStatesTypes_ |= ~defaultRSMask_;

	// ---------------------------------------------
	
	// make a map of pairs ['rs_hash' => 'rs_state']
	//for (const RSTypes rs : rasterStates)
	//	hashesToRS_.insert({ (1 << rs), rs });

	// make a map of pairs ['bs_hash' => 'bs_state']
	for (const RSTypes bs : blendingStates)
		hashesToBS_.insert({ (1 << bs), bs });

	MakeDisablingMasks();
}

///////////////////////////////////////////////////////////

RenderStatesSystem::~RenderStatesSystem()
{
	pRSComponent_ = nullptr;
}



///////////////////////////////////////////////////////////

void RenderStatesSystem::AddWithDefaultStates(const std::vector<EntityID>& ids)
{
	// add new records only with ids which aren't exist in the component yet;
	// and apply the same default set of states to each of these new entts;

	RenderStates& comp = *pRSComponent_;
	u32 hash = defaultRSMask_;
	std::vector<EntityID> newIds;

	GetNewEntts(ids, newIds);

	// add records: id => default_rs_hash
	for (const EntityID id : ids)
	{
		const ptrdiff_t pos = Utils::GetPosForID(comp.ids_, id);

		Utils::InsertAtPos(comp.ids_, pos, id);
		Utils::InsertAtPos(comp.statesHashes_, pos, hash);
	}
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateStates(const EntityID id, const RSTypes state)
{
	UpdateStates(std::vector<EntityID>{id}, std::vector<RSTypes>{state});
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateStates(
	const EntityID id,
	const std::vector<RSTypes>& states)
{
	UpdateStates(std::vector<EntityID>{id}, states);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateStates(
	const std::vector<EntityID>& ids,
	const std::vector<RSTypes>& states)
{
	// update each input entt with the same set of states;
	// 
	// NOTE: if some ids don't exist in the component we firstly add new records
	//       with default states values for these entt and then update them

	RenderStates& comp = *pRSComponent_;
	std::vector<EntityID> newIds;

	// add records with new ids and default render states
	GetNewEntts(ids, newIds);
	AddWithDefaultStates(newIds);
	newIds.clear();

	// update render states of each input entt 
	UpdateRecords(ids, states);
}





///////////////////////////////////////////////////////////

void RenderStatesSystem::SeparateEnttsByRenderStates(
	const std::vector<EntityID>& ids,
	EnttsRenderStatesData& outData)
{
	const RenderStates& comp = *pRSComponent_;
	const size idsCount = std::ssize(ids);

	std::vector<u32> hashes(idsCount);
	std::vector<ptrdiff_t> idxs;
	
	Utils::GetIdxsInSortedArr(comp.ids_, ids, idxs);

	// get render states hashes by idxs
	for (int i = 0; const ptrdiff_t idx : idxs)
		hashes[i++] = comp.statesHashes_[idx];

	GetEnttsWithDefaultRS(ids, hashes, outData.enttsDefault_);
	GetEnttsWithAlphaClipCullNone(ids, hashes, outData.enttsAlphaClipping_);
	GetEnttsBlended(ids, hashes, outData.enttsBlended_);

	//GetEnttsReflection(idxs, hashes, outData);
}

///////////////////////////////////////////////////////////
#if 0
void RenderStatesSystem::GetEnttsByStates(
	const std::vector<ptrdiff_t>& idxsToEntts,
	const std::vector<RSTypes>& states,
	std::vector<EntityID>& outEnttsWithStates,
	std::vector<ptrdiff_t>& outIdxsToOther)     // to other entts that don't fit to the input states
{
	// out: 1. arr of entts ids which have such set of states
	//      2. arr of idxs to entts which don't have such set of states


	// arrays of idxs to entts
	std::vector<ptrdiff_t> idxsToFit;
	std::vector<std::vector<ptrdiff_t>*> ptrsToOutArrs = { &outIdxsToOther, &idxsToFit };

	// make a hash mask by input states
	u32 hashMask = 0;

	for (const RSTypes rs : states)
		hashMask |= (1 << rs);

	// go through each input idx and filter the ones that fit to the input set of states
	for (const ptrdiff_t idx : idxsToEntts)
	{
		// has == 0 -> idxsToOther
		// has == 1 -> idxsOnlyWithAlphaClipping
		bool fit = (pRSComponent_->statesHashes_[idx] == hashMask);
		ptrsToOutArrs[fit]->push_back(idx);
	}

	// get ids to entts which fit the input set of states
	GetIdsByIdxs(idxsToFit, outEnttsWithStates);
}
#endif




// ***********************************************************************************
// 
//                               PRIVATE METHODS
// 
// ***********************************************************************************

void RenderStatesSystem::GetNewEntts(
	const std::vector<EntityID>& ids,
	std::vector<EntityID>& newIds)
{
	// out: ids of entts which aren't stored in the component yet

	const RenderStates& comp = *pRSComponent_;
	std::vector<bool> exists;

	Utils::GetExistingFlags(comp.ids_, ids, exists);
	newIds.reserve(std::size(ids));

	// if entt exists in the component we store its ID
	for (int i = 0; const bool exist : exists)
		if (!exist)
			newIds.push_back(ids[i++]);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetHashByStates(
	const std::vector<RSTypes>& states,
	u32& outHash)
{
	// generate a hash by input arr of states
	for (const RSTypes state : states)
		outHash |= (1 << state);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetStatesByHash(
	const u32 hash,
	std::vector<RSTypes>& outStates)
{
	// FOR DEBUG: get states by hash

	for (int i = 0; i < (int)RSTypes::LAST_RS_TYPE; ++i)
	{
		if (hash & (1 << i))
			outStates.push_back(RSTypes(i));
	}
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::MakeDisablingMasks()
{
	// these hash masks are used to reset all render states of particular type
	// (for instance: disable all cull mode flags)

	for (const RSTypes fillMode : allFillModes_)
		disableAllFillModesMask_ &= ~(1 << fillMode);

	for (const RSTypes cullMode : allCullModes_)
		disableAllCullModesMask_ &= ~(1 << cullMode);

	for (const RSTypes alphaClippingState : allAlphaClipping_)
		disableAllAlphaClippingMask_ &= ~(1 << alphaClippingState);

	for (const RSTypes blendingState : allBS_)
		disableAllBlendingMask_ &= ~(1 << blendingState);

	for (const RSTypes reflectionState : allReflections_)
		disableAllReflectionMask_ &= ~(1 << reflectionState);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetIdsByIdxs(
	const std::vector<ptrdiff_t>& idxs,
	std::vector<EntityID>& outIds)
{
	// get entts ids from the component by input idxs
	outIds.resize(std::ssize(idxs));

	for (int i = 0; const index idx : idxs)
		outIds[i++] = pRSComponent_->ids_[idx];
}

///////////////////////////////////////////////////////////

void GetEnttsWithHash(
	const std::vector<EntityID>& ids,
	const std::vector<u32>& rsHashes,
	const u32 hashMask,
	std::vector<EntityID>& outIds)
{
	// out: entts which have exact render states hash

	const size enttsCount = std::ssize(ids);
	int outIdIdx = 0;

	outIds.resize(enttsCount);

	for (int i = 0; i < (int)enttsCount; ++i)
	{
		outIds[outIdIdx] = ids[i];
		outIdIdx += (rsHashes[i] == hashMask);
	}

	outIds.resize(outIdIdx);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetEnttsWithDefaultRS(
	const std::vector<EntityID>& ids,
	const std::vector<u32>& hashes,
	EnttsDefaultState& outData)
{
	// get entts which have only default render states
	GetEnttsWithHash(ids, hashes, defaultRSMask_, outData.ids_);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetEnttsWithAlphaClipCullNone(
	const std::vector<EntityID>& ids,
	const std::vector<u32>& hashes,
	EnttsAlphaClipping& outData)
{
	// get entts which have the following specific render states:
	// alpha clipping and cull none
	GetEnttsWithHash(ids, hashes, alphaClipCullNoneMask_, outData.ids_);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::FilterEnttsOnlyBlending(
	const std::vector<EntityID>& ids,
	const std::vector<u32>& hashes,
	std::vector<EntityID>& enttWithBS,
	std::vector<u32>& hashesWithBS)
{
	// filter ids and hashes of entts which has blending and other default render states
	
	const int enttsCount = (int)std::ssize(ids);
	int blendedCount = 0;

	enttWithBS.resize(enttsCount);
	hashesWithBS.resize(enttsCount);

	// to check if entt has default render states (but without NO_BLENDING)
	u32 defaultHashMask = defaultRSMask_;
	defaultHashMask &= ~(1 << NO_BLENDING);

	// get entts only with blending (but all the other states are default)
	for (int i = 0; i < (int)enttsCount; ++i)
	{
		bool hasDefaultRS = (defaultHashMask == (hashes[i] & defaultHashMask));

		enttWithBS[blendedCount] = ids[i];
		hashesWithBS[blendedCount] = hashes[i] & blendingRSMask_;          // == 1 if has blending state
		blendedCount += (bool(hashesWithBS[blendedCount]) & hasDefaultRS); // if has bs and all the other default rs
	}

	enttWithBS.resize(blendedCount);
	hashesWithBS.resize(blendedCount);
}

///////////////////////////////////////////////////////////


void RenderStatesSystem::GetBlendingStatesByHashes(
	const std::vector<u32>& hashes,
	std::vector<RSTypes>& blendStates)
{
	// get blending state code for each hash

	const size hashCount = std::ssize(hashes);
	blendStates.resize(hashCount);
	
	for (int i = 0; i < hashCount; ++i)
	{
		switch (hashes[i])
		{
			case (1 << ALPHA_ENABLE):
			{
				blendStates[i] = ALPHA_ENABLE;
				break;
			}
			case (1 << ADDING):
			{
				blendStates[i] = ADDING;
				break;
			}
			case (1 << SUBTRACTING):
			{
				blendStates[i] = SUBTRACTING;
				break;
			}
			case (1 << MULTIPLYING):
			{
				blendStates[i] = MULTIPLYING;
				break;
			}
			case (1 << TRANSPARENCY):
			{
				blendStates[i] = TRANSPARENCY;
				break;
			}
			default:
			{
				Log::Error("unknown blending state hash: " + std::to_string(hashes[i]));
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetEnttsBlended(
	const std::vector<EntityID>& ids,
	const std::vector<u32>& hashes,
	EnttsBlended& outData)
{
	std::vector<EntityID> enttWithBS;
	std::vector<u32> hashesWithBS;
	std::vector<RSTypes> bsCodes;
	std::map<RSTypes, std::vector<EntityID>> bsToIdxs;
	

	FilterEnttsOnlyBlending(ids, hashes, enttWithBS, hashesWithBS);
	GetBlendingStatesByHashes(hashesWithBS, bsCodes);

	// separate entts by blending states
	for (int i = 0; const RSTypes bs : bsCodes)
		bsToIdxs[bs].push_back(enttWithBS[i++]);

	const size bsCount = std::ssize(bsToIdxs);
	outData.ids_.reserve(std::ssize(enttWithBS));
	outData.instanceCountPerBS_.resize(bsCount);
	outData.states_.resize(bsCount);

	// clear some memory since we already don't need it
	enttWithBS.clear();
	hashesWithBS.clear();
	bsCodes.clear();

	// put entts ids into one array in sorted order by blending states
	for (int i = 0; const auto& it : bsToIdxs)
		Utils::AppendToArray(outData.ids_, it.second);

	// store instances count per blending state
	for (int i = 0; const auto& it : bsToIdxs)
		outData.instanceCountPerBS_[i++] = (u32)std::ssize(it.second);

	// store blending states
	for (int i = 0; const auto & it : bsToIdxs)
		outData.states_[i++] = it.first;
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateRecords(
	const std::vector<EntityID>& ids,
	const std::vector<RSTypes>& states)
{
	// update records by ids with new render states values

	RenderStates& comp = *pRSComponent_;
	std::vector<index> idxs;
	std::vector<u32> rsHashes(std::ssize(ids));

	Utils::GetIdxsInSortedArr(comp.ids_, ids, idxs);

	// get render states hashes related to the input entts
	for (int i = 0; index idx : idxs)
		rsHashes[i++] = comp.statesHashes_[idx];

	UpdateRenderStatesForHashes(states, rsHashes);

	// store updated render states hashes by idxs
	for (int i = 0; index idx : idxs)
		comp.statesHashes_[idx] = rsHashes[i++];
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateRenderStatesForHashes(
	const std::vector<RSTypes>& states,
	std::vector<u32>& hashes)
{
	// go through each hash and update its values according to input render states;

	for (const RSTypes state : states)
	{
		for (u32& hash : hashes)
		{
			switch (state)
			{
			case FILL_SOLID:
			case FILL_WIREFRAME:
			{
				ChangeRenderStateForHash(hash, state, disableAllFillModesMask_);
				break;
			}
			case CULL_BACK:
			case CULL_FRONT:
			case CULL_NONE:
			{
				ChangeRenderStateForHash(hash, state, disableAllCullModesMask_);
				break;
			}
			case NO_RENDER_TARGET_WRITES:
			case NO_BLENDING:
			case ALPHA_ENABLE:
			case ADDING:
			case SUBTRACTING:
			case MULTIPLYING:
			case TRANSPARENCY:
			{
				ChangeRenderStateForHash(hash, state, disableAllBlendingMask_);
				break;
			}
			case NO_ALPHA_CLIPPING:
			case ALPHA_CLIPPING:
			{
				ChangeRenderStateForHash(hash, state, disableAllAlphaClippingMask_);
				break;
			}
			case REFLECTION_PLANE:
			case NOT_REFLECTION_PLANE:
			{
				ChangeRenderStateForHash(hash, state, disableAllReflectionMask_);
				break;
			}
			default:
			{
				Log::Error("unknown render state: " + std::to_string(state));
			}
			} // switch
		} // for
	} // for
}

}