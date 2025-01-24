Point and vector coordinates are specified relative to many different spaces
(e.g., local space, world space, view space, homogeneous space). 
When reading code, it might not be which coordinate system the coordinates of the 
point/vector are relative to. Therefore, we often use the following suffixes to denote 
the space:
- L (for local space),
- W (for world space),
- V (for view space),
- H (for homogeneous clip space);

Examples: 
 float3 iPosL;     // local space
 float3 gEyePosW;  // world space
 float3 normalV;   // view space
 float4 posH;      // homogeneous clip space