/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: direct_sound.cpp
    Desc:     encapsulates the basic DirectSound functionality

    Created:  27.11.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "direct_sound.h"


namespace Core
{

DirectSound::DirectSound()
{
}

DirectSound::~DirectSound()
{
    LogMsg("%srelease direct sound%s\n", CYAN, RESET);
    Shutdown();
}

//---------------------------------------------------------
// Desc:  handles getting an interface pointer to DirectSound and
//        the default primary sound buffer.
//---------------------------------------------------------
bool DirectSound::Init(HWND hwnd)
{
    HRESULT hr = S_OK;
    DSBUFFERDESC bufDesc;
    WAVEFORMATEX waveFormat;

    // init the direct sound interface pointer to the default sound device
    hr = DirectSoundCreate8(NULL, &pDirectSound_, NULL);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't init the direct sound interface pointer");
        return false;
    }

    // set the cooperative level to priority so the format of the
    // primary sound buffer can be modified
    hr = pDirectSound_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't set cooperative level to priority");
        return false;
    }


    // setup primary buffer description
    bufDesc.dwSize          = sizeof(DSBUFFERDESC);
    bufDesc.dwFlags         = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
    bufDesc.dwBufferBytes   = 0;
    bufDesc.dwReserved      = 0;
    bufDesc.lpwfxFormat     = NULL;
    bufDesc.guid3DAlgorithm = GUID_NULL;

    // get control of the primary sound buffer on the default sound device
    hr = pDirectSound_->CreateSoundBuffer(&bufDesc, &pPrimaryBuffer_, NULL);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create a primary sound buffer");
        return false;
    }

    // setup the format of the primary sound buffer, in this case it is a .WAV file
    // recorded at 44,100 samples per second in 16-bit stereo
    waveFormat.wFormatTag       = WAVE_FORMAT_PCM;
    waveFormat.nSamplesPerSec   = 44100;
    waveFormat.wBitsPerSample   = 16;
    waveFormat.nChannels        = 2;
    waveFormat.nBlockAlign      = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec  = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize           = 0;

    // set the primary buffer to be the wave format specified
    hr = pPrimaryBuffer_->SetFormat(&waveFormat);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't set a wave format for the primary buffer");
        return false;
    }

    // setup a listener interface to represent where in the 3D world the person
    // will be listening from
    hr = pPrimaryBuffer_->QueryInterface(IID_IDirectSound3DListener8, (LPVOID*)&pListener_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't obtain a listener interface");
        return false;
    }

    // set the initial position of the 3D listener
    pListener_->SetPosition(0, 0, 0, DS3D_IMMEDIATE);

    pPrimaryBuffer_->Release();
    pPrimaryBuffer_ = nullptr;

    return true;
}

//---------------------------------------------------------
// Desc:  release the primary buffer and DirectSound interfaces
//---------------------------------------------------------
void DirectSound::Shutdown()
{
    SafeRelease(&pListener_);
    SafeRelease(&pPrimaryBuffer_);
    SafeRelease(&pDirectSound_);
}

//---------------------------------------------------------
// Desc:  get Direct Sound interface 
//---------------------------------------------------------
IDirectSound8* DirectSound::GetDirectSound()
{
    return pDirectSound_;
}

} // namespace
