// ================================================================================
// Filename:     StringHelper.h
// Description:  functional for strings convertation
// 
// Created:      
// ================================================================================
#pragma once

#include <string>

class StringHelper
{
public:
	// converters
	static std::wstring StringToWide(const std::string& str);
	static std::string ToString(const std::wstring& wstr);

	static std::string GetPathFromProjRoot(const std::string& fullPath);
};