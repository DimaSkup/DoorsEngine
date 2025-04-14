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

#include <UICommon/Types.h>
#include "Color.h"
#include "Vectors.h"
#include <UICommon/cvector.h>

#include <d3d11.h>
#include <DirectXCollision.h>


namespace UI
{


enum eEnttComponentType : uint8_t
{
    NameComponent,                 // REQUIRED: attach some name for the entity
    TransformComponent,            // REQUIRED: set that entity has properties: position (x,y,z), direction (quaternion), and scale (uniform)
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

class IFacadeEngineToUI
{
public:
    ID3D11ShaderResourceView* pFrameBufTexSRV_ = nullptr;


    virtual ~IFacadeEngineToUI() {};


    virtual ModelID GetModelIdByName(const std::string& name) { return 0; }


    // =============================================================================
    // get/set camera properties
    // =============================================================================
    virtual void GetCameraViewAndProj(const EntityID camEnttID, float* view, float* proj) { assert(0 && "TODO: implement this virtual method in children"); }
    virtual void FocusCameraOnEntity(const EntityID id) { assert(0 && "TODO: implement this virtual method in children"); }
    

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
    virtual EntityID GetEnttIDByName  (const std::string& name)                                       const { return 0; }
    virtual bool     GetEnttNameByID  (const EntityID id, std::string& outName)                       const { return false; }

    // extract entities with particular component
    virtual bool GetEnttsOfModelType  (const EntityID*& enttsIDs, int& numEntts)                            { return false; }
    virtual bool GetEnttsOfCameraType (const EntityID*& enttsIDs, int& numEntts)                            { return false; }
    virtual bool GetEnttsOfLightType  (const EntityID*& enttsIDs, int& numEntts)                            { return false; }

    virtual bool GetEnttTransformData (const EntityID id, Vec3& pos, Vec4& rotQuat, float& uniScale)  const { return false; }
    virtual bool GetEnttWorldMatrix   (const EntityID id, DirectX::XMMATRIX& outMat)                  const { return false; }

    // get/set entity position/rotation_quat/uniform_scale
    virtual Vec3 GetEnttPosition     (const EntityID id)                                               const { return GetInvalidVec3(); }
    virtual Vec4 GetEnttDirectionQuat(const EntityID id)                                              const { return GetInvalidVec4(); }
    virtual float GetEnttScale       (const EntityID id)                                               const { return GetInvalidFloat(); }

    virtual bool SetEnttPosition     (const EntityID id, const Vec3& pos)                                    { return false; }
    virtual bool SetEnttDirectionQuat(const EntityID id, const Vec4& dirQuat)                               { return false; }
    virtual bool SetEnttUniScale     (const EntityID id, const float scale)                                  { return false; }

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

    virtual ColorRGBA GetPointLightAmbient (const EntityID id)                    const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetPointLightDiffuse (const EntityID id)                    const { return GetInvalidRGBA(); }
    virtual ColorRGBA GetPointLightSpecular(const EntityID id)                    const { return GetInvalidRGBA(); }
    virtual Vec3 GetPointLightPos          (const EntityID id)                    const { return GetInvalidVec3(); }
    virtual Vec3 GetPointLightAttenuation  (const EntityID id)                    const { return GetInvalidVec3(); }
    virtual float GetPointLightRange       (const EntityID id)                    const { return GetInvalidFloat(); }


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
    virtual bool GetFogData(ColorRGB& fogColor, float& fogStart, float& fogRange) { return false; }
    virtual bool SetFogParams(const ColorRGB& color, const float start, const float range) { return false; }


    // =============================================================================
    // for the debug
    // =============================================================================
    virtual bool SwitchDebugState(const int debugType) { return false; }


    // =============================================================================
    // for assets: models/textures/sounds/etc.
    // =============================================================================
    virtual bool GetModelsNamesList(cvector<std::string>& names) { return false; }

    virtual bool GetShaderResourceViewByTexID(const TexID textureID, SRV*& pSRV) { return false; }
    virtual bool GetArrShaderResourceViews(SRV**& outArrShadersResourceViews, size& outNumViews)  const { return false; }
    virtual bool GetTextureIdByIdx(const index idx, TexID& outTextureID) const { return false; }

    virtual bool SetEnttMaterial(
        const EntityID enttID,
        const SubsetID subsetID,
        const MaterialID matID) { return false; }

    virtual bool RenderMaterialsIcons(
        ID3D11ShaderResourceView** outArrShaderResourceViews,
        const size numShaderResourceViews,
        const int iconWidth,
        const int iconHeight) { return false; }


private:
    inline float GetInvalidFloat()    const { return NAN; }
    inline Vec3 GetInvalidVec3()      const { return { NAN, NAN, NAN }; }
    inline Vec4 GetInvalidVec4()      const { return { NAN, NAN, NAN, NAN }; }
    inline ColorRGBA GetInvalidRGB()  const { return { NAN, NAN, NAN, NAN }; }
    inline ColorRGBA GetInvalidRGBA() const { return { NAN, NAN, NAN, NAN }; }
};

} // namespace UI
