////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     TerrainCellClass.h
// Description:  TerrainCellClass is a class that encapsulates the
//               functionality of rendering and other calculations for 
//               idividual terrain cells. Each terrain cell is created 
//               from subset of the terrain model and represents a unique
//               33x33 (or someting else) vertex section of that terrain.
//               This class also has additional buffers and structures to build
//               the orange line list bounding box around this cell for debugging
//               purposes.
// Created:      01.05.23
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once


//////////////////////////////////
// INCLUDES
//////////////////////////////////
#if 0
#include "../Model/TerrainCellLineBoxClass.h"   // line box model around a terrain cell




//////////////////////////////////
// Class name: TerrainCellClass
//////////////////////////////////
class TerrainCellClass final : public Model
{
public:
	// a struct which we will use during creation of each terrain cell;
	// most of data is common for all the cells so we just have to change 
	// each time only a couple of these fields;
	struct InitTerrainCellData
	{
		UINT nodeIndexX;                                          // different for each cell
		UINT nodeIndexY;                                          // different for each cell
		UINT cellWidth;                                           // the same for each cell
		UINT cellHeight;                                          // the same 
		UINT terrainWidth;                                        // the same 
		ModelToShaderMediatorInterface* pModelToShaderMediator;   // the same 
		std::string renderingShaderName;                          // the same 
		std::map<std::string, aiTextureType> texturesPaths;       // the same 
		UINT numVerticesInQuad;                                   // the same for each; the number of vertices in a single terrain quad
		UINT modelIndexStride;                                    // how many vertices we have to go through before we get to the next line of quads of the cell
		UINT quadWidthOfCell;                                     // cell width in vertices
		UINT quadHeightOfCell;                                    // cell height in vertices
		UINT vertexNumInCellRow;                                  // how many vertices are in one row of quads
		UINT verticesCountInCell;                                 // how many vertices are in one terrain cell                        
	};

public:


	TerrainCellClass(ID3D11Device* pDevice,	ID3D11DeviceContext* pDeviceContext);
	~TerrainCellClass();

	// for initialization of terrain cells we don't use this virtual function but use
	// another implementation of the Initialize (look down)
	virtual bool Initialize(const std::string & filePath) override 
	{ 
		assert("YOU CAN'T USE THIS FUNCTION AS VIRTUAL BECAUSE THERE IS ANOTHER INITIALIZE FUNCTION FOR INITIALIZATION" && 0);
	};

	bool Initialize(InitTerrainCellData* pInitData,
		const std::vector<Vertex3D> & terrainVerticesArr,
		const std::vector<UINT> & terrainIndicesArr);

	void Shutdown();

	// render the terrain cell / cell line box
	void RenderCell();
	void RenderLineBuffers();

	/////////////////////////////
	//  GETTERS
	/////////////////////////////
	const DirectX::XMVECTOR & GetVertexPosByIndex(const UINT index) const;
	void GetVertexPosByIndex(DirectX::XMVECTOR & vertexPos, const UINT index);

	UINT GetTerrainCellVertexCount() const;
	UINT GetTerrainCellIndexCount() const;
	UINT GetCellLinesIndexCount() const;
	bool CheckIfPosInsideCell(const float posX, const float posZ) const;

	void GetCellDimensions(DirectX::XMFLOAT3 & max, DirectX::XMFLOAT3 & min);
	const DirectX::XMFLOAT3 & TerrainCellClass::GetCellMinDimensions() const;
	const DirectX::XMFLOAT3 & TerrainCellClass::GetCellMaxDimensions() const;


private:  // restrict a copying of this class instance
	TerrainCellClass(const TerrainCellClass & obj);
	TerrainCellClass & operator=(const TerrainCellClass & obj);

private:
	bool InitializeTerrainCell(InitTerrainCellData* pInitData,
		const std::vector<Vertex3D> & terrainVerticesArr,
		const std::vector<UINT> & terrainIndicesArr);

	bool InitializeCellLineBox();

	bool InitializeTerrainCellBuffers(InitTerrainCellData* pInitData, 
		const std::vector<Vertex3D> & terrainVerticesArr,
		const std::vector<UINT> & terrainIndicesArr);

	bool InitializeCellLinesBuffers();
	void CalculateCellDimensions();

private:

	// dimensions of the node (terrain cell)
	DirectX::XMFLOAT3 maxDimensions_{ -1000000.0f, -1000000.0f, -1000000.0f };
	DirectX::XMFLOAT3 minDimensions_{ 1000000.0f, 1000000.0f, 1000000.0f };
	DirectX::XMFLOAT3 cellCenterPosition_{ 0.0f, 0.0f, 0.0f };      // the center position of this cell

	TerrainCellLineBoxClass* pLineBoxModel_ = nullptr;              // each terrain cell has its own line box around itself
	std::vector<DirectX::XMVECTOR> cellVerticesCoordsArr_;          // contains the position data of all the vertices of this cell
};


#endif