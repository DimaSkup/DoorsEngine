////////////////////////////////////////////////////////////////////
// Filename:    Settings.cpp
// Description: contains settings for the engine; uses a singleton pattern
//
// Revising:    27.11.22
////////////////////////////////////////////////////////////////////
#include "Settings.h"
#include <CoreCommon/FileSystemPaths.h>


namespace Core
{

Settings::Settings()
{
    if (!LoadSettingsFromFile())
    {
        sprintf(g_String, "can't load settings for the engine");
        LogErr(g_String);
        throw EngineException(g_String);
    }
}

Settings::~Settings()
{
    settingsList_.clear();
}

///////////////////////////////////////////////////////////

bool Settings::LoadSettingsFromFile()
{
    // load engine settings from the "settings.txt" file

    const char* pathSettingsFile = "data/settings.txt";

    // open file
    FILE* pFile = nullptr;
    if ((pFile = fopen(pathSettingsFile, "r")) == nullptr)
    {
        sprintf(g_String, "can't open the settings file: %s", pathSettingsFile);
        LogErr(g_String);
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
            sprintf(g_String, "can't insert a pair [key=>value] into the settings list: [key: %s => value: %s]", key, val);
            LogErr(g_String);
            return false;
        }
    }

    fclose(pFile);
    return true;
}

///////////////////////////////////////////////////////////

int Settings::GetInt(const char* key) const
{
    // get an integer setting parameter by the input key

    if (settingsList_.contains(key))
    {
        const char* val = settingsList_.at(key).c_str();
        return atoi(val);
    }
    else
    {
        sprintf(g_String, "there is no integer value by key: %s", key);
        LogErr(g_String);
        exit(-1);
    }
}

///////////////////////////////////////////////////////////

float Settings::GetFloat(const char* key) const 
{
    // get a float setting parameter by the input key

    if (settingsList_.contains(key))
    {
        return (float)atof(settingsList_.at(key).c_str());
    }
    else
    {
        sprintf(g_String, "there is no float value by key: %s", key);
        LogErr(g_String);
        exit(-1);
    }
}

///////////////////////////////////////////////////////////

bool Settings::GetBool(const char* key) const
{
    // get a boolean setting parameter by the input key

    if (settingsList_.contains(key))
    {
        const char* val = settingsList_.at(key).c_str();

        if (strcmp(val, "true") == 0)
        {
            return true;
        }
        else if (strcmp(val, "false") == 0)
        {
            return false;
        }

        sprintf(g_String, "we have some invalid value for setting by key: %s", key);
        LogErr(g_String);
        exit(-1);
    }
    else
    {
        sprintf(g_String, "there is no boolean value by key: %s", key);
        LogErr(g_String);
        exit(-1);
    }
}

///////////////////////////////////////////////////////////

const char* Settings::GetString(const char* key) const
{
    // get a string setting parameter by the input key
    if (settingsList_.contains(key))
    {
        return settingsList_.at(key).c_str();
    }
    else
    {
        sprintf(g_String, "there is no string value by key: %s", key);
        LogErr(g_String);
        exit(-1);
    }
}

///////////////////////////////////////////////////////////

void Settings::UpdateSettingByKey(const char* key, const std::string& val)
{
    if ((key == nullptr) || (key[0] == '\0'))
    {
        LogErr("can't update setting (input key is empty)");
    }

    if (val.empty())
    {
        sprintf(g_String, "can't update setting by key (%s): input value is empty", key);
        LogErr(g_String);
    }

    // ------------------------------------------

    if (settingsList_.contains(key))
    {
        settingsList_[key] = val;
    }
    else
    {
        sprintf(g_String, "there is no setting by key: %s", key);
        LogErr(g_String);
    }
}

} // namespace Core
