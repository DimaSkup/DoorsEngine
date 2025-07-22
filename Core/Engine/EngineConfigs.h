// =================================================================================
// Filename:    EngineConfigs.h
// Description: configs container for the engine
//
// Revising:    27.11.22
// =================================================================================
#pragma once

#include <log.h>
#include <string>
#include <map>
#include <typeinfo>


namespace Core
{

class EngineConfigs
{
public:
    EngineConfigs();
    ~EngineConfigs();

    EngineConfigs(EngineConfigs& other) = delete;   // should not be cloneable
    void operator=(const EngineConfigs&) = delete;  // should not be assignable


    // get a setting value in a particular type
    int         GetInt   (const char* key) const;
    float       GetFloat (const char* key) const;
    bool        GetBool  (const char* key) const;
    const char* GetString(const char* key) const;

    // for string source type we use this function for updating some setting by a key
    void UpdateSettingByKey(const char* key, const std::string & src);

    // for simple source types (int, float, bool, etc.) we use this function for
    // updating some setting by a key
    template<typename T>
    void UpdateSettingByKey(const char* key, T src);

    bool LoadSettingsFromFile();

private:
    std::map<std::string, std::string> settingsList_;
};

///////////////////////////////////////////////////////////

template<typename T>
void EngineConfigs::UpdateSettingByKey(const char* key, T val)
{
    // update a setting parameter with new value by a particular key

    if (!settingsList_.contains(key))
    {
        LogErr(LOG, "there is no such a key in settings: %s", key);
        return;
    }

    const std::type_info& valType = typeid(val);

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
        const char* typeName = typeid(T).name();
        LogErr(LOG, "failed to set key (%s): wrong source type: %s", key, typeName);
    }
}

} // namespace Core
