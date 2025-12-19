1. NOTATION

Point and vector coordinates are specified relative to many different spaces
(e.g., local space, world space, view space, homogeneous space). 

So we use suffixes to denote particular space:
- L (for local space),
- W (for world space),
- V (for view space),
- H (for homogeneous clip space);

Examples: 
 float3 posL;      // position in local space
 float3 gCamPosW;  // camera pos world space
 float3 normalV;   // normal vector in view space
 float4 posH;      // pos in homogeneous clip space



2. FOLDERS
- const_buffers:  directory to hold different declarations of const buffers
                  so we just need to include necessary header into our shader to use it
- helpers:        different little helpers for math and generation noise/random
- types:          declarations of shaders differentinput/output types
