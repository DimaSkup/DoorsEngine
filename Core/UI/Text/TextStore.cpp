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

//---------------------------------------------------------

TextStore::TextStore()
{
    LogDbg(LOG, "constructor");
}

TextStore::~TextStore() 
{
    LogDbg(LOG, "destructor");
}


// =================================================================================
// public modification API
// =================================================================================
void TextStore::SetFont(FontClass* pFont)
{
    if (pFont == nullptr)
    {
        LogErr(LOG, "can't set another font: input ptr to font == nullptr");
        return;
    }

    pFont_ = pFont;
    needUpdConstVB_ = true;
}

//---------------------------------------------------------
// Desc:  initialize the text storage
//        (build text vertices and create vertex/index buffers)
//---------------------------------------------------------
bool TextStore::Init(ID3D11Device* pDevice, FontClass* pFont)
{
    if (pDevice == nullptr)
    {
        LogErr(LOG, "can't init text storage: input ptr to DX11 device == nullptr");
        return false;
    }

    if (pFont == nullptr)
    {
        LogErr(LOG, "can't init text storage: input ptr to font == nullptr");
        return false;
    }

    // setup a font for text storage
    pFont_ = pFont;

    constexpr size vbSize = MAX_NUM_VERTICES_IN_DBG_TEXT;
    constexpr size ibSize = MAX_NUM_CHARS_IN_DBG_TEXT * 6;  // 6 indices per symbol
    const bool     isDynamicVB = true;
    constexpr bool isDynamicIB = false;


    // init a vertex buffer for const sentences   
    RebuildConstVB(pDevice);


    // init a vertex buffer for dynamic sentences
    const cvector<Core::VertexFont> vertices(vbSize);

    if (!vbDbgDynText_.Initialize(pDevice, vertices.data(), vbSize, isDynamicVB))
    {
        LogErr(LOG, "can't create a dynamic vertex buffer for debug text");
        return false;
    }


    // init an common index buffer for sentences
    const cvector<UINT> indices(ibSize, 0);
    pFont->BuildIndexArray(indices.data(), ibSize);
    
    if (!ib_.Initialize(pDevice, indices.data(), ibSize, isDynamicIB))
    {
        LogErr(LOG, "can't create an index buffer for debug (dynamic) text");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   call this method after we add a new const sentence
//         so we rebuild vertices for all the const strings
//         and update the related vertex buffer
//---------------------------------------------------------
void TextStore::RebuildConstVB(ID3D11Device* pDevice)
{
    if (!pDevice)
    {
        LogErr(LOG, "can't rebuild VB for const sentences: input ptr to DX11 device == nullptr");
        return;
    }

    constexpr size vertsPerChar = 4;
    index vOffset = 0;                  // offset in the buf of raw vertices

    Core::VertexFont rawVertices[MAX_NUM_VERTICES_IN_DBG_TEXT];
    memset(rawVertices, 0, sizeof(rawVertices));


    for (index idx = 0; idx < numDbgConstStr_; ++idx)
    {
        const DbgConstSentence& sentence = dbgConstSentences_[idx];
        const size len              = (size)strlen(sentence.text);
        const size numVertices      = len * vertsPerChar;

        // rebuild vertices for this sentence
        pFont_->BuildVertexArray(
            rawVertices + vOffset,
            numVertices,
            sentence.text,
            sentence.drawAtX,
            sentence.drawAtY);
        
        if (vOffset + numVertices >= MAX_NUM_VERTICES_IN_DBG_TEXT)
        {
            LogErr(LOG, "too much vertices for the vertex buffer of debug const sentences");
            idx = numDbgConstStr_;   // go out from the for-loop
        }

        vOffset += numVertices;
    }

    // compute actual number of indices (vOffset / vertices_per_sym * indices_per_sym)
    numIndicesDbgConstText_ = (u32)(vOffset / 4 * 6);

    // REinit a VB for const sentences
    vbDbgConstText_.Shutdown();

    const bool isDynamic = false;
    if (!vbDbgConstText_.Initialize(pDevice, rawVertices, (int)vOffset, !isDynamic))
    {
        LogErr(LOG, "can't create a const vertex buffer for debug text");
        return;
    }
}

//---------------------------------------------------------
// Desc:  rebuild vertices for updated dynamic strings
//        and also update the vertex buffer with these new vertices
//---------------------------------------------------------
void TextStore::RebuildDynVB(ID3D11DeviceContext* pContext)
{
    constexpr size vertsPerChar = 4;
    index vOffset = 0;                    // offset in the buf of raw vertices

    Core::VertexFont vertices[MAX_NUM_VERTICES_IN_DBG_TEXT];
    memset(vertices, 0, sizeof(vertices));

    for (index idx = 0; idx < numDbgDynStr_; ++idx)
    {
        // number of vertices for the current sentence
        const DbgDynamicSentence& sentence = dbgDynSentences_[idx];
        const size len                     = (size)strlen(sentence.text);
        const size numVertices             = len * vertsPerChar;

        // rebuild vertices for this sentence
        pFont_->BuildVertexArray(
            vertices + vOffset,
            numVertices,
            sentence.text,
            sentence.drawAtX,
            sentence.drawAtY);

        if (vOffset + numVertices >= MAX_NUM_VERTICES_IN_DBG_TEXT)
        {
            LogErr(LOG, "too much vertices for the vertex buffer of debug const sentences");
            idx = numDbgConstStr_;   // go out from the for-loop
        }

        vOffset += numVertices;
    }

    // compute actual number of indices (vOffset / vertices_per_sym * indices_per_sym)
    numIndicesDbgDynText_ = (uint)(vOffset / 4 * 6);

    vbDbgDynText_.UpdateDynamic(pContext, vertices, vOffset);
}

//---------------------------------------------------------
// Desc:    add a new debug const string;
// Args:    - text:              initial text content
//          - drawAtX, drawAtY:  top-left corner (relatively to screen center)
//---------------------------------------------------------
SentenceID TextStore::AddDebugConstStr(
    const char* text,
    const float drawAtX,
    const float drawAtY)
{
    if (StrHelper::IsEmpty(text) || strlen(text) >= MAX_SENTENCE_LEN)
    {
        LogErr(LOG, "input text is invalid (empty or too big (max: %d))", MAX_SENTENCE_LEN);
        return 0;
    }

    const index idx = numDbgConstStr_;
    assert(idx < MAX_NUM_DBG_SENTENCES);
    ++numDbgConstStr_;

    // when Update we will rebuild the vertex buffer for const strings
    needUpdConstVB_ = true;

    // save text
    DbgConstSentence& sentence = dbgConstSentences_[idx];
    strcpy(sentence.text, text);
    sentence.drawAtX = drawAtX;
    sentence.drawAtY = drawAtY;

    // add ID for this new sentence
    SentenceID id = staticID_++;
    ids_.push_back(id);

    return id;
}

//---------------------------------------------------------
// Desc:    add a new debug dynamic string
// Args:    - key:               semantic text key
//          - text:              content
//          - maxLen:            max possible length for this dynamic string
//          - drawAtX, drawAtY:  top-left corner (relatively to screen center)
//---------------------------------------------------------
SentenceID TextStore::AddDebugDynamicStr(
    const char* key,
    const char* text,
    const uint  maxLen,
    const float drawAtX,
    const float drawAtY)
{
    // check input args
    if (StrHelper::IsEmpty(key) || strlen(key) >= MAX_SENTENCE_KEY_LEN)
    {
        LogErr(LOG, "input key is invalid (empty or too big (max: %d))", MAX_SENTENCE_KEY_LEN);
        return 0;
    }

    if (StrHelper::IsEmpty(text) || strlen(text) >= MAX_SENTENCE_LEN)
    {
        LogErr(LOG, "input text is invalid (empty or too big (max: %d))", MAX_SENTENCE_LEN);
        return 0;
    }


    const index idx = numDbgDynStr_;
    assert(idx < MAX_NUM_DBG_SENTENCES);
    ++numDbgDynStr_;

    // save the key
    strcpy(dbgDynStrKeys_[idx].key, key);

    // save the text
    DbgDynamicSentence& sentence = dbgDynSentences_[idx];
    strcpy(sentence.text, text);
    sentence.maxLen = maxLen;
    sentence.drawAtX = drawAtX;
    sentence.drawAtY = drawAtY;

    // add ID for this new sentence
    SentenceID id = staticID_++;
    ids_.push_back(id);

    return id;
}

//---------------------------------------------------------
// Desc:   get buffers and its metadata for rendering
//---------------------------------------------------------
void TextStore::GetRenderingData(
    ID3D11Buffer** outConstVbPtr,    // a ptr to the const vertex buf of const debug text
    ID3D11Buffer** outDynamicVbPtr,  // a ptr to the dynamic vertex buf of dynamic debug text
    ID3D11Buffer** outIbPtr,
    u32& outConstIndexCount,         // index count for debug const sentences
    u32& outDynamicIndexCount)       // index count for debug dynamic sentences
{
    *outConstVbPtr       = vbDbgConstText_.Get();
    *outDynamicVbPtr     = vbDbgDynText_.Get();
    *outIbPtr            = ib_.Get();
    outConstIndexCount   = numIndicesDbgConstText_;
    outDynamicIndexCount = numIndicesDbgDynText_;
}

//---------------------------------------------------------
// Desc:   update the content of the dynamic text
//---------------------------------------------------------
void TextStore::Update(
    ID3D11DeviceContext* pContext,
    const Core::SystemState& sysState)
{
    UpdateDynDbgText(sysState);
    RebuildDynVB(pContext);

    // if we have added a new const string
    if (needUpdConstVB_)
    {
        ID3D11Device* pDevice = nullptr;
        pContext->GetDevice(&pDevice);
        RebuildConstVB(pDevice);

        needUpdConstVB_ = false;
    }
}

//---------------------------------------------------------
// Desc:  update text content of dynamic string by key
//---------------------------------------------------------
void TextStore::UpdateStrByKey(const char* key, const char* fmt, const float val)
{
    assert(key && fmt && "input key or format == nullptr");

    if (char* str = GetDynStr(key))
        snprintf(str, MAX_SENTENCE_LEN, fmt, val);
}

//---------------------------------------------------------

void TextStore::UpdateStrByKey(const char* key, const char* fmt, const int val)
{
    assert(key && fmt && "input key or format == nullptr");

    if (char* str = GetDynStr(key))
        snprintf(str, MAX_SENTENCE_LEN, fmt, val);
}

//---------------------------------------------------------

void TextStore::UpdateStrByKey(const char* key, const char* fmt, const uint32 val)
{
    assert(key && fmt && "input key or format == nullptr");

    if (char* str = GetDynStr(key))
        snprintf(str, MAX_SENTENCE_LEN, fmt, val);
}

//---------------------------------------------------------
// Desc:  update rendering timings
// Args:  - key:        what type of timing we want to update
//        - avgTiming:  timing averaged for last 0.5 seconds
//---------------------------------------------------------
void TextStore::UpdateDynTiming(const char* key, const float avgTiming)
{
    assert(key != nullptr);

    if (char* str = GetDynStr(key))
        snprintf(str, MAX_SENTENCE_LEN, "%05.2f ms", avgTiming);
}

//-----------------------------------------------------
// Desc:  return a ptr to dynamic string by input semantic key
//-----------------------------------------------------
inline char* TextStore::GetDynStr(const char* key)
{
    assert(key && key[0] != '\0');

    for (index i = 0; i < numDbgDynStr_; ++i)
    {
        if (strcmp(dbgDynStrKeys_[i].key, key) == 0)
            return dbgDynSentences_[i].text;
    }

    LogErr(LOG, "there is no debug dynamic string by key: %s", key);
    return nullptr;
}

//-----------------------------------------------------
// Desc:  return an index of dynamic string by input semantic key
//-----------------------------------------------------
inline index TextStore::GetDynStrIdx(const char* key)
{
    assert(key && key[0] != '\0');

    for (index i = 0; i < numDbgDynStr_; ++i)
    {
        if (strcmp(dbgDynStrKeys_[i].key, key) == 0)
            return i;
    }

    LogErr(LOG, "there is no debug dynamic string by key: %s", key);
    return -1;
}

//---------------------------------------------------------
// update text content of each dynamic debug string
//---------------------------------------------------------
void TextStore::UpdateDynDbgText(const Core::SystemState& sysState)
{
    using enum Core::eRenderTimingType;

    // NOTE:
    //       rnd  - rendered (number of)
    //       trn  - terrain
    //       inst - instance(s)

    // common frame info
    UpdateStrByKey("fps",             "%d",         sysState.fps);
    UpdateStrByKey("frame_time",      "%05.2fms",   sysState.frameTime);
    UpdateStrByKey("update_time",     "%05.2fms",   sysState.updateTime);
    UpdateStrByKey("update_time_avg", "(%05.2fms)", sysState.updateTimeAvg);
    UpdateStrByKey("rnd_time",        "%05.2fms",   sysState.msRenderTimings[RND_TIME_FULL_FRAME]);

    // render timings
    UpdateDynTiming("rnd_time_scene_grass",       sysState.msRenderTimingsAvg[RND_TIME_GRASS]);
    UpdateDynTiming("rnd_time_scene_masked",      sysState.msRenderTimingsAvg[RND_TIME_MASKED]);
    UpdateDynTiming("rnd_time_scene_opaque",      sysState.msRenderTimingsAvg[RND_TIME_OPAQUE]);
    UpdateDynTiming("rnd_time_scene_skinned",     sysState.msRenderTimingsAvg[RND_TIME_SKINNED_MODELS]);
    UpdateDynTiming("rnd_time_scene_terrain",     sysState.msRenderTimingsAvg[RND_TIME_TERRAIN]);
    UpdateDynTiming("rnd_time_scene_sky",         sysState.msRenderTimingsAvg[RND_TIME_SKY]);
    UpdateDynTiming("rnd_time_scene_sky_plane",   sysState.msRenderTimingsAvg[RND_TIME_SKY_PLANE]);
    UpdateDynTiming("rnd_time_scene_blended",     sysState.msRenderTimingsAvg[RND_TIME_BLENDED]);
    UpdateDynTiming("rnd_time_scene_transparent", sysState.msRenderTimingsAvg[RND_TIME_TRANSPARENT]);
    UpdateDynTiming("rnd_time_scene_particles",   sysState.msRenderTimingsAvg[RND_TIME_PARTICLE]);
    UpdateDynTiming("rnd_time_scene_weapon",      sysState.msRenderTimingsAvg[RND_TIME_WEAPON]);
    UpdateDynTiming("rnd_time_scene_dbg_shapes",  sysState.msRenderTimingsAvg[RND_TIME_DBG_SHAPES]);
    UpdateDynTiming("rnd_time_post_fx",           sysState.msRenderTimingsAvg[RND_TIME_POST_FX]);

    UpdateDynTiming("rnd_scene_time",             sysState.msRenderTimingsAvg[RND_TIME_3D_SCENE]);
    UpdateDynTiming("rnd_render_ui",              sysState.msRenderTimingsAvg[RND_TIME_UI]);


    // pos info
    UpdateStrByKey("pos_x", "%.2f", sysState.cameraPos.x);
    UpdateStrByKey("pos_y", "%.2f", sysState.cameraPos.y);
    UpdateStrByKey("pos_z", "%.2f", sysState.cameraPos.z);

    // rotation info
    UpdateStrByKey("dir_x", "%.2f", sysState.cameraDir.x);
    UpdateStrByKey("dir_y", "%.2f", sysState.cameraDir.y);
    UpdateStrByKey("dir_z", "%.2f", sysState.cameraDir.z);

    // terrain render info
    UpdateStrByKey("rnd_trn_patch",  "%u", sysState.numDrawnTerrainPatches);
    UpdateStrByKey("cull_trn_patch", "%u", sysState.numCulledTerrainPatches);

    // geometry render info
    UpdateStrByKey("rnd_verts",       "%u", sysState.numDrawnAllVerts);
    UpdateStrByKey("rnd_tris",        "%u", sysState.numDrawnAllTris);
    UpdateStrByKey("rnd_inst",        "%u", sysState.numDrawnEnttsInstances);
    UpdateStrByKey("inst_draw_calls", "%u", sysState.numDrawCallsEnttsInstances);

    // lights info
    UpdateStrByKey("num_vis_pointL", "%u", sysState.numVisiblePointLights);
    UpdateStrByKey("num_vis_spotL",  "%u", sysState.numVisibleSpotlights);
}

} // namespace UI
