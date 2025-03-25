// *********************************************************************************
// Filename:     Textured.h
// Description:  an ECS component which contains textures data of entities;
// 
// Created:      28.06.24
// *********************************************************************************
#pragma once

#include "../Common/Types.h"
#include "../Common/Assert.h"
#include "../Common/cvector.h"

namespace ECS
{

constexpr size TEXTURES_TYPES_COUNT = 22;


struct TexturedData
{
    TexturedData() {}

    TexturedData(const TexID* inTexIDs, const int inSubmeshID) :
        submeshID(inSubmeshID)
    {
        Assert::NotNullptr(texIDs, "a ptr to tex arr == nullptr");
        Assert::True(submeshID >= 0, "wrong submesh ID value");

        // copy 22 textures IDs
        std::copy(inTexIDs, inTexIDs + TEXTURES_TYPES_COUNT, texIDs);
    }

    TexID    texIDs[22]{ 0 };    // each submesh can have 22 texture types
    uint32_t submeshID = -1;     // ID of entt's model submesh
};

// --------------------------------------------------------

struct Textured
{
    cvector<EntityID>     ids;
    cvector<TexturedData> data;

    ComponentType         type = ComponentType::TexturedComponent;
};

}
