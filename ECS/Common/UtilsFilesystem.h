// *********************************************************************************
// Filename:     UtilsFilesystem.h
// Description:  contains some common utils for work with files (writing/reading)
// *********************************************************************************
#pragma once

#include <fstream>

namespace ECS
{

	// ****************************************************************************
	// file read/write API

	
	template<typename T>
	inline static void FileWrite(std::ofstream& fout, T* pData, const u32 count = 1)
	{
		// write into the file stream bytes in quantity of count;
		// NOTE: if count == 1 it means that we want to write only one basic (int, u32, ect.) variable
		fout.write((const char*)pData, count * sizeof(T));
	}

	// ----------------------------------------------------

	template<typename T>
	inline static void FileWrite(std::ofstream& fout, T data)
	{
		// write single value of T type into the file
		fout.write((const char*)&data, sizeof(T));
	}

	// ----------------------------------------------------

	template<typename T>
	inline static void FileRead(std::ifstream& fin, T* pData, const u32 count = 1)
	{
		// read in from the file stream an array bytes in quantity of count
		// NOTE: if count == 1 it means that we want to read only one basic (int, u32, ect.) variable
		fin.read((char*)pData, count * sizeof(T));
	}

	// ----------------------------------------------------

	template<typename T>
	inline static void FileRead(std::ifstream& fin, T& val)
	{
		// read in a single value from the input file stream
		fin.read((char*)&val, sizeof(T));
	}


}; // namespace Utils
