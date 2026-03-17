#include "../Common/pch.h"
#include "light_initializer.h"
#include <parse_helpers.h>              // helpers for parsing string buffers, or reading data from file
#include <DirectXMath.h>

#include "quad_tree_attach_control.h"

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;
using DirectX::XMVECTOR;

namespace Game
{

//---------------------------------------------------------
// Helper structure for light sources initialization
//---------------------------------------------------------
struct LightSrcInitParams
{
    char enttName[MAX_LEN_ENTT_NAME]{'\0'};
    char parentEnttName[MAX_LEN_ENTT_NAME]{'\0'};
    char archetype[32]{'\0'};

    ECS::LightType  lightType = ECS::NUM_LIGHT_TYPES;
    XMFLOAT3        pos       = { 0,0,0 };            // position
    XMFLOAT3        dir       = { 0,0,0 };            // direction
    XMFLOAT4        dirQuat   = { 0,0,0,1 };          // direction quaternion
    int             isActive  = true;                 // 0 - inactive, 1 - active

    void Reset()
    {
        memset(enttName, '\0', MAX_LEN_ENTT_NAME);
        memset(parentEnttName, '\0', MAX_LEN_ENTT_NAME);
        memset(archetype, '\0', 32);

        pos = { 0,0,0 };
        dir = { 0,0,0 };
        dirQuat = { 0,0,0,1 };
        lightType = ECS::NUM_LIGHT_TYPES;
        isActive = true;
    }
};

//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void InitDirectedLightEntt(ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params);
void InitPointLightEntt   (ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params);
void InitSpotlightEntt    (ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params);

//---------------------------------------------------------
// Desc:  load light entities params from file and create these entities
// Args:  - filepath:  a path to file with definitions of light sources (entities)
//---------------------------------------------------------
bool LightInitializer::Init(const char* filepath, ECS::EntityMgr& mgr)
{
    LogMsg(LOG, "Initialize light entities");

    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "empty filename");
        return false;
    }

    FILE* pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file: %s", filepath);
        return false;
    }

    char buf[128]{'\0'};
    LightSrcInitParams initParams;

    while (fgets(buf, sizeof(buf), pFile))
    {
        // we always expect declaration of new entity after reading a line
        if (buf[0] != 'n')
        {
            LogErr(LOG, "buffer doesn't contain declaration of newentt: %s", buf);
            LogErr(LOG, "interrupt creation of light sources");
            return false;
        }

        // read in a name and light type for a new entity
        char typeName[32]{ '\0' };
        ReadStr    (buf,   "newentt %s", initParams.enttName);
        ReadFileStr(pFile, "type", typeName);


        if (typeName[0] == 'd')
            InitDirectedLightEntt(mgr, pFile, initParams);

        else if (typeName[0] == 'p')
            InitPointLightEntt(mgr, pFile, initParams);

        else if (typeName[0] == 's')
            InitSpotlightEntt(mgr, pFile, initParams);

        else
        {
            LogErr(LOG, "uknown type of light: %s", typeName);
            LogErr(LOG, "interrupt creation of light sources");
            return false;
        }

        // after all reset initial params for the following entity
        initParams.Reset();

    } // while fgets

    fclose(pFile);
    return true;
}


//---------------------------------------------------------
// Desc:  read params from file, create and setup a new directed light entity
//---------------------------------------------------------
void InitDirectedLightEntt(ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params)
{
    assert(pFile);
    printf("\tinit directed light: %s\n", params.enttName);

    ECS::DirLight light;

    // read in params from file
    ReadFileFloat4(pFile, "ambient",   &light.ambient.x);
    ReadFileFloat4(pFile, "diffuse",   &light.diffuse.x);
    ReadFileFloat4(pFile, "specular",  &light.specular.x);
    ReadFileFloat3(pFile, "direction", &params.dir.x);
    ReadFileInt   (pFile, "active",    &params.isActive);
    ReadFileStr   (pFile, "archetype",  params.archetype);

    if (strcmp(params.archetype, "light") != 0)
    {
        LogErr(LOG, "entt archetype isn't \"light\": (name: %s, archetype: %s)", params.enttName, params.archetype);
        return;
    }

    // create entity and add components
    const EntityID enttId  = mgr.CreateEntity(params.enttName);
    const XMFLOAT3 pos     = { 0,0,0 };
    const XMVECTOR dirQuat = { params.dir.x, params.dir.y, params.dir.z };

    mgr.AddTransformComponent(enttId, params.pos, dirQuat);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.isActive);
}

//---------------------------------------------------------
// Desc:  read params from file, create and setup a new point light entity
//---------------------------------------------------------
void InitPointLightEntt(ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params)
{
    assert(pFile);
    printf("\tinit point light: %s\n", params.enttName);

    ECS::PointLight light;

    // read in params from file
    ReadFileFloat3(pFile, "pos",       &params.pos.x);
    ReadFileFloat4(pFile, "ambient",   &light.ambient.x);
    ReadFileFloat4(pFile, "diffuse",   &light.diffuse.x);
    ReadFileFloat4(pFile, "specular",  &light.specular.x);
    ReadFileFloat3(pFile, "att",       &light.att.x);      // attenuation
    ReadFileFloat (pFile, "range",     &light.range);
    ReadFileInt   (pFile, "active",    &params.isActive);
    ReadFileStr   (pFile, "parent",    params.parentEnttName);
    ReadFileStr   (pFile, "archetype", params.archetype);

    if (strcmp(params.archetype, "light") != 0)
    {
        LogErr(LOG, "entt archetype isn't \"light\": (name: %s, archetype: %s)", params.enttName, params.archetype);
        return;
    }

    // create entity and add components
    const EntityID enttId = mgr.CreateEntity(params.enttName);

    // setup boundings
    const DirectX::BoundingSphere localSphere({ 0,0,0 }, light.range);
    const DirectX::BoundingSphere worldSphere(params.pos, light.range);

    mgr.AddTransformComponent(enttId, params.pos);
    mgr.AddBoundingComponent(enttId, localSphere, worldSphere);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.isActive);

    // set parent for this point light if we have any
    // (so it will move together with its parent)
    if (params.parentEnttName[0] != '0')
    {
        const EntityID parentId = mgr.nameSys_.GetIdByName(params.parentEnttName);
        mgr.hierarchySys_.AddChild(parentId, enttId);
    }

#if ATTACH_POINT_LIGHT_TO_QUADTREE
    mgr.AttachEnttToQuadTree(enttId);
#endif
}

//---------------------------------------------------------
// Desc:  read params from file, create and setup a new spotlight entity
//---------------------------------------------------------
void InitSpotlightEntt(ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params)
{
    assert(pFile);
    printf("\tinit spotlight: %s\n", params.enttName);

    ECS::SpotLight light;

    // read in params from file
    ReadFileFloat3(pFile, "pos",         &params.pos.x);
    ReadFileFloat4(pFile, "dir_quat",    &params.dirQuat.x);
    ReadFileFloat4(pFile, "ambient",     &light.ambient.x);
    ReadFileFloat4(pFile, "diffuse",     &light.diffuse.x);
    ReadFileFloat4(pFile, "specular",    &light.specular.x);
    ReadFileFloat3(pFile, "att",         &light.att.x);       // attenuation
    ReadFileFloat (pFile, "range",       &light.range);       // distance
    ReadFileFloat (pFile, "spot_fallof", &light.spot);        // light intensity fallof (for control the spotlight cone)
    ReadFileInt   (pFile, "active",      &params.isActive);
    ReadFileStr   (pFile, "parent",       params.parentEnttName);
    ReadFileStr   (pFile, "archetype",    params.archetype);

    if (strcmp(params.archetype, "light") != 0)
    {
        LogErr(LOG, "entt archetype isn't \"light\": (name: %s, archetype: %s)", params.enttName, params.archetype);
        return;
    }

    // create entity and add components
    const EntityID enttId = mgr.CreateEntity(params.enttName);

    mgr.AddTransformComponent(enttId, params.pos, XMLoadFloat4(&params.dirQuat), 1.0f);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.isActive);

    // set parent for this point light if we have any
    // (so it will move together with its parent)
    if (params.parentEnttName[0] != '0')
    {
        const EntityID parentId = mgr.nameSys_.GetIdByName(params.parentEnttName);
        mgr.hierarchySys_.AddChild(parentId, enttId);
    }

#if ATTACH_SPOT_LIGHT_TO_QUADTREE
    mgr.AttachEnttToQuadTree(enttId);
#endif
}

} // namespace
