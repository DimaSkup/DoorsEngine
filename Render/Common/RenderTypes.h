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

// =================================================================================
// DEFINES / TYPEDEFS / CONSTANTS
// =================================================================================
using SRV = ID3D11ShaderResourceView;
constexpr int SUBSET_NAME_LENGTH_LIMIT = 32;


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
    DBG_SHOW_MATERIAL_AMBIENT,
    DBG_SHOW_MATERIAL_DIFFUSE,
    DBG_SHOW_MATERIAL_SPECULAR,
    DBG_SHOW_MATERIAL_REFLECTION,
};


// =================================================================================
// STRUCTURES
// =================================================================================

//---------------------------------------------------------
// Desc:   a container for entities materials (its part: textures + render states)
//---------------------------------------------------------
struct MaterialDataForBinding
{
    uint32 properties;                        // bitfild about render states
    SRV*   texturesSRVs[NUM_TEXTURE_TYPES];   // textures of this material
};

//---------------------------------------------------------
// Desc:   a container for data which is updated each frame
//---------------------------------------------------------
struct PerFrameData
{
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

    //-------------------------------------------

    void ResizeLightData(
        const int nDirLights,                    // number of dir lights
        const int nPointLights,
        const int nSpotLights)
    {
        // do we need a reallocation for dir lights?
        if (nDirLights > numDirLights)
        {
            SafeDeleteArr(dirLights);

            dirLights = NEW DirLight[nDirLights];
            if (!dirLights)
            {
                LogErr(LOG, "can't allocate memory for directed lights data buffer");
            }
        }
        numDirLights = nDirLights;


        // do we need a reallocation for point lights?
        if (nPointLights > numPointLights)
        {
            SafeDeleteArr(pointLights);

            pointLights = NEW PointLight[nPointLights];
            if (!pointLights)
            {
                LogErr(LOG, "can't allocate memory for point lights data buffer");
            }
        }
        numPointLights = nPointLights;

        // do we need a reallocation for spot lights?
        if (nSpotLights > numSpotLights)
        {
            SafeDeleteArr(spotLights);

            spotLights = NEW SpotLight[nSpotLights];
            if (!spotLights)
            {
                LogErr(LOG, "can't allocate memory for spotlights data buffer");
            }
        }
        numSpotLights = nSpotLights;
    }
};

//---------------------------------------------------------
// Desc:   a transient data container for the instances buffer
//---------------------------------------------------------
class InstancesBuf
{
public:
    DirectX::XMMATRIX* worlds_    = nullptr;  
    MaterialColors*    materials_ = nullptr;  // material color data (ambient/diffuse/specular/reflection/etc.)

private:
    int                capacity_ = 0;               // how many elements we can put into this buffer
    int                size_ = 0;                   // the current number of data elements

public:
    InstancesBuf()  {}
    ~InstancesBuf() { Shutdown(); }

    // ---------------------------------------------------

    void Shutdown()
    {
        SafeDeleteArr(worlds_);
        SafeDeleteArr(materials_);
        capacity_ = 0;
        size_ = 0;
    }

    // ---------------------------------------------------

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

                worlds_    = new DirectX::XMMATRIX[newSize];
                materials_ = new MaterialColors[newSize];
                capacity_  = newSize;	
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
};

//---------------------------------------------------------
// Desc:   a container for subset (mesh) data of the model
//---------------------------------------------------------
struct Subset
{
    char    name[SUBSET_NAME_LENGTH_LIMIT] {'\0'};       // for debugging
    uint32  vertexStart = 0;                             // start pos of vertex in the common buffer
    uint32  vertexCount = 0;                             // how many vertices this subset has
    uint32  indexStart = 0;                              // start pos of index in the common buffer
    uint32  indexCount = 0;                              // how many indices this subset has
};

//---------------------------------------------------------
// Desc:   a data container for rendering instances
//         (instance is a set of geometry with some rendering states:
//          it can be a simple box with a single subset/mesh and single material)
//---------------------------------------------------------
struct InstanceBatch
{
    int                    numInstances = 0;             // how many times this instance will be rendered (at different positions)
    UINT                   vertexStride = 0;             // size in bytes of a single vertex

    ID3D11Buffer*          pVB = nullptr;                // vertex buffer
    ID3D11Buffer*          pIB = nullptr;                // index buffer
    Subset                 subset;                       // mesh metadata
    uint32                 renderStates;                 // a bitfield about render states of the current material
    SRV*                   textures[NUM_TEXTURE_TYPES];  // textures of this material

    // debug data
    char                   name[32]{ '\0' };
    SubsetID               subsetId = 1000;


    inline int GetNumVertices() const
    {
        return subset.vertexCount;
    }
};

//---------------------------------------------------------
// Desc:   a data container of a sky model instance
//---------------------------------------------------------
struct SkyInstance
{
    UINT              indexCount = 0;
    UINT              vertexStride = 0;             // size in bytes of a single vertex
    ID3D11Buffer*     pVB = nullptr;                // vertex buffer
    ID3D11Buffer*     pIB = nullptr;                // index buffer
    uint32            renderStates;                 // a bitfield about render states of the current material
    SRV*              texSRVs[NUM_TEXTURE_TYPES];   // textures arr
    DirectX::XMFLOAT3 colorCenter;                  // horizon sky color (for gradient)
    DirectX::XMFLOAT3 colorApex;                    // top sky color (for gradient)
};

//---------------------------------------------------------
// Desc:   a data container for the terrain model
//---------------------------------------------------------
struct TerrainInstance
{
    UINT           numVertices   = 0;
    UINT           indexCount    = 0;
    UINT           baseIndex     = 0;
    UINT           baseVertex    = 0;
    UINT           vertexStride  = 0;
    ID3D11Buffer*  pVB           = nullptr;
    ID3D11Buffer*  pIB           = nullptr;
    //SRV*           skyBoxTexture = nullptr;
    //SRV*           textures[NUM_TEXTURE_TYPES]{nullptr};
    MaterialColors matColors;
    bool           wantDebug = false;
};

//---------------------------------------------------------
// Desc:   stores render data of bunches of instances with different render states
//---------------------------------------------------------
struct RenderDataStorage
{
    void Clear()
    {
        masked.clear();
        opaque.clear();
        blended.clear();
        blendedTransparent.clear();
    }

    InstancesBuf                instancesBuf;

    cvector<InstanceBatch>      masked;                // grass, foliage, wireframe, etc.
    cvector<InstanceBatch>      opaque;                // fully opaque geometry
    cvector<InstanceBatch>      blended;               // blended geometry (add/sub/mul)
    cvector<InstanceBatch>      blendedTransparent;    // blended geometry with transparency
};

}  // namespace
