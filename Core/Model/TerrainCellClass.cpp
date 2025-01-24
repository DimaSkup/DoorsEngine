////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     TerrainCellClass.cpp
// Description:  implementation of the TerrainCellClass functional
//
// Created:      02.05.23
////////////////////////////////////////////////////////////////////////////////////////////
#include "TerrainCellClass.h"

#include <algorithm>



#if 0

TerrainCellClass::TerrainCellClass(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
	: Model(pDevice, pDeviceContext)
{
	try
	{
		// allocate memory for the terrain cell line box model (cube around the terrain cell)
		pLineBoxModel_ = new TerrainCellLineBoxClass(pDevice, pDeviceContext);

		// setup a model's type
		this->modelType_ = "terrain_cell";

		this->gameObjType_ = GameObject::GAME_OBJ_ZONE_ELEMENT;

		// also we init the game object's ID with the name of the model's type;
		// NOTE: DON'T CHANGE ID after this game object was added into the game objects list;
		//
		// but if you really need it you have to change the game object's ID manually inside of the game object list
		// and here as well using the SetID() function.
		this->ID_ = this->modelType_;   // default ID
	}
	catch (std::bad_alloc & e)
	{
		this->Shutdown();

		Log::Error(e.what());
		throw EngineException("can't allocate memory for some terrain cell's parts");
	}
}

TerrainCellClass::~TerrainCellClass()
{
	//std::string debugMsg{ "destroyment of the " + pTerrainCellModel_->GetID() };
	//Log::Debug(debugMsg.c_str());
	this->Shutdown();
}



////////////////////////////////////////////////////////////////////////////////////////////
// 
//                             COMMON PUBLIC FUNCTIONS
//
////////////////////////////////////////////////////////////////////////////////////////////

bool TerrainCellClass::Initialize(InitTerrainCellData* pInitData,
	const std::vector<Vertex3D> & terrainVerticesArr,
	const std::vector<UINT> & terrainIndicesArr)
{
	// check input params
	Assert::NotZero(terrainVerticesArr.size(), "the input array of vertices is empty");
	Assert::NotZero(terrainIndicesArr.size(), "the input array of indices is empty");

	try
	{
		bool result = false;

		// initialize a terrain cell by some index
		result = this->InitializeTerrainCell(pInitData,
			terrainVerticesArr,
			terrainIndicesArr);
		Assert::True(result, "can't initialize the terrain cell model");

		// setup this terrain cell for rendering
		this->SetModelToShaderMediator(pInitData->pModelToShaderMediator);
		this->SetRenderShaderName(pInitData->renderingShaderName);

		// initialize the bounding box lines of this terrain cell
		result = this->InitializeCellLineBox();
		Assert::True(result, "can't initialize a model with lines of the bounding box");
	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		Log::Error("can't initialize the terrain cell or bounding line box");
		return false;
	}
		
	return true;
} // end Initialize

///////////////////////////////////////////////////////////

// release the memory from the terrain cell's elements/data
void TerrainCellClass::Shutdown()
{
	_DELETE(pLineBoxModel_);

	return;
}


// puts the terrain cell vertex and index buffer on the GPU for rendering;
// and renders it using a shader
void TerrainCellClass::RenderCell()
{
	// put the vertex and index buffers on the graphics pipeline to prepare 
	// them for drawing, and render it using a shader
	this->Render();
	
	return;
}


// puts the vertex and index buffer for the bounding box on the GPU for rendering;
// it is rendered as a line list
void TerrainCellClass::RenderLineBuffers()
{
	// put the vertex and index buffers on the graphics pipeline to prepare 
	// them for drawing, and render it using a shader
	pLineBoxModel_->Render(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	return;
}




////////////////////////////////////////////////////////////////////////////////////////////
//
//                                 PUBLIC GETTERS
//
////////////////////////////////////////////////////////////////////////////////////////////

const DirectX::XMVECTOR & TerrainCellClass::GetVertexPosByIndex(const UINT index) const
{
	// return a position of vertex by index in a XMVECTOR format
	return this->cellVerticesCoordsArr_[index];
}

///////////////////////////////////////////////////////////

void TerrainCellClass::GetVertexPosByIndex(DirectX::XMVECTOR & vertexPos, const UINT index)
{
	// store a position of vertex by index in the input vertexPos variable 
	vertexPos = this->cellVerticesCoordsArr_[index];
	return;
}

///////////////////////////////////////////////////////////

UINT TerrainCellClass::GetTerrainCellVertexCount() const
{
	// return a number of vertices in this terrain cell
	return this->GetVertexCount();
}

///////////////////////////////////////////////////////////

UINT TerrainCellClass::GetTerrainCellIndexCount() const
{
	// return a number of indices in this terrain cell
	return this->GetIndexCount();
}

///////////////////////////////////////////////////////////


UINT TerrainCellClass::GetCellLinesIndexCount() const
{
	// return a number of indices in the line box around this terrain cell
	return pLineBoxModel_->GetIndexCount();
}

///////////////////////////////////////////////////////////

void TerrainCellClass::GetCellDimensions(DirectX::XMFLOAT3 & max, 
	DirectX::XMFLOAT3 & min)
{
	// setup values of maximal and minimal dimensions of this cell
	max = maxDimensions_;
	min = minDimensions_;

	return;
}

///////////////////////////////////////////////////////////

const DirectX::XMFLOAT3 & TerrainCellClass::GetCellMinDimensions() const
{
	return minDimensions_;
}

const DirectX::XMFLOAT3 & TerrainCellClass::GetCellMaxDimensions() const
{
	return maxDimensions_;
}

///////////////////////////////////////////////////////////

bool TerrainCellClass::CheckIfPosInsideCell(const float posX, const float posZ) const
{
	return ((posX < maxDimensions_.x) && (posX > minDimensions_.x) &&
		    (posZ < maxDimensions_.z) && (posZ > minDimensions_.z));
}




////////////////////////////////////////////////////////////////////////////////////////////
//
//                              PRIVATE FUNCTIONS
//
////////////////////////////////////////////////////////////////////////////////////////////


bool TerrainCellClass::InitializeTerrainCell(InitTerrainCellData* pInitData,
	const std::vector<Vertex3D> & terrainVerticesArr,
	const std::vector<UINT> & terrainIndicesArr)
{
	
	bool result = false;

	// load the rendering buffers with the terrain data for this cell index
	result = InitializeTerrainCellBuffers(pInitData,
		terrainVerticesArr,
		terrainIndicesArr);
	Assert::True(result, "can't initialize buffers for the terrain cells");


	
	// calculate the dimensions of this cell
	CalculateCellDimensions();

	return true;

} // end InitializeTerrainCell

///////////////////////////////////////////////////////////


bool TerrainCellClass::InitializeCellLineBox()
{
	try
	{
		// build the debug line buffers to produce the bounding box around this cell
		bool result = InitializeCellLinesBuffers();
		Assert::True(result, "can't build buffers for the cell bounding box");

		// setup this line box for rendering
		pLineBoxModel_->SetModelToShaderMediator(this->pModelToShaderMediator_);
		pLineBoxModel_->SetRenderShaderName("ColorShaderClass"); 
	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		Log::Error("can't initialize a cell's line box");
		return false;
	}

	return true;

} // end InitializeCellLineBox

///////////////////////////////////////////////////////////


bool TerrainCellClass::InitializeTerrainCellBuffers(InitTerrainCellData* pInitData,
	const std::vector<Vertex3D> & terrainVerticesArr,
	const std::vector<UINT> & terrainIndicesArr)
{
	// creates the buffers used for rendering the terrain cell. It also creates a vertex list
	// that is used for other calculations such as determining the size of this cell.
	// The terrain model that was built in the TerrainClass is passed into this function, and
	// then an index into the terrain model is created based on the physical location of this
	// cell using nodeIndexX and nodeIndexY

	UINT modelIndex = 0;   // an index into the terrain model data
	UINT index = 0;        // an index into the vertices array
	
	
	try
	{
		// arrays for vertices/indices data
		std::vector<Vertex3D> verticesArr(pInitData->verticesCountInCell);
		std::vector<UINT> indicesArr(verticesArr.size());

		// create a public vertex array that will be used for accessing vertex information about this cell
		cellVerticesCoordsArr_.resize(verticesArr.size());

		///////////////////////////////////////////////////

		// setup the indices into the terrain model data and the local vertex/index array

		modelIndex = ((pInitData->nodeIndexX * pInitData->quadWidthOfCell) +                        // horizontal offset
			          (pInitData->nodeIndexY * pInitData->quadHeightOfCell * (pInitData->terrainWidth - 1))) * // vertical offset
			           pInitData->numVerticesInQuad;


		auto itVertexFrom = terrainVerticesArr.begin() + modelIndex;
		auto itVertexTo = terrainVerticesArr.begin() + modelIndex + pInitData->vertexNumInCellRow;

		// load the vertex array and index array with data
		for (UINT j = 0; j < pInitData->quadHeightOfCell; j++)
		{
			// copy vertices
			std::copy(terrainVerticesArr.begin() + modelIndex, // copy from
				terrainVerticesArr.begin() + modelIndex + pInitData->vertexNumInCellRow, // copy until
				verticesArr.begin() + index);                  // copy into

			modelIndex += pInitData->vertexNumInCellRow;

			// go through the line of quads and copy it into the final arrays
			for (UINT i = 0; i <  pInitData->vertexNumInCellRow; i++)
			{
				// copy full data from the terrain vertex into the terrain cell vertex
				indicesArr[index] = index;

				// store the position as a XMVECTOR so later it will be easier 
				// for using it during intersection computation
				cellVerticesCoordsArr_[index] = XMLoadFloat3(&verticesArr[index].position);

				index++;
			}

			// move to the next line of quads
			modelIndex += pInitData->modelIndexStride;
		}

		///////////////////////////////////////////////////

		// each terrain cell has only one mesh so create it and initialize with data
		this->InitializeOneMesh(verticesArr, indicesArr, pInitData->texturesPaths, false);

	}
	catch (std::bad_alloc & e)
	{
		Log::Error(e.what());
		throw EngineException("can't allocate memory for the terrain cell vertices/indices/vertex_list");
	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		return false;
	}

	return true;

} // end InitializeTerrainCellBuffers

///////////////////////////////////////////////////////////

void TerrainCellClass::CalculateCellDimensions()
{
	// this function is used to determine the size of this cell;
	// it uses the vertex list we created in InitializeDefaultBuffers to do so.

	// get the minimal and maximal value of X coordinate of this cell
	const auto minmax_X = std::minmax_element(cellVerticesCoordsArr_.begin(), // begin of the range
		cellVerticesCoordsArr_.end(),                                         // end of the range
		[](const DirectX::XMVECTOR & a, const DirectX::XMVECTOR & b)
		{
			return a.m128_f32[0] < b.m128_f32[0];  // compare X
		});

	// get the minimal and maximal value of Y coordinate of this cell
	const auto minmax_Y = std::minmax_element(cellVerticesCoordsArr_.begin(), // begin of the range
		cellVerticesCoordsArr_.end(),                                         // end of the range
		[](const DirectX::XMVECTOR & a, const DirectX::XMVECTOR & b)
		{
			return a.m128_f32[1] < b.m128_f32[1];  // compare Y
		});

	// get the minimal and maximal value of Z coordinate of this cell
	const auto minmax_Z = std::minmax_element(cellVerticesCoordsArr_.begin(), // begin of the range
		cellVerticesCoordsArr_.end(),                                         // end of the range
		[](const DirectX::XMVECTOR & a, const DirectX::XMVECTOR & b)
		{
			return a.m128_f32[2] < b.m128_f32[2];  // compare Z
		});


	minDimensions_.x = minmax_X.first->m128_f32[0];   // get min X
	minDimensions_.y = minmax_Y.first->m128_f32[1];   // get min Y
	minDimensions_.z = minmax_Z.first->m128_f32[2];   // get min Z

	maxDimensions_.x = minmax_X.second->m128_f32[0];  // get max X
	maxDimensions_.y = minmax_Y.second->m128_f32[1];  // get max Y
	maxDimensions_.z = minmax_Z.second->m128_f32[2];  // get max Z

	// calculate the center position of this cell
	cellCenterPosition_.x = (maxDimensions_.x - minDimensions_.x) * 0.5f;
	cellCenterPosition_.y = (maxDimensions_.y - minDimensions_.y) * 0.5f;
	cellCenterPosition_.z = (maxDimensions_.z - minDimensions_.z) * 0.5f;

	return;
}

///////////////////////////////////////////////////////////

bool TerrainCellClass::InitializeCellLinesBuffers()
{
	// creates the bouding box that surrounds the terrain cell. It is made up of series of 
	// lines creating a box around the exact dimensions of the terrain cell. This is used
	// for debugging purposes mostly

	constexpr UINT vertexCount = 8;    // set the number of line box vertices in the vertex array
	const bool isVertexBufferDynamic = false;   // set that the vertex buffer of the model is not dynamic

												// arrays for vertices/indices data
	std::vector<Vertex3D> verticesArr(vertexCount);
	std::vector<UINT> indicesArr;

	// setup vertices position of the bounding box:

	// bottom side of the box
	verticesArr[0].position = { minDimensions_.x, minDimensions_.y, minDimensions_.z };  // near left
	verticesArr[1].position = { maxDimensions_.x, minDimensions_.y, minDimensions_.z };  // near right
	verticesArr[2].position = { maxDimensions_.x, minDimensions_.y, maxDimensions_.z };  // far right
	verticesArr[3].position = { minDimensions_.x, minDimensions_.y, maxDimensions_.z };

	// top side of the box
	verticesArr[4].position = { minDimensions_.x, maxDimensions_.y, minDimensions_.z };  // near left
	verticesArr[5].position = { maxDimensions_.x, maxDimensions_.y, minDimensions_.z };  // near right
	verticesArr[6].position = { maxDimensions_.x, maxDimensions_.y, maxDimensions_.z };  // far right
	verticesArr[7].position = { minDimensions_.x, maxDimensions_.y, maxDimensions_.z };


	// setup the indices for the cell lines box
	indicesArr.insert(indicesArr.begin(), {

		// bottom
		0, 1, 0,
		1, 2, 1,
		2, 3, 2,
		3, 0, 3,

		// front
		4, 5, 4,
		5, 1, 5,
		1, 0, 1,
		0, 4, 0,

		// top
		7, 6, 7,
		6, 5, 6,
		5, 4, 5,
		4, 7, 4,

		// back
		6, 7, 6,
		7, 3, 7,
		3, 2, 3,
		2, 6, 2,

		// left
		7, 4, 7,
		4, 0, 4,
		0, 3, 0,
		3, 7, 3,

		// right
		5, 6, 5,
		6, 2, 6,
		2, 1, 2,
		1, 5, 1
	});

	// each line box has only one mesh so create it and initialize with vertices/indices data
	pLineBoxModel_->InitializeOneMesh(verticesArr, indicesArr, {}, isVertexBufferDynamic);
	
	return true;
} // end InitializeCellLinesBuffers

///////////////////////////////////////////////////////////


#endif