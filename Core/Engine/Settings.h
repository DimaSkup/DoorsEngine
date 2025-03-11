// =================================================================================
// Filename:    Settings.h
// Description: contains settings for the engine; uses a singleton pattern
//
// Revising:    27.11.22
// =================================================================================
#pragma once

#include <CoreCommon/log.h>
#include <string>
#include <map>


namespace Core
{

class Settings
{
public:
	Settings();
	~Settings();

	Settings(Settings& other) = delete;        // should not be cloneable
	void operator=(const Settings&) = delete;  // should not be assignable


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

	void LoadSettingsFromFile();

private:
	std::map <std::string, std::string> settingsList_;
};

///////////////////////////////////////////////////////////

template<typename T>
void Settings::UpdateSettingByKey(const char* key, T val)
{
	// update a setting parameter with new value by a particular key

	if (!settingsList_.contains(key))
	{
		Log::Error("there is no such a key in settings: " + std::string(key));
		return;
	}

	int valType = typeid(val);

	// check if the src type is allowed
	if ((valType == typeid(float)) ||
		(valType == typeid(bool)) ||
		(valType == typeid(int)))
	{
		settingsList_[key] = std::to_string(val);
	}
	// we have wrong type
	else
	{
		std::string typeName{ typeid(T).name() };
		Log::Error("failed to set key (" + std::string(key) + "wrong source type : " + typeName);
	}
}

} // namespace Core