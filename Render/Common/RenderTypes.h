// ********************************************************************************
// Filename:     RenderTypes.h
// Description:  some common types for the Render module
// 
// Created:      17.10.24
// ********************************************************************************
#pragma once

#include "MemHelpers.h"
#include "MaterialLightTypes.h"
#include "Assert.h"
#include "log.h"


#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>


namespace Render
{


// *********************************************************************************

//
// TYPEDEFS
//
using SRV = ID3D11ShaderResourceView;


//
// ENUMS
//
enum DebugState
{
	TURN_OFF,               // turn off the debug shader and use the default shader
	SHOW_NORMALS,
	SHOW_TANGENTS,
	SHOW_BINORMALS,
	SHOW_BUMPED_NORMALS,
	SHOW_ONLY_LIGTHING,
	SHOW_ONLY_DIRECTED_LIGHTING,
	SHOW_ONLY_POINT_LIGHTING,
	SHOW_ONLY_SPOT_LIGHTING,
	SHOW_ONLY_DIFFUSE_MAP,
	SHOW_ONLY_NORMAL_MAP,
};

enum EnttsSetType
{
	DEFAULT,                // fill solid, cull back, no blending, no alpha clipping
	ALPHA_CLIP_CULL_NONE,
	BLENDED,
	ANOTHER
};



//
// STRUCTURES
//
struct PerFrameData
{
	// constains data which is updated each frame

	// common data
	DirectX::XMMATRIX WVO;                       // is used for 2D rendering (world * basic_view * ortho)
	DirectX::XMMATRIX proj;
	DirectX::XMMATRIX viewProj;                  // (is already transposed)
	DirectX::XMFLOAT3 cameraPos;
	DirectX::XMFLOAT3 cameraDir;

	// light data
	DirLight*         dirLights   = nullptr;     // directional light sources
	PointLight*       pointLights = nullptr;
	SpotLight*        spotLights  = nullptr;
	int               numDirLights   = 0;        // number of directional light src
	int               numPointLights = 0;
	int               numSpotLights  = 0;

	float             deltaTime      = 0;        // time passed since the previous frame
	float             totalGameTime  = 0;        // time passed since the start of the application


	void ResizeLightData(
		const int nDirLights,                    // number of dir lights
		const int nPointLights,
		const int nSpotLights)
	{
		// do we need a reallocation for dir lights?
		if (nDirLights > numDirLights)
		{
			SafeDeleteArr(dirLights);
			dirLights = new DirLight[nDirLights];
		}
		numDirLights = nDirLights;


		// do we need a reallocation for point lights?
		if (nPointLights > numPointLights)
		{
			SafeDeleteArr(pointLights);
			pointLights = new PointLight[nPointLights];
		}
		numPointLights = nPointLights;

		// do we need a reallocation for spot lights?
		if (nSpotLights > numSpotLights)
		{
			SafeDeleteArr(spotLights);
			spotLights = new SpotLight[nSpotLights];
		}
		numSpotLights = nSpotLights;
	}
};

///////////////////////////////////////////////////////////

class InstBuffData
{
	//
	// constains transient data for the instance buffer
	//

public:

	InstBuffData() {}
	~InstBuffData()
	{
		SafeDeleteArr(worlds_);
		SafeDeleteArr(texTransforms_);
		SafeDeleteArr(materials_);
	}

	void Resize(const int newSize)
	{
		try
		{
			Assert::True(newSize > 0, "wrong value of new size: " + std::to_string(newSize));

			// if we need a reallocation (just do nothing if we have enough memory)
			if (newSize > capacity_)
			{
				worlds_        = new DirectX::XMMATRIX[newSize];
				texTransforms_ = new DirectX::XMMATRIX[newSize];
				materials_     = new Material[newSize];
				capacity_      = newSize;	
			}

			// update the number of elements for this frame
			size_ = newSize;
		}
		catch (const std::bad_alloc& e)
		{
			this->~InstBuffData();
			Log::Error(e.what());
			Log::Error("can't allocate memory for the instanced transient data buffer");
		}
		catch (LIB_Exception& e)
		{
			this->~InstBuffData();
			Log::Error(e);
			Log::Error("can't setup instanced transient data buffer");
		}
	}

	const int GetSize() const { return size_; }

	DirectX::XMMATRIX* worlds_ = nullptr;
	DirectX::XMMATRIX* texTransforms_ = nullptr;
	Material*          materials_ = nullptr;

private:
	int                capacity_ = 0;   // how many elements we can put into this buffer
	int                size_ = 0;       // the current number of data elements
};

///////////////////////////////////////////////////////////

struct Subset
{
	// subset (mesh) data of the model
	Subset() {}

	std::string name;                   // for debugging
	int         vertexStart = 0;        // start pos of vertex in the common buffer
	int         vertexCount = 0;        // how many vertices this subset has
	int         indexStart = 0;         // start pos of index in the common buffer
	int         indexCount = 0;         // how many indices this subset has
};

///////////////////////////////////////////////////////////

struct Instance
{
	// data of a single model instance
	Instance() {}

	std::string           name;              // for debugging
	int                   numInstances = 0;  // how many instances will be rendered
	UINT                  vertexStride = 0;  // size in bytes of a single vertex

	ID3D11Buffer*         pVB = nullptr;     // vertex buffer
	ID3D11Buffer*         pIB = nullptr;     // index buffer
	std::vector<SRV*>     texSRVs;           // textures arr for each mesh
	std::vector<Subset>   subsets;           // subInstance (mesh) data
	std::vector<Material> materials;         // material for each subInstance (mesh)
	
	// --------------------------------

	Instance& operator=(const Instance& rhs)
	{
		if (this == &rhs)
			return *this;

		name          = rhs.name;
		numInstances  = rhs.numInstances;
		vertexStride  = rhs.vertexStride;

		pVB           = rhs.pVB;
		pIB           = rhs.pIB;
		texSRVs       = rhs.texSRVs;
		subsets       = rhs.subsets;
		materials     = rhs.materials;

		return *this;
	}

	// --------------------------------

	int GetNumVertices() const
	{
		return subsets.back().vertexStart + subsets.back().vertexCount;
	}

	// --------------------------------
	
	void Clear()
	{
		pVB = nullptr;
		pIB = nullptr;
		texSRVs.clear();
		subsets.clear();
		materials.clear();
	}
};

///////////////////////////////////////////////////////////

struct SkyInstance
{
	// data of a sky model instance

	UINT              indexCount = 0;
	UINT              vertexStride = 0;  // size in bytes of a single vertex
	ID3D11Buffer*     pVB = nullptr;     // vertex buffer
	ID3D11Buffer*     pIB = nullptr;     // index buffer
	std::vector<SRV*> texSRVs;           // textures arr
	DirectX::XMFLOAT3 colorCenter;       // horizon sky color (for gradient)
	DirectX::XMFLOAT3 colorApex;         // top sky color (for gradient)
};

///////////////////////////////////////////////////////////

struct RenderDataStorage
{
	// stores render data of bunches of instances with different render states

	void Clear()
	{
		modelInstances_.clear();
		alphaClippedModelInstances_.clear();
		blendedModelInstances_.clear();
		boundingLineBoxInstances_.clear();
	}

	InstBuffData          modelInstBuffer_;
	InstBuffData          alphaClippedModelInstBuffer_;
	InstBuffData          blendedModelInstBuffer_;
	InstBuffData          boundingLineBoxBuffer_;

	std::vector<Instance> modelInstances_;              // models with default render states
	std::vector<Instance> alphaClippedModelInstances_;
	std::vector<Instance> blendedModelInstances_;
	std::vector<Instance> boundingLineBoxInstances_;
};

///////////////////////////////////////////////////////////


}  // namespace Render