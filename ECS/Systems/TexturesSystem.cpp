// **********************************************************************************
// Filename:      TexturesSystem.cpp
// Description:   implementation of the TexturesSystem's functional
// 
// Created:       28.06.24
// **********************************************************************************
#include "TexturesSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"

namespace ECS
{

TexturesSystem::TexturesSystem(Textured* pTextures) : pTexturesComponent_(pTextures)
{
    Assert::NotNullptr(pTextures, "input ptr to the Textures component == nullptr");

    // setup default (invalid) textures set
    constexpr TexID texIDs[TEXTURES_TYPES_COUNT]{ INVALID_TEXTURE_ID };
    constexpr int submeshID = 0;

    Textured& comp = *pTexturesComponent_;
    comp.ids.push_back(INVALID_ENTITY_ID);
    comp.data.push_back(TexturedData(texIDs, submeshID));
}

///////////////////////////////////////////////////////////

void TexturesSystem::Serialize(std::ofstream& fout, u32& offset)
{
    Assert::True(false, "TODO: implement it!");
}

///////////////////////////////////////////////////////////

void TexturesSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
    Assert::True(false, "TODO: implement it!");
}

///////////////////////////////////////////////////////////

void TexturesSystem::AddRecord(
    const EntityID enttID,
    const TexID* texIDs,
    const size numTextures,
    const int submeshID)            
{
    // add own textures set to each input entity;
    // in: submeshID: to which mesh of this particular entt we set new textures

    Assert::True(enttID != INVALID_ENTITY_ID, "invalid entity");
    Assert::NotNullptr(texIDs, "a ptr to textures arr == nullptr");
    Assert::True(numTextures == 22, "wrong numTextures (must be == 22)");
    Assert::True(submeshID >= 0, "wrong submesh id (must be >= 0)");


    Textured& comp = *pTexturesComponent_;

    // if there is already a record with such entt ID
    if (comp.ids.binary_search(enttID))
    {
        Log::Error("can't add record: there is already an entity (ID: {}, name: {})" + std::to_string(enttID));
        return;
    }

    // add a record (here we execute sorted insertion into the data arrays)
    const index idx = comp.ids.get_insert_idx(enttID);

    comp.ids.insert_before(idx, enttID);
    comp.data.insert_before(idx, TexturedData(texIDs, submeshID));
}

///////////////////////////////////////////////////////////

const TexturedData& TexturesSystem::GetDataByEnttID(const EntityID id)
{
    // get Textured component data for entity by enttID

    const Textured& comp = *pTexturesComponent_;
    const bool exist = comp.ids.binary_search(id);

    // if there no entity by ID our idx == 0, so we return "invalid" data
    const index idx = exist * comp.ids.get_idx(id);

    return comp.data[idx];
}

///////////////////////////////////////////////////////////

void TexturesSystem::GetDataByEnttsIDs(
    const EntityID* ids,
    const size numEntts,
    cvector<TexturedData>& outData)
{
    // get Textured component data for all the input entts;
    // NOTE: this method expects that all the input entts have the Textured component;

    Assert::True(ids != nullptr, "input ptr to IDs arr == nullptr");
    Assert::True(numEntts > 0, "input number of entts must be > 0");

    const Textured& comp = *pTexturesComponent_;
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    // prepare the output arr and fill it with data
    outData.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outData[i++] = comp.data[idx];
}

///////////////////////////////////////////////////////////

void TexturesSystem::FilterEnttsWhichHaveOwnTex(
    const EntityID* ids,
    const size numEntts,
    cvector<EntityID>& outEnttsWithOwnTex,
    cvector<EntityID>& outEnttsWithMeshTex)
{
    // out: 1. entts with Textured component (so they have some specific textures)
    //      2. entts without Textured component (will be painted with its mesh textures)

    Assert::True(ids != nullptr, "input ptr to IDs arr == nullptr");
    Assert::True(numEntts > 0, "input number of entts must be > 0");


    Textured& comp = *pTexturesComponent_;
    cvector<bool> texExist;
    size numHasOwnTex = 0;

    comp.ids.binary_search(ids, numEntts, texExist);

    for (index i = 0; i < numEntts; ++i)
        numHasOwnTex += (size)(texExist[i]);

    outEnttsWithOwnTex.resize(numHasOwnTex);
    outEnttsWithMeshTex.resize(numEntts - numHasOwnTex);

    int idx1 = 0;
    int idx2 = 0;

    for (index i = 0; i < numEntts; ++i)
    {
        if (texExist[i])
            outEnttsWithOwnTex[idx1++] = ids[i];     
    }

    for (index i = 0; i < numEntts; ++i)
    {
        if (!texExist[i])
            outEnttsWithMeshTex[idx2++] = ids[i];
    }
}


}
