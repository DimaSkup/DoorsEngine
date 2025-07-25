////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     textclass.h
// Description:  handles all the 2D text drawing that the application
//               will need. It renders 2D text to the screen.
//               It uses FontClass to create the vertex buffer for strings
//               and then uses FontShaderClass to render this buffer;
//
// Revising:     04.06.22
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "fontclass.h"              // text font

#include <Types.h>
#include <cvector.h>
#include <CoreCommon/SystemState.h>

#include "../../Mesh/Vertex.h"
#include "../../Mesh/VertexBuffer.h"
#include "../../Mesh/IndexBuffer.h"

#include <map>
#include <DirectXMath.h>
#include <string>


namespace UI
{

// setup some limitations for the debug text
constexpr size maxNumDbgSentences      = 32;
constexpr size maxNumCharsPerSentence  = 32;
constexpr size maxNumCharsInDbgText    = maxNumDbgSentences * maxNumCharsPerSentence;
constexpr size maxNumVerticesInDbgText = maxNumCharsInDbgText * 4;   // 4 vertices per symbol

class TextStore
{
public:

    TextStore();	
    ~TextStore();

    // restrict a copying of this class instance
    TextStore(const TextStore& obj) = delete;
    TextStore& operator=(const TextStore& obj) = delete;

    // ---------------------------------------------

    void InitDebugText(ID3D11Device* pDevice, FontClass& font);

    SentenceID CreateConstSentence(
        ID3D11Device* pDevice,
        FontClass& font,                     // font for the text
        const std::string& textContent,      // the content of the text
        const DirectX::XMFLOAT2& drawAt);    // upper left corner of the sentence

    SentenceID CreateSentence(
        ID3D11Device* pDevice,
        FontClass& font,      
        const std::string& textContent,       
        const size maxStrSize,
        const DirectX::XMFLOAT2& drawAt,
        const bool isDynamic);

    void AddDebugConstSentence(
        const char* key,
        const char* text,
        const float drawAtX,
        const float drawAtY);

    void AddDebugDynamicSentence(
        const char* key,
        const char* text,
        const float drawAtX,
        const float drawAtY);

    void GetRenderingData(
        ID3D11Buffer** outConstVbPtr,
        ID3D11Buffer** outDynamicVbPtr,
        ID3D11Buffer** outIbPtr,
        u32& outConstIndexCount,    // index count for debug const sentences
        u32& outDynamicIndexCount); // index count for debug dynamic sentences

    void Update(
        ID3D11DeviceContext* pContext,
        FontClass& font,
        const Core::SystemState& systemState);

private:

    void BuildTextVerticesIndices(
        ID3D11Device* pDevice,
        const size maxStrSize,                             // maximal size for this string (if it will be bigger we will have a vertex buffer overflow)
        const std::string& textContent,
        const DirectX::XMFLOAT2& drawAt,                   // upper left position of the str
        FontClass& font,                                   // font for the text
        cvector<Core::VertexFont>& vertices,
        cvector<UINT>& indices);

    void UpdateDebugText(
        ID3D11DeviceContext* pContext,
        FontClass& font,
        const Core::SystemState& sysState);
    
    void UpdateSentenceByIdx(
        ID3D11DeviceContext* pContext,
        FontClass& font,
        const index idx,
        const char* newStr);

    void UpdateDebugSentences(ID3D11DeviceContext* pContext, FontClass& font);

private:
    struct DbgSentence
    {
        char  text[maxNumCharsPerSentence]{'\0'};
        float drawAtX = 0;
        float drawAtY = 0;
    };

private:
    static SentenceID staticID_;

    cvector<SentenceID>               ids_;
    cvector<std::string>              textContent_;

    // semantic keys for navigation (is useful to find a str to update it);
    // these keys are set manually using the SetKeyByID() method;
    std::map<std::string, SentenceID> keyToID_;       

    cvector<DirectX::XMFLOAT2>        drawAt_;         // upper left corner of sentence
    cvector<size>                     maxStrSize_;     // maximal number of vertices per each string
    cvector<bool>                     isDynamic_;      // is this str modifiable?

    cvector<Core::VertexBuffer<Core::VertexFont>> vertexBuffers_;
    cvector<Core::IndexBuffer<UINT>>              indexBuffers_;


    size                numDbgConstSentences_ = 0;
    size                numDbgDynamicSentences_ = 0;
    DbgSentence         dbgConstSentences_[maxNumDbgSentences];
    DbgSentence         dbgDynamicSentences_[maxNumDbgSentences];
    Core::VertexFont    dbgTextRawVertices_[maxNumVerticesInDbgText];

    Core::VertexBuffer<Core::VertexFont> vbDbgConstText_;
    Core::VertexBuffer<Core::VertexFont> vbDbgDynamicText_;
    Core::IndexBuffer<UINT>              ibDbgText_;  // we use the same index buffer for const and dynamic debug sentences since vertex order is the same for both
    u32 indexCountDbgConstText_ = 0;
    u32 indexCountDbgDynamicText_ = 0;

};

} // namespace UI
