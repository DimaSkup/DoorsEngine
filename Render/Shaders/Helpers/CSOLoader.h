#pragma once

#include "../../Common/MemHelpers.h"
#include "../../Common/Assert.h"
#include "../../Common/log.h"
#include <fstream>

namespace Render
{

static void LoadCSO(
	const std::string& shaderPath,
	uint8_t** blob,
	std::streampos& len)
{
	//
	// read in bytecode from the input CSO (compiled shader object) file
	//

	std::ifstream inShaderFile;

	try
	{
		inShaderFile.open(shaderPath, std::ios::binary | std::ios::ate);
		Assert::True(inShaderFile.is_open(), "can't load shader bytecode from: " + shaderPath);

		// define a length of the compiled shader object (CSO)
		len = inShaderFile.tellg();
		Assert::True(!inShaderFile.fail(), "can't read data from: " + shaderPath);

		// alloc memory for the shader bytecode
		*blob = new uint8_t[len]{ 0 };

		inShaderFile.seekg(0, std::ios::beg);
		Assert::True(!inShaderFile.fail(), "can't read data from: " + shaderPath);
	
		// read in shader's bytecode
		inShaderFile.read((char*)*blob, len);
		Assert::True(!inShaderFile.fail(), "can't read data from: " + shaderPath);

		inShaderFile.close();
	}
	catch (std::bad_alloc& e)
	{
		inShaderFile.close();
		Log::Error(e.what());
		Log::Error("can't allocate memory for shader bytecode from file:" + shaderPath);
		throw LIB_Exception("can't load CSO: " + shaderPath);
	}
	catch (LIB_Exception& e)
	{
		inShaderFile.close();
		SafeDeleteArr(blob);

		Log::Error(e);
		Log::Error("can't load compiled shader obj from file:" + shaderPath);
		throw LIB_Exception("can't load CSO: " + shaderPath);
	}
}

} // namespace Render