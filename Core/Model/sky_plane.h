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
#include <types.h>
#include <Mesh/vertex.h>
#include <Mesh/vertex_buffer.h>
#include <Mesh/index_buffer.h>


namespace Core
{

class SkyPlane
{
public:

    // initialization data:
    //  - skyPlaneResolution:  how many quads we have along X and Z axis
    //  - texRepeat:           how many times to repeat a texture over the sky plane
    //  - skyPlaneLength:      length of the plane
    //  - skyPlaneTop:         1 upper central point of the plane
    //  - skyPlaneBottom:      4 bottom corner points of the plane
    //  - brightness:          cloud brightness, lower values give clouds more faded look.
    //  - translationSpeed:    arr of 4 values, how fast we translate the cloud textures over the sky plane
    //  - materialName:        a name of material used for the sky plane
    struct SkyPlaneParams
    {
        int   skyPlaneResolution;
        int   texRepeat;
        float skyPlaneLength;
        float skyPlaneTop;
        float skyPlaneBottom;
        float brightness;
        float translationSpeed[4];
        char  materialName[MAX_LEN_MAT_NAME];
    };

public:
    SkyPlane();
    ~SkyPlane();

    // restrict move constructor/assignment
    SkyPlane(SkyPlane&& rhs) noexcept = delete;
    SkyPlane& operator=(SkyPlane&& rhs) noexcept = delete;

    // restrict shallow copying
    SkyPlane(const SkyPlane&) = delete;
    SkyPlane& operator=(const SkyPlane&) = delete;


    bool Init(const SkyPlaneParams& params);
    void Shutdown();
    void Update(const float dt);

    MaterialID GetMaterialId() const;
    int        GetNumIndices() const;
    float      GetBrightness() const;

    const VertexBuffer<VertexPosTex>& GetVB() const;
    const IndexBuffer<uint16>&        GetIB() const;

    float GetTranslation(const int i) const;


private:
    bool InitBuffers();

private:
    MaterialID matId_ = INVALID_MAT_ID;
    char       name_[MAX_LEN_SKY_MODEL_NAME] = "sky_plane";

    // clouds params
    float      brightness_ = 0.0f;
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

//---------------------------------------------------------
// INLINE FUNCTIONS
//---------------------------------------------------------

inline MaterialID SkyPlane::GetMaterialId() const { return matId_; }
inline int        SkyPlane::GetNumIndices() const { return numIndices_; }
inline float      SkyPlane::GetBrightness() const { return brightness_; }

inline const VertexBuffer<VertexPosTex>& SkyPlane::GetVB() const
{
    return vb_;
}

inline const IndexBuffer<uint16>& SkyPlane::GetIB() const
{
    return ib_;
}

inline float SkyPlane::GetTranslation(const int i) const
{
    assert(i >= 0 && i < 4);
    return textureTranslation_[i];
}

} // namespace
