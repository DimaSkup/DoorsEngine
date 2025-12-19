#include <CoreCommon/pch.h>
#include "quad_tree_rect.h"


namespace Core
{

void QuadTreeRect::Convert(const Rect3d& worldRect, const Vec3& offset, const Vec3& scale)
{
    // converted rectangle
    Rect3d rect(worldRect);

    // reposition and scale world coords to quad tree coords
    rect += offset;
    rect *= scale;

    // reduce by a tiny amount to handle tiled data
    rect.x1 -= 0.01f;
    rect.y1 -= 0.01f;
    rect.z1 -= 0.01f;

    rect.x1 = Max(rect.x0, rect.x1);
    rect.y1 = Max(rect.y0, rect.y1);
    rect.z1 = Max(rect.z0, rect.z1);

    // convert to integer values, taking the floor of each real
    x0 = (int)floorf(rect.x0);
    x1 = (int)floorf(rect.x1);
    y0 = (int)floorf(rect.y0);
    y1 = (int)floorf(rect.y1);
    z0 = (int)floorf(rect.z0);
    z1 = (int)floorf(rect.z1);

    // we must be positive :)
    x0 = Clamp(x0, 0, 254);
    y0 = Clamp(z0, 0, 30);
    z0 = Clamp(y0, 0, 254);

    // we must be at least one unit large
    x1 = Clamp(x1, x0 + 1, 255);
    y1 = Clamp(y1, y0 + 1, 31);
    z1 = Clamp(z1, z0 + 1, 255);
}

} // namespace
