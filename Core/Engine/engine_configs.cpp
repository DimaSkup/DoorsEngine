////////////////////////////////////////////////////////////////////
// Filename:    EngineConfigs.cpp
//
// Revising:    27.11.22
////////////////////////////////////////////////////////////////////
#include "engine_configs.h"
#include <assert.h>

#pragma warning (disable : 4996)


namespace Core
{

EngineConfigs::EngineConfigs()
{
    const char* cfgPath = "data/settings.cfg";

    if (!LoadSettingsFromFile(cfgPath))
    {
        LogFatal(LOG, "can't load engine's settings/configs file: %s", cfgPath);
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

    FILE* pFile = fopen(path, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open a configs file: %s", path);
        return false;
    }

    int count = 0;
    char buf[256]{ '\0' };
    char key[128]{ '\0' };
    char val[128]{ '\0' };

    // read in all the pairs [setting_key => setting_value]
    while (fgets(buf, sizeof(buf), pFile))
    {
        if (buf[0] == '\n')
            continue;

        count = sscanf(buf, "%s %s", key, val);
        assert(count == 2);

        // skip comment
        if (key[0] == '#')
            continue;

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
        return true;

    else if (strcmp(val, "false") == 0)
        return false;

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
        LogFatal(LOG, "there is no string value by key: %s", key);
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
