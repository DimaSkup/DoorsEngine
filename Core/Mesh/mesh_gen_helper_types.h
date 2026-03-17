// *********************************************************************************
// Filename:        MeshHelperTypes.h
// Description:     data structures for params of basic meshes
// *********************************************************************************
#pragma once


struct MeshGeometryParams {};

///////////////////////////////////////////////////////////

struct MeshCylinderParams : public MeshGeometryParams
{
	MeshCylinderParams() {}

	MeshCylinderParams(
		float _bottomRadius,
		float _topRadius,
		float _height,
		int _numSlices,
		int _numStacks)
		:
		bottomRadius(_bottomRadius),
		topRadius(_topRadius),
		height(_height),
		sliceCount(_numSlices),
		stackCount(_numStacks) {}

	float bottomRadius = 0.5f;
	float topRadius    = 0.3f;
	float height       = 3;
	int   sliceCount   = 10;
	int   stackCount   = 10;
};

///////////////////////////////////////////////////////////

struct MeshSphereParams : public MeshGeometryParams
{
	MeshSphereParams() {}

	MeshSphereParams(float _radius, int _sliceCount, int _stackCount)
		: radius(_radius), sliceCount(_sliceCount),	stackCount(_stackCount) {}

	float radius     = 0.5f;
	int   sliceCount = 10;
	int   stackCount = 10;
};

///////////////////////////////////////////////////////////

struct MeshGeosphereParams : public MeshGeometryParams
{
	MeshGeosphereParams() {}

	MeshGeosphereParams(float _radius, int _numSubdivisions)
		: radius(_radius), numSubdivisions(_numSubdivisions) {}

	float radius = 1.0f;
	int numSubdivisions = 10;   // defatization level
};

///////////////////////////////////////////////////////////

struct MeshPyramidParams : public MeshGeometryParams
{
	MeshPyramidParams(int) {}

	float height = 10;
	float baseWidth = 5;         // size of pyramid base by X
	float baseDepth = 5;         // size of pyramid base by Z
};
