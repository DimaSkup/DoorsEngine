// *********************************************************************************
// Filename:        MeshHelperTypes.h
// Description:     data structures for params of basic meshes
// *********************************************************************************
#pragma once


struct MeshGeometryParams {};

struct MeshWavesParams : public MeshGeometryParams
{
	float spatialStep;
	float timeStep;
	float speed;
	float damping;
	int numRows;
	int numColumns;
};

///////////////////////////////////////////////////////////

struct MeshCylinderParams : public MeshGeometryParams
{
	MeshCylinderParams() {}

	MeshCylinderParams(
		float bottomRadius,
		float topRadius,
		float height,
		int numSlices,
		int numStacks) 
		:
		bottomRadius_(bottomRadius),
		topRadius_(topRadius),
		height_(height),
		sliceCount_(numSlices),
		stackCount_(numStacks) {}

	float bottomRadius_ = 0.5f;
	float topRadius_ = 0.3f;
	float height_ = 3;
	int sliceCount_ = 10;
	int stackCount_ = 10;
};

///////////////////////////////////////////////////////////

struct MeshSphereParams : public MeshGeometryParams
{
	MeshSphereParams() {}

	MeshSphereParams(float radius, int sliceCount, int stackCount)
		: radius_(radius), sliceCount_(sliceCount),	stackCount_(stackCount) {}

	float radius_ = 0.5f;
	int sliceCount_ = 10;
	int stackCount_ = 10;
};

///////////////////////////////////////////////////////////

struct MeshGeosphereParams : public MeshGeometryParams
{
	MeshGeosphereParams() {}

	MeshGeosphereParams(float radius, int numSubdivisions)
		: radius_(radius), numSubdivisions_(numSubdivisions) {}

	float radius_ = 1.0f;
	int numSubdivisions_ = 10;   // defatization level
};

///////////////////////////////////////////////////////////

struct MeshPyramidParams : public MeshGeometryParams
{
	MeshPyramidParams(int) {}

	float height = 10;
	float baseWidth = 5;         // size of pyramid base by X
	float baseDepth = 5;         // size of pyramid base by Z
};
