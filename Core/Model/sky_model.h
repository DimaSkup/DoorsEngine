// =================================================================================
// Filename:      SkyModel.h
// Description:   skybox (because it is a specific model)
// 
// Created:       23.12.24
// =================================================================================
#pragma once

#include "../Mesh/vertex.h"
#include "../Mesh/vertex_buffer.h"
#include "../Mesh/index_buffer.h"
#include "../Texture/enum_texture_types.h"

namespace Core
{

class SkyModel
{
public:

    // initialization data:
    // - texName:        path to cubemap texture
    // - materialName:   a name of sky material
    // - size:           size of skybox side
    // - bLoadCubeMap:   (true) load prepared cubemap texture or (false) create it manually
    // - offsetY:        offset of the skybox along Y-axis
    // - cubeMapParams:  params for manual creation of the cubemap
    struct SkyBoxParams
    {
        char     texName[MAX_LEN_TEX_NAME]{ '\0' };
        char     materialName[MAX_LEN_MAT_NAME]{ '\0' };
        float    size = 0;
        int      bLoadCubeMap = 0;
        float    offsetY = 0;
        CubeMapInitParams cubeMapParams;
    };


public:
    SkyModel();
    ~SkyModel();

    void Shutdown();

    // move constructor/assignment
    SkyModel(SkyModel&& rhs) noexcept = delete;
    SkyModel& operator=(SkyModel&& rhs) noexcept = delete;

    // restrict shallow copying
    SkyModel(const SkyModel&) = delete;
    SkyModel& operator=(const SkyModel&) = delete;


    // initialize vb/ib with input data
    bool InitBuffers(
        const Vertex3DPos* vertices,
        const USHORT* indices,
        const int numVertices,
        const int numIndices);

    //
    // Getters
    //
    inline int GetNumVertices()                       const { return vb_.GetVertexCount(); };
    inline int GetNumIndices()                        const { return ib_.GetIndexCount(); }
    inline int GetVertexStride()                      const { return vb_.GetStride(); }

    inline const VertexBuffer<Vertex3DPos>& GetVB()   const { return vb_; }
    inline const IndexBuffer<USHORT>&       GetIB()   const { return ib_; }

    inline const char*   GetName()                    const { return name_; }
    inline MaterialID    GetMaterialId()              const { return materialId_; }


    //
    // Setters
    //
    void SetMaterialId(const MaterialID id);
    void SetName(const char* name);

private:
    MaterialID                materialId_ = INVALID_MAT_ID;
    char                      name_[MAX_LEN_SKY_MODEL_NAME] = "skybox";

    VertexBuffer<Vertex3DPos> vb_;
    IndexBuffer<USHORT>       ib_;
};

} // namespace Core
