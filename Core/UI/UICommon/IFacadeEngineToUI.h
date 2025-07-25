// =================================================================================
// Filename:      IFacadeEngineToUI.h
// 
// Description:   the Facade class provides an interface to the complex
//                logic of the sereral engine subsystem (render, textures, ECS, etc.)
//                The Facade delegates the client requests to the appropritate
//                objects withing the subsystem. All of this shields the client 
//                from the undesired complexity of the subsystem.
// 
// Created:       31.12.24
// =================================================================================
#pragma once

#include <Types.h>
#include "Color.h"
#include "Vectors.h"
#include <cvector.h>

#include <string>
#include <d3d11.h>
#include <DirectXCollision.h>


namespace UI
{
//
// TYPEDEFS
// 
using SRV = ID3D11ShaderResourceView;


enum eEnttComponentType : uint8_t
{
    NameComponent,                 // REQUIRED: attach some name for the entity
    TransformComponent,            // REQUIRED: set that entity has properties: position (x,y,z), direction (vec3), and scale (uniform)
    MoveComponent,                 // set that this entity must be transformed over the time using some transformation matrix (for instance rotate around itself and go into some particular direction)
    RenderedComponent,             // set that this entity is renderable (preferably it is a model), set that this entity must be rendered with particular kind of shader, maybe with specific primitive topology
    ModelComponent,                // attach to entity a 2D/3D model by ID

    CameraComponent,               // attach to entity a camera
    MaterialComponent,
    TextureTransformComponent,     // set that texture has some kind of transformation (maybe it is a translation over some atlas texture so we create an animation, or rotation around texture center -- creates a rotating fireball)
    LightComponent,                // attach to entity some type of light source (directed, point, spotlight, etc.)
    RenderStatesComponent,         // for using different render states: blending, alpha clipping, fill mode, cull mode, etc.
    BoundingComponent,             // for using AABB, OBB, bounding spheres


    // NOT IMPLEMENTED YET
    AIComponent,
    HealthComponent,
    DamageComponent,
    EnemyComponent,
    ColliderComponent,

    PhysicsTypeComponent,
    VelocityComponent,
    GroundedComponent,
    CollisionComponent,

    NUM_COMPONENTS
};

///////////////////////////////////////////////////////////

struct MaterialData
{
    MaterialID id = 0;

    Vec4   ambient  = { 1,1,1,1 };
    Vec4   diffuse  = { 1,1,1,1 };
    Vec4   specular = { 0,0,0,1 };                              // w-component is a specPower (specular power)
    Vec4   reflect  = { .5f, .5f, .5f, 1 };

    char     name[MAX_LENGTH_MATERIAL_NAME]{ '\0' };
    TexID    textureIDs[NUM_TEXTURE_TYPES]{ INVALID_TEXTURE_ID };
    uint32_t properties = 0;                                      // bitfield for materials properties
};

///////////////////////////////////////////////////////////

class IFacadeEngineToUI
{
public:
    ID3D11ShaderResourceView*          pMaterialBigIcon_ = nullptr; // big material icon for browsing/editing particular chosen material
    cvector<ID3D11ShaderResourceView*> materialIcons_;              // list of icons in the editor material browser
    float                              deltaTime = 0.0f;

    virtual ~IFacadeEngineToUI() {};

    virtual ModelID GetModelIdByName(const std::string& name) { return 0; }


    // =============================================================================
    // get/set camera properties
    // =============================================================================
    virtual void GetCameraViewAndProj(const EntityID camEnttID, float* view, float* proj) { assert(0 && "TODO: implement this virtual method in children"); }
    virtual void FocusCameraOnEntity(const EntityID id)                                   { assert(0 && "TODO: implement this virtual method in children"); }
    

    // =============================================================================
    // common methods for entity editor
    // =============================================================================
    virtual EntityID CreateEntity() { return 0; }

    virtual bool GetEntityAddedComponentsNames(const EntityID id, cvector<std::string>& componentsNames)               const { return false; }
    virtual bool GetEntityAddedComponentsTypes(const EntityID id, cvector<eEnttComponentType>& componentTypes)         const { return false; }

    virtual bool AddNameComponent     (const EntityID id, const std::string& name)                                          { return false; }
    virtual bool AddTransformComponent(const EntityID id, const Vec3& pos, const Vec3& direction, const float uniformScale) { return false; }
    virtual bool AddModelComponent    (const EntityID enttID, const uint32_t modelID)                                       { return false; }
    virtual bool AddRenderedComponent (const EntityID enttID)                                                               { return false; }
    virtual bool AddBoundingComponent (const EntityID id, const int boundType, const DirectX::BoundingBox& aabb)            { return false; }


    virtual bool     GetAllEnttsIDs   (const EntityID*& outPtrToEnttsIDsArr, int& outNumEntts)        const { return false; }
    virtual EntityID GetEnttIdByName  (const std::string& name)                                       const { return 0; }
    virtual bool     GetEnttNameById  (const EntityID id, std::string& outName)                       const { return false; }

    // extract entities with particular component
    virtual bool GetEnttsOfModelType  (const EntityID*& enttsIDs, int& numEntts)                             { return false; }
    virtual bool GetEnttsOfCameraType (const EntityID*& enttsIDs, int& numEntts)                             { return false; }
    virtual bool GetEnttsOfLightType  (const EntityID*& enttsIDs, int& numEntts)                             { return false; }

    virtual bool GetEnttTransformData (const EntityID id, Vec3& pos, Vec3& direction, float& uniScale) const { return false; }
    virtual bool GetEnttWorldMatrix   (const EntityID id, DirectX::XMMATRIX& outMat)                   const { return false; }

    // get/set entity position/direction/uniform_scale
    virtual Vec3  GetEnttPosition     (const EntityID id)                                               const { return GetInvalidVec3(); }
    virtual Vec3  GetEnttDirection    (const EntityID id)                                               const { return GetInvalidVec3(); }
    virtual float GetEnttScale        (const EntityID id)                                               const { return GetInvalidFloat(); }

    virtual bool SetEnttPosition  (const EntityID id, const Vec3& pos)                                    { return false; }
    virtual bool SetEnttDirection (const EntityID id, const Vec3& dir)                                    { return false; }
    virtual bool SetEnttUniScale  (const EntityID id, const float scale)                                  { return false; }

    virtual bool RotateEnttByQuat(const EntityID id, const Vec4& rotQuat) { return false; }

    virtual bool GetEnttLightType(const EntityID id, int& lightType)                                 const { return false; }


    // =============================================================================
    // get all the data of light entity by ID
    // =============================================================================
    virtual bool GetEnttDirectedLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular)
    {
        return false;
    }

    virtual bool GetEnttPointLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& attenuation,
        float& range)
    {
        return false;
    }

    virtual bool GetEnttSpotLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& attenuation,
        float& range,
        float& spotExponent)
    {
        return false;
    }


    // =============================================================================
    // set/get directed light props
    // =============================================================================
    virtual bool SetDirectedLightAmbient  (const EntityID id, const ColorRGBA& rgba)    { return false; }
    virtual bool SetDirectedLightDiffuse  (const EntityID id, const ColorRGBA& rgba)    { return false; }
    virtual bool SetDirectedLightSpecular (const EntityID id, const ColorRGBA& rgba)    { return false; }
    virtual bool SetDirectedLightDirection(const EntityID id, const Vec3& dir)          { return false; }

    virtual ColorRGBA GetDirectedLightAmbient  (const EntityID id)                const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetDirectedLightDiffuse  (const EntityID id)                const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetDirectedLightSpecular (const EntityID id)                const { return GetInvalidRGBA(); }
    virtual Vec3      GetDirectedLightDirection(const EntityID id)                const { return GetInvalidVec3(); }


    // =============================================================================
    // set/get point light props
    // =============================================================================
    virtual bool SetPointLightAmbient    (const EntityID id, const ColorRGBA& color)    { return false; }
    virtual bool SetPointLightDiffuse    (const EntityID id, const ColorRGBA& color)    { return false; }
    virtual bool SetPointLightSpecular   (const EntityID id, const ColorRGBA& color)    { return false; }
    virtual bool SetPointLightPos        (const EntityID id, const Vec3& pos)           { return false; }
    virtual bool SetPointLightRange      (const EntityID id, const float range)         { return false; }
    virtual bool SetPointLightAttenuation(const EntityID id, const Vec3& attenuation)   { return false; }

    virtual ColorRGBA GetPointLightAmbient    (const EntityID id)                 const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetPointLightDiffuse    (const EntityID id)                 const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetPointLightSpecular   (const EntityID id)                 const { return GetInvalidRGBA(); }
    virtual Vec3      GetPointLightPos        (const EntityID id)                 const { return GetInvalidVec3(); }
    virtual Vec3      GetPointLightAttenuation(const EntityID id)                 const { return GetInvalidVec3(); }
    virtual float     GetPointLightRange      (const EntityID id)                 const { return GetInvalidFloat(); }


    // =============================================================================
    // set/get spotlight props
    // =============================================================================
    virtual bool SetSpotLightAmbient     (const EntityID id, const ColorRGBA& color)  { return false; };
    virtual bool SetSpotLightDiffuse     (const EntityID id, const ColorRGBA& color)  { return false; };
    virtual bool SetSpotLightSpecular    (const EntityID id, const ColorRGBA& color)  { return false; };
    virtual bool SetSpotLightPos         (const EntityID id, const Vec3& pos)         { return false; };
    virtual bool SetSpotLightDirection   (const EntityID id, const Vec3& direction)   { return false; };
    virtual bool SetSpotLightAttenuation (const EntityID id, const Vec3& att)         { return false; };
    virtual bool SetSpotLightRange       (const EntityID id, const float range)       { return false; };
    virtual bool SetSpotLightSpotExponent(const EntityID id, const float spotExp)     { return false; };

    virtual ColorRGBA GetSpotLightAmbient (const EntityID id)                   const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetSpotLightDiffuse (const EntityID id)                   const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetSpotLightSpecular(const EntityID id)                   const { return GetInvalidRGBA(); }
    virtual Vec3 GetSpotLightPos          (const EntityID id)                   const { return GetInvalidVec3(); }
    virtual Vec3 GetSpotLightDirection    (const EntityID id)                   const { return GetInvalidVec3(); }
    virtual Vec3 GetSpotLightAttenuation  (const EntityID id)                   const { return GetInvalidVec3(); }
    virtual float GetSpotLightRange       (const EntityID id)                   const { return GetInvalidFloat(); }
    virtual float GetSpotLightSpotExponent(const EntityID id)                   const { return GetInvalidFloat(); }


    // =============================================================================
    // get/set sky props
    // =============================================================================
    virtual bool GetSkyData(const uint32_t skyEnttID, ColorRGB& center, ColorRGB& apex, Vec3& offset) { return false; };

    virtual bool SetSkyColorCenter(const ColorRGB& color) { return false; }
    virtual bool SetSkyColorApex(const ColorRGB& color) { return false; }
    virtual bool SetSkyOffset(const Vec3& offset) { return false; }
    virtual bool SetSkyTexture(const int idx, const uint32_t textureID) { return false; }


    // =============================================================================
    // get/set fog props
    // =============================================================================
    virtual bool GetFogData(ColorRGB& fogColor, float& fogStart, float& fogRange, bool& fogEnabled) { return false; }

    virtual bool SetFogStart  (const float start)     { return false; }
    virtual bool SetFogRange  (const float range)     { return false; }
    virtual bool SetFogEnabled(const bool enabled)    { return false; }
    virtual bool SetFogColor  (const ColorRGB& color) { return false; }
    

    // =============================================================================
    // for the debug
    // =============================================================================
    virtual bool SwitchDebugState(const int debugType) { return false; }


    // =============================================================================
    // for assets managers: models/textures/materials/sounds/etc.
    // =============================================================================
    virtual bool GetModelsNamesList  (cvector<std::string>& names)                                  const { return false; }
    virtual bool GetTextureIdByIdx   (const index idx, TexID& outTextureID)                         const { return false; }
    virtual bool GetMaterialIdByIdx  (const index idx, MaterialID& outMatID)                        const { return false; }
    virtual bool GetMaterialNameById (const MaterialID id, char** outName, const int nameMaxLength) const { return false; }
    virtual bool GetNumMaterials     (size& numMaterials)                                           const { return false; }

    virtual bool GetMaterialDataById(const MaterialID id, MaterialData& outData) const { return false; }

    virtual bool SetMaterialColorData(
        const MaterialID id,
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,                      // vec3(specular_color) + float(specular_power)
        const Vec4& reflect) { return false; }

    // get SRV (shader resource view)
    virtual bool GetSRVByTexID      (const TexID textureID, SRV*& pSRV)                   const { return false; }  // get SRV of a single texture by its ID
    virtual bool GetArrTexturesSRVs (SRV**& outArrShaderResourceViews, size& outNumViews) const { return false; }  // get array of all the textures SRVs
    virtual bool GetArrMaterialsSRVs(SRV**& outArrShaderResourceViews, size& outNumViews) const { return false; }  // get array of SRVs which contains visualization of materials (sphere + material)

    virtual bool SetEnttMaterial(
        const EntityID enttID,
        const SubsetID subsetID,
        const MaterialID matID) { return false; }

    virtual bool RenderMaterialsIcons(
        const int startMaterialIdx,            // render materials of some particular range [start, start + num_of_views]
        const size numIcons,
        const int iconWidth,
        const int iconHeight) { return false; }

    virtual bool RenderMaterialBigIconByID(
        const MaterialID matID,
        const int iconWidth,
        const int iconHeight,
        const float yAxisRotation) { return false; }

private:
    inline float     GetInvalidFloat() const { return NAN; }
    inline Vec3      GetInvalidVec3()  const { return { NAN, NAN, NAN }; }
    inline Vec4      GetInvalidVec4()  const { return { NAN, NAN, NAN, NAN }; }
    inline ColorRGBA GetInvalidRGB()   const { return { NAN, NAN, NAN, NAN }; }
    inline ColorRGBA GetInvalidRGBA()  const { return { NAN, NAN, NAN, NAN }; }
};

} // namespace UI
