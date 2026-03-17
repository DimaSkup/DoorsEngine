/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: scene_object.h
    Desc:     implementation of scene object (member of quad tree node)

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  15.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <geometry/rect3d.h>

namespace ECS
{

//---------------------------------------------------------
// forward declarations (pointer use only)
//---------------------------------------------------------
class QuadTree;
class QuadTreeNode;


//---------------------------------------------------------
// class name:  SceneObject
//---------------------------------------------------------
class SceneObject
{
public:

    // creators...
    SceneObject();
    ~SceneObject();

    // mutators...

};

} // ECS
