#pragma once

#include <geometry/rect_3d.h>
#include <math/vec3.h>


namespace Core
{

//==================================================================================
// Helper class: QuadTreeRect
//==================================================================================
class QuadTreeRect
{
public:
    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    int x0 = 0;
    int x1 = 0;
    int y0 = 0;
    int y1 = 0;
    int z0 = 0;
    int z1 = 0;

    //-----------------------------------------------------
    // constructors / desturctors
    //-----------------------------------------------------
    QuadTreeRect() {};
    ~QuadTreeRect() {};

    inline QuadTreeRect(const QuadTreeRect& src) :
        x0(src.x0), x1(src.x1),
        y0(src.y0), y1(src.y1),
        z0(src.z0), z1(src.z1)
    {
    }

    inline QuadTreeRect(const int _x0, const int _x1,
        const int _y0, const int _y1,
        const int _z0, const int _z1) :
        x0(_x0), x1(_x1),
        y0(_y0), y1(_y1),
        z0(_z0), z1(_z1)
    {
    }

    //-----------------------------------------------------

    void Convert(const Rect3d& worldRect, const Vec3& offset, const Vec3& scale);
};

} // namespace
