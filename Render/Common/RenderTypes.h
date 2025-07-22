// ********************************************************************************
// Filename:     RenderTypes.h
// Description:  some common types for the Render module
// 
// Created:      17.10.24
// ********************************************************************************
#pragma once

#include "MaterialLightTypes.h"

#include <MemHelpers.h>
#include <CAssert.h>
#include <Types.h>
#include <log.h>
#include <cvector.h>

#include <d3d11.h>
#include <DirectXMath.h>


#pragma warning (disable : 4996)

namespace Render
{

constexpr int SUBSET_NAME_LENGTH_LIMIT = 32;


// =================================================================================
// TYPEDEFS
// =================================================================================
using SRV = ID3D11ShaderResourceView;


// =================================================================================
// ENUMS
// =================================================================================
enum eDebugState
{
    DBG_TURN_OFF,               // turn off the debug shader and use the default shader
    DBG_SHOW_NORMALS,
    DBG_SHOW_TANGENTS,
    DBG_SHOW_BUMPED_NORMALS,
    DBG_SHOW_ONLY_LIGTHING,
    DBG_SHOW_ONLY_DIRECTED_LIGHTING,
    DBG_SHOW_ONLY_POINT_LIGHTING,
    DBG_SHOW_ONLY_SPOT_LIGHTING,
    DBG_SHOW_ONLY_DIFFUSE_MAP,
    DBG_SHOW_ONLY_NORMAL_MAP,
    DBG_WIREFRAME,
};

enum EnttsSetType
{
    DEFAULT,                // fill solid, cull back, no blending, no alpha clipping
    ALPHA_CLIP_CULL_NONE,
    BLENDED,
    ANOTHER
};


// =================================================================================
// STRUCTURES
// =================================================================================
struct PerFrameData
{
    // constains data which is updated each frame

    // common data
    DirectX::XMMATRIX WVO;                       // is used for 2D rendering (world * basic_view * ortho)
    DirectX::XMMATRIX proj;
    DirectX::XMMATRIX viewProj;                  // (is already transposed)
    DirectX::XMFLOAT3 cameraPos;
    DirectX::XMFLOAT3 cameraDir;

    // light data
    DirLight*         dirLights   = nullptr;     // directional light sources
    PointLight*       pointLights = nullptr;
    SpotLight*        spotLights  = nullptr;
    int               numDirLights   = 0;        // number of directional light src
    int               numPointLights = 0;
    int               numSpotLights  = 0;

    float             deltaTime      = 0;        // time passed since the previous frame
    float             totalGameTime  = 0;        // time passed since the start of the application


    void ResizeLightData(
        const int nDirLights,                    // number of dir lights
        const int nPointLights,
        const int nSpotLights)
    {
        // do we need a reallocation for dir lights?
        if (nDirLights > numDirLights)
        {
            SafeDeleteArr(dirLights);
            dirLights = new DirLight[nDirLights];
        }
        numDirLights = nDirLights;


        // do we need a reallocation for point lights?
        if (nPointLights > numPointLights)
        {
            SafeDeleteArr(pointLights);
            pointLights = new PointLight[nPointLights];
        }
        numPointLights = nPointLights;

        // do we need a reallocation for spot lights?
        if (nSpotLights > numSpotLights)
        {
            SafeDeleteArr(spotLights);
            spotLights = new SpotLight[nSpotLights];
        }
        numSpotLights = nSpotLights;
    }
};

///////////////////////////////////////////////////////////

class InstBuffData
{
    //
    // constains transient data for the instance buffer
    //

public:

    InstBuffData() {}

    ~InstBuffData()
    {
        Shutdown();
    }

    /// ---------------------------------------------------

    void Shutdown()
    {
        SafeDeleteArr(worlds_);
        SafeDeleteArr(texTransforms_);
        SafeDeleteArr(materials_);
        capacity_ = 0;
        size_ = 0;
    }

    /// ---------------------------------------------------

    void Resize(const int newSize)
    {
        try
        {
            if (newSize < 0)
            {
                sprintf(g_String, "wrong value of new size: %d", newSize);
                throw EngineException(g_String);
            }

            // if we need a reallocation (or just do nothing if we have enough memory)
            if (newSize > capacity_)
            {
                // release old memory before allocation of new memory
                Shutdown();

                worlds_             = new DirectX::XMMATRIX[newSize];
                texTransforms_      = new DirectX::XMMATRIX[newSize];
                materials_          = new Material[newSize];
                capacity_           = newSize;	
            }

            // update the number of elements for this frame
            size_ = newSize;
        }
        catch (const std::bad_alloc& e)
        {
            Shutdown();
            LogErr(e.what());
            LogErr("can't allocate memory for the instanced transient data buffer");
        }
        catch (EngineException& e)
        {
            Shutdown();
            LogErr(e);
            LogErr("can't setup instanced transient data buffer");
        }
    }

    // ---------------------------------------------------

    inline const int GetSize() const { return size_; }

public:
    DirectX::XMMATRIX* worlds_ = nullptr;
    DirectX::XMMATRIX* texTransforms_ = nullptr;
    Material*          materials_ = nullptr;

private:
    int                capacity_ = 0;   // how many elements we can put into this buffer
    int                size_ = 0;       // the current number of data elements
};

///////////////////////////////////////////////////////////

struct Subset
{
    // subset (mesh) data of the model
    Subset() {}

    char     name[SUBSET_NAME_LENGTH_LIMIT] {'\0'};  // for debugging
    uint32_t vertexStart = 0;                        // start pos of vertex in the common buffer
    uint32_t vertexCount = 0;                        // how many vertices this subset has
    uint32_t indexStart = 0;                         // start pos of index in the common buffer
    uint32_t indexCount = 0;                         // how many indices this subset has
};

///////////////////////////////////////////////////////////

struct Instance
{
    // data of a single model instance
    Instance() {}

    char                name[32]{ '\0' };
    int                 numInstances = 0;  // how many instances will be rendered
    UINT                vertexStride = 0;  // size in bytes of a single vertex

    ID3D11Buffer*       pVB = nullptr;     // vertex buffer
    ID3D11Buffer*       pIB = nullptr;     // index buffer
    cvector<SRV*>       texSRVs;           // textures arr for each mesh
    cvector<Subset>     subsets;           // subInstance (mesh) data
    cvector<uint32_t>   materialIDs;
    

    // --------------------------------

    inline int GetNumVertices() const
    {
        return subsets.back().vertexStart + subsets.back().vertexCount;
    }

    // --------------------------------
    
    inline void Clear()
    {
        pVB = nullptr;
        pIB = nullptr;
        texSRVs.clear();
        subsets.clear();
        materialIDs.clear();
    }
};

///////////////////////////////////////////////////////////

struct SkyInstance
{
    // data of a sky model instance

    UINT              indexCount = 0;
    UINT              vertexStride = 0;             // size in bytes of a single vertex
    ID3D11Buffer*     pVB = nullptr;                // vertex buffer
    ID3D11Buffer*     pIB = nullptr;                // index buffer
    SRV*              texSRVs[NUM_TEXTURE_TYPES];   // textures arr
    DirectX::XMFLOAT3 colorCenter;                  // horizon sky color (for gradient)
    DirectX::XMFLOAT3 colorApex;                    // top sky color (for gradient)
};

///////////////////////////////////////////////////////////

struct TerrainInstance
{
    // data of the terrain model
    UINT          numVertices   = 0;
    UINT          indexCount    = 0;
    UINT          baseIndex     = 0;
    UINT          baseVertex    = 0;
    UINT          vertexStride  = 0;
    ID3D11Buffer* pVB           = nullptr;
    ID3D11Buffer* pIB           = nullptr;
    SRV*          skyBoxTexture = nullptr;
    SRV*          textures[NUM_TEXTURE_TYPES]{nullptr};
    Material      material;
    bool          wantDebug = false;
};

///////////////////////////////////////////////////////////

struct RenderDataStorage
{
    // stores render data of bunches of instances with different render states

    void Clear()
    {
        modelInstances.clear();
        alphaClippedModelInstances.clear();
        blendedModelInstances.clear();
        boundingLineBoxInstances.clear();
    }

    InstBuffData          modelInstBuffer;
    InstBuffData          alphaClippedModelInstBuffer;
    InstBuffData          blendedModelInstBuffer;
    InstBuffData          boundingLineBoxBuffer;

    cvector<Instance> modelInstances;              // models with default render states
    cvector<Instance> alphaClippedModelInstances;
    cvector<Instance> blendedModelInstances;
    cvector<Instance> boundingLineBoxInstances;
};

///////////////////////////////////////////////////////////


}  // namespace
