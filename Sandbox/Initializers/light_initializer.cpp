#include "../Common/pch.h"
#include "light_initializer.h"
#include <parse_helpers.h>              // helpers for parsing string buffers, or reading data from file
#include <DirectXMath.h>

#include "quad_tree_attach_control.h"


namespace Game
{

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;
using DirectX::XMVECTOR;

//---------------------------------------------------------
// Helper structure for light sources initialization
//---------------------------------------------------------
struct LightInitParams
{
    char lightType[32];
    char enttName[MAX_LEN_ENTT_NAME];
    char parentEnttName[MAX_LEN_ENTT_NAME];

    XMFLOAT3 pos;               // position
    XMFLOAT3 dir;               // direction (for directed lights)
    XMFLOAT4 rotQuat;           // rotation quaternion
    int      bActive;           // 0 - inactive, 1 - active light source

    // color props
    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;

    XMFLOAT3 attenuation;
    float    range;
    float    spotFallof;

    void Reset()
    {
        memset(lightType, 0, sizeof(lightType));
        memset(enttName, 0, sizeof(enttName));
        memset(parentEnttName, 0, sizeof(parentEnttName));

        pos      = { 0,0,0 };
        dir      = { 0,0,1 };
        rotQuat  = { 0,0,0,1 };
        bActive  = true;

        ambient  = { 0,0,0,1 };
        diffuse  = { 0,0,0,1 };
        specular = { 0,0,0,1 };
    }
};

//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void ReadParamsFromFile     (FILE* pFile, LightInitParams& initParams);

void CreateDirectedLightEntt(ECS::EntityMgr& mgr, LightInitParams& params);
void CreatePointLightEntt   (ECS::EntityMgr& mgr, LightInitParams& params);
void CreateSpotlightEntt    (ECS::EntityMgr& mgr, LightInitParams& params);


//---------------------------------------------------------
// extract color (RGBA) from input str buffer
//---------------------------------------------------------
void ReadColor(const char* buf, XMFLOAT4& c)
{
    assert(buf && buf[0] != '\0');

    char key[32];
    int count = sscanf(buf, "%s %f %f %f %f", key, &c.x, &c.y, &c.z, &c.w);
    if (count != 5)
    {
        LogErr(LOG, "failed to parse a str with light prop: %s", buf);
    }
}

//---------------------------------------------------------
// Desc:  load light entities params from file and create these entities
// Args:  - filepath:  a path to file with definitions of light sources (entities)
//---------------------------------------------------------
bool LightInitializer::Init(const char* filepath, ECS::EntityMgr& mgr)
{
    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "empty filename");
        return false;
    }

    LogMsg(LOG, "Initialize light entities from file: %s", filepath);


    int count = 0;
    FILE* pFile = nullptr;
    char buf[128];

    LightInitParams initParams;
    initParams.Reset();


    // open file for reading
    pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file: %s", filepath);
        return false;
    }

    // read in declarations of light entities
    while (fgets(buf, sizeof(buf), pFile))
    {
        if (buf[0] == '\n')
            continue;

        // skip comment
        if (buf[0] == ';')
            continue;


        // read in a type of the light source and its name
        count = sscanf(buf, "%s \"%s", initParams.lightType, initParams.enttName);

        if (count != 2)
        {
            LogErr(LOG, "can't parse a string with light declaration: %s", buf);
            LogErr(LOG, "skip creation of the light source");

            // skip this declaration
            do {
                fgets(buf, sizeof(buf), pFile);
            } while (buf[0] != '}');

            continue;
        }

        // skip last quote (") symbol from the entity name
        initParams.enttName[strlen(initParams.enttName) - 1] = '\0';

        // create a light source according to its type
        if (initParams.lightType[0] == 'd')
        {
            ReadParamsFromFile(pFile, initParams);
            CreateDirectedLightEntt(mgr, initParams);
        }
        else if (initParams.lightType[0] == 'p')
        {
            ReadParamsFromFile(pFile, initParams);
            CreatePointLightEntt(mgr, initParams);
        }
        else if (initParams.lightType[0] == 's')
        {
            ReadParamsFromFile(pFile, initParams);
            CreateSpotlightEntt(mgr, initParams);
        }
        else
            LogErr(LOG, "unknown light type: %s", initParams.lightType);


        // reset initial params for the following entity
        initParams.Reset();

    } // while fgets

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:  read in parameters of a light source from file
//        and store them into output argument (params)
//---------------------------------------------------------
void ReadParamsFromFile(FILE* pFile, LightInitParams& params)
{
    assert(pFile);

    int count = 0;
    char buf[128];
    char key[32];

    memset(buf, 0, sizeof(buf));
    memset(key, 0, sizeof(key));


    while (fgets(buf, sizeof(buf), pFile))
    {
        // if the end of the light declaration
        if (buf[0] == '}')
            return;

        // get a property key
        count = sscanf(buf, "%s", key);
        assert(count == 1);

        // read in a property according to the key
        if (strcmp(key, "pos") == 0)
            ReadFloat3(buf+1, "pos %f %f %f", &params.pos.x);

        else if (strcmp(key, "rot_quat") == 0)
            ReadFloat4(buf+1, "rot_quat %f %f %f %f", &params.rotQuat.x);

        else if (strcmp(key, "direction") == 0)
            ReadFloat3(buf+1, "direction %f %f %f\n", &params.dir.x);

        else if (strcmp(key, "ambient") == 0)
            ReadColor(buf, params.ambient);

        else if (strcmp(key, "diffuse") == 0)
            ReadColor(buf, params.diffuse);

        else if (strcmp(key, "specular") == 0)
            ReadColor(buf, params.specular);

        else if (strcmp(key, "is_active") == 0)
            ReadInt(buf+1, "is_active %d", &params.bActive);

        else if (strcmp(key, "att") == 0)
            ReadFloat3(buf+1, "att %f %f %f", &params.attenuation.x);

        else if (strcmp(key, "range") == 0)
            ReadFloat(buf+1, "range %f", &params.range);

        else if (strcmp(key, "spot_fallof") == 0)
            ReadFloat(buf+1, "spot_fallof %f", &params.spotFallof);

        else if (strcmp(key, "parent") == 0)
            ReadStr(buf+1, "parent %s", params.parentEnttName);

        // ERROR
        else
            LogErr(LOG, "invalid light prop key: %s (from buffer: %s)", key, buf);
    }
}

//---------------------------------------------------------
// Desc:  create and setup a new directed light entity
//---------------------------------------------------------
void CreateDirectedLightEntt(ECS::EntityMgr& mgr, LightInitParams& params)
{
    printf("\tcreate directed light: %s\n", params.enttName);

    ECS::DirLight light;

    light.ambient = params.ambient;
    light.diffuse = params.diffuse;
    light.specular = params.specular;

    // create entity and add components
    const EntityID enttId  = mgr.CreateEntity(params.enttName);
    const XMFLOAT3 pos     = { 0,0,0 };
    const XMVECTOR dirQuat = { params.dir.x, params.dir.y, params.dir.z };

    mgr.AddTransformComponent(enttId, params.pos, dirQuat);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.bActive);
}

//---------------------------------------------------------
// Desc:  create and setup a new point light entity
//---------------------------------------------------------
void CreatePointLightEntt(ECS::EntityMgr& mgr, LightInitParams& params)
{
    printf("\tcreate point light: %s\n", params.enttName);

    ECS::PointLight light;

    light.ambient = params.ambient;
    light.diffuse = params.diffuse;
    light.specular = params.specular;
    light.att = params.attenuation;
    light.range = params.range;

    // create entity and add components
    const EntityID enttId = mgr.CreateEntity(params.enttName);

    mgr.AddTransformComponent(enttId, params.pos);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.bActive);

    // setup boundings
    const DirectX::BoundingSphere localSphere({ 0,0,0 }, light.range);
    const DirectX::BoundingSphere worldSphere(params.pos, light.range);
    mgr.AddBoundingComponent(enttId, localSphere, worldSphere);

    // set a parent for this point light if we have any
    // (so it will move together with its parent)
    if (params.parentEnttName[0] != '\0')
    {
        const EntityID parentId = mgr.nameSys_.GetIdByName(params.parentEnttName);
        mgr.hierarchySys_.AddChild(parentId, enttId);
    }

#if ATTACH_POINT_LIGHT_TO_QUADTREE
    mgr.AttachEnttToQuadTree(enttId);
#endif
}

//---------------------------------------------------------
// Desc:  create and setup a new spotlight entity
//---------------------------------------------------------
void CreateSpotlightEntt(ECS::EntityMgr& mgr, LightInitParams& params)
{
    printf("\tcreate spotlight: %s\n", params.enttName);

    ECS::SpotLight light;

    light.ambient  = params.ambient;
    light.diffuse  = params.diffuse;
    light.specular = params.specular;
    light.att      = params.attenuation;
    light.range    = params.range;
    light.spot     = params.spotFallof;

    // create entity and add components
    const EntityID enttId = mgr.CreateEntity(params.enttName);

    mgr.AddTransformComponent(enttId, params.pos, XMLoadFloat4(&params.rotQuat), 1.0f);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.bActive);

    // set parent for this point light if we have any
    // (so it will move together with its parent)
    if (params.parentEnttName[0] != '\0')
    {
        const EntityID parentId = mgr.nameSys_.GetIdByName(params.parentEnttName);
        mgr.hierarchySys_.AddChild(parentId, enttId);
    }

#if ATTACH_SPOT_LIGHT_TO_QUADTREE
    mgr.AttachEnttToQuadTree(enttId);
#endif
}

} // namespace
