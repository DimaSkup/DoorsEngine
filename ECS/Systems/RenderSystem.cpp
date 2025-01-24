// *********************************************************************************
// Filename:     RenderSystem.cpp
// Description:  implementation of the ECS RenderSystem's functional
// 
// Created:      21.05.24
// *********************************************************************************
#include "RenderSystem.h"

#include "../Common/Utils.h"
#include "../Common/UtilsFilesystem.h"
#include "../Common/log.h"
#include "../Common/Assert.h"

#include <stdexcept>
#include <fstream>

using namespace Utils;

namespace ECS
{

RenderSystem::RenderSystem(
	Rendered* pRenderComponent)
	:
	pRenderComponent_(pRenderComponent)
{
	Assert::NotNullptr(pRenderComponent, "ptr to the Rendered component == nullptr");
}


// *********************************************************************************
//                            PUBLIC FUNCTIONS
// *********************************************************************************

void RenderSystem::Serialize(std::ofstream& fout, u32& offset)
{
	// serialize all the data from the Rendered component into the data file

	// store offset of this data block so we will use it later for deserialization
	offset = static_cast<u32>(fout.tellp());

	Rendered& component = *pRenderComponent_;
	const u32 dataBlockMarker = static_cast<u32>(pRenderComponent_->type_);
	const u32 dataCount = (u32)std::ssize(component.ids_);

	// write serialized data into the file
	Utils::FileWrite(fout, &dataBlockMarker);
	Utils::FileWrite(fout, &dataCount);

	Utils::FileWrite(fout, component.ids_);
	Utils::FileWrite(fout, component.shaderTypes_);
	Utils::FileWrite(fout, component.primTopologies_);
}

/////////////////////////////////////////////////

void RenderSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
	// deserialize the data from the data file into the Rendered component
	
	// read data starting from this offset
	fin.seekg(offset, std::ios_base::beg);

	// check if we read the proper data block
	u32 dataBlockMarker = 0;
	Utils::FileRead(fin, &dataBlockMarker);

	const bool isProperDataBlock = (dataBlockMarker == static_cast<u32>(ComponentType::RenderedComponent));
	Assert::True(isProperDataBlock, "read wrong data block during deserialization of the Rendered component data from a file");

	// ------------------------------------------

	// read in how much data will we have
	u32 dataCount = 0;
	Utils::FileRead(fin, &dataCount);

	std::vector<EntityID>& ids = pRenderComponent_->ids_;
	std::vector<RenderShaderType>& shaderTypes = pRenderComponent_->shaderTypes_;
	std::vector<D3D11_PRIMITIVE_TOPOLOGY>& topologies = pRenderComponent_->primTopologies_;

	// prepare enough amount of memory for data
	ids.resize(dataCount);
	shaderTypes.resize(dataCount);
	topologies.resize(dataCount);

	Utils::FileRead(fin, ids);
	Utils::FileRead(fin, shaderTypes);
	Utils::FileRead(fin, topologies);
}

/////////////////////////////////////////////////

void RenderSystem::AddRecords(
	const std::vector<EntityID>& ids, 
	const std::vector<RenderShaderType>& shaderTypes,
	const std::vector<D3D11_PRIMITIVE_TOPOLOGY>& topologyTypes)
{
	// set that input entities by IDs will be rendered onto the screen;
	// also setup rendering params for each input entity;

	Assert::NotEmpty(ids.empty(), "the array of entities IDs is empty");
	Assert::True(CheckArrSizesEqual(ids, shaderTypes), "entities count != count of the input shaders types");
	Assert::True(CheckArrSizesEqual(ids, topologyTypes), "entities count != count of the input primitive topoECS::Logy types");

	Rendered& comp = *pRenderComponent_;

	for (int i = 0; i < (int)ids.size(); ++i)
	{
		const EntityID id = ids[i];

		// check if there is no record with such entity ID
		if (!std::binary_search(comp.ids_.begin(), comp.ids_.end(), id))
		{
			// execute sorted insertion into the data arrays
			const ptrdiff_t insertAtPos = Utils::GetPosForID(comp.ids_, id);

			Utils::InsertAtPos(comp.ids_, insertAtPos, id);
			Utils::InsertAtPos(comp.shaderTypes_, insertAtPos, shaderTypes[i]);
			Utils::InsertAtPos(comp.primTopologies_, insertAtPos, topologyTypes[i]);
		}
	}	
}

/////////////////////////////////////////////////

void RenderSystem::RemoveRecords(const std::vector<EntityID>& enttsIDs)
{
}

/////////////////////////////////////////////////

void RenderSystem::GetRenderingDataOfEntts(
	const std::vector<EntityID>& enttsIDs,
	std::vector<RenderShaderType>& outShaderTypes)
{
	// get necessary data for rendering of each curretly visible entity;
	// 
	// in:     array of entities IDs;
	// out:    shader type for each entity 

	GetShaderTypesOfEntts(enttsIDs, outShaderTypes);
}

/////////////////////////////////////////////////

void RenderSystem::ClearVisibleEntts()
{
	// clear an arr of entities that were visible in the previous frame;
	// so we will be able to use it again for the current frame;
	pRenderComponent_->visibleEnttsIDs_.clear();
}


// *********************************************************************************
//                           PRIVATE FUNCTIONS
// *********************************************************************************

void RenderSystem::GetShaderTypesOfEntts(
	const std::vector<EntityID>& enttsIDs,
	std::vector<RenderShaderType>& outShaderTypes)
{
	// get shader types of each input entity by its ID;
	// 
	// in:  SORTED array of entities IDs
	// out: array of rendering shader types

	
	std::vector<ptrdiff_t> idxs; 

	idxs.reserve(std::ssize(enttsIDs));
	outShaderTypes.reserve(std::ssize(enttsIDs));

	// get index into array of each input entity by ID
	for (const EntityID& enttID : enttsIDs)
		idxs.push_back(Utils::GetIdxInSortedArr(pRenderComponent_->ids_, enttID));

	// get shader type of each input entity
	for (const ptrdiff_t idx : idxs)
		outShaderTypes.push_back(pRenderComponent_->shaderTypes_[idx]);
}

}