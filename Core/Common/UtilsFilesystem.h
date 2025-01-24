// *********************************************************************************
// Filename:     UtilsFilesystem.h
// Description:  contains some common utils for work with files (writing/reading)
// 
// Created:      28.10.24
// *********************************************************************************
#pragma once

#include <fstream>
#include <vector>

namespace CoreUtils
{

// ****************************************************************************
// file read/write API

template<typename T>
inline static void FileWrite(std::ofstream& fout, const std::vector<T>& arr)
{
	// write all the arr content into the file stream
	fout.write((const char*)arr.data(), arr.size() * sizeof(T));
}

template<typename T>
inline static void FileWrite(std::ofstream& fout, T* pData, const size_t count = 1)
{
	// write into the file stream bytes in quantity of count;
	// NOTE: if count == 1 it means that we want to write only one basic (int, size_t, ect.) variable
	fout.write((const char*)pData, count * sizeof(T));
}

template<typename T>
inline static void FileRead(std::ifstream& fin, std::vector<T>& arr)
{
	// read in data from the file stream into the arr
	fin.read((char*)arr.data(), arr.size() * sizeof(T));
}

template<typename T>
inline static void FileRead(std::ifstream& fin, T* pData, const size_t count = 1)
{
	// read in from the file stream an array bytes in quantity of count
	// NOTE: if count == 1 it means that we want to read only one basic (int, size_t, ect.) variable
	fin.read((char*)pData, count * sizeof(T));
}


}