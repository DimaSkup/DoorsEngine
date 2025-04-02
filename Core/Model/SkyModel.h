// =================================================================================
// Filename:      SkyModel.h
// Description:   a class for the sky model (because it is a specific model)
// 
// Created:       23.12.24
// =================================================================================
#pragma once

#include "../Mesh/Vertex.h"
#include "../Mesh/VertexBuffer.h"
#include "../Mesh/IndexBuffer.h"

#include <d3d11.h>
//#include <DirectXCollision.h>


namespace Core
{

class SkyModel
{
public:
    SkyModel();
    ~SkyModel();

    // move constructor/assignment
    SkyModel(SkyModel&& rhs) noexcept = delete;
    SkyModel& operator=(SkyModel&& rhs) noexcept = delete;

    // restrict shallow copying
    SkyModel(const SkyModel&) = delete;
    SkyModel& operator=(const SkyModel&) = delete;


    // initialize vb/ib with input data
    void InitializeBuffers(
        ID3D11Device* pDevice,
        const Vertex3DPos* vertices,
        const USHORT* indices,
        const int numVertices,
        const int numIndices);

    //
    // Getters
    //
    inline int GetNumVertices()              const { return vb_.GetVertexCount(); };
    inline int GetNumIndices()               const { return ib_.GetIndexCount(); }
    inline int GetVertexStride()             const { return vb_.GetStride(); }
    inline int GetMaxTexturesNum()           const { return maxTexNum_; }

    inline ID3D11Buffer* GetVertexBuffer()   const { return vb_.Get(); }
    inline ID3D11Buffer* GetIndexBuffer()    const { return ib_.Get(); }
    inline const ModelName& GetName()        const { return name_; }
    inline const TexID* GetTexIDs()          const { return texIDs_; }

    inline const DirectX::XMFLOAT3& GetColorCenter()  const { return colorCenter_; }
    inline const DirectX::XMFLOAT3& GetColorApex()    const { return colorApex_; }


    //
    // Setters
    //
    void SetTexture(const int idx, const TexID texID);
    void SetName(const std::string& name);

    inline void SetColorCenter(const DirectX::XMFLOAT3& c) { colorCenter_ = c; }
    inline void SetColorApex  (const DirectX::XMFLOAT3& c) { colorApex_ = c; }


private:
    const int                 maxTexNum_ = 6;                // how many textures the sky can have

    TexID                     texIDs_[6]{0};
    ModelName                 name_ = "sky_model";

    VertexBuffer<Vertex3DPos> vb_;
    IndexBuffer<USHORT>       ib_;

    // gradient params
    DirectX::XMFLOAT3 colorCenter_{1,1,1};   // sky horizon color
    DirectX::XMFLOAT3 colorApex_{1,1,1};     // sky top color
};

} // namespace Core
