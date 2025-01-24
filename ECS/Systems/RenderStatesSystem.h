// *********************************************************************************
// Filename:     RenderStatesSystem.h
// Description:  an ECS system which is responsible for
//               handling blending of the entities;
//
// Created:      28.08.24
// *********************************************************************************
#pragma once

#include "../Components/RenderStates.h"
#include <set>
#include <map>

namespace ECS
{



class RenderStatesSystem final
{
	using RenderStatesTypesSet = std::set<RSTypes>;

public:

	struct EnttsRenderStatesData
	{
		// is used to get entts render states data for rendering

		EnttsDefaultState  enttsDefault_;
		EnttsAlphaClipping enttsAlphaClipping_;
		EnttsBlended       enttsBlended_;
		EnttsReflection    enttsReflection_;
		
		void Clear()
		{
			enttsDefault_.Clear();
			enttsAlphaClipping_.Clear();
			enttsBlended_.Clear();
			enttsReflection_.Clear();
		}
	};

public:
	RenderStatesSystem(RenderStates* pRenderStatesComponent);
	~RenderStatesSystem();

	// restrict a copying of this class instance 
	RenderStatesSystem(const RenderStatesSystem& obj) = delete;
	RenderStatesSystem& operator=(const RenderStatesSystem& obj) = delete;

	// ---------------------------------------------

	void AddWithDefaultStates(const std::vector<EntityID>& ids);

	// update:
	// 1. one state of single entt
	// 2. multiple states of single entt
	// 3. multiple states of multiple entts
	void UpdateStates(const EntityID id, const RSTypes state);
	void UpdateStates(const EntityID id, const std::vector<RSTypes>& states);
	void UpdateStates(const std::vector<EntityID>& ids,	const std::vector<RSTypes>& states);

	void SeparateEnttsByRenderStates(
		const std::vector<EntityID>& ids,
		EnttsRenderStatesData& outData);

	inline void ChangeRenderStateForHash(u32& hash, const RSTypes newState, const u32 disablingMask)
	{
		hash &= disablingMask;    // disable all the other related render states (for instance: disable all blending states)
		hash |= (1 << newState);  // enable some render state
	}

private:

	void GetNewEntts(const std::vector<EntityID>& ids, std::vector<EntityID>& newIds);

	void GetStatesByHash(const u32 hash, std::vector<RSTypes>& outStates);
	void GetHashByStates(const std::vector<RSTypes>& states, u32& outHash);

	void MakeDisablingMasks();

	void GetIdsByIdxs(
		const std::vector<ptrdiff_t>& idxs,
		std::vector<EntityID>& outIds);

	void GetEnttsWithDefaultRS(
		const std::vector<EntityID>& ids,
		const std::vector<u32>& hashes,
		EnttsDefaultState& outData);

	void GetEnttsWithAlphaClipCullNone(
		const std::vector<EntityID>& ids,
		const std::vector<u32>& hashes,
		EnttsAlphaClipping& outData);

	void FilterEnttsOnlyBlending(
		const std::vector<EntityID>& ids,
		const std::vector<u32>& hashes,
		std::vector<EntityID>& enttWithBS,
		std::vector<u32>& hashesWithBS);

	void GetBlendingStatesByHashes(
		const std::vector<u32>& hashes,
		std::vector<RSTypes>& blendStates);

	void GetEnttsBlended(
		const std::vector<EntityID>& ids,
		const std::vector<u32>& hashes,
		EnttsBlended& outData);

	void UpdateRecords(
		const std::vector<EntityID>& ids,
		const std::vector<RSTypes>& states);

	void UpdateRenderStatesForHashes(
		const std::vector<RSTypes>& states,
		std::vector<u32>& hashes);

#if 0

	void GetEnttsByStates(
		const std::vector<ptrdiff_t>& idxsToEntts,
		const std::vector<RSTypes>& states,
		std::vector<EntityID>& outEnttsWithStates,
		std::vector<ptrdiff_t>& outIdxsToEnttsWithoutInputStates);

	void GetRenderStatesTypesByDataIdxs(
		const std::vector<ptrdiff_t>& idxsToEnttsWithSpecificRS,
		EnttsRenderStatesData& outData);
#endif
	


private:


	RenderStates* pRSComponent_ = nullptr;

	// all possible fill/cull/alpha clipping states
	RSTypes allFillModes_[2]     = { FILL_SOLID, FILL_WIREFRAME };
	RSTypes allCullModes_[3]     = { CULL_BACK, CULL_FRONT, CULL_NONE };
	RSTypes allAlphaClipping_[2] = { NO_ALPHA_CLIPPING, ALPHA_CLIPPING };
	RSTypes allReflections_[2]   = { REFLECTION_PLANE, NOT_REFLECTION_PLANE };

	// all possible blending states (BS)
	RSTypes allBS_[7] =
	{
		NO_RENDER_TARGET_WRITES, NO_BLENDING, ALPHA_ENABLE,
		ADDING,	SUBTRACTING, MULTIPLYING, TRANSPARENCY,
	};


	// by these hash masks we get entts to render
	u32 defaultRSMask_ = 0;                          // default render states hash mask
	u32 alphaClipCullNoneMask_ = 0;                  
	u32 blendingRSMask_ = 0;                         // blending render states hash mask

	// a specific hash to define if entt has any not default render state
	//u32 specRenderStatesTypes_ = 0;

	//u32 specRasterStates_ = 0;                  // to check if entt has any specific raster state except of fill_solid / cull_back
	u32 blendStatesMask_ = 0;                   // to check if entt has any blend state
	//u32 alphaClippingMask_ = 0;                 // to check if entt has alpha clipping

	u32 disableAllFillModesMask_     = UINT32_MAX;
	u32 disableAllCullModesMask_     = UINT32_MAX;
	u32 disableAllAlphaClippingMask_ = UINT32_MAX;
	u32 disableAllBlendingMask_      = UINT32_MAX;
	u32 disableAllReflectionMask_ = UINT32_MAX;
	
	std::map<u32, RSTypes> hashesToRS_;   // hashes to RASTER states
	std::map<u32, RSTypes> hashesToBS_;   // hashes to BLENDING states
};


}  // namespace ECS