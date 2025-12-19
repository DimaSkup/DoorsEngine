// =================================================================================
// Filename:      SkyModel.h
// Description:   a class for the sky model (because it is a specific model)
// 
// Created:       23.12.24
// =================================================================================
#pragma once

#include "../Mesh/vertex.h"
#include "../Mesh/vertex_buffer.h"
#include "../Mesh/index_buffer.h"

#include <d3d11.h>


namespace Core
{

class SkyModel
{
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
    bool InitializeBuffers(
        ID3D11Device* pDevice,
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

    inline const DirectX::XMFLOAT3& GetColorCenter()  const { return colorCenter_; }
    inline const DirectX::XMFLOAT3& GetColorApex()    const { return colorApex_; }


    //
    // Setters
    //
    void SetMaterialId(const MaterialID id);
    void SetName(const char* name);

    inline void SetColorCenter(const DirectX::XMFLOAT3& c) { colorCenter_ = c; }
    inline void SetColorApex  (const DirectX::XMFLOAT3& c) { colorApex_ = c; }


private:
    MaterialID                materialId_ = INVALID_MATERIAL_ID;
    char                      name_[MAX_LEN_SKY_MODEL_NAME] = "sky_model";

    VertexBuffer<Vertex3DPos> vb_;
    IndexBuffer<USHORT>       ib_;

    // gradient params
    DirectX::XMFLOAT3 colorCenter_{1,1,1};   // sky horizon color
    DirectX::XMFLOAT3 colorApex_{1,1,1};     // sky top color
};

} // namespace Core
