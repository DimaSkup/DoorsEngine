// *********************************************************************************
// Filename:     Utils.h
// Description:  contains different utils for ECS components testing
// 
// Created:      26.05.24
// *********************************************************************************

#pragma once


#include <vector>
#include <algorithm>
#include <random>


#include "Entity/EntityManager.h"
#include "MathHelper.h"
#include "HelperTypes.h"

#include "../Engine/log.h"
#include "../Engine/EngineException.h"
#include "../Common/Assert.h"

#include <fstream>
#include <filesystem>



namespace TestUtils
{

// *******************************************************************************
// 
// 	                             COMMON UTILS
// 
// *******************************************************************************

using namespace DirectX;
namespace fs = std::filesystem;


static void RemoveFile(const std::string& filepath)
{
	// if a file by filepath exists we remove it from the OS filesystem
	if (fs::exists(filepath))
	{
		if (!fs::remove(fs::path(filepath)))
			Log::Error("can't remove a file by path: " + filepath);
	}
}

// --------------------------------------------------------

static void SysSerialDeserialHelper(
	const std::string filepath,
	ECS::ComponentType type,
	ECS::EntityManager& fromMgr,
	ECS::EntityManager& intoMgr)
{
	// in: filepath - path to the data file with the serialized data
	//     type     - handle data related to this component
	//     fromMgr  - serialize data from this entity manager
	//     intoMgr  - deserialize data into this entity manager

	u32 offset = 0;
	std::ifstream fin;

	std::ofstream fout(filepath, std::ios::binary);
	Assert::True(fout.is_open(), "can't open a file for writing: " + filepath);

	using namespace ECS;

	switch (type)
	{
		case TransformComponent:
		{
			fromMgr.transformSystem_.Serialize(fout, offset);
			fout.close();
			fin.open(filepath, std::ios::binary);
			intoMgr.transformSystem_.Deserialize(fin, offset);

			break;
		}
		case MoveComponent:
		{
			fromMgr.moveSystem_.Serialize(fout, offset);
			fout.close();
			fin.open(filepath, std::ios::binary);
			intoMgr.moveSystem_.Deserialize(fin, offset);

			break;
		}
		case NameComponent:
		{
			fromMgr.nameSystem_.Serialize(fout, offset);
			fout.close();
			fin.open(filepath, std::ios::binary);
			intoMgr.nameSystem_.Deserialize(fin, offset);
			
			break;
		}
		case MeshComp:
		{
			fromMgr.meshSystem_.Serialize(fout, offset);
			fout.close();
			fin.open(filepath, std::ios::binary);
			intoMgr.meshSystem_.Deserialize(fin, offset);

			break;
		}
		case RenderedComponent:
		{
			fromMgr.renderSystem_.Serialize(fout, offset);
			fout.close();
			fin.open(filepath, std::ios::binary);
			intoMgr.renderSystem_.Deserialize(fin, offset);
			break;
		}
		case TexturedComponent:
		{
			assert("TODO: implement is!" && 0);
			break;
		}
		case TextureTransformComponent:
		{
			assert("TODO: implement it!" && 0);
			break;
		}
	}

	fin.close();
	TestUtils::RemoveFile(filepath);
}

// --------------------------------------------------------

static bool CheckEnttsHaveComponent(
	ECS::EntityManager& entityMgr,
	const std::vector<EntityID>& ids,
	const ECS::ComponentType type)
{

	bool haveComponent = true;
	std::vector<bool> hasComponentFlags;

	// get boolean flag for each input entt which defines if this entt has the component
	entityMgr.CheckEnttsHaveComponents(ids, { type }, hasComponentFlags);

	// go through each boolean flag and define if all the input entts have the component
	for (const bool hasComponent : hasComponentFlags)
		haveComponent &= hasComponent;

	return haveComponent;
}

// --------------------------------------------------------

static std::string GetRandAlnumStr(u32 length)
{
	// return randomly generated string of length

	assert(length > 0);

	const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	thread_local std::mt19937 rg{ std::random_device{}() };
	thread_local std::uniform_int_distribution<std::string::size_type> pick(0, charset.size() - 1);

	std::string str;
	str.reserve(length);

	while (length--) str += charset[pick(rg)];

	return str;
}

// --------------------------------------------------------

template<class T, class Pred = std::equal_to<>>
static bool ContainerCompare(
	const T& lhs,
	const T& rhs,
	Pred pred = {})
{
	// check two STL containers (vector, map, set, etc.) for complete equality
	// using the passed predicate (pred);
	// return: true -- if two containers are completely equal

	return (lhs.size() == rhs.size()) &&
		std::equal(lhs.begin(), lhs.end(), rhs.begin(), pred);
}

// --------------------------------------------------------

static void GetArrOfRandUINTs(
	const u32 count,
	std::vector<u32>& arr,
	const u32 minVal,
	const u32 maxVal)
{
	// fill in the input array with random UINTs in quantity of count
	// and in range [minVal, maxVal)

	assert((count > 0) && "the number of random UINTs must be > 0");
	assert((minVal < maxVal) && "min must be < max");

	arr.reserve(count);

	for (u32 idx = 0; idx < count; ++idx)
		arr.push_back(MathHelper::RandUINT(minVal, maxVal));
}

// --------------------------------------------------------

static void GetRandEnttsNames(
	const u32 enttsCount,
	const u32 nameLength,
	std::vector<EntityName>& outNames)
{
	// generate unique names for entities in quantity enttsCount
	// out: array of entities names;

	for (u32 idx = 0; idx < enttsCount; ++idx)
		outNames.emplace_back(GetRandAlnumStr(nameLength));
}

// --------------------------------------------------------

template<typename T>
static void GetArrOfRandXM(const u32 arrSize, std::vector<T>& outArr)
{
	// go through each element of the array and generate for it
	// random data according to the template parameter (XMFLOAT.../XMVECTOR)
	outArr.reserve(arrSize);

	for (u32 idx = 0; idx < arrSize; ++idx)
		outArr.push_back(MathHelper::RandXM<T>());
}

// --------------------------------------------------------

static void GetArrOfRandFloats(
	const u32 arrSize,
	std::vector<float>& outArr,
	const float minVal = 0,
	const float maxVal = 0)
{
	// fill in the input array with random floats in quantity of count
	// and in range [minVal, maxVal)

	assert((arrSize > 0) && "arr size must be > 0");
	assert((minVal <= maxVal) && "min must be <= max");

	outArr.reserve(arrSize);

	const bool isRanged = (minVal != maxVal);

	if (isRanged)
	{
		// generate random floats in particular range [minVal, maxVal)
		for (u32 idx = 0; idx < arrSize; ++idx)
			outArr.push_back(MathHelper::RandF(minVal, maxVal));
	}
	else
	{
		// generate absolutely random floats
		for (u32 idx = 0; idx < arrSize; ++idx)
			outArr.push_back(MathHelper::RandF());
	}
}

// --------------------------------------------------------

static XMMATRIX BuildMatrix(
	const XMVECTOR& pos,
	const XMVECTOR& dir,
	const XMVECTOR& scale)
{
	return	
		DirectX::XMMatrixScalingFromVector(scale) *
		DirectX::XMMatrixRotationRollPitchYawFromVector(dir) *
		DirectX::XMMatrixTranslationFromVector(pos);
}




// *******************************************************************************
// 
// 	                     UTILS RELATED TO ECS COMPONENTS
// 
// *******************************************************************************

static void GetRandTransformData(
	const u32 elemCount,
	TransformData& outTransform)
{
	// generate random values for positions/directions/scales
	GetArrOfRandXM(elemCount, outTransform.positions);
	GetArrOfRandXM(elemCount, outTransform.dirQuats);
	GetArrOfRandFloats(elemCount, outTransform.uniformScales);
}

// --------------------------------------------------------

static void GetRandMoveData(
	const u32 elemCount,
	MoveData& outMove)
{
	// generate random values for translations/rotations_quaternions/scale_changes
	GetArrOfRandXM(elemCount, outMove.translations);
	GetArrOfRandXM(elemCount, outMove.rotQuats);
	GetArrOfRandFloats(elemCount, outMove.uniformScales);
}

// --------------------------------------------------------

static void GetRandRenderedData(
	const u32 elemCount,
	RenderedData& rendered)
{
	// generate pseudo random data for the Rendered component
	rendered.shaderTypes.resize(elemCount, ECS::RENDERING_SHADERS::COLOR_SHADER);
	rendered.primTopologyTypes.resize(elemCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------

static void GetRandDirLightsData(
	const u32 numLights,
	ECS::DirLightsInitParams& outParams)
{
	// generate pseudo random data for the directional light sources
	TestUtils::GetArrOfRandXM(numLights, outParams.ambients);
	TestUtils::GetArrOfRandXM(numLights, outParams.diffuses);
	TestUtils::GetArrOfRandXM(numLights, outParams.speculars);
	TestUtils::GetArrOfRandXM(numLights, outParams.directions);
}

// --------------------------------------------------------

static void GetRandPointLightsData(
	const u32 numLights,
	ECS::PointLightsInitParams& outParams)
{
	// generate pseudo random data for the point light sources
	TestUtils::GetArrOfRandXM(numLights, outParams.ambients);
	TestUtils::GetArrOfRandXM(numLights, outParams.diffuses);
	TestUtils::GetArrOfRandXM(numLights, outParams.speculars);
	TestUtils::GetArrOfRandXM(numLights, outParams.positions);
	TestUtils::GetArrOfRandXM(numLights, outParams.attenuations);
	TestUtils::GetArrOfRandFloats(numLights, outParams.ranges);
}

// --------------------------------------------------------

static void GetRandSpotLightsData(
	const u32 numLights,
	ECS::SpotLightsInitParams& outParams)
{
	// generate pseudo random data for the spot light sources
	TestUtils::GetArrOfRandXM(numLights, outParams.ambients);
	TestUtils::GetArrOfRandXM(numLights, outParams.diffuses);
	TestUtils::GetArrOfRandXM(numLights, outParams.speculars);
	TestUtils::GetArrOfRandXM(numLights, outParams.positions);
	TestUtils::GetArrOfRandXM(numLights, outParams.directions);
	TestUtils::GetArrOfRandXM(numLights, outParams.attenuations);
	TestUtils::GetArrOfRandFloats(numLights, outParams.ranges);
	TestUtils::GetArrOfRandFloats(numLights, outParams.spotExponents);
}

// --------------------------------------------------------

static void MergeIntoFloat4(
	const std::vector<XMFLOAT3>& arrOfFloat3,
	const std::vector<float>& arrOfFloat,
	std::vector<XMFLOAT4>& outArrOfFloat4)
{
	// merge input XMFLOAT3 and float into XMFLOAT4;
	// XMFLOAT3 is placed as (x,y,z) and float is placed as (w) components

	const ptrdiff_t dataCount = std::ssize(arrOfFloat3);
	Assert::True(dataCount == std::ssize(arrOfFloat), "arr size of XMFLOAT3s must be equal to arr size of floats");

	outArrOfFloat4.clear();
	outArrOfFloat4.reserve(dataCount);

	for (ptrdiff_t idx = 0; idx < dataCount; ++idx)
	{
		const XMFLOAT3& vec3 = arrOfFloat3[idx];
		const float f = arrOfFloat[idx];

		outArrOfFloat4.emplace_back(vec3.x, vec3.y, vec3.z, f);
	}
}

// --------------------------------------------------------

static void NormalizeQuatsArr(
	const std::vector<XMVECTOR>& inQuats,
	std::vector<XMVECTOR>& outNormQuats)
{
	// in:   array of quaternions
	// out:  array of normalized quaternions

	outNormQuats.clear();
	outNormQuats.reserve(std::ssize(inQuats));

	for (const XMVECTOR& quat : inQuats)
		outNormQuats.emplace_back(DirectX::XMQuaternionNormalize(quat));
}

// --------------------------------------------------------

static void CompareTransformData(
	const ECS::Transform& transComp,
	const std::vector<EntityID>& ids,
	const TransformData& data)
{
	// here we check if input transform data is the same
	// as the data from the Transform component

	std::vector<XMFLOAT4> origPosAndUniScales;
	std::vector<XMVECTOR> origNormDirQuats;

	// convert origin data to the proper format 
	MergeIntoFloat4(data.positions, data.uniformScales, origPosAndUniScales);
	NormalizeQuatsArr(data.dirQuats, origNormDirQuats);

	// check if data from the Transform component is equal to the origin data
	const bool areIDsValid             = ContainerCompare(transComp.ids_, ids);
	const bool arePosAndScalesCorrect  = ContainerCompare(transComp.posAndUniformScale_, origPosAndUniScales);
	const bool areDirQuatsCorrect      = ContainerCompare(transComp.dirQuats_, origNormDirQuats, DirectX::CompareXMVECTOR());

	Assert::True(areIDsValid,            "TEST: IDs data from the Transform component isn't valid");
	Assert::True(arePosAndScalesCorrect, "TEST: positions and uniform scale data isn't correct");
	Assert::True(areDirQuatsCorrect,     "TEST: direction quaternions data isn't correct");
}

// --------------------------------------------------------

static void CompareMoveData(
	const ECS::Movement& component,
	const std::vector<EntityID>& ids,
	const MoveData& data)
{
	// here we check if input movement data is the same
	// as the data from the Movement component

	std::vector<XMFLOAT4> origTransAndUniScales;  
	std::vector<XMVECTOR> origNormRotQuats;       

	// convert origin data to the proper format 
	MergeIntoFloat4(data.translations, data.uniformScales, origTransAndUniScales);
	NormalizeQuatsArr(data.rotQuats, origNormRotQuats);

	// check if data from the Movement component is equal to the origin (input) data
	const bool areIDsValid             = ContainerCompare(component.ids_, ids);
	const bool areTransAndScalesValid  = ContainerCompare(component.translationAndUniScales_, origTransAndUniScales);
	const bool areRotQuatsValid        = ContainerCompare(component.rotationQuats_, origNormRotQuats, DirectX::CompareXMVECTOR());

	Assert::True(areIDsValid,            "TEST: deserialized IDs data isn't correct");
	Assert::True(areTransAndScalesValid, "TEST: translations and uniform scale data isn't correct");
	Assert::True(areRotQuatsValid,       "TEST: rotations quaternions data isn't correct");
}

// --------------------------------------------------------

static void CompareNameData(
	const ECS::Name& component,
	const std::vector<EntityID>& ids,
	const std::vector<EntityName>& names)
{
	// check if the input data is the same as the data from the Name component

	const bool areIDsValid    = ContainerCompare(ids, component.ids_);
	const bool areNamesValid  = ContainerCompare(names, component.names_);

	Assert::True(areIDsValid,   "IDs data from the Name component isn't correct");
	Assert::True(areNamesValid, "names data from the Name component isn't correct");
}

// --------------------------------------------------------

static void CompareMeshData(
	ECS::MeshSystem& system,
	const std::vector<EntityID>& enttsIDs,
	const std::vector<MeshID>& meshesIDs)
{
	// check if the input data is the same as the data from the Mesh component

	std::vector<EntityID> enttsIDsFromMeshComponent;
	std::vector<MeshID> meshesIDsFromMeshComponent;

	system.GetEnttsIDsFromMeshComponent(enttsIDsFromMeshComponent);
	system.GetAllMeshesIDsFromMeshComponent(meshesIDsFromMeshComponent);

	const bool areEnttsIDsCorrect   = ContainerCompare(enttsIDs, enttsIDsFromMeshComponent);
	const bool areMeshesIDsCorrect  = ContainerCompare(meshesIDs, meshesIDsFromMeshComponent);

	Assert::True(areEnttsIDsCorrect,  "the Mesh component doesn't have a record about some entity");
	Assert::True(areMeshesIDsCorrect, "the Mesh component doesn't have a record about some mesh");
}

// --------------------------------------------------------

static void CompareRenderedData(
	const ECS::Rendered& component,
	const std::vector<EntityID>& ids,
	const RenderedData& data)
{
	// check if the input data is the same as the data from the Rendered component

	const bool areIDsValid          = ContainerCompare(component.ids_, ids);
	const bool areShaderTypesValid  = ContainerCompare(component.shaderTypes_, data.shaderTypes);
	const bool areTopologiesValid   = ContainerCompare(component.primTopologies_, data.primTopologyTypes);

	Assert::True(areIDsValid,         "TEST SYSTEMS: deserialized IDs data isn't correct");
	Assert::True(areShaderTypesValid, "TEST SYSTEMS: deserialized shader types data isn't correct");
	Assert::True(areTopologiesValid,  "TEST SYSTEMS: deserialized primitive topologies data isn't correct");
}

// --------------------------------------------------------

static void CompareTexTransformationsStatic(
	const ECS::TextureTransform& component,
	const std::vector<EntityID>& ids,
	const ECS::StaticTexTransParams& params)
{
	// check if the input data of static texture transformations is the same
	// as the data in the TextureTransform component

	const bool areIDsValid            = ContainerCompare(component.ids_, ids);
	const bool areTexTransformsValid  = ContainerCompare(component.texTransforms_, params.initTransform_);

	Assert::True(areIDsValid,           "ids data isn't correct");
	Assert::True(areTexTransformsValid, "textures transformation data isn't correct");
}

// --------------------------------------------------------

static void CompareTexTransformationsAtlasAnimations(
	const ECS::TextureTransform& component,
	const std::vector<EntityID>& ids,
	const ECS::AtlasAnimParams& params)
{
	// check if the input data of atlas animations is the same as the data 
	// in the TextureTransform component

	bool areTexRowsValid = true;
	bool areTexColumnsValid = true;
	bool areAnimDurationsValid = true;
	const bool areIDsValid = ContainerCompare(component.ids_, ids);
	const std::vector<ECS::TexAtlasAnimationData>& dataInComponent = component.texAtlasAnim_.data_;

	// test if texture rows/columns and animation durations are stored correctly
	for (u32 idx = 0; const ECS::TexAtlasAnimationData& data : dataInComponent)
		areTexRowsValid &= (data.texRows_ == params.texRows_[idx++]);
	
	for (u32 idx = 0; const ECS::TexAtlasAnimationData& data : dataInComponent)
		areTexColumnsValid &= (data.texColumns_ == params.texColumns_[idx++]);
	
	for (u32 idx = 0; const ECS::TexAtlasAnimationData& data : dataInComponent)
		areAnimDurationsValid &= (data.animDuration_ == params.animDurations_[idx++]);

	Assert::True(areIDsValid, "ids data isn't correct");
	Assert::True(areTexRowsValid, "texture rows values are invalid");
	Assert::True(areTexColumnsValid, "texture columns values are invalid");
	Assert::True(areAnimDurationsValid, "animation durations values are invalid");
}

// --------------------------------------------------------

static void CheckDirLightsProps(
	const ECS::Light& component,
	const std::vector<EntityID>& ids,
	const ECS::DirLightsInitParams& params)
{
	// here we check if directional light entities are 
	// stored correctly in Light component 

	const ECS::DirLights& lights = component.dirLights_;

	// check if ids are stored correctly in the component and in the dir lights container
	bool isIdsDataInCompValid = TestUtils::ContainerCompare(component.ids_, ids);
	bool isIdsDataInContainerValid = TestUtils::ContainerCompare(lights.ids_, ids);

	// check if each property of each stored light source is correct
	bool isAmbientsDataValid = true;
	bool isDiffusesDataValid = true;
	bool isSpecularsDataValid = true;
	bool isDirectionsDataValid = true;

	for (u32 idx = 0; const ECS::DirLight & light : lights.data_)
		isAmbientsDataValid &= (light.ambient_ == params.ambients[idx++]);

	for (u32 idx = 0; const ECS::DirLight & light : lights.data_)
		isDiffusesDataValid &= (light.diffuse_ == params.diffuses[idx++]);

	for (u32 idx = 0; const ECS::DirLight & light : lights.data_)
		isSpecularsDataValid &= (light.specular_ == params.speculars[idx++]);

	for (u32 idx = 0; const ECS::DirLight & light : lights.data_)
		isDirectionsDataValid &= (light.direction_ == params.directions[idx++]);


	Assert::True(isIdsDataInCompValid, "DIR LIGHT: the ids which are stored in the Light component are invalid");
	Assert::True(isIdsDataInContainerValid, "DIR LIGHT: the ids which are stored in the directional light container are invalid");

	Assert::True(isAmbientsDataValid,   "DIR LIGHT: ambients data aren't stored correctly in the component");
	Assert::True(isDiffusesDataValid,   "DIR LIGHT: diffuses data aren't stored correctly in the component");
	Assert::True(isSpecularsDataValid,  "DIR LIGHT: speculars data aren't stored correctly in the component");
	Assert::True(isDirectionsDataValid, "DIR LIGHT: directions data aren't stored correctly in the component");
}

// --------------------------------------------------------

static void CheckPointLightsProps(
	const ECS::Light& component,
	const std::vector<EntityID>& ids,
	const ECS::PointLightsInitParams& params)
{
	// here we check if point light entities are 
	// stored correctly in Light component 

	const ECS::PointLights& lights = component.pointLights_;

	// check if ids are stored correctly in the component and in the point lights container
	bool isIdsDataInCompValid = TestUtils::ContainerCompare(component.ids_, ids);
	bool isIdsDataInContainerValid = TestUtils::ContainerCompare(lights.ids_, ids);

	// check if each property of each stored light source is correct
	bool isAmbientsDataValid = true;
	bool isDiffusesDataValid = true;
	bool isSpecularsDataValid = true;
	bool isPositionsDataValid = true;
	bool isRangesDataValid = true;
	bool isAttenuationsDataValid = true;

	for (u32 idx = 0; const ECS::PointLight& light : lights.data_)
		isAmbientsDataValid &= (light.ambient_ == params.ambients[idx++]);

	for (u32 idx = 0; const ECS::PointLight& light : lights.data_)
		isDiffusesDataValid &= (light.diffuse_ == params.diffuses[idx++]);

	for (u32 idx = 0; const ECS::PointLight& light : lights.data_)
		isSpecularsDataValid &= (light.specular_ == params.speculars[idx++]);

	for (u32 idx = 0; const ECS::PointLight& light : lights.data_)
		isPositionsDataValid &= (light.position_ == params.positions[idx++]);

	for (u32 idx = 0; const ECS::PointLight& light : lights.data_)
		isRangesDataValid &= (light.range_ == params.ranges[idx++]);

	for (u32 idx = 0; const ECS::PointLight& light : lights.data_)
	{
		// attenuation params are inverted before storing into the component 
		// so we have to do the same here before the comparison

		const XMFLOAT3& origAtt = params.attenuations[idx++];
		XMFLOAT3 invAttParam;
		invAttParam.x = (origAtt.x) ? (1.0f / origAtt.x) : 0.0f;
		invAttParam.y = (origAtt.y) ? (1.0f / origAtt.y) : 0.0f;
		invAttParam.z = (origAtt.z) ? (1.0f / origAtt.z) : 0.0f;

		isAttenuationsDataValid &= (light.att_ == invAttParam);
	}


	Assert::True(isIdsDataInCompValid, "POINT LIGHT: the ids which are stored in the Light component are invalid");
	Assert::True(isIdsDataInContainerValid, "POINT LIGHT: the ids which are stored in the directional light container are invalid");

	Assert::True(isAmbientsDataValid, "POINT LIGHT: ambients data aren't stored correctly in the component");
	Assert::True(isDiffusesDataValid, "POINT LIGHT: diffuses data aren't stored correctly in the component");
	Assert::True(isSpecularsDataValid, "POINT LIGHT: speculars data aren't stored correctly in the component");

	Assert::True(isPositionsDataValid, "POINT LIGHT: positions data aren't stored correctly in the component");
	Assert::True(isRangesDataValid, "POINT LIGHT: ranges data aren't stored correcly in the component");
	Assert::True(isAttenuationsDataValid, "POINT LIGHT: attenuations data aren't stored correcly in the component");
}

// --------------------------------------------------------

static void CheckSpotLightsProps(
	const ECS::Light& component,
	const std::vector<EntityID>& ids,
	const ECS::SpotLightsInitParams& params)
{
	// here we check if spot light entities are 
	// stored correctly in Light component 

	const ECS::SpotLights& lights = component.spotLights_;

	// check if ids are stored correctly in the component and in the spot lights container
	bool isIdsDataInCompValid = TestUtils::ContainerCompare(component.ids_, ids);
	bool isIdsDataInContainerValid = TestUtils::ContainerCompare(lights.ids_, ids);

	// check if each property of each stored light source is correct
	bool isAmbientsDataValid = true;
	bool isDiffusesDataValid = true;
	bool isSpecularsDataValid = true;
	bool isPositionsDataValid = true;
	bool isDirectionsDataValid = true;
	bool isRangesDataValid = true;
	bool isSpotExpDataValid = true;
	bool isAttenuationsDataValid = true;

	for (u32 idx = 0; const ECS::SpotLight& light : lights.data_)
		isAmbientsDataValid &= (light.ambient_ == params.ambients[idx++]);

	for (u32 idx = 0; const ECS::SpotLight& light : lights.data_)
		isDiffusesDataValid &= (light.diffuse_ == params.diffuses[idx++]);

	for (u32 idx = 0; const ECS::SpotLight& light : lights.data_)
		isSpecularsDataValid &= (light.specular_ == params.speculars[idx++]);

	for (u32 idx = 0; const ECS::SpotLight& light : lights.data_)
		isPositionsDataValid &= (light.position_ == params.positions[idx++]);

	for (u32 idx = 0; const ECS::SpotLight& light : lights.data_)
		isRangesDataValid &= (light.range_ == params.ranges[idx++]);

	for (u32 idx = 0; const ECS::SpotLight & light : lights.data_)
		isDirectionsDataValid &= (light.direction_ == params.directions[idx++]);

	for (u32 idx = 0; const ECS::SpotLight & light : lights.data_)
		isSpotExpDataValid &= (light.spot_ == params.spotExponents[idx++]);

	for (u32 idx = 0; const ECS::SpotLight & light : lights.data_)
	{
		// attenuation params are inverted before storing into the component 
		// so we have to do the same here before the comparison

		const XMFLOAT3& origAtt = params.attenuations[idx++];
		XMFLOAT3 invAttParam;
		invAttParam.x = (origAtt.x) ? (1.0f / origAtt.x) : 0.0f;
		invAttParam.y = (origAtt.y) ? (1.0f / origAtt.y) : 0.0f;
		invAttParam.z = (origAtt.z) ? (1.0f / origAtt.z) : 0.0f;

		isAttenuationsDataValid &= (light.att_ == invAttParam);
	}
		

	Assert::True(isIdsDataInCompValid, "SPOT LIGHT: the ids which are stored in the Light component are invalid");
	Assert::True(isIdsDataInContainerValid, "SPOT LIGHT: the ids which are stored in the directional light container are invalid");

	Assert::True(isAmbientsDataValid, "SPOT LIGHT: ambients data aren't stored correctly in the component");
	Assert::True(isDiffusesDataValid, "SPOT LIGHT: diffuses data aren't stored correctly in the component");
	Assert::True(isSpecularsDataValid, "SPOT LIGHT: speculars data aren't stored correctly in the component");
	Assert::True(isAttenuationsDataValid, "SPOT LIGHT: attenuations data aren't stored correcly in the component");

	Assert::True(isPositionsDataValid, "SPOT LIGHT: positions data aren't stored correctly in the component");
	Assert::True(isDirectionsDataValid, "SPOT LIGHT: directions data aren't stored correctly in the component");
	Assert::True(isRangesDataValid, "SPOT LIGHT: ranges data aren't stored correcly in the component");
	Assert::True(isSpotExpDataValid, "SPOT LIGHT: spot exponents data aren't stored correcly in the component");
}


};  // namespace Utils
