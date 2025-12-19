// *********************************************************************************
// Filename:     Transform.h
// Description:  an ECS component which contains transformation data of entities;
// 
// Created:
// *********************************************************************************
#pragma once

#include <Types.h>
#include <cvector.h>
#include <DirectXMath.h>

namespace ECS
{

__declspec(align(16)) struct Transform
{
    Transform()
    {
        // add invalid data; this data is returned when we ask for wrong entity
        ids.push_back(INVALID_ENTITY_ID);
        posAndUniformScale.push_back(DirectX::XMFLOAT4{ NAN, NAN, NAN, NAN });
        directions.push_back(DirectX::XMVECTOR{ NAN, NAN, NAN, NAN });


        const cvector<float> nanArray(16, NAN);
        DirectX::XMMATRIX nanMatrix(nanArray.data());

        worlds.push_back(nanMatrix);
        invWorlds.push_back(nanMatrix); // inverse world matrix
    }


    cvector<EntityID> ids;
    cvector<DirectX::XMMATRIX> worlds;
    cvector<DirectX::XMMATRIX> invWorlds;           // inverse world matrices
    cvector<DirectX::XMFLOAT4> posAndUniformScale;  // pos (x,y,z); uniform scale (w)
    cvector<DirectX::XMVECTOR> directions;          // normalized direction vector
};

}
