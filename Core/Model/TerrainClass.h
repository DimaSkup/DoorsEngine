////////////////////////////////////////////////////////////////////////////////////////////
// Filename::    TerrainClass.h
// Description:  the terrain class will encapsulate the model data and 
//               rendering functionality for drawing the terrain (or terrain grid)
//
// Created:      11.03.23
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once


//////////////////////////////////
// INCLUDES
//////////////////////////////////
#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>


#if 0



#include "../Model/Model.h"
#include "../Model/TerrainCellClass.h"
#include "../Render/frustumclass.h"
#include "../Model/ModelMath.h"
#include "../Model/TerrainInitializerInterface.h"
#include "../Model/TerrainInitializer.h"
#include "../Model/RenderableGameObject.h"
#include "../Physics/IntersectionWithGameObjects.h"    // for intersection/collision tests


//////////////////////////////////
// Class name: TerrainClass
//////////////////////////////////
class TerrainClass : public Model
{
public:
	TerrainClass(ID3D11Device* pDevice,	ID3D11DeviceContext* pDeviceContext);
	~TerrainClass();

	virtual bool Initialize(const std::string & filePath) override;

	void Shutdown();
	
	// a function for culling and polygon calculations each frame
	void Update();   

	// render a terrain cell by particular index
	bool CheckIfSeeCellByIndex(const UINT cellID, 
		FrustumClass* pFrustum);       

	// render the terrain cell by an index
	void RenderCellByIndex(const UINT index);

	// render line bounding box of a cell by particular index
	void RenderCellLines(const UINT index); 


	//
	// GETTERS
	//
	UINT GetCellIndexCount(UINT index) const;
	UINT GetCellLinesIndexCount(UINT index) const;
	UINT GetCellCount() const;
	RenderableGameObject* GetTerrainCellGameObjByIndex(const UINT index) const;
	TerrainCellClass* GetTerrainCellModelByIndex(const UINT index) const;

	// functions for rendering the polygon render count, the cells drawn count, and the cells culled count
	UINT GetCellsDrawn() const;
	UINT GetCellsCulled() const;

	float GetWidth() const;
	float GetHeight() const;

	// a function to get the current height at the current position on the terrain
	bool GetHeightAtPosition(const float posX, const float posZ, float & height);
	
private:  // restrict a copying of this class instance
	TerrainClass(const TerrainClass & obj);
	TerrainClass & operator=(const TerrainClass & obj);


private:
	void SetupParamsAfterInitialization();


	// the function for determining the height of a triangle from the terrain cell
	bool CheckHeightOfTriangle(const float inputX, 
		const float inputZ, 
		float & height, 
		const DirectX::XMVECTOR & vertex1,
		const DirectX::XMVECTOR & vertex2,
		const DirectX::XMVECTOR & vertex3); 
#if 0
	bool CalculateDeterminant(const  DirectX::XMVECTOR & Q,  // an intersection vector
		const  DirectX::XMVECTOR & edge,                     // an edge of triangle
		const  DirectX::XMVECTOR & normal,                   // a normal of triangle
		const  DirectX::XMVECTOR & vecOfVertex);             // a vector of the triangle's vertex  
#endif

private:
	const DirectX::XMVECTOR DEFAULT_DOWNWARD_VECTOR_ { 0.0f, -1.0f, 0.0f };   // the direction the ray is being cast (downward)

	// terrain parameters
	UINT terrainWidth_ = 0;
	UINT terrainHeight_ = 0;

	// some variables to describe terrain rendering state for each frame
	UINT renderedPolygonsCount_ = 0;    // a number of terrain vertices which were rendered onto the screen
	UINT cellsDrawn_ = 0;     // a number of terrain cells which were rendered
	UINT cellsCulled_ = 0;    // a number of terrain cells which were culled (not rendered)

	std::unique_ptr<IntersectionWithGameObjects> pIntersection_ = std::make_unique<IntersectionWithGameObjects>();
	std::shared_ptr<TerrainSetupData> pTerrainSetupData_ = std::make_shared<TerrainSetupData>();
	
	// an array of pointers to pointer to terrain cell game objects
	std::vector<RenderableGameObject*> terrainCellsArr_;

};


#endif