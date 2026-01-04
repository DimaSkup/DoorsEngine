/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sound.cpp
    Desc:     encapsulates the .wav audio loading and playing capabilities

    Created:  27.11.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "sound.h"

#pragma warning(disable : 4996)

namespace Core
{

Sound::Sound()
{
}

Sound::~Sound()
{
    ReleaseTrack();
}

//---------------------------------------------------------
// Desc:  load in the .wav audio file
//---------------------------------------------------------
bool Sound::LoadTrack(
    IDirectSound8* pDirectSound,
    const char* filename,
    const long volume)
{
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "input filename is empty!");
        return false;
    }

    if (!LoadStereoWaveFile(pDirectSound, filename, volume))
    {
        LogErr(LOG, "can't load a wave file: %s", filename);
        return false;
    }

    if (FAILED(pSecondaryBuffer_->SetVolume(volume)))
    {
        LogErr(LOG, "can't set the volume of the secondary buffer");
        return false;
    }


    return true;
}

//---------------------------------------------------------
// Desc:  release the loaded audio buffer data
//---------------------------------------------------------
void Sound::ReleaseTrack()
{
    // release the wave file buffers
    ReleaseWaveFile();
}

//---------------------------------------------------------
// Desc:  start the .wav file playing
//---------------------------------------------------------
bool Sound::PlayTrack(const ULONG flags)
{
/*
    The PlayWaveFile() function will play the audio file stored in the secondary buffer.
    The moment you use the Play function it will automatically mix the audio onto the primary
    buffer and start it playing if it wasn't already. Also note that we set the position
    to start playing at the beginning of the secondary sound buffer otherwise it will continue
    from where it last stopped playing. And since we set the capabilities of the buffer
    to allow us to control the sound we set the volume to maximum here.
*/

    HRESULT hr = S_OK;

    if (!pSecondaryBuffer_)
    {
        LogErr(LOG, "can't play track: you didn't load any yet");
        return false;
    }

    hr = pSecondaryBuffer_->SetCurrentPosition(0);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't setup a position at the beginning of the sound buffer");
        return false;
    }

    // if looping is on the play the contents of the secondary sound buffer in a loop,
    // otherwise just play it once
    hr = pSecondaryBuffer_->Play(0, 0, flags);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't start playing sound");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  stop the secondary buffer from playing the sound
//---------------------------------------------------------
bool Sound::StopTrack()
{
    HRESULT hr = S_OK;

    hr = pSecondaryBuffer_->SetCurrentPosition(0);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't setup a position at the beginning of the sound buffer");
        return false;
    }

    hr = pSecondaryBuffer_->Stop();
    if (FAILED(hr))
    {
        LogErr(LOG, "can't stop playing sound");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   load in a .wav audio file and then
//         copy the data onto a new secondary buffer
//---------------------------------------------------------
bool Sound::LoadStereoWaveFile(
    IDirectSound8* pDirectSound,
    const char* filename,
    const long volume)
{
/*
    to start first open the .wav file and read in the header of the file. The header
    will contain all the information about the audio file so we can use that to
    create a secondary buffer to accommodate the audio data. The audio file header
    also tells us where the data begins and how big it is. You will notice I check for
    all the needed tags to ensure the audio file is not corrupt and is the proper
    wave file format containing RIFF, WAVE tags.
    I also do a couple other checks to ensure it is a 44.1KHz stereo 16bit audio file.
    If it is mono, 22.1KHz, 8bit, or anything else then it will fail ensuring we are
    only loading the exact format we want.
*/

    RiffWaveHeader      riffWaveHeader;
    SubChunkHeader      subChunkHeader;
    FmtType             fmtData;
    WAVEFORMATEX        waveFormat;
    DSBUFFERDESC        bufDesc;
    HRESULT             hr = S_OK;
    IDirectSoundBuffer* pTmpBuf = nullptr;
    UCHAR*              waveData = nullptr;
    UCHAR*              buffer = nullptr;
    size_t              count = 0;
    ULONG               dataSize = 0;
    ULONG               bufSize = 0;
    long                seekSize = 0;
    int                 error = 0;
    bool                foundFormat = false;
    bool                foundData = false;


    // open file for reading in binary
    FILE* pFile = fopen(filename, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open audio file");
        return false;
    }

    // read in the file header
    count = fread(&riffWaveHeader, sizeof(riffWaveHeader), 1, pFile);
    if (count != 1)
    {
        LogErr(LOG, "can't read in file header");
        return false;
    }

    if (strncmp(riffWaveHeader.chunkId, "RIFF", 4) != 0)
    {
        LogErr(LOG, "chunk ID isn't the RIFF format");
        return false;
    }

    if (strncmp(riffWaveHeader.format, "WAVE", 4) != 0)
    {
        LogErr(LOG, "file isn't the WAVE format");
        return false;
    }

    // .wav file are made up of sub chunks, and the first sub chunk we need to find
    // in the file is the fmt sub chunk. We parse through the file until it is found
    while (foundFormat == false)
    {
        count = fread(&subChunkHeader, sizeof(subChunkHeader), 1, pFile);
        if (count != 1)
        {
            LogErr(LOG, "can't read in the sub chunk header");
            return false;
        }

        // check if it is the fmt header, if not then move to the next chunk
        if (strncmp(subChunkHeader.subChunkId, "fmt ", 4) == 0)
        {
            foundFormat = true;
        }
        else
        {
            fseek(pFile, subChunkHeader.subChunkSize, SEEK_CUR);
        }
    }

    // verify that the format of the audio file is correct
    count = fread(&fmtData, sizeof(fmtData), 1, pFile);
    if (count != 1)
    {
        LogErr(LOG, "can't read in the format data");
        return false;
    }

    if (fmtData.audioFormat != WAVE_FORMAT_PCM)
    {
        LogErr(LOG, "audio format isn't WAVE_FORMAT_PCM");
        return false;
    }

    if (fmtData.numChannels != 2)
    {
        LogErr(LOG, "wave file wasn't recorded in stereo format");
        return false;
    }

    if (fmtData.sampleRate != 44100)
    {
        LogErr(LOG, "wave file wasn't recorded at a sample rate of 44.1 KHz");
        return false;
    }

    if (fmtData.bitsPerSample != 16)
    {
        LogErr(LOG, "the wave file wasn't recorded in 16 bit format");
        return false;
    }


    // find the actual data sub chunk
    seekSize = subChunkHeader.subChunkSize - 16;
    fseek(pFile, seekSize, SEEK_CUR);

    while (foundData == false)
    {
        // read in the sub chunk header
        count = fread(&subChunkHeader, sizeof(subChunkHeader), 1, pFile);
        if (count != 1)
        {
            LogErr(LOG, "can't read in the sub chunk header");
            return false;
        }

        // check if it is the data header, if not then move to the next chunk
        if (strncmp(subChunkHeader.subChunkId, "data", 4) == 0)
        {
            foundData = true;
        }
        else
        {
            fseek(pFile, subChunkHeader.subChunkSize, SEEK_CUR);
        }
    }

    // store the size of the data chunk
    dataSize = subChunkHeader.subChunkSize;

/*
    setup the secondary buffer that we will load the audio data onto. We have to
    first set the wave format and buffer description of the secondary buffer
    similar to how we did for the primary buffer (look direct_sound.cpp).
    There are some changes though since this is secondary and not primary in terms
    of dwFlags and dwBufferBytes
*/
    waveFormat.wFormatTag       = WAVE_FORMAT_PCM;
    waveFormat.nSamplesPerSec   = fmtData.sampleRate;
    waveFormat.wBitsPerSample   = fmtData.bitsPerSample;
    waveFormat.nChannels        = fmtData.numChannels;
    waveFormat.nBlockAlign      = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec  = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize           = 0;

    bufDesc.dwSize              = sizeof(DSBUFFERDESC);
    bufDesc.dwBufferBytes       = dataSize;
    bufDesc.dwReserved          = 0;
    bufDesc.lpwfxFormat         = &waveFormat;
    bufDesc.guid3DAlgorithm     = GUID_NULL;
    bufDesc.dwFlags             = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY; // stereo track
 
/*
    now the way to create a secondary buffer is fairly strange. The first step is that
    you create a temporary IDirectSoundBuffer with the sound buffer description you
    setup for the secondary buffer. If this succeeds then you can use that temporary
    buffer to create a IDirectSoundBuffer8 secondary buffer by calling QueryInterface
    with the IID_IDirectSoundBuffer8 parameter. If this succeeds then you can release
    the temporary buffer and the secondary buffer is ready for use
*/
    hr = pDirectSound->CreateSoundBuffer(&bufDesc, &pTmpBuf, NULL);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create a temporary sound buffer");
        return false;
    }

    hr = pTmpBuf->QueryInterface(IID_IDirectSoundBuffer8, (void**)&pSecondaryBuffer_);
    if (FAILED(hr))
    {
        LogErr(LOG, "failed to test the buffer format against the direct sound 8 interface and create the secondary buffer");
        return false;
    }

    SafeRelease(&pTmpBuf);


/*
    Load the wave data from the audio file. First, load it into a memory buffer so
    we can check and modify the data if we need to. Then lock the secondary buffer,
    copy the data to it, and then unlock it. This secondary buffer is now ready for use.

    Note that locking the secondary buffer can actually take in two pointers and
    two positions to write to. This is because it is a circular buffer and if you
    start by writing to the middle of it you will need the size of the buffer from that
    point so that you don't write outside the bounds of it. This is useful for
    streaming audio and such.
*/

    waveData = NEW UCHAR[dataSize];
    if (!waveData)
    {
        LogErr(LOG, "can't alloc memory for temp buffer to hold the wave file data");
        return false;
    }

    count = fread(waveData, 1, dataSize, pFile);
    if (count != dataSize)
    {
        LogErr(LOG, "can't read in the wave file data into the temp buffer");
        SafeDeleteArr(buffer);
        return false;
    }

    error = fclose(pFile);
    if (error != 0)
    {
        LogErr(LOG, "can't close the audio file");
        SafeDeleteArr(buffer);
        return false;
    }

    hr = pSecondaryBuffer_->Lock(0, dataSize, (void**)&buffer, (DWORD*)&bufSize, NULL, 0, 0);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't lock the secondary buffer to write data into it");
        SafeDeleteArr(buffer);
        return false;
    }

    // copy the wave data into the buffer
    memcpy(buffer, waveData, dataSize);

    hr = pSecondaryBuffer_->Unlock((void*)buffer, bufSize, NULL, 0);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't unlock the secondary buffer after the data has been written to it");
        SafeDeleteArr(buffer);
        return false;
    }

    // release the wave data since it was copied into the secondary buffer
    SafeDeleteArr(waveData);

    return true;
}

//---------------------------------------------------------
// Desc:  helper for releasing the secondary buffer
//---------------------------------------------------------
void Sound::ReleaseWaveFile()
{
    if (pSecondaryBuffer_)
    {
        pSecondaryBuffer_->Stop();
        pSecondaryBuffer_->Release();
        pSecondaryBuffer_ = nullptr;
    }
}

} // namespace 
