// =================================================================================
// Filename:     RenderStatesSystem.cpp
// Description:  implementation of the RenderStatesSystem functional
// 
// Created:      29.08.24
// =================================================================================
#include "RenderStatesSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"


namespace ECS
{
	
RenderStatesSystem::RenderStatesSystem(RenderStates* pRenderStatesComponent)
{
	Assert::NotNullptr(pRenderStatesComponent, "a ptr to the component == nullptr");
	pRSComponent_ = pRenderStatesComponent;

#if 0
	const cvector<eRenderState> notDefaultRasterStates =
	{
		FILL_WIREFRAME, CULL_FRONT,	CULL_NONE,
	};
#endif

	const cvector<eRenderState> blendingStates =
	{
		ALPHA_ENABLE, ADDING, SUBTRACTING, MULTIPLYING,	TRANSPARENCY,
	};


	// hash masks to filter entts by main states sets
	GetHashByStates(g_DefaultStates, defaultRSMask_);
	GetHashByStates(g_AlphaClipCullNoneStates, alphaClipCullNoneMask_);

	// make a hash mask to get entts: default + blending
	GetHashByStates(blendingStates, blendingRSMask_);

	// make a map of pairs ['bs_hash' => 'bs_state']
	for (const eRenderState bs : blendingStates)
		hashesToBS_.insert({ (1 << bs), bs });

	MakeDisablingMasks();
}

///////////////////////////////////////////////////////////

RenderStatesSystem::~RenderStatesSystem()
{
	pRSComponent_ = nullptr;
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::AddWithDefaultStates(const EntityID* ids, const size numEntts)
{
	// add new records only with ids which aren't exist in the component yet;
	// and apply the same default set of states to each of these new entts;

    Assert::True((ids != nullptr) && (numEntts > 0), "invalid input args");

	RenderStates& comp = *pRSComponent_;
	const u32 hash = defaultRSMask_;
	cvector<EntityID> newIds;

    // filter entts
	GetNewEntts(ids, numEntts, newIds);

    const size numToAdd = newIds.size();

    cvector<index> idxs;
    comp.ids_.get_insert_idxs(newIds, idxs);

    // execute sorted insertion of IDs
    for (index i = 0; i < numToAdd; ++i)
        comp.ids_.insert_before(idxs[i] + i, newIds[i]);

    // execute storing of hashes according to related IDs
    for (index i = 0; i < numToAdd; ++i)
        comp.statesHashes_.insert_before(idxs[i] + i, hash);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateStates(const EntityID id, const eRenderState state)
{
	UpdateStates(cvector<EntityID>{id}, cvector<eRenderState>{state});
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateStates(
	const EntityID id,
	const cvector<eRenderState>& states)
{
	UpdateStates(cvector<EntityID>{id}, states);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateStates(
	const cvector<EntityID>& ids,
	const cvector<eRenderState>& states)
{
	// update each input entt with the same set of states
	UpdateRecords(ids, states);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::SeparateEnttsByRenderStates(
	const cvector<EntityID>& ids,
	EnttsRenderStatesData& outData)
{
	const RenderStates& comp = *pRSComponent_;
	const size idsCount = std::ssize(ids);

	cvector<u32> hashes(idsCount);
	cvector<index> idxs;

    comp.ids_.get_idxs(ids, idxs);

	// get render states hashes by idxs
	for (int i = 0; const index idx : idxs)
		hashes[i++] = comp.statesHashes_[idx];

	GetEnttsWithDefaultRS(ids, hashes, outData.enttsDefault_);
	GetEnttsWithAlphaClipCullNone(ids, hashes, outData.enttsAlphaClipping_);
	GetEnttsBlended(ids, hashes, outData.enttsBlended_);
}


// ================================================================================= 
//                               PRIVATE METHODS
// =================================================================================
void RenderStatesSystem::GetNewEntts(
	const EntityID* ids,
    const size numEntts,
	cvector<EntityID>& newIds)
{
	// out: ids of entts which aren't stored in the component yet

	const RenderStates& comp = *pRSComponent_;
	cvector<bool> exists(numEntts, false);

    for (index i = 0; i < numEntts; ++i)
        exists[i] |= comp.ids_.binary_search(ids[i]);

	newIds.reserve(numEntts);

	// if entt exists in the component we store its ID
	for (int i = 0; const bool exist : exists)
		if (!exist)
			newIds.push_back(ids[i++]);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetHashByStates(
	const cvector<eRenderState>& states,
	u32& outHash)
{
	// generate a hash by input arr of states
	for (const eRenderState state : states)
		outHash |= (1 << state);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetStatesByHash(
	const u32 hash,
	cvector<eRenderState>& outStates)
{
	// FOR DEBUG: get states by hash

	for (int i = 0; i < (int)eRenderState::LAST_RS_TYPE; ++i)
	{
		if (hash & (1 << i))
			outStates.push_back(eRenderState(i));
	}
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::MakeDisablingMasks()
{
	// these hash masks are used to reset all render states of particular type
	// (for instance: disable all cull mode flags)

	for (const eRenderState fillMode : allFillModes_)
		disableAllFillModesMask_ &= ~(1 << fillMode);

	for (const eRenderState cullMode : allCullModes_)
		disableAllCullModesMask_ &= ~(1 << cullMode);

	for (const eRenderState alphaClippingState : allAlphaClipping_)
		disableAllAlphaClippingMask_ &= ~(1 << alphaClippingState);

	for (const eRenderState blendingState : allBS_)
		disableAllBlendingMask_ &= ~(1 << blendingState);

	for (const eRenderState reflectionState : allReflections_)
		disableAllReflectionMask_ &= ~(1 << reflectionState);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetIdsByIdxs(
	const cvector<ptrdiff_t>& idxs,
	cvector<EntityID>& outIds)
{
	// get entts ids from the component by input idxs
	outIds.resize(std::ssize(idxs));

	for (int i = 0; const index idx : idxs)
		outIds[i++] = pRSComponent_->ids_[idx];
}

///////////////////////////////////////////////////////////

void GetEnttsWithHash(
	const cvector<EntityID>& ids,
	const cvector<u32>& rsHashes,
	const u32 hashMask,
	cvector<EntityID>& outIds)
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
	const cvector<EntityID>& ids,
	const cvector<u32>& hashes,
	EnttsDefaultState& outData)
{
	// get entts which have only default render states
	GetEnttsWithHash(ids, hashes, defaultRSMask_, outData.ids_);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetEnttsWithAlphaClipCullNone(
	const cvector<EntityID>& ids,
	const cvector<u32>& hashes,
	EnttsAlphaClipping& outData)
{
	// get entts which have the following specific render states:
	// alpha clipping and cull none
	GetEnttsWithHash(ids, hashes, alphaClipCullNoneMask_, outData.ids_);
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::FilterEnttsOnlyBlending(
	const cvector<EntityID>& ids,
	const cvector<u32>& hashes,
	cvector<EntityID>& enttWithBS,
	cvector<u32>& hashesWithBS)
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
	const cvector<u32>& hashes,
	cvector<eRenderState>& blendStates)
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
                sprintf(g_String, "unknown blending state hash: %ud", hashes[i]);
				LogErr(g_String);
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::GetEnttsBlended(
	const cvector<EntityID>& ids,
	const cvector<u32>& hashes,
	EnttsBlended& outData)
{
	cvector<EntityID> enttWithBS;
	cvector<u32> hashesWithBS;
	cvector<eRenderState> bsCodes;
	std::map<eRenderState, cvector<EntityID>> bsToIdxs;
	

	FilterEnttsOnlyBlending(ids, hashes, enttWithBS, hashesWithBS);
	GetBlendingStatesByHashes(hashesWithBS, bsCodes);

	// separate entts by blending states
	for (int i = 0; const eRenderState bs : bsCodes)
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
    for (int i = 0; const auto & it : bsToIdxs)
        outData.ids_.append_vector(it.second);

	// store instances count per blending state
	for (int i = 0; const auto& it : bsToIdxs)
		outData.instanceCountPerBS_[i++] = it.second.size();

	// store blending states
	for (int i = 0; const auto & it : bsToIdxs)
		outData.states_[i++] = it.first;
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateRecords(
	const cvector<EntityID>& ids,
	const cvector<eRenderState>& states)
{
	// update records by ids with new render states values

	RenderStates& comp = *pRSComponent_;
	cvector<u32> rsHashes(std::ssize(ids));
    cvector<index> idxs;

    comp.ids_.get_idxs(ids, idxs);

	// get render states hashes related to the input entts
	for (int i = 0; const index idx : idxs)
		rsHashes[i++] = comp.statesHashes_[idx];

	UpdateRenderStatesForHashes(states, rsHashes);

	// store updated render states hashes by idxs
	for (int i = 0; const index idx : idxs)
		comp.statesHashes_[idx] = rsHashes[i++];
}

///////////////////////////////////////////////////////////

void RenderStatesSystem::UpdateRenderStatesForHashes(
	const cvector<eRenderState>& states,
	cvector<u32>& hashes)
{
	// go through each hash and update its values according to input render states;

	for (const eRenderState state : states)
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
                sprintf(g_String, "unknown render state: %d", state);
                LogErr(g_String);
			}
			} // switch
		} // for
	} // for
}

}
