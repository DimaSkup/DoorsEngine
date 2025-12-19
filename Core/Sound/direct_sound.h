/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: direct_sound.h
    Desc:     encapsulates the basic DirectSound functionality

    Created:  27.11.2025  by DimaSkup
\**********************************************************************************/
#pragma once

//---------------------------
// LINKING
//---------------------------
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

//---------------------------
// INCLUDES
//---------------------------
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>


//---------------------------
// Class name: CDirectSound
//---------------------------
namespace Core
{

class DirectSound
{
public:
    DirectSound();
    DirectSound(const DirectSound&) = delete;
    ~DirectSound();

    bool Init(HWND hwnd);
    void Shutdown();

    IDirectSound8* GetDirectSound();

private:
    IDirectSound8*           pDirectSound_    = nullptr;
    IDirectSoundBuffer*      pPrimaryBuffer_  = nullptr;
    IDirectSound3DListener8* pListener_       = nullptr;
};

} // namespace
