// =================================================================================
// Filename:    LightEnttsInitializer.h
// Description: hardcoded initialization of the light sources in the scene
//
// Created:     18.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include "Common/Types.h"
#include "Common/MathHelper.h"
#include "Entity/EntityMgr.h"

using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMVECTOR = DirectX::XMVECTOR;
using XMMATRIX = DirectX::XMMATRIX;


namespace Game
{

void InitDirectedLightEntities(ECS::EntityMgr& mgr)
{
    // setup and create directed light entities

    constexpr size numDirLights = 3;

    if constexpr (numDirLights > 0)
    {

        ECS::DirLightsInitParams dirLightsParams;
        dirLightsParams.data.resize(numDirLights);

        ECS::DirLight& dirLight0 = dirLightsParams.data[0];
        ECS::DirLight& dirLight1 = dirLightsParams.data[1];
        ECS::DirLight& dirLight2 = dirLightsParams.data[2];

        // setup main directed light source
        //dirLight0.ambient = { 0.6f, 0.6f, 0.6f, 1.0f };
        //dirLight0.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
        dirLight0.ambient = { 0,0,0,1 };
        dirLight0.diffuse = { 0,0,0,1 };
        dirLight0.specular = { 0.3f, 0.3f, 0.3f, 1.0f };

        // setup 2nd directed light source
        //dirLight1.ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
        //dirLight1.diffuse = { 0.2f, 0.2f, 0.2f, 1.0f };
        dirLight1.ambient = { 0,0,0,1 };
        dirLight1.diffuse = { 0,0,0,1 };
        dirLight1.specular = { 0.25f, 0.25f, 0.25f, 1.0f };

        // setup 3rd directed light source
        //dirLight2.ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
        //dirLight2.diffuse = { 0.2f, 0.2f, 0.2f, 1.0f };
        dirLight2.ambient = { 0,0,0,1 };
        dirLight2.diffuse = { 0,0,0,1 };
        dirLight2.specular = { 0.0f, 0.0f, 0.0f, 1.0f };


        // create directional light entities and add components to them
        const ECS::cvector<EntityID> dirLightsIds = mgr.CreateEntities(numDirLights);
        const EntityID* ids = dirLightsIds.data();

        const std::string names[numDirLights] =
        {
            "dir_light_1",
            "dir_light_2",
            "dir_light_3"
        };

        mgr.AddLightComponent(ids, numDirLights, dirLightsParams);
        mgr.AddNameComponent(ids, names, numDirLights);


        const DirectX::XMFLOAT3 directions[numDirLights] =
        {
            { 0.57735f, -0.9f, 0.57735f },
            { -0.57735f, -0.57735f, 0.57735f },
            { 0.0f, -0.707f, -0.707f }
        };

        // add transform component to each directed light because we may need to manipulate directed lights icons (in editor we can change icons positions in the scene or manipulate light direction using gizmo) 
        for (index i = 0; i < numDirLights; ++i)
        {
            const XMFLOAT3 pos = { 3, 3, (float)i };
            const XMVECTOR dirQuat = DirectX::XMLoadFloat3(&directions[i]);

            mgr.AddTransformComponent(dirLightsIds[i], pos, dirQuat, 1.0f);
        }
    }
}

///////////////////////////////////////////////////////////

void GenerateLightColors(
    XMFLOAT4& ambient,
    XMFLOAT4& diffuse,
    XMFLOAT4& specular,
    const float ambFactor,
    const float diffFactor,
    const float specFactor)
{
    // setup light color parameters

    const XMFLOAT4 color = MathHelper::RandColorRGBA();

    ambient.x = color.x * ambFactor;
    ambient.y = color.y * ambFactor;
    ambient.z = color.z * ambFactor;
    ambient.w = 1.0f;

    diffuse.x = color.x * diffFactor;
    diffuse.y = color.y * diffFactor;
    diffuse.z = color.z * diffFactor;
    diffuse.w = 1.0f;

    specular.x = color.x * specFactor;
    specular.y = color.y * specFactor;
    specular.z = color.z * specFactor;
    specular.w = 3.0f;          // setup specular power
}

///////////////////////////////////////////////////////////

void InitPointLightEntities(ECS::EntityMgr& mgr)
{
    constexpr size numPointLights = 20;

    if (numPointLights > 0)
    {
        ECS::PointLightsInitParams pointLightsParams;
        pointLightsParams.data.resize(numPointLights);

        // generate ambient/diffuse/specular color for each point light source
        for (ECS::PointLight& light : pointLightsParams.data)
        {
            GenerateLightColors(light.ambient, light.diffuse, light.specular, 0.3f, 0.7f, 0.8f);
        }

        // setup attenuation params
        for (index i = 0; i < numPointLights; ++i)
            pointLightsParams.data[i].att = { 0, 0.1f, 0.005f };

        for (index i = 0; i < numPointLights; ++i)
            pointLightsParams.data[i].range = 30;

        // ------------------------------------------------

        // setup transformation params because we may need to manipulate point
        // lights icons (in editor we can change icons positions in the scene or
        // manipulate light position using gizmo) 
        XMFLOAT3 positions[numPointLights];
        XMVECTOR dirQuats[numPointLights];
        float uniformScales[numPointLights];

        // generate random position for each point light source
        for (index i = 0; i < numPointLights; ++i)
        {
            positions[i].x = MathHelper::RandF(-100, 100);
            positions[i].y = 4.0f;
            positions[i].z = MathHelper::RandF(-100, 100);
        }

        for (index i = 0; i < numPointLights; ++i)
            dirQuats[i] = { 0,0,0,1 };

        for (index i = 0; i < numPointLights; ++i)
            uniformScales[i] = pointLightsParams.data[i].range;


        // generate name for each point light src
        std::string names[numPointLights];

        for (index i = 0; i < numPointLights; ++i)
            names[i] = "point_light_" + std::to_string(i);

        // setup bounding params
        constexpr size numSubsets = 1;
        const ECS::BoundingType boundTypes[numSubsets]{ ECS::BoundingType::SPHERE };
        DirectX::BoundingSphere boundSpheres[numPointLights];

        // setup bounding sphere for each point light src
        for (index i = 0; i < numPointLights; ++i)
            boundSpheres[i].Center = positions[i];

        for (index i = 0; i < numPointLights; ++i)
            boundSpheres[i].Radius = pointLightsParams.data[i].range;


        // ------------------------------------------------
        // create and setup point light entities

        const ECS::cvector<EntityID> pointLightsIds = mgr.CreateEntities(numPointLights);
        const EntityID* ids = pointLightsIds.data();

        mgr.AddTransformComponent(ids, numPointLights, positions, dirQuats, uniformScales);
        mgr.AddLightComponent(ids, numPointLights, pointLightsParams);
        mgr.AddNameComponent(ids, names, numPointLights);
        //mgr.AddBoundingComponent(ids, boundSpheres, numPointLights);
    }
}

///////////////////////////////////////////////////////////

void InitSpotLightEntities(ECS::EntityMgr& mgr)
{
    constexpr size numSpotLights = 10;

    if (numSpotLights > 0)
    {
        ECS::SpotLightsInitParams spotLightsParams;

        spotLightsParams.data.resize(numSpotLights);

        // setup ambient/diffuse/specular for the flashlight in a separate way
        ECS::SpotLight& flashlight = spotLightsParams.data[0];
        flashlight.ambient  = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);
        flashlight.diffuse  = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
        flashlight.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

        // generate ambient/diffuse/specular color for each spotlight source
        for (index i = 1; i < numSpotLights; ++i)
        {
            ECS::SpotLight& light = spotLightsParams.data[i];
            GenerateLightColors(light.ambient, light.diffuse, light.specular, 0.1f, 0.7f, 0.01f);
        }

        // setup attenuation params (const, linear, quadratic)
        for (index i = 0; i < numSpotLights; ++i)
            spotLightsParams.data[i].att = { 1.0f, 100.0f, 100.0f };

        // setup ranges (how far the spotlight can lit)
        for (index i = 0; i < numSpotLights; ++i)
            spotLightsParams.data[i].range = 100;

        // setup spot exponent: light intensity fallof (for control the spotlight cone)
        for (index i = 0; i < numSpotLights; ++i)
            spotLightsParams.data[i].spot = 5;


        //
        // create and setup spotlight entities
        //
        const ECS::cvector<EntityID> spotLightsIds = mgr.CreateEntities(numSpotLights);
        const EntityID* enttsIDs = spotLightsIds.data();
        const EntityID flashLightID = spotLightsIds[0];

        // setup names
        std::string spotLightsNames[numSpotLights];
        spotLightsNames[0] = "flashlight";

        for (index i = 1; i < numSpotLights; ++i)
            spotLightsNames[i] = "spot_light_" + std::to_string(spotLightsIds[i]);


        // generate transform data for spotlight sources
        XMFLOAT3 positions[numSpotLights];
        XMVECTOR directions[numSpotLights];
        float uniformScales[numSpotLights];

        // generate positions: 2 rows of spot light sources
        for (index i = 0, z = 0; i < numSpotLights / 2; z += 30, i += 2)
        {
            positions[i + 0] = { -8, 10, (float)z };
            positions[i + 1] = { +8, 10, (float)z };
        }

      
        // generate directions
        constexpr float PI_DIV6 = DirectX::XM_PI * 0.333f;
        constexpr float dir1 = -DirectX::XM_PIDIV2 - PI_DIV6;
        constexpr float dir2 = -DirectX::XM_PIDIV2 + PI_DIV6;

        for (index i = 0; i < numSpotLights / 2; i += 2)
        {
            directions[i + 0] = { 0.0f, dir1, 0.0f, 1.0f };
            directions[i + 1] = { 0.0f, dir2, 0.0f, 1.0f };
        }

        // setup positon/direction of flashlight in a separate way
        positions[0]  = { 0,0,1 };
        directions[0] = { 0,0, -DirectX::XM_PIDIV2 + PI_DIV6 };


        for (index i = 0; i < numSpotLights; ++i)
            uniformScales[i] = 1.0f;


        // add components to each spotlight entity
        mgr.AddTransformComponent(enttsIDs, numSpotLights, positions, directions, uniformScales);
        mgr.AddLightComponent(enttsIDs, numSpotLights, spotLightsParams);
        mgr.AddNameComponent(enttsIDs, spotLightsNames, numSpotLights);

        // main flashlight is inactive by default
        mgr.lightSystem_.SetLightIsActive(flashLightID, false);
    }
}


} // namespace Game
