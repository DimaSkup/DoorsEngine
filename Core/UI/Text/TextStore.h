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

#include "../../Mesh/vertex.h"
#include "../../Mesh/vertex_buffer.h"
#include "../../Mesh/index_buffer.h"

#include <types.h>
#include <cvector.h>
#include <CoreCommon/system_state.h>


// NOTATION:
//       dbg - debug
//       dyn - dynamic
//       VB - vertex buffer
//       IB - index buffer

namespace UI
{

// setup some limitations for the debug text
constexpr size MAX_SENTENCE_KEY_LEN         = 32;
constexpr size MAX_NUM_DBG_SENTENCES        = 64;
constexpr size MAX_SENTENCE_LEN             = 32;
constexpr size MAX_NUM_CHARS_IN_DBG_TEXT    = MAX_NUM_DBG_SENTENCES * MAX_SENTENCE_LEN;
constexpr size MAX_NUM_VERTICES_IN_DBG_TEXT = MAX_NUM_CHARS_IN_DBG_TEXT * 4;   // 4 vertices per symbol


class TextStore
{
public:

    TextStore();	
    ~TextStore();

    // restrict a copying of this class instance
    TextStore(const TextStore& obj) = delete;
    TextStore& operator=(const TextStore& obj) = delete;

    // ---------------------------------------------

    bool Init(ID3D11Device* pDevice, FontClass* pFont);
    void SetFont(FontClass* pFont);

    SentenceID AddDebugConstStr(
        const char* text,
        const float drawAtX,
        const float drawAtY);

    SentenceID AddDebugDynamicStr(
        const char* key,
        const char* text,
        const uint  maxLen,
        const float drawAtX,
        const float drawAtY);

    void GetRenderingData(
        ID3D11Buffer** outConstVbPtr,
        ID3D11Buffer** outDynamicVbPtr,
        ID3D11Buffer** outIbPtr,
        uint32& outConstIndexCount,    // index count for debug const sentences
        uint32& outDynamicIndexCount); // index count for debug dynamic sentences

    void Update(
        ID3D11DeviceContext* pContext,
        const Core::SystemState& systemState);

private:
    void RebuildConstVB(ID3D11Device* pDevice);
    void RebuildDynVB  (ID3D11DeviceContext* pContext);

    void UpdateDynDbgText(const Core::SystemState& sysState);

    void UpdateStrByKey(const char* key, const char* fmt, const float val);
    void UpdateStrByKey(const char* key, const char* fmt, const int val);
    void UpdateStrByKey(const char* key, const char* fmt, const uint32 val);

    void UpdateDynTiming(const char* key, const float avgTiming);

    //-----------------------------------------------------
    // return a ptr to dynamic string by input semantic key
    //-----------------------------------------------------
    char* GetDynStr(const char* key);

    //-----------------------------------------------------
    // return an index of dynamic string by input semantic key
    //-----------------------------------------------------
    index GetDynStrIdx(const char* key);


private:
    struct SentenceKey
    {
        char key[MAX_SENTENCE_KEY_LEN]{'\0'};
    };

    struct DbgConstSentence
    {
        char text[MAX_SENTENCE_LEN]{ '\0' };
        float drawAtX = 0;
        float drawAtY = 0;
    };

    struct DbgDynamicSentence
    {
        char text[MAX_SENTENCE_LEN]{'\0'};
        float drawAtX = 0;
        float drawAtY = 0;
        uint  maxLen = 0;
    };

private:
    // currently selected font
    FontClass* pFont_ = nullptr;   

    static SentenceID staticID_;
    cvector<SentenceID> ids_;

    // semantic keys for navigation (is useful to find a str to update it);
    SentenceKey dbgDynStrKeys_[MAX_NUM_DBG_SENTENCES];

    // current number of strings
    size                numDbgConstStr_   = 0;
    size                numDbgDynStr_ = 0;

    DbgConstSentence    dbgConstSentences_[MAX_NUM_DBG_SENTENCES];
    DbgDynamicSentence  dbgDynSentences_  [MAX_NUM_DBG_SENTENCES];

    Core::VertexBuffer<Core::VertexFont> vbDbgConstText_;
    Core::VertexBuffer<Core::VertexFont> vbDbgDynText_;

    // we use the same index buffer for const and dynamic debug
    // sentences since vertex order is the same for both
    Core::IndexBuffer<UINT> ib_;  

    uint numIndicesDbgConstText_   = 0;
    uint numIndicesDbgDynText_ = 0;

    bool needUpdConstVB_ = false;
};

} // namespace UI
