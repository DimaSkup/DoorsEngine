// ********************************************************************************
// Filename:    TextureTypes.h
// Description: enumeration of textures types
// 
// Created:     05.06.24
// ********************************************************************************
#pragma once


enum eTexType
{
    TEX_TYPE_NONE,
    TEX_TYPE_DIFFUSE,        // The texture is combined with the result of the diffuse lighting equation. Or PBR Specular / Glossiness
    TEX_TYPE_SPECULAR,       // The texture is combined with the result of the specular lighting equation. Or PBR Specular / Glossiness
    TEX_TYPE_AMBIENT,        // The texture is combined with the result of the ambient lighting equation.
    TEX_TYPE_EMISSIVE,       // The texture is added to the result of the lighting calculation. It isn't influenced by incoming light.
    TEX_TYPE_HEIGHT,         // The texture is a height map. By convention, higher gray-scale values stand for higher elevations from the base height.
    TEX_TYPE_NORMALS,        // The texture is a (tangent space) normal-map.
    TEX_TYPE_SHININESS,      // The texture defines the glossiness of the material. The glossiness is in fact the exponent of the specular (phong)lighting equation.
    TEX_TYPE_OPACITY,        // The texture defines per-pixel opacity. Usually 'white' means opaque and 'black' means 'transparency'. Or quite the opposite. Have fun.
    TEX_TYPE_DISPLACEMENT,   // Displacement texture The exact purpose and format is application-dependent. Higher color values stand for higher vertex displacements.
    TEX_TYPE_LIGHTMAP,       // Lightmap texture (aka Ambient Occlusion). Both 'Lightmaps' and dedicated 'ambient occlusion maps' are covered by this material property. The texture contains a scaling value for the final color value of a pixel. Its intensity is not affected by incoming light.
    TEX_TYPE_REFLECTION,     // Reflection texture. Contains the color of a perfect mirror reflection.

    /*
        PBR Materials
        PBR definitions from maya and other modelling packages now use this standard.
        This was originally introduced around 2012.
        Support for this is in game engines like Godot, Unreal or Unity3D.
        Modelling packages which use this are very common now.
    */
    TEX_TYPE_BASE_COLOR,
    TEX_TYPE_NORMAL_CAMERA,
    TEX_TYPE_EMISSION_COLOR,
    TEX_TYPE_METALNESS,
    TEX_TYPE_DIFFUSE_ROUGHNESS,
    TEX_TYPE_AMBIENT_OCCLUSION,

    /** Sheen
    * Generally used to simulate textiles that are covered in a layer of microfibers
    * eg velvet
    * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_sheen
    */
    TEX_TYPE_SHEEN,

    /** Clearcoat
    * Simulates a layer of 'polish' or 'lacquer' layered on top of a PBR substrate
    * https://autodesk.github.io/standard-surface/#closures/coating
    * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_clearcoat
    */
    TEX_TYPE_CLEARCOAT,

    /** Transmission
    * Simulates transmission through the surface
    * May include further information such as wall thickness
    */
    TEX_TYPE_TRANSMISSION,

    // how many texture types we have
    NUM_TYPES,
};
