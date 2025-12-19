////////////////////////////////////////////////////////////////////
// Filename:    EngineConfigs.cpp
//
// Revising:    27.11.22
////////////////////////////////////////////////////////////////////
#include "engine_configs.h"
#include <assert.h>

namespace Core
{

EngineConfigs::EngineConfigs()
{
    const char* cfgPath = "data/settings.txt";

    if (!LoadSettingsFromFile(cfgPath))
    {
        LogErr(LOG, "can't load settings/configs for the engine from file: %s", cfgPath);
        exit(-1);
    }
}

EngineConfigs::~EngineConfigs()
{
    settingsList_.clear();
}

//---------------------------------------------------------
// load engine settings from file by input path
//---------------------------------------------------------
bool EngineConfigs::LoadSettingsFromFile(const char* path)
{
    assert(path && path[0] != '\0');

    // open file
    FILE* pFile = nullptr;
    if ((pFile = fopen(path, "r")) == nullptr)
    {
        LogErr(LOG, "can't open a configs file: %s", path);
        return false;
    }


    constexpr int bufsize = 256;
    char buf[bufsize]{ '\0' };
    char key[bufsize]{ '\0' };
    char val[bufsize]{ '\0' };

    // read in all the pairs [setting_key => setting_value]
    while (fgets(buf, bufsize, pFile))
    {
        if (buf[0] == '\n')
            continue;

        sscanf(buf, "%s %s", key, val);

        // try to insert a pair [key => value]; if we didn't manage to do it we throw an exception
        if (!settingsList_.insert({ key, val }).second) 
        {
            fclose(pFile);
            LogErr(LOG, "can't insert a pair [key=>value] into the settings list: [key: %s => value: %s]", key, val);
            return false;
        }
    }

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:  get an integer setting parameter by the input key
//---------------------------------------------------------
int EngineConfigs::GetInt(const char* key) const
{
    assert(key && key[0] != '\0');

    if (!settingsList_.contains(key))
    {
        LogErr(LOG, "there is no integer value by key: %s", key);
        return 0;
    }

    const char* val = settingsList_.at(key).c_str();
    return atoi(val);
}

//---------------------------------------------------------
// Desc:  get a float setting parameter by the input key
//---------------------------------------------------------
float EngineConfigs::GetFloat(const char* key) const 
{
    assert(key && key[0] != '\0');

    if (!settingsList_.contains(key))
    {
        LogErr(LOG, "there is no float value by key: %s", key);
        return 0;
    }

    return (float)atof(settingsList_.at(key).c_str());
}

//---------------------------------------------------------
// Desc:  get a boolean setting parameter by the input key
//---------------------------------------------------------
bool EngineConfigs::GetBool(const char* key) const
{
    assert(key && key[0] != '\0');

    if (!settingsList_.contains(key))
    {
        LogErr(LOG, "there is no boolean value by key: %s", key);
        return false;
    }

    const char* val = settingsList_.at(key).c_str();

    if (strcmp(val, "true") == 0)
    {
        return true;
    }
    else if (strcmp(val, "false") == 0)
    {
        return false;
    }

    LogErr(LOG, "we have some invalid value (%s) for setting by key: %s", val, key);
    return false;
}

//---------------------------------------------------------
// Desc:  get a string setting parameter by the input key
//---------------------------------------------------------
const char* EngineConfigs::GetString(const char* key) const
{
    assert(key && key[0] != '\0');

    if (!settingsList_.contains(key))
    {
        LogErr(LOG, "there is no string value by key: %s", key);
        exit(-1);
    }

    return settingsList_.at(key).c_str();
   
}

//---------------------------------------------------------
// Desc:  update a value by key
//---------------------------------------------------------
void EngineConfigs::UpdateSettingByKey(const char* key, const std::string& val)
{
    assert(key && key[0] != '\0');

    if (val.empty())
    {
        LogErr(LOG, "can't update setting by key (%s): input value is empty", key);
        return;
    }

    if (!settingsList_.contains(key))
    {
        LogErr(LOG, "there is no setting by key: %s", key);
        return;
    }

    settingsList_[key] = val;
}

} // namespace Core
