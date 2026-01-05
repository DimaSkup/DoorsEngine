/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sound.h
    Desc:     encapsulates the .wav audio loading and playing capabilities

    Created:  27.11.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include "direct_sound.h"


namespace Core
{

class Sound
{
private:
    // the structs used for .wave file format. If you're using a different format you
    // will want to replace these strusts with the ones required for your audio format
    struct RiffWaveHeader
    {
        char  chunkId[4];
        ULONG chunkSize;
        char  format[4];
    };

    struct SubChunkHeader
    {
        char  subChunkId[4];
        ULONG subChunkSize;
    };

    struct FmtType
    {
        USHORT audioFormat;
        USHORT numChannels;
        ULONG  sampleRate;
        ULONG  bytesPerSecond;
        USHORT blockAlign;
        USHORT bitsPerSample;
    };

public:
    Sound();
    Sound(const Sound&) = delete;
    ~Sound();

    bool LoadTrack(IDirectSound8* pDirectSound, const char* filename, const long volume);
    void ReleaseTrack();

    void CreateDoneEvent();

    bool PlayTrack(ULONG flags = 0);
    bool StopTrack();

    inline IDirectSoundBuffer8* GetBuffer() const { return pSecondaryBuffer_; }

private:
    bool LoadStereoWaveFile(IDirectSound8* pDirectSound, const char* filename, const long volume);
    void ReleaseWaveFile();

private:
    IDirectSoundBuffer8* pSecondaryBuffer_ = nullptr;
    IDirectSoundNotify8* pNotifyDone_      = nullptr;
    HANDLE               eventDone_        = nullptr;
};

} // namespace
