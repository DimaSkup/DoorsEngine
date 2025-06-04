// =================================================================================
// Filename:     RenderSystem.cpp
// Description:  implementation of the ECS RenderSystem's functional
// 
// Created:      21.05.24
// =================================================================================
#include "../Common/pch.h"
#include "RenderSystem.h"


namespace ECS
{

RenderSystem::RenderSystem(Rendered* pRenderComponent) :
    pRenderComponent_(pRenderComponent)
{
    CAssert::NotNullptr(pRenderComponent, "ptr to the Rendered component == nullptr");

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
}

/////////////////////////////////////////////////

void RenderSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
    // deserialize the data from the data file into the Rendered component
}

/////////////////////////////////////////////////

void RenderSystem::AddRecords(
    const EntityID* ids, 
    const RenderInitParams* params,
    const size numEntts)
{
    // set that input entities by IDs will be rendered onto the screen;
    // also setup rendering params for each input entity;

    CAssert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(params != nullptr, "input ptr to render init params == nullptr");
    CAssert::True(numEntts > 0, "input number of entts must be > 0");

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

    CAssert::True((ids != nullptr) && (numEntts > 0), "invalid input args");
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
