// =================================================================================
// Filename:     ModelExporter.cpp
// Created:      11.11.24
// =================================================================================
#include "ModelExporter.h"

#include <CoreCommon/FileSystemPaths.h>
#include "../Texture/TextureTypes.h"
#include "ImgConverter.h"
#include "../Texture/TextureMgr.h"

#include <exception>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

namespace Core
{

ModelExporter::ModelExporter()
{
	// create a directory for this exported model (if not exist)
	if (!fs::exists(g_RelPathAssetsDir))
		fs::create_directory(g_RelPathAssetsDir);
}

///////////////////////////////////////////////////////////

void ModelExporter::ExportIntoDE3D(
	ID3D11Device* pDevice,
	const BasicModel& model,
	const std::string& dstRelativePath)
{
	// store model's data into a .de3d file by path

	const fs::path fullPath = g_RelPathAssetsDir + dstRelativePath;
	const fs::path targetDir = fullPath.parent_path().string();

	// create a directory for this exported model (if not exist)
	if (!fs::exists(targetDir))
		fs::create_directory(targetDir);

	// open .de3d file for writing model's data
	std::ofstream fout(fullPath.string(), std::ios::out | std::ios::binary);
	if (!fout)
	{
		Log::Error("can't open a file for exporting: " + fullPath.string());
		return;
	}

	
	WriteHeader(fout, model);
	WriteMaterials(pDevice, fout, model, targetDir.string());
	WriteSubsetTable(fout, model.meshes_.subsets_, model.numSubsets_);

	WriteModelSubsetsAABB(fout, model.modelAABB_, model.subsetsAABB_, model.numSubsets_);
	WriteVertices(fout, model.vertices_, model.numVertices_);
	WriteIndices(fout, model.indices_, model.numIndices_);
}


// =================================================================================
//                               PRIVATE METHODS
// =================================================================================

void ModelExporter::WriteHeader(std::ofstream& fout, const BasicModel& model)
{
	fout << "***************Header***************\n";
	fout << "#ModelID "   << model.id_ << '\n';
	fout << "#ModelName " << model.name_ << '\n';
	fout << "#ModelType " << (int)model.type_ << '\n';
	fout << "#Meshes "    << model.numSubsets_ << '\n';
	fout << "#Vertices "  << model.numVertices_ << '\n';
	fout << "#Indices "   << model.numIndices_ << '\n';
	fout << "#Bones "     << model.numBones_ << '\n';
	fout << "#AnimationClips " << model.numAnimClips_ << '\n';
	fout << '\n';
}

///////////////////////////////////////////////////////////

void WriteTextureIntoFile(
	const fs::path& texFullPath,
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	ID3D11Resource* pTexResource)
{
	// store texture (if necessary) as .dds texture by texFullPath
	// 
	// 1. define if a texture by path exist (if not exist we go to step 3)
	// 2. if exist we check if we need to rewrite it (if no we just go out)
	// 3. load texture from memory and process it
	// 4. write a texture into the .dds file

	// target image params
	//DXGI_FORMAT targetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT targetFormat = DXGI_FORMAT_BC3_UNORM;
	const DirectX::TEX_FILTER_FLAGS filter = DirectX::TEX_FILTER_DEFAULT;


	// ---------------------------------------------

	// defines if we need to rewrite a .dds texture by input path
	bool needToRewrite = false;

	// if such .dds texture already exists
	if (fs::exists(texFullPath))
	{
		DirectX::TexMetadata metadata;

		HRESULT hr = DirectX::GetMetadataFromDDSFile(
			texFullPath.wstring().c_str(),
			DirectX::DDS_FLAGS_NONE,
			metadata);

		Assert::NotFailed(hr, "can't get metadata from file: " + texFullPath.string());

		// check if it has proper params
		needToRewrite |= (metadata.format != targetFormat);
		needToRewrite |= (metadata.mipLevels == 1);

		// if there is no need to rewrite a texture file we just go out
		if (!needToRewrite)
			return;
	}

	// ---------------------------------------------

	// load a texture data from memory and store it as .dds file
	DirectX::ScratchImage srcImage;
	//DirectX::ScratchImage procImage;
	
	DirectX::ScratchImage dstImage;

	ImgReader::ImgConverter converter;
	converter.LoadFromMemory(pDevice, pContext, pTexResource, srcImage);

	// if we need to execute any processing
	if (srcImage.GetMetadata().format != targetFormat)
	{
		DirectX::ScratchImage mipChain(converter.GenMipMaps(srcImage, filter));

		converter.ProcessImage(mipChain, targetFormat, dstImage);
		converter.SaveToFile(dstImage, DirectX::DDS_FLAGS_NONE, texFullPath);
	}
	// src texture is already has the proper format
	else
	{
		dstImage = std::move(converter.GenMipMaps(srcImage, filter));
		converter.SaveToFile(dstImage, DirectX::DDS_FLAGS_NONE, texFullPath);
	}
}

///////////////////////////////////////////////////////////

void WriteSubsetTextures(
	std::ofstream& fout, 
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	TexID* texIDs, 
	const int subsetID,
	const std::string& texDirFullPath)
{
#if 0
	const int offset = subsetID * NUM_TEXTURE_TYPES;           // each subset (mesh) has 22 textures types
	int numTexTypes = 0;                        // of this mesh (subset)

	TextureMgr& texMgr = *TextureMgr::Get();
	Texture* textures[NUM_TEXTURE_TYPES]{ nullptr };
	TexType texTypes[NUM_TEXTURE_TYPES];
	ID3D11Resource* texResources[NUM_TEXTURE_TYPES]{ nullptr };

	std::string texFilenames[NUM_TEXTURE_TYPES];


	// define what kinds of textures this mesh has and store them as .dds files
	for (int type = 0; type < NUM_TEXTURE_TYPES; ++type)
	{
		TexType texType = TexType(type);
		TexID texID = texIDs[offset + texType];

		// if we have some real texture (not an unloaded)
		if (texID != 0)
		{
			textures[numTexTypes] = texMgr.GetTexPtrByID(texID);
			texTypes[numTexTypes] = texType;

			// how many textures this mesh has
			++numTexTypes;
		}
	}

	// get resource of each texture
	for (int i = 0; i < numTexTypes; ++i)
		texResources[i] = textures[i]->GetResource();

	// generate target filenames
	for (int i = 0; i < numTexTypes; ++i)
		texFilenames[i] = fs::path(textures[i]->GetName()).stem().string() + +".dds";

	// write the number of textures for this mesh
	fout << "TexturesNumber: " << numTexTypes << '\n';

	// write "tex_type_code => filename" for each texture
	for (int i = 0; i < numTexTypes; ++i)
		fout << "TexType " << texTypes[i] << " : " << texFilenames[i] << '\n';

	// --------------------------------------------

	// store model's textures as .dds files
	for (int i = 0; i < numTexTypes; ++i)
	{
		fs::path texDstFullPath = texDirFullPath + texFilenames[i];
		WriteTextureIntoFile(texDstFullPath, pDevice, pContext, texResources[i]);
	}

	fout << std::endl;
#endif
}


///////////////////////////////////////////////////////////


void ModelExporter::WriteMaterials(
	ID3D11Device* pDevice,
	std::ofstream& fout, 
	const BasicModel& model,
	const std::string& targetDirFullPath)
{
#if 0
	// write materials (light properties) and textures data of each model's subset

	ID3D11DeviceContext* pContext = nullptr;
	pDevice->GetImmediateContext(&pContext);

	const std::string texDirFullPathStr = targetDirFullPath + "/textures/";
	const fs::path texDirFullPath = texDirFullPathStr;

	// create dir for textures
	if (!fs::exists(texDirFullPath))
		fs::create_directory(texDirFullPath);


	fout << "***************Materials*********************\n";

	// write materials/textures data of each model's mesh (subset)
	for (int i = 0; i < model.numSubsets_; ++i)
	{
		WriteMaterialProps(fout, model.materials_[i]);

		fout << "AlphaClip: " << model.meshes_.subsets_[i].alphaClip_ << '\n';
		fout << "Effect: " << "Normal\n";

		// --------------------------------------------

		WriteSubsetTextures(
			fout, 
			pDevice, 
			pContext,
			model.texIDs, 
			i,
			texDirFullPathStr);
	}
#endif
}

///////////////////////////////////////////////////////////

void ModelExporter::WriteSubsetTable(
	std::ofstream& fout,
	const MeshGeometry::Subset* subsets,
	const int numSubsets)
{
	// write data of each subset in the model

	Assert::True((subsets != nullptr) && (numSubsets > 0), "wrong subsets data");

	fout << "***************SubsetTable*******************\n";

	for (int i = 0; i < numSubsets; ++i)
	{
		fout << "SubsetID: "    << subsets[i].id << ' '
			 << "VertexStart: " << subsets[i].vertexStart << ' '
			 << "VertexCount: " << subsets[i].vertexCount << ' '
			 << "IndexStart: "  << subsets[i].indexStart << ' '
			 << "IndexCount: "  << subsets[i].indexCount << std::endl;
	}

	fout << std::endl;
}

///////////////////////////////////////////////////////////

void ModelExporter::WriteModelSubsetsAABB(
	std::ofstream& fout,
	const DirectX::BoundingBox& modelAABB,
	const DirectX::BoundingBox* subsetsAABBs,
	const int numAABB)
{
	Assert::True((subsetsAABBs != nullptr) && (numAABB > 0), "wrong input data");

	// write data about AABB of the whole model and each model's subset

	fout << "***************AABB(center,extents)*******************\n";

	fout << "Model: ";
	WriteAABB(fout, modelAABB);

	for (int i = 0; i < numAABB; ++i)
	{
		fout << "Subset_" << i << ": ";
		WriteAABB(fout, subsetsAABBs[i]);
	}

	fout << std::endl;
}

///////////////////////////////////////////////////////////


void ModelExporter::WriteVertices(
	std::ofstream& fout,
	const Vertex3D* vertices,
	const int numVertices)
{
	// write data of each vertex in the model

	Assert::True((vertices != nullptr) && (numVertices > 0), "wrong vertices data");

	fout << "***************Vertices**********************\n";
	fout.write((char*)vertices, sizeof(Vertex3D) * numVertices);
}

///////////////////////////////////////////////////////////

void ModelExporter::WriteIndices(
	std::ofstream& fout,
	const UINT* indices,
	const int numIndices)
{
	// write indices data (faces/triangles) of the model

	Assert::True((indices != nullptr) && (numIndices > 0), "wrong subsets data");

	fout << "***************Triangles*********************\n";
	fout.write((char*)indices, sizeof(UINT) * numIndices);
}

///////////////////////////////////////////////////////////

void ModelExporter::WriteMaterialProps(std::ofstream& fout, const Material& mat)
{
	// write input material into file output stream

	fout << "Ambient: "
		<< mat.ambient.x << ' '
		<< mat.ambient.y << ' '
		<< mat.ambient.z << '\n';

	fout << "Diffuse: "
		<< mat.diffuse.x << ' '
		<< mat.diffuse.y << ' '
		<< mat.diffuse.z << '\n';

	fout << "Specular: "
		<< mat.specular.x << ' '
		<< mat.specular.y << ' '
		<< mat.specular.z << '\n';

	fout << "SpecPower: " << mat.specular.w << '\n';

	fout << "Reflectivity: "
		<< mat.reflect.x << ' '
		<< mat.reflect.y << ' '
		<< mat.reflect.z << '\n';
}

///////////////////////////////////////////////////////////

void ModelExporter::WriteAABB(
	std::ofstream& fout,
	const DirectX::BoundingBox& aabb)
{
	// write the AABB data into the file output stream
	fout << aabb.Center.x << ' '
		<< aabb.Center.y << ' '
		<< aabb.Center.z << ' '
		<< aabb.Extents.x << ' '
		<< aabb.Extents.y << ' '
		<< aabb.Extents.z << '\n';
}

} // namespace Core
