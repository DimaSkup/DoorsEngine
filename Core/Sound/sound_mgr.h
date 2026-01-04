/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sound_mgr.h
    Desc:     a manager (container) for all the currently loaded sounds

    Created:  02.01.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>
#include <cvector.h>


namespace Core
{
// forward declaration (pointer use only)
class DirectSound;
class Sound;

//---------------------------------------------------------

class SoundMgr
{
public:
    SoundMgr();
    ~SoundMgr();

    bool    Init(const char* filename, const HWND& hwnd);
    void    Shutdown();

    Sound*  GetSound(const SoundID id);
    SoundID GetSoundIdByName(const char* name);

private:
    bool    AddSound(const char* name, const char* path, const long volume);

    void    InitDirectSound(const HWND& hwnd);
    void    InitDummySound();

private:
    SoundID            lastSoundId_ = 0;
    DirectSound*       pDirectSound_ = nullptr;

    cvector<SoundID>   ids_;
    cvector<SoundName> names_;
    cvector<Sound*>    sounds_;

    char               soundsDirectory_[13] = { "data/sounds/" };
};

//---------------------------------------------------------
// GLOBAL instance of the sound manager
//---------------------------------------------------------
extern SoundMgr g_SoundMgr;

} // namespace
