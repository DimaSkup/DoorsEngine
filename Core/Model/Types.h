// ************************************************************************************
// Filename:        Types.cpp
// Description:     enum of different types of MODELS
//
// Created:         28.10.24
// ************************************************************************************
#pragma once


enum ModelType
{
	Invalid,
	Cube,
	Cylinder,
	Plane,
	Pyramid,
	Skull,
	Sphere,
	GeoSphere,
	Imported,    // if we load a model from the file
	Terrain,
	LineBox,     // is used to visualise bounding boxes (AABB)
	Sky,         // this model is used to render the sky
};
