// =================================================================================
// Filename: textclass.cpp
// Revising: 04.07.22
// =================================================================================
#include <CoreCommon/pch.h>
#include "TextStore.h"
#pragma warning (disable : 4996)


namespace UI
{

// static
SentenceID TextStore::staticID_ = 0;

TextStore::TextStore()
{
}

TextStore::~TextStore() 
{
    LogDbg(LOG, "destructor");
}


// =================================================================================
// public modification API
// =================================================================================
void TextStore::InitDebugText(ID3D11Device* pDevice, FontClass& font)
{
    constexpr size      vbSize = maxNumVerticesInDbgText;
    constexpr size      ibSize = maxNumCharsInDbgText * 6;  // 6 indices per symbol
    const cvector<UINT> indices(ibSize, 0);
    const bool          isDynamic = true;

    // ----------------------------------------------------
   
    // offset in the buf of raw vertices
    index vOffset = 0;  

    // build vertices for the whole debug const text
    Core::VertexFont rawVertices[maxNumVerticesInDbgText];

    for (index idx = 0; idx < numDbgConstSentences_; ++idx)
    {
        // number of vertices for the current sentence
        const size numVertices = (size)strlen(dbgConstSentences_[idx].text) * 4;

        // rebuild vertices for this sentence
        font.BuildVertexArray(
            rawVertices + vOffset,
            numVertices,
            dbgConstSentences_[idx].text,
            dbgConstSentences_[idx].drawAtX,
            dbgConstSentences_[idx].drawAtY);

        vOffset += numVertices;

        if (vOffset >= maxNumVerticesInDbgText)
        {
            LogErr("too much vertices for the vertex buffer of debug const sentences");
            idx = numDbgConstSentences_;   // go out from the for-loop
        }
    }
    // compute actual number of indices (vOffset / vertices_per_sym * indices_per_sym)
    indexCountDbgConstText_ = (u32)(vOffset / 4 * 6);

    // ----------------------------------------------------

    // init vertex buffer for debug const and dynamic sentences
    if (!vbDbgConstText_.Initialize(pDevice, rawVertices, (int)vOffset, !isDynamic))
    {
        LogErr("can't create a const vertex buffer for debug text");
        return;
    }
    if (!vbDbgDynamicText_.Initialize(pDevice, dbgTextRawVertices_, vbSize, isDynamic))
    {
        LogErr("can't create a dynamic vertex buffer for debug text");
        return;
    }

    // init common index buffer (is used by both debug const and dynamic sentences)
    font.BuildIndexArray(indices.data(), ibSize);

    constexpr bool isDynamicBuf = false;

    if (!ibDbgText_.Initialize(pDevice, indices.data(), ibSize, isDynamicBuf))
    {
        LogErr("can't create an index buffer for debug (dynamic) text");
        return;
    }
}

///////////////////////////////////////////////////////////

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
        CAssert::True(!textContent.empty(), "the input string is empty");

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
    catch (EngineException& e)
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
        CAssert::True(!textContent.empty(), "the input string is empty");
        CAssert::True(maxStrSize >= std::ssize(textContent), "max string size must be >= input text string");

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
    catch (EngineException & e)
    {
        LogErr(e, false);
        sprintf(g_String, "can't create a sentence with the text: %s", textContent.c_str());
        LogErr(g_String);
        return 0;
    }
}

///////////////////////////////////////////////////////////

void TextStore::AddDebugConstSentence(
    const char* key,
    const char* text,
    const float drawAtX,
    const float drawAtY)
{
    const index idx = numDbgConstSentences_;
    ++numDbgConstSentences_;

    if (text)
        strcpy(dbgConstSentences_[idx].text, text);

    dbgConstSentences_[idx].drawAtX = drawAtX;
    dbgConstSentences_[idx].drawAtY = drawAtY;
}

///////////////////////////////////////////////////////////

void TextStore::AddDebugDynamicSentence(
    const char* key,
    const char* text,
    const float drawAtX,
    const float drawAtY)
{
    const index idx = numDbgDynamicSentences_;
    ++numDbgDynamicSentences_;

    if (text)
        strcpy(dbgDynamicSentences_[idx].text, text);

    dbgDynamicSentences_[idx].drawAtX = drawAtX;
    dbgDynamicSentences_[idx].drawAtY = drawAtY;
}

///////////////////////////////////////////////////////////

void TextStore::GetRenderingData(
    ID3D11Buffer** outConstVbPtr,    // a ptr to the const vertex buf of const debug text
    ID3D11Buffer** outDynamicVbPtr,  // a ptr to the dynamic vertex buf of dynamic debug text
    ID3D11Buffer** outIbPtr,
    u32& outConstIndexCount,         // index count for debug const sentences
    u32& outDynamicIndexCount)       // index count for debug dynamic sentences
{
    *outConstVbPtr       = vbDbgConstText_.Get();
    *outDynamicVbPtr     = vbDbgDynamicText_.Get();
    *outIbPtr            = ibDbgText_.Get();
    outConstIndexCount   = indexCountDbgConstText_;
    outDynamicIndexCount = indexCountDbgDynamicText_;
}


// ====================================================================================
//                               PUBLIC UPDATE API
// ====================================================================================
void TextStore::UpdateDebugText(
    ID3D11DeviceContext* pContext,
    FontClass& font,
    const Core::SystemState& sysState)
{
    // update text content of each debug string
    int i = 0;

    sprintf(dbgDynamicSentences_[i++].text, "%d",       sysState.fps);
    sprintf(dbgDynamicSentences_[i++].text, "%05.2fms", sysState.frameTime);
    sprintf(dbgDynamicSentences_[i++].text, "%05.2fms", sysState.updateTime);
    sprintf(dbgDynamicSentences_[i++].text, "%05.2fms", sysState.renderTime);

    // pos info
    sprintf(dbgDynamicSentences_[i++].text, "%.2f", sysState.cameraPos.x);
    sprintf(dbgDynamicSentences_[i++].text, "%.2f", sysState.cameraPos.y);
    sprintf(dbgDynamicSentences_[i++].text, "%.2f", sysState.cameraPos.z);

    // rotation info
    sprintf(dbgDynamicSentences_[i++].text, "%.2f", sysState.cameraDir.x);
    sprintf(dbgDynamicSentences_[i++].text, "%.2f", sysState.cameraDir.y);
    sprintf(dbgDynamicSentences_[i++].text, "%.2f", sysState.cameraDir.z);

    // render info
    sprintf(dbgDynamicSentences_[i++].text, "%d", sysState.visibleObjectsCount);
    sprintf(dbgDynamicSentences_[i++].text, "%d", sysState.visibleVerticesCount);
    sprintf(dbgDynamicSentences_[i++].text, "%d", sysState.visibleVerticesCount / 3);
    sprintf(dbgDynamicSentences_[i++].text, "%d", sysState.cellsDrawn);
    sprintf(dbgDynamicSentences_[i++].text, "%d", sysState.cellsCulled);

    UpdateDebugSentences(pContext, font);
}

///////////////////////////////////////////////////////////

void TextStore::Update(
    ID3D11DeviceContext* pContext,
    FontClass& font,
    const Core::SystemState& sysState)

{
    // update the content of the dynamic text and dynamic vertex buffers
    UpdateDebugText(pContext, font, sysState);
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
    // build a vertex and index buffer for the input string by its 
    // textContent and places its vertices at the drawAt position;
    try
    {
        CAssert::True(!textContent.empty(), "the input str is empty");
        CAssert::True(maxStrSize >= std::ssize(textContent), "maxStrSize must be >= sentence size");

        constexpr size numVerticesPerChar = 4;
        constexpr size numIndicesPerChar  = 6;
        const     size numVertices        = maxStrSize * numVerticesPerChar;
        const     size numIndices         = maxStrSize * numIndicesPerChar;

        vertices.resize(numVertices);
        indices.resize(numIndices, 0);
        
        // fill in vertex and index arrays with initial data
        font.BuildVertexArray(vertices.data(), numVertices, textContent.c_str(), drawAt.x, drawAt.y);
        font.BuildIndexArray(indices.data(), numIndices);
    }
    catch (EngineException & e)
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
    const char* newStr)
{
    // update the sentence by idx with new content;
    // also we rebuild its vertices according to this new content
    // and update its VB

    textContent_[idx] = newStr;

    constexpr size numMaxVertices = 256;             // suppose max 64 symbols * 4 vertex per each symbol
    Core::VertexFont vertices[numMaxVertices];
    constexpr size verticesPerChar = 4;
    const size maxNumVerticesInSentence = maxStrSize_[idx] * verticesPerChar;

    // rebuild vertices for this sentence
    font.BuildVertexArray(
        vertices,
        maxNumVerticesInSentence,
        newStr,
        drawAt_[idx].x,
        drawAt_[idx].y);

    // update VB with new vertices
    vertexBuffers_[idx].UpdateDynamic(pContext, vertices, maxNumVerticesInSentence);
}

///////////////////////////////////////////////////////////

void TextStore::UpdateDebugSentences(ID3D11DeviceContext* pContext, FontClass& font)
{
    // update text content of the debug dynamic strings, rebuild vertices,
    // and also update the vertex buffer with these new vertices

    constexpr size verticesPerChar = 4;
    index vOffset = 0;                    // offset in the buf of raw vertices

    for (index idx = 0; idx < numDbgDynamicSentences_; ++idx)
    {
        // number of vertices for the current sentence
        const size numVertices = (size)strlen(dbgDynamicSentences_[idx].text) * verticesPerChar;

        // rebuild vertices for this sentence
        font.BuildVertexArray(
            dbgTextRawVertices_ + vOffset,
            numVertices,
            dbgDynamicSentences_[idx].text,
            dbgDynamicSentences_[idx].drawAtX,
            dbgDynamicSentences_[idx].drawAtY);

        vOffset += numVertices;
    }

    // compute actual number of indices (vOffset / vertices_per_sym * indices_per_sym)
    indexCountDbgDynamicText_ = (u32)(vOffset / 4 * 6);  

    vbDbgDynamicText_.UpdateDynamic(pContext, dbgTextRawVertices_, vOffset);
}

} // namespace UI
