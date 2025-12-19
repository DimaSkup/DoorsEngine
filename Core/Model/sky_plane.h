/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sky_plane.h
    Desc:     encapsulates everything related to the plane used for render the clouds

    I wrote this code during listening to Egor Letov's song
    called "Hey, brother Luber", so if you like punk rock I recommend you to listen
    to this masterpiece 

    Created:  29.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include <Types.h>
#include <Mesh/vertex.h>
#include <Mesh/vertex_buffer.h>
#include <Mesh/index_buffer.h>


namespace Core
{

class SkyPlane
{
public:
    SkyPlane();
    ~SkyPlane();

    bool Init(const char* configFilename);
    void Shutdown();
    void Update(const float deltaTime);


    inline MaterialID GetMaterialId() const { return matId_; }
    inline int        GetNumIndices() const { return numIndices_; }
    inline float      GetBrightness() const { return brightness_; }

    const VertexBuffer<VertexPosTex>& GetVB() const { return vb_; }
    const IndexBuffer<uint16>&        GetIB() const { return ib_; }

    // we have only 4 translation values so if input idx is > 4
    // we return translation by index 0
    inline float GetTranslation(const int i) const
    {
        assert(i >= 0 && i < 4);
        return textureTranslation_[i];
    }

    // restrict move constructor/assignment
    SkyPlane(SkyPlane&& rhs) noexcept = delete;
    SkyPlane& operator=(SkyPlane&& rhs) noexcept = delete;

    // restrict shallow copying
    SkyPlane(const SkyPlane&) = delete;
    SkyPlane& operator=(const SkyPlane&) = delete;

private:
    bool InitBuffers();

private:
    MaterialID matId_ = INVALID_MATERIAL_ID;
    char       name_[MAX_LEN_SKY_MODEL_NAME] = "sky_plane";

    // clouds params
    float      brightness_ = 1.0f;
    float      textureTranslation_[4]{0};    // current translation values
    float      translationSpeed_[4]{0};

    // geometry
    VertexPosTex* vertices_ = nullptr;
    uint16*       indices_ = nullptr;

    int numVertices_ = 0;
    int numIndices_  = 0;
    UINT vertexStride_ = 0;

    VertexBuffer<VertexPosTex>  vb_;
    IndexBuffer<uint16>         ib_;
};

} // namespace
