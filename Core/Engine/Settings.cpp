////////////////////////////////////////////////////////////////////
// Filename:    Settings.cpp
// Description: contains settings for the engine; uses a singleton pattern
//
// Revising:    27.11.22
////////////////////////////////////////////////////////////////////
#include "Settings.h"
#include <CoreCommon/Assert.h>
#include <CoreCommon/FileSystemPaths.h>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace Core
{

Settings::Settings()
{
	try
	{
		LoadSettingsFromFile();
	}
	catch (EngineException& e)
	{
		const std::string errorMsg = "can't load settings file";
		Log::Error(e);
		Log::Error(errorMsg);
		throw EngineException(errorMsg);
	}
}

Settings::~Settings()
{
	settingsList_.clear();
}

///////////////////////////////////////////////////////////

void Settings::LoadSettingsFromFile()
{
	// load engine settings from the "settings.txt" file

	const std::string pathToSettingsFile = "data/settings.txt";

	// check if we have such a file
	fs::path settingsPath = pathToSettingsFile;
	Assert::True(fs::exists(settingsPath), "there is no file: " + settingsPath.string());

	// open file
	std::fstream fin(pathToSettingsFile);
	Assert::True(fin.is_open(), "can't open the settings file");

	// read in all the pairs [setting_key => setting_value]
	while (!fin.eof())
	{
		std::string key;
		std::string value;

		fin >> key >> value;

		// try to insert a pair [key => value]; if we didn't manage to do it we throw an exception
		if (!settingsList_.insert({ key, value }).second) 
		{
			throw EngineException("can't insert a pair [key=>value] into "
				                  "the settings list: "
				                  "[key: " + key + " => value: " + value);
		}
	}

	fin.close();
}

///////////////////////////////////////////////////////////

int Settings::GetInt(const char* key) const
{
	// get an integer setting parameter by the input key

	try
	{
		const std::string& val = settingsList_.at(key);
		return stoi(val);
		
	}
	catch (std::out_of_range& e)
	{
		Log::Error(e.what());
		throw EngineException("can't find an integer by key: " + std::string(key));
	}
}

///////////////////////////////////////////////////////////

float Settings::GetFloat(const char* key) const 
{
	// get a float setting parameter by the input key

	try
	{
		const std::string& val = settingsList_.at(key);
		return stof(val);
	}
	catch (std::out_of_range& e)
	{
		Log::Error(e.what());
		throw EngineException("can't find a float by key: " + std::string(key));
	}
}

///////////////////////////////////////////////////////////

bool Settings::GetBool(const char* key) const
{
	// get a boolean setting parameter by the input key

	try
	{
		const std::string& val = settingsList_.at(key);

		if (val == "true")
		{
			return true;
		}
		else if (val == "false")
		{
			return false;
		}
		else
		{
			throw EngineException("can't convert value from string into bool by key: " + std::string(key));
		}
	}
	catch (std::out_of_range& e)
	{
		Log::Error(e.what());
		throw EngineException("can't find a boolean by key: " + std::string(key));
	}
}

///////////////////////////////////////////////////////////

std::string Settings::GetString(const char* key) const
{
	// get a string setting parameter by the input key

	try
	{
		const std::string& str = settingsList_.at(key);
		Assert::NotEmpty(str.empty(), "the setting value by key (" + std::string(key) + ") is empty");

		return str;
	}
	catch (std::out_of_range& e)
	{
		Log::Error(e.what());
		throw EngineException("can't find a string by key: " + std::string(key));
	}
}

///////////////////////////////////////////////////////////

void Settings::UpdateSettingByKey(const char* key, const std::string& val)
{
	try 
	{
		settingsList_.at(key) = val;
	}
	catch (std::out_of_range& e)
	{
		Log::Error(e.what());
		throw EngineException("can't update a setting by key: " + std::string(key));
	}
}

} // namespace Core