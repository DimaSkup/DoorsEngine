////////////////////////////////////////////////////////////////////
// Filename:    Settings.cpp
// Description: contains settings for the engine; uses a singleton pattern
//
// Revising:    27.11.22
////////////////////////////////////////////////////////////////////
#include "Settings.h"


namespace Core
{

Settings::Settings()
{
    if (!LoadSettingsFromFile())
    {
        LogErr(LOG, "can't load settings for the engine");
        exit(-1);
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
        LogErr(LOG, "can't open the settings file: %s", pathSettingsFile);
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
        LogErr(LOG, "there is no integer value by key: %s", key);
        return 0;
    }
}

///////////////////////////////////////////////////////////

float Settings::GetFloat(const char* key) const 
{
    // get a float setting parameter by the input key

    if (settingsList_.contains(key))
    {
        const std::string& str = settingsList_.at(key);
        return (float)atof(str.c_str());
    }
    else
    {
        LogErr(LOG, "there is no float value by key: %s", key);
        return 0;
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

        LogErr(LOG, "we have some invalid value for setting by key: %s", key);
        return false;
    }
    else
    {
        LogErr(LOG, "there is no boolean value by key: %s", key);
        return false;
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
        LogErr(LOG, "there is no string value by key: %s", key);
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
        LogErr(LOG, "can't update setting by key (%s): input value is empty", key);
    }

    // ------------------------------------------

    if (settingsList_.contains(key))
    {
        settingsList_[key] = val;
    }
    else
    {
        LogErr(LOG, "there is no setting by key: %s", key);
    }
}

} // namespace Core
