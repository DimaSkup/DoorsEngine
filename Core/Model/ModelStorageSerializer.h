#pragma once
#include <CoreCommon/Types.h>
#include <fstream>
#include <string>

class ModelStorageSerializer
{
public:
	ModelStorageSerializer();

	void WriteHeader(
		std::ofstream& fout, 
		const size numModels, 
		const ModelID lastModelID);

	void GenerateModelsRelativePaths(
		const std::string* names, 
		std::string* outPaths, 
		const size numNames);

	void WriteModelsInfo(
		std::ofstream& fout,
		const ModelID* ids, 
		const std::string* paths, 
		const size numModels);
};

