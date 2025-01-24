// =================================================================================
// Filename:    Settings.h
// Description: contains settings for the engine; uses a singleton pattern
//
// Revising:    27.11.22
// =================================================================================
#pragma once

#include "../Common/log.h"
#include <string>
#include <map>
#include <stdexcept>

class Settings
{
public:
	Settings();
	~Settings();


	// get a setting value in a particular type
	int GetInt(const char* key) const;
	float GetFloat(const char* key) const;
	bool GetBool(const char* key) const;
	std::string GetString(const char* key) const;

	// for string source type we use this function for updating some setting by a key
	void UpdateSettingByKey(const char* key, const std::string & src);

	// for simple source types (int, float, bool, etc.) we use this function for
	// updating some setting by a key
	template<typename T>
	void UpdateSettingByKey(const char* key, T src);

private:
	Settings(Settings & other) = delete;        // should not be cloneable
	void operator=(const Settings &) = delete;  // should not be assignable

	void LoadSettingsFromFile();

private:
	std::map <std::string, std::string> settingsList_;
};

///////////////////////////////////////////////////////////


template<typename T>
void Settings::UpdateSettingByKey(const char* key, T val)
{
	// update a setting parameter with new value by a particular key

	try
	{
		std::string& param = settingsList_.at(key);

		// check if the src type is allowed
		if ((typeid(val) == typeid(float)) ||
			(typeid(val) == typeid(bool)) ||
			(typeid(val) == typeid(int)))
		{
			param = std::to_string(val);
		}
		// we have wrong type
		else
		{
			std::string typeName{ typeid(T).name() };
			Log::Error("wrong source type: " + typeName);
		}
	}
	catch (std::out_of_range& e)
	{
		Log::Error(e.what());
		throw EngineException("can't find a param by key: " + std::string(key));
	}
}

