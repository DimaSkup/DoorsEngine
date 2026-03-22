#include "math_helpers.h"


//
// this function computes the distance from the origin <0,0,0> to <x,y,z>
//
float FastDistance3D(const float fx, const float fy, const float fz)
{
    // make sure that input values are all posivite
    int x = ((int)fabs(fx) * 1024);
    int y = ((int)fabs(fy) * 1024);
    int z = ((int)fabs(fz) * 1024);

    // sort values
    if (y < x) SWAP(x, y);
    if (z < y) SWAP(y, z);
    if (y < x) SWAP(x, y);

    const int dist = (z + 11 * (y >> 5) + (x >> 2));

    // compute the distance with 8% error
    return (float)(dist >> 10);

} // end Fast_Distance_3D
