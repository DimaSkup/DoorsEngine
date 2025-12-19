

///////////////////////////////////////
// DATA STRUCTURES
///////////////////////////////////////
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = specPower
    float4 reflect;
};

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float  pad;
};

struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float  range;

    float3 att;
    float  pad;
};

struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float  range;

    float3 direction;
    float  spot;

    float3 att;
    float  pad;
};


///////////////////////////////////////
// COMPUTE DIRECTIONAL LIGHT
///////////////////////////////////////
void ComputeDirectionalLight(
    Material mat, 
    DirectionalLight L,
    float3 normal,         // unit vector
    float3 toEye,
    float specStrength,    // specular strength
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // this HLSL function outputs the lit color of a point given a material, directional 
    // light source, surface normal, and the unit vector from the surface being lit to the eye

    // initialize outputs
    ambient = 0;
    diffuse = 0;
    spec    = 0;

    
    // the light vector aims opposite the direction the light rays travel
    float3 lightVec = -L.direction;

    // add ambient term
    ambient = mat.ambient * L.ambient;

    // use Lambert's cosine law to define a magnitude of the light intensity
    float diffuseFactor = saturate(dot(lightVec, normal));

    // flatten to avoit dynamic branching
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 R = reflect(lightVec, normal);
        float specFactor = pow(saturate(dot(R, toEye)), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec    = specStrength * specFactor * mat.specular * L.specular;
    }

    
    // toon shading
    /*
    if (diffuseFactor < 0.0f)
    {
        diffuse = 0.4f;
    }
    else if (diffuseFactor <= 0.3f)
    {
        diffuse = 0.5f;
    }
    else if (diffuseFactor <= 0.5f)
    {
        diffuse = 0.6f;
    }
    else if (diffuseFactor <= 0.8f)
    {
        diffuse = 0.8f;
    }
    else if (diffuseFactor <= 1.0f)
    {
        diffuse = 1.0f;
    }
    */
}

//---------------------------------------------------------
// Desc:  calculate directed light contribution but only for ambient + diffuse components
//        (we use it preferably for LODs with low detalization)
//---------------------------------------------------------
void CalcDirLightAmbDiff(
    float4 matAmbient,
    float4 matDiffuse,
    DirectionalLight L,
    float3 normal,         // unit vector
    out float4 ambient,
    out float4 diffuse)
{
    // this HLSL function outputs the lit color of a point given a material, directional 
    // light source, surface normal, and the unit vector from the surface being lit to the eye

    // initialize outputs
    ambient = 0;
    diffuse = 0;

    // the light vector aims opposite the direction the light rays travel
    float3 lightVec = -L.direction;

    // use Lambert's cosine law to define a magnitude of the light intensity
    const float diffuseFactor = saturate(dot(lightVec, normal));
    const float hasDiffuse    = (diffuseFactor > 0.0);
    
    ambient = matAmbient * L.ambient;
    diffuse = (diffuseFactor * hasDiffuse) * (matDiffuse * L.diffuse);
}

///////////////////////////////////////
// COMPUTE POINT LIGHT
///////////////////////////////////////
void ComputePointLight(
    Material mat, 
    PointLight L,
    float3 pos,          // position of the vertex
    float3 normal,
    float3 toEye,
    float specPower,     // specular power (values from the spec map)
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // this HLSL function outputs the lit color of a point given a material, point light
    // source, surface position, surface normal, and the unit vector from the surface
    // point being lit to the eye

    // initialize output
    ambient = 0;
    diffuse = 0;
    spec    = 0;

    float3 lightVec = L.position - pos;

    float d = length(lightVec);

    if (d > L.range)
        return;

    lightVec /= d;

    ambient = mat.ambient * L.ambient;

    float diffuseFactor = dot(lightVec, normal);

    if (diffuseFactor > 0.0f)
    {
        float3 R = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(R, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec    = specFactor * mat.specular * L.specular;
    }

    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    ambient *= att;
    spec *= att;
}

//---------------------------------------------------------
// Desc:  calculate point light contribution but only for ambient + diffuse components
//        (we use it preferably for LODs with low detalization)
//---------------------------------------------------------
void CalcPointLightAmbDiff(
    const float4 matAmbient,
    const float4 matDiffuse,
    const PointLight L,
    const float3 pos,
    const float3 normal,
    out float4 ambient,
    out float4 diffuse)
{
    // this HLSL function outputs the lit color of a point given a material, point light
    // source, surface position, surface normal, and the unit vector from the surface
    // point being lit to the eye
    
    // initialize outputs
    ambient = 0;
    diffuse = 0;

    float3 lightVec = L.position - pos;

    // if farther than range we go out
    if (dot(lightVec, lightVec) > L.range*L.range)
        return;

    const float d = length(lightVec);
    lightVec      = normalize(lightVec);
   
    float diffuseFactor = dot(lightVec, normal);
    float hasDiffuse    = (diffuseFactor > 0.0);
    
    ambient = matAmbient * L.ambient;
    diffuse = (hasDiffuse * diffuseFactor) * (matDiffuse * L.diffuse);

    float att = 1.0f / dot(L.att, float3(1.0, d, d*d));
    diffuse *= att;
    ambient *= att;
}

//---------------------------------------------------------
//---------------------------------------------------------
void ComputePointLight2(
    Material mat, 
    PointLight L,
    float3 pos,      // position of the vertex
    float3 normal,
    float3 toEye,
    float specPower,    // specular strength
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // this HLSL function outputs the lit color of a point given a material, point light
    // source, surface position, surface normal, and the unit vector from the surface
    // point being lit to the eye

    // initialize output
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVec = L.position - pos;

    float d = length(lightVec);

    if (d > L.range)
        return;

    lightVec /= d;

    ambient = mat.ambient * L.ambient;

    float diffuseFactor = dot(lightVec, normal);

    if (diffuseFactor > 0.0f)
    {
        float3 R = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(R, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec    = specPower * specFactor * mat.specular * L.specular;
    }

    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    ambient *= att;
    spec *= att;
}


///////////////////////////////////////
// COMPUTE POINT LIGHT (FOR FOLIAGE WE DO SOME SPECIFIC STUFF)
///////////////////////////////////////
void ComputePointLightFoliage(
    Material mat,
    PointLight L,
    float3 pos,      // position of the vertex
    float3 normal,
    float3 toEye,
    float3 specularMap,
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // this HLSL function outputs the lit color of a point given a material, point light
    // source, surface position, surface normal, and the unit vector from the surface
    // point being lit to the eye

    // initialize output
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVec = L.position - pos;

    float d = length(lightVec);

    if (d > L.range)
        return;

    lightVec /= d;

    ambient = mat.ambient * L.ambient;

    float diffuseFactor = dot(lightVec, normal);

    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
    else
    {
        diffuse = mat.diffuse * L.diffuse * 0.7f;
    }


    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    ambient *= att;
    spec *= att;
}

///////////////////////////////////////
// COMPUTE SPOTLIGHT
///////////////////////////////////////
void ComputeSpotLight(
    Material mat, 
    SpotLight L,
    float3 pos,
    float3 normal,
    float3 toEye,
    float specularPower,
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // this HLSL function outputs the lit color of a point given a material, spotlight
    // source, surface position, surface normal, and the unit vector from the surface
    // point being lit to the eye

    // initialize outputs
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // the vector from the surface to the light
    float3 lightVec = L.position - pos;
    
    float distSqr = dot(lightVec, lightVec);
    
    // range test (just compare squares of the distance and range)
    if (distSqr > dot(L.range, L.range))
        return;

    // 1.0f / sqrt(distSqrt)
    float distInv = rsqrt(distSqr);
    
    // normalize the light vector
    lightVec *= distInv;
    
    // ambient term
    ambient = mat.ambient * L.ambient;

    // add diffuse and specular term, provided the surface is in
    // the line of site of the light
    float diffuseFactor = dot(lightVec, normal);

    // flatten to avoid dynamic branching
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        const float3 R          = reflect(-lightVec, normal);
        const float  specFactor = pow(max(dot(R, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specularPower * specFactor * mat.specular * L.specular;
    }

    // scale by spotlight factor and attenuate
    float3 lightDir = normalize(L.direction);
    
    float spot = pow(max(dot(-lightVec, lightDir), 0.0f), L.spot);
    float att = spot * dot(L.att, float3(1.0f, distInv, pow(distInv, 2)));

    ambient *= att;
    diffuse *= att;
    spec *= att;
}

//---------------------------------------------------------
// Desc:  calculate spotlight contribution but only for ambient + diffuse components
//        (we use it preferably for LODs with low detalization)
//---------------------------------------------------------
void CalcSpotLightAmbDiff(
    const float4 matAmbient,
    const float4 matDiffuse,
    const SpotLight L,
    const float3 pos,
    const float3 normal,
    out float4 ambient,
    out float4 diffuse)
{
    // this HLSL function outputs the lit color of a point given a material, spotlight
    // source, surface position, surface normal, and the unit vector from the surface
    // point being lit to the eye

    // initialize outputs
    ambient = 0;
    diffuse = 0;
    
    // the vector from the surface to the light
    float3 lightVec = L.position - pos;
    float  distSqr  = dot(lightVec, lightVec);
    
    // range test (just compare squares of the distance and range)
    if (distSqr > dot(L.range, L.range))
        return;

    // 1.0f / sqrt(distSqrt)
    float distInv = rsqrt(distSqr);
    
    // normalize the light vector
    lightVec *= distInv;
    
    const float diffuseFactor = dot(lightVec, normal);
    const float hasDiffuse    = (diffuseFactor > 0.0);
    
    ambient = matAmbient * L.ambient;
    diffuse = (hasDiffuse * diffuseFactor) * (matDiffuse * L.diffuse);

    // scale by spotlight factor and attenuate
    float3 lightDir = normalize(L.direction);
    
    float spot = pow(max(dot(-lightVec, lightDir), 0.0f), L.spot);
    float att  = spot * dot(L.att, float3(1.0, distInv, pow(distInv, 2)));

    ambient *= att;
    diffuse *= att;
}

//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(
    const float3 normalMapSample, 
    const float3 N,                 // unit normal vector
    const float4 tangentW)
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 tsNormal = 2.0f * normalMapSample - 1.0f;

    // Build orthonormal basis.
    float3 T = normalize(tangentW.xyz - N * dot(tangentW.xyz, N)) * tangentW.w;
    float3 B = cross(N, T);
    
    // Transform from tangent space to world space.
    return mul(tsNormal, float3x3(T, B, N));
}
