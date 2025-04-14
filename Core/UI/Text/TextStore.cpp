// =================================================================================
// Filename: textclass.cpp
// Revising: 04.07.22
// =================================================================================
#include "TextStore.h"

#include <CoreCommon/Log.h>
#include <stdexcept>

using namespace Core;

namespace UI
{

// static
SentenceID TextStore::staticID_ = 0;

TextStore::TextStore() 
{
}

TextStore::~TextStore() 
{
    Core::LogDbg("");
}



// =================================================================================
//                             public modification API
// =================================================================================

SentenceID TextStore::CreateConstSentence(
    ID3D11Device* pDevice,
    FontClass& font,                     // font for the text
    const std::string& textContent,      // the content of the text
    const DirectX::XMFLOAT2& drawAt)
{
    // the content of this string won't be changed;
    // you can only change its position on the screen;

    try
    {
        Core::Assert::True(!textContent.empty(), "the input string is empty");

        // add ID for this new sentence
        SentenceID id = staticID_++;
        ids_.push_back(id);

        // upper left rendering pos
        drawAt_.push_back(drawAt);

        maxStrSize_.push_back(std::ssize(textContent));
        textContent_.push_back(textContent);

        cvector<Core::VertexFont> vertices;
        cvector<UINT>             indices;

        BuildTextVerticesIndices(
            pDevice,
            maxStrSize_.back(),
            textContent,
            drawAt,
            font,
            vertices,
            indices);

        // initialize the VB/IB for this text string
        constexpr bool isDynamic = false;
        Core::VertexBuffer<Core::VertexFont> vb(pDevice, vertices.data(), (int)vertices.size(), isDynamic);
        Core::IndexBuffer<UINT>              ib(pDevice, indices.data(), (int)indices.size());

        vertexBuffers_.push_back(std::move(vb));
        indexBuffers_.push_back(std::move(ib));

        return ids_.back();
    }
    catch (Core::EngineException& e)
    {
        LogErr(e, false);
        sprintf(g_String, "can't create a sentence with the text: %s", textContent.c_str());
        LogErr(g_String);

        return 0;
    }
}

///////////////////////////////////////////////////////////

SentenceID TextStore::CreateSentence(
    ID3D11Device* pDevice,
    FontClass& font,                     // font for the text
    const std::string& textContent,      // the content of the text
    const size maxStrSize,               // maximal length for this string
    const DirectX::XMFLOAT2& drawAt,     // upper left position of the text in the window
    const bool isDynamic)                // will this sentence be changed from frame to frame?
{
    // the content of this string is supposed to be changed from frame to frame;
    // you can also change its position on the screen;

    try
    {
        // check input params
        Core::Assert::True(!textContent.empty(), "the input string is empty");
        Core::Assert::True(maxStrSize >= std::ssize(textContent), "max string size must be >= input text string");

        // add ID for this new sentence
        SentenceID id = staticID_++;
        ids_.push_back(id);

        // upper left rendering pos
        drawAt_.push_back(drawAt);
           
        maxStrSize_.push_back(maxStrSize);       
        textContent_.push_back(textContent);

        cvector<Core::VertexFont> vertices;
        cvector<UINT> indices;
        
        BuildTextVerticesIndices(
            pDevice,
            maxStrSize,
            textContent,
            drawAt,
            font, 
            vertices,
            indices);

        // initialize the VB/IB for this text string
        Core::VertexBuffer<Core::VertexFont> vb(pDevice, vertices.data(), (int)vertices.size(), isDynamic);
        Core::IndexBuffer<UINT>              ib(pDevice, indices.data(), (int)indices.size());

        vertexBuffers_.push_back(std::move(vb));
        indexBuffers_.push_back(std::move(ib));

        return id;
    }
    catch (Core::EngineException & e)
    {
        LogErr(e, false);
        sprintf(g_String, "can't create a sentence with the text: ", textContent.c_str());
        LogErr(g_String);
        return 0;
    }
}

///////////////////////////////////////////////////////////

void TextStore::SetKeyByID(const std::string& key, const SentenceID id)
{
    // create a semantic text key for the input sentence ID;
    // so we will be able to get this ID by key;
    const auto result = keyToID_.insert({ key, id });

    if (!result.second)
    {
        sprintf(g_String, "can't set a key (%s) by sentence ID (%ud)", key.c_str(), id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void TextStore::GetRenderingData(
    cvector<ID3D11Buffer*>& outVbPtrs,
    cvector<ID3D11Buffer*>& outIbPtrs,
    cvector<u32>& outIndexCounts)

{
    // get VB, IB, and index count of each sentence from the storage

    try
    {
        const size numBuffers = std::ssize(vertexBuffers_);

        outVbPtrs.resize(numBuffers);
        outIbPtrs.resize(numBuffers);
        outIndexCounts.resize(numBuffers);

        // go through vertex buffers and get pointers to it
        for (int i = 0; const auto& vb : vertexBuffers_)
            outVbPtrs[i++] = vb.Get();

        // go through index buffers and get pointers to it
        for (int i = 0; const auto& ib : indexBuffers_)
            outIbPtrs[i++] = ib.Get();

        // go through index buffers and get index counts
        for (int i = 0; const auto& ib : indexBuffers_)
            outIndexCounts[i++] = ib.GetIndexCount();
    }
    catch (Core::EngineException & e)
    {
        Core::LogErr(e);
        throw Core::EngineException("can't render the sentence");
    }
}


// ====================================================================================
//                               PUBLIC UPDATE API
// ====================================================================================
void TextStore::Update(
    ID3D11DeviceContext* pContext,
    FontClass& font,
    const Core::SystemState& sysState)

{
    // Update() changes the contents of the dynamic vertex buffer for the input text.

    try
    {
        constexpr size numDynamicSentences = 13;

        // sentences by these keys are supposed to update if its values are
        // differ from the sysState
        const std::string keys[numDynamicSentences] =
        {
            "fps", "frame_time",

            // position info
            "posX",	"posY",	"posZ",

            // rotation info
            "rotX", "rotY", "rotZ",  

            // render info
            "models_drawn", "vertices_drawn", "faces_drawn",
            "cells_drawn", "cells_culled",
        };

     
        // check yourself
        Core::Assert::True(numDynamicSentences == ARRAYSIZE(keys), "num of dynamic sentences != size of semantic keys arr");

        SentenceID  ids[numDynamicSentences];
        std::string values[numDynamicSentences];
        cvector<index> idxs(numDynamicSentences);

        // get ids by keys
        for (int i = 0; const std::string& key : keys)
            ids[i++] = keyToID_.at(key);

        // get idxs by ids
        ids_.get_idxs(ids, numDynamicSentences, idxs);


        // set values so later we will compare them to the previous ones
        values[0] = std::to_string(sysState.fps);
        values[1] = std::to_string(sysState.frameTime);

        // pos info
        values[2] = std::to_string(sysState.cameraPos.x);
        values[3] = std::to_string(sysState.cameraPos.y);
        values[4] = std::to_string(sysState.cameraPos.z);

        // rotation info
        values[5] = std::to_string(sysState.cameraDir.x);
        values[6] = std::to_string(sysState.cameraDir.y);
        values[7] = std::to_string(sysState.cameraDir.z);

        // render info
        values[8]  = std::to_string(sysState.visibleObjectsCount);
        values[9]  = std::to_string(sysState.visibleVerticesCount);
        values[10] = std::to_string(sysState.visibleVerticesCount / 3);
        values[11] = std::to_string(sysState.cellsDrawn);
        values[12] = std::to_string(sysState.cellsCulled);
        

        // update the sentence if necessary
        for (index i = 0; i < numDynamicSentences; ++i)
        {
            const index idx = idxs[i];

            if (textContent_[idx] != values[i])
                UpdateSentenceByIdx(pContext, font, idx, values[i]);
        }
    }
    catch (const std::out_of_range& e)
    {
        Core::LogErr(e.what());
        Core::LogErr("can't find an dynamic sentences of UI");
    }
    catch (Core::EngineException& e)
    {
        Core::LogErr(e);
        Core::LogErr("failed to update the text vertex buffer with new data");
        throw Core::EngineException("can't update the sentence");
    }
}


// ====================================================================================
//                            PRIVATE MODICATION API 
// ====================================================================================
void TextStore::BuildTextVerticesIndices(
    ID3D11Device* pDevice,
    const size maxStrSize,
    const std::string& textContent,
    const DirectX::XMFLOAT2& drawAt,
    FontClass& font,                      // font for the text
    cvector<Core::VertexFont>& vertices,
    cvector<UINT>& indices)
{ 
    // THIS FUNC builds a vertex and index buffer for the input string by its 
    // textContent and places its vertices at the drawAt position;

    try
    {
        Core::Assert::True(!textContent.empty(), "the input str is empty");
        Core::Assert::True(maxStrSize >= std::ssize(textContent), "maxStrSize must be >= sentence size");

        const size numVerticesInSymbol = 4;
        const size numIndicesInSymbol  = 6;
        const size numVertices         = maxStrSize * numVerticesInSymbol;
        const size numIndices          = maxStrSize * numIndicesInSymbol;

        vertices.resize(numVertices);
        indices.resize(numIndices, 0);
        
        // fill in vertex and index arrays with initial data
        font.BuildVertexArray(vertices.data(), numVertices, textContent, drawAt);
        font.BuildIndexArray(indices.data(), numIndices);
    }
    catch (Core::EngineException & e)
    {
        LogErr(e);
        sprintf(g_String, "can't build buffers for the sentence: %s", textContent.c_str());
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void TextStore::UpdateSentenceByIdx(
    ID3D11DeviceContext* pContext,
    FontClass& font,
    const index idx,
    const std::string& newStr)
{
    // update the sentence by idx with new content;
    // also we rebuild its vertices according to this new content
    // and update its VB

    textContent_[idx] = newStr;

    cvector<Core::VertexFont> vertices(maxStrSize_[idx] * 4);

    // rebuild vertices for this sentence
    font.BuildVertexArray(
        vertices.data(),
        std::ssize(vertices),
        newStr,
        drawAt_[idx]);

    // update VB with new vertices
    vertexBuffers_[idx].UpdateDynamic(
        pContext,
        vertices.data(),
        std::ssize(vertices));
}

} // namespace UI
