// =================================================================================
// Filename:     RenderSystem.cpp
// Description:  implementation of the ECS RenderSystem's functional
// 
// Created:      21.05.24
// =================================================================================
#include "RenderSystem.h"

#include "../Common/UtilsFilesystem.h"
#include "../Common/Assert.h"

#include <fstream>

namespace ECS
{

RenderSystem::RenderSystem(Rendered* pRenderComponent) :
    pRenderComponent_(pRenderComponent)
{
    Assert::NotNullptr(pRenderComponent, "ptr to the Rendered component == nullptr");

    Rendered& comp = *pRenderComponent;
    constexpr size newCapacity = 128;

    comp.ids.reserve(newCapacity);
    comp.shaderTypes.reserve(newCapacity);
    comp.primTopologies.reserve(newCapacity);
}


// =================================================================================
//                            PUBLIC METHODS
// =================================================================================
void RenderSystem::Serialize(std::ofstream& fout, u32& offset)
{
    // serialize all the data from the Rendered component into the data file

    // store offset of this data block so we will use it later for deserialization
    offset = static_cast<u32>(fout.tellp());

    Rendered& comp = *pRenderComponent_;
    const u32 dataBlockMarker = static_cast<u32>(pRenderComponent_->type);
    const u32 dataCount = (u32)comp.ids.size();

    // write serialized data into the file
    FileWrite(fout, &dataBlockMarker);
    FileWrite(fout, &dataCount);

    FileWrite(fout, comp.ids);
    FileWrite(fout, comp.shaderTypes);
    FileWrite(fout, comp.primTopologies);
}

/////////////////////////////////////////////////

void RenderSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
    // deserialize the data from the data file into the Rendered component
    
    // read data starting from this offset
    fin.seekg(offset, std::ios_base::beg);

    // check if we read the proper data block
    u32 dataBlockMarker = 0;
    FileRead(fin, &dataBlockMarker);

    const bool isProperDataBlock = (dataBlockMarker == static_cast<u32>(eComponentType::RenderedComponent));
    Assert::True(isProperDataBlock, "read wrong data block during deserialization of the Rendered component data from a file");

    // ------------------------------------------

    // read in how much data will we have
    u32 dataCount = 0;
    FileRead(fin, &dataCount);

    Rendered& comp = *pRenderComponent_;

    // prepare enough amount of memory for data
    comp.ids.resize(dataCount);
    comp.shaderTypes.resize(dataCount);
    comp.primTopologies.resize(dataCount);

    FileRead(fin, comp.ids.data());
    FileRead(fin, comp.shaderTypes.data());
    FileRead(fin, comp.primTopologies.data());
}

/////////////////////////////////////////////////

void RenderSystem::AddRecords(
    const EntityID* ids, 
    const RenderInitParams* params,
    const size numEntts)
{
    // set that input entities by IDs will be rendered onto the screen;
    // also setup rendering params for each input entity;

    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(params != nullptr, "input ptr to render init params == nullptr");
    Assert::True(numEntts > 0, "input number of entts must be > 0");

    Rendered& comp = *pRenderComponent_;
    cvector<index> idxs;

    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec indices correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of input values
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.shaderTypes.insert_before(idxs[i], params[i].shaderType);

    for (index i = 0; i < numEntts; ++i)
        comp.primTopologies.insert_before(idxs[i], params[i].topologyType);
}

/////////////////////////////////////////////////

void RenderSystem::RemoveRecords(const EntityID* ids, const size numEntts)
{
    assert(0 && "TODO");
}

/////////////////////////////////////////////////

void RenderSystem::GetRenderingDataOfEntts(
    const EntityID* ids,
    const size numEntts,
    cvector<RenderShaderType>& outShaderTypes)
{
    // get necessary data for rendering of each curretly visible entity;
    // 
    // out:    shader type for each entity 

    Assert::True((ids != nullptr) && (numEntts > 0), "invalid input args");
    GetShaderTypesOfEntts(ids, numEntts, outShaderTypes);
}


// =================================================================================
//                           PRIVATE METHODS
// =================================================================================
void RenderSystem::GetShaderTypesOfEntts(
    const EntityID* ids,
    const size numEntts,
    cvector<ECS::RenderShaderType>& outShaderTypes)
{
    // get shader types of each input entity by its ID;
    // 
    // in:  SORTED array of entities IDs
    // out: array of rendering shader types

    const Rendered& comp = *pRenderComponent_;
    cvector<index> idxs;


    // get index into array of each input entity by ID
    comp.ids.get_idxs(ids, numEntts, idxs);
   
    // get shader type of each input entity
    outShaderTypes.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outShaderTypes[i++] = comp.shaderTypes[idx];
}

} // namespace ECS
