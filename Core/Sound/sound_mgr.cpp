/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sound_mgr.cpp
    Desc:     impelementation a manager (container)
              for all the currently loaded sounds

    Created:  02.01.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "sound_mgr.h"
#include "sound.h"
#include "direct_sound.h"
#include <Timers/game_timer.h>
#pragma warning (disable : 4996)


namespace Core
{

//---------------------------------------------------------
// GLOBAL instance of the sound manager
//---------------------------------------------------------
SoundMgr g_SoundMgr;

//---------------------------------------------------------

SoundMgr::SoundMgr()
{
}

SoundMgr::~SoundMgr()
{
    Shutdown();
    pDirectSound_->Shutdown();
    SafeDelete(pDirectSound_);
}

//---------------------------------------------------------
// Desc:  initialize all the sounds loading its metadata from file by filename
//
//        0. init direct sound interface and primary buffer
//        1. register a "dummy" sound which serve us as invalid
//           (we will receive it in cases when try to get a sound by wrong name of ID)
//
//        Then open a file with game sounds definitions:
//        2. read in sound's name and path
//        3. load a sound file by path
//        4. register sound in the manager (give it an ID and name)
//
// Ret:   true if everything is ok
//---------------------------------------------------------
bool SoundMgr::Init(const char* filename, const HWND& hwnd)
{
    assert(filename && filename[0] != '\0');

    SetConsoleColor(YELLOW);
    LogMsg("---------------------------------------------------------");
    LogMsg("                INITIALIZATION: SOUNDS                   ");
    LogMsg("---------------------------------------------------------");


    const TimePoint start = GameTimer::GetTimePoint();
    char buf[512];
    char soundName[MAX_LEN_SOUND_NAME];
    char soundPath[256];
    int  volume = 0;
    int  count = 0;
    FILE* pFile = nullptr;

    InitDirectSound(hwnd);
    InitDummySound();

    pFile = fopen(filename, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", filename);
        return false;
    }

    // skip comments section
    do
    {
        fgets(buf, sizeof(buf), pFile);
    }
    while (buf[0] == ';');


    // check if we read a correct file
    if (strncmp(buf, "sounds", 6) != 0)
    {
        LogErr(LOG, "read wrong file for sounds: %s", filename);
        fclose(pFile);
        return false;
    }

    // read in sounds definitions one by one [sound_name, sound_path, sound_volume]
    LogMsg(LOG, "load sounds from file: %s", filename);

    while (!feof(pFile) || buf[0] != '}')
    {
        fgets(buf, sizeof(buf), pFile);

        if (buf[0] != '\t')
            continue;

        count = sscanf(buf, "%s %s %d", soundName, soundPath, &volume);
        if (count != 3)
        {
            LogErr(LOG, "can't parse a sound definition from str buffer: %s", buf);
            continue;
        }

        AddSound(soundName, soundPath, volume);
        LogMsg("\tsound is loaded: %s", soundName);
    }


    const TimePoint      end = GameTimer::GetTimePoint();
    const TimeDurationMs dur = end - start;

    SetConsoleColor(MAGENTA);
    LogMsg("--------------------------------------");
    LogMsg("Init of sounds took: %.3f sec", dur.count() * 0.001f);
    LogMsg("--------------------------------------\n");
    SetConsoleColor(RESET);

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:  load and register a new sound object
// Args:  - name:  a name which is used to distinguish sound objects
//        - path:  sound file to load
//        - volume:  initial volume (in thousandths of decibel where 0 is full volume,
//                   and values goes in negative direction so -3000 means -30 db)
//---------------------------------------------------------
bool SoundMgr::AddSound(const char* name, const char* path, const long volume)
{
    assert(name && name[0] != '\0');
    assert(path && path[0] != '\0');


    // init a sound object
    Sound* pSound = NEW Sound();
    assert(pSound && "can't alloc memory for a sound object");

    // make a full path to the sound file
    char soundPath[128]{ '\0' };
    strcat(soundPath, soundsDirectory_);
    strcat(soundPath, path);

    if (!pSound->LoadTrack(pDirectSound_->GetDirectSound(), soundPath, volume))
    {
        LogErr(LOG, "can't load a sound track: %s", soundPath);

        pSound->ReleaseTrack();
        SafeDelete(pSound);

        return false;
    }


    // we managed to load a sound track so register it in the manager
    SoundID id = lastSoundId_++;

    ids_.push_back(id);
    names_.push_back(SoundName());
    sounds_.push_back(pSound);

    // setup a name
    strncpy(names_.back().name, name, MAX_LEN_SOUND_NAME);

    return true;
}

//---------------------------------------------------------
// Desc:  release sound objects and related memory
//---------------------------------------------------------
void SoundMgr::Shutdown()
{
    for (Sound* pSound : sounds_)
    {
        pSound->ReleaseTrack();
        SafeDelete(pSound);
    }

    ids_.purge();
    names_.purge();
    sounds_.purge();
}

//---------------------------------------------------------
// Desc:  return a ptr to sounds object by input id
//---------------------------------------------------------
Sound* SoundMgr::GetSound(const SoundID id)
{
    const index idx = ids_.get_idx(id);

    if ((idx > 0) && (idx < ids_.size()) && (ids_[idx] == id))
    {
        return sounds_[idx];
    }

    // return a ptr to "dummy" sound
    return sounds_[0];
}

//---------------------------------------------------------
// Desc:  return a sound ID by input name or
//        return 0 if there is no sound by such name
//---------------------------------------------------------
SoundID SoundMgr::GetSoundIdByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name");
        return INVALID_SOUND_ID;
    }

    for (index i = 0; i < sounds_.size(); ++i)
    {
        if (strcmp(names_[i].name, name) == 0)
            return ids_[i];
    }

    LogErr(LOG, "there is no sound by name: %s", name);
    return INVALID_SOUND_ID;
}

//---------------------------------------------------------
// Desc:  initialize a direct sound interface
//---------------------------------------------------------
void SoundMgr::InitDirectSound(const HWND& hwnd)
{
    pDirectSound_ = new DirectSound();
    assert(pDirectSound_);

    bool result = pDirectSound_->Init(hwnd);
    assert(result && "can't init direct sound");
}

//---------------------------------------------------------
// Desc: register a "dummy" sound which serve us as invalid
//       (we will receive it in cases when try to get a sound by wrong name of ID)
//---------------------------------------------------------
void SoundMgr::InitDummySound()
{
    assert(pDirectSound_);

    const long volume = 0;
    bool result = false;
    const char* filename = "dummy.wav";

    if (!AddSound("invalid_sound", filename, volume))
    {
        LogErr(LOG, "can't load a file for dummy sound: %s", filename);
        exit(0);
    }
}


} // namespace
