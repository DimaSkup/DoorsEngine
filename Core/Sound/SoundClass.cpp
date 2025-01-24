////////////////////////////////////////////////////////////////////
// Filename:     SoundClass.cpp
// Description:  implementation of the SoundClass
// Created:      05.01.23
// Revised:      07.01.23
////////////////////////////////////////////////////////////////////
#include "SoundClass.h"
#include "../Common/MemHelpers.h"
#include "../Common/Assert.h"
#include "../Common/log.h"

const std::string g_DirPathToAudio = "../data/audio/";


SoundClass::SoundClass() 
{
}

SoundClass::~SoundClass() 
{ 
	Log::Debug();
	Shutdown();
};




////////////////////////////////////////////////////////////////////
//
//                     PUBLIC FUNCTIONS
//
////////////////////////////////////////////////////////////////////


// Firstly we initialize the DirectSound API as well as the primary buffer.
// Once that is initialized then the LoadWaveFile() function can be called 
// which will load in the .wav audio file and initialize the secondary 
// buffer with the audio information from the .wav file.
// After loading is completed then PlayWaveFile() is called which then plays
// the .wav file once
bool SoundClass::Initialize(HWND hwnd)
{
	try
	{
		Log::Debug();

		bool result = false;

		// initialize DirectSound and the primary sound buffer
		result = InitializeDirectSound(hwnd);
		Assert::True(result, "can't initialize DirectSound");

		std::string filePath = g_DirPathToAudio + "bateman.wav";

		// load a wave audio file onto a secondary buffer
		result = LoadWaveFile(filePath.c_str(), &pSecondaryBuffer1_);
		Assert::True(result, "can't load in a wave audio file");

		// play the wave file now that it has been loaded
		result = PlayWaveFile();
		Assert::True(result, "can't play the wave audio file");

	}
	catch (EngineException& e)
	{
		Log::Error(e);
		Shutdown();
		return false;
	}

	return true;
}


// The Shutdown() function first releases the secondary buffer which held the .wav file
// audio data. Once that completes this function then  releases
// the primary buffer and the DirectSound interface.
void SoundClass::Shutdown()
{
	SafeRelease(&pSecondaryBuffer1_);         // release the secondary buffer

	// shutdown the Direct Sound API
	SafeRelease(&pPrimaryBuffer_);            // release the primary sound buffer pointer
	SafeRelease(&pDirectSound_);              // release the direct sound interface pointer

	Log::Debug();
}

/*
	The PlayWaveFile() function will play the audio file stored in the secondary buffer. 
	The moment you use the Play function it will automatically mix the audio onto the primary
	buffer and start it playing if it wasn't already. Also note that we set the position 
	to start playing at the beginning of the secondary sound buffer otherwise it will continue
	from where it last stopped playing. And since we set the capabilities of the buffer 
	to allow us to control the sound we set the volume to maximum here.
*/
bool SoundClass::PlayWaveFile()
{
	HRESULT hr = S_OK;

	// set the position at the beginning of the sound buffer
	hr = pSecondaryBuffer1_->SetCurrentPosition(0);
	Assert::NotFailed(hr, "can't set the position in the sound buffer");

	// set volume of the buffer to 100%
	hr = pSecondaryBuffer1_->SetVolume(DSBVOLUME_MAX);
	Assert::NotFailed(hr, "can't set volume of the secondary buffer");

	// play the contents of the secondary sound buffer
	hr = pSecondaryBuffer1_->Play(0, 0, 0);
	Assert::NotFailed(hr, "can't play the contents of the secondary buffer");

	return true;
}


////////////////////////////////////////////////////////////////////
//
//                      PRIVATE FUNCTIONS
//
////////////////////////////////////////////////////////////////////

// InitializeDirectSound() handles getting an interface pointer to DirectSound and 
// the default primary sound buffer. Note that you can query the syster for all the sound
// devices and then grab the pointer to the primary sound buffer for a specific device.
bool SoundClass::InitializeDirectSound(HWND hwnd)
{
	HRESULT hr = S_OK;
	DSBUFFERDESC bd;
	WAVEFORMATEX waveFormat;

	// initialize the DirectSound interface pointer for the default sound device
	hr = DirectSoundCreate8(NULL, &pDirectSound_, NULL);
	Assert::NotFailed(hr, "can't initialize the DirectSound interface");

	// set the cooperative level to priority so the format of the primary sound buffer
	// can be modified
	hr = pDirectSound_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	Assert::NotFailed(hr, "can't set the cooperation level");

	// we have to setup the description of how we want to access the primary buffer. 
	// The dwFlags are the important part of this structure. In this case we just want 
	// to setup a primary buffer description with the capability of adjesting its volume.
	// There are other capabilities you can grab but we are keeping it simple for now.

	// setup the primary buffer description
	bd.dwSize = sizeof(DSBUFFERDESC);
	bd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bd.dwBufferBytes = 0;
	bd.dwReserved = 0;
	bd.lpwfxFormat = nullptr;
	bd.guid3DAlgorithm = GUID_NULL;

	// get control of the primary sound buffer on the default sound device
	hr = pDirectSound_->CreateSoundBuffer(&bd, &pPrimaryBuffer_, NULL);
	Assert::NotFailed(hr, "can't create a primary sound buffer");


	// now that we have control of the primary buffer on the default sound device we want 
	// to change its format to our desired audio file format. Now we have high quality
	// sound so we will set it to uncompressed CD audio quality.

	// setup the format of the primary sound buffer;
	// in this case it is a .WAV file recorded at 44,100 samples per second in 
	// 16-bit stereo (cd audio format)
	SetDefaultWaveFormat(waveFormat);

	// set the primery buffer to be the wave format specified
	hr = pPrimaryBuffer_->SetFormat(&waveFormat);
	Assert::NotFailed(hr, "can't set a format for the primary buffer");

	return true;
}




// the LoadWaveFile() function is what handles loading in a .wav audio file and then 
// copies the data onto a new secondary buffer. If you are looking to do different 
// formats your would replace this function or write a similar one
bool SoundClass::LoadWaveFile(const char* filename, IDirectSoundBuffer8** secondaryBuffer)
{
	bool result = false;
	errno_t error = 0;
	HRESULT hr = S_OK;
	FILE* filePtr = nullptr;

	size_t count = 0;
	WaveHeaderType waveFileHeader;
	

	/*
		to start first open the .wav file and read in the header of the file. The header
		will contain all the information about the audio file so we can use that to
		create a secondary buffer to accommodate the audio data. The audio file header
		also tells us where the data begins and how big it is. You will notice I check for
		all the needed tags to ensure the audio file is not corrupt and is the proper
		wave file format containing RIFF, WAVE, fmt, data, and WAVE_FORMAT_PCM tags.
		I also do a couple other checks to ensure it is a 44.1KHz stereo 16bit audio file.
		If it is mono, 22.1KHz, 8bit, or anything else then it will fail ensuring we are
		only loading the exact format we want.	
	*/
	
	// open the wave file in binary
	error = fopen_s(&filePtr, filename, "rb");
	Assert::True(error == 0, "can't open the wave file in binary");

	// read in the wave file header
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
	Assert::True(count == 1, "can't read in the wave file header");

	// verify the wave header file so we ensure everything is correct
	this->VerifyWaveHeaderFile(waveFileHeader);

	// setup and create a secondary sound buffer
	this->CreateSecondaryBuffer(waveFileHeader, secondaryBuffer);

	// read in the wave file data and write it into the secondary buffer
	this->ReadWaveData(waveFileHeader, secondaryBuffer, filePtr);

	return true;
}


// verify the wave header file so we ensure everything is correct
bool SoundClass::VerifyWaveHeaderFile(const WaveHeaderType& waveFileHeader)
{
	int isEqual = 0;

	// check that the chunk ID is the RIFF format
	isEqual = strncmp(waveFileHeader.chunkId, "RIFF", 4);
	Assert::True(isEqual == 0, "chunk ID isn't the RIFF format");

	// check that the format is the WAVE format
	isEqual = strncmp(waveFileHeader.format, "WAVE", 4);
	Assert::True(isEqual == 0, "the file format is not the WAVE format");

	// check that the sub chunk ID is the fmt format
	isEqual = strncmp(waveFileHeader.subChunkId, "fmt ", 4);
	Assert::True(isEqual == 0, "the sub chunk Id is not the fmt format");

	// check that the audio format is WAVE_FORMAT_PCM
	Assert::True(waveFileHeader.audioFormat == WAVE_FORMAT_PCM, "the audio format is not WAVE_FORMAT_PCM");

	// check that the wave file was recorded in stereo format
	Assert::True(static_cast<bool>(waveFileHeader.numChannels == 2), "the wave file wasn't recorded in stereo format");

	// check that the wave file was recorded at a sample rate of 44.1KHz
	Assert::True(static_cast<bool>(waveFileHeader.sampleRate == 44100), "the wave file wasn't recorded at a sample rate of 44.1KHz");

	// ensure that the wave file was recorded in 16 bit format
	Assert::True(static_cast<bool>(waveFileHeader.bitsPerSample == 16), "the wave file wasn't recorded in 16 bit format");

	// check for the data chunk header
	isEqual = strncmp(waveFileHeader.dataChunkId, "data", 4);
	Assert::True(isEqual == 0, "wrong data chunk header");

	return true;
}


// set up the wave format with default parameters for the sound buffer
void SoundClass::SetDefaultWaveFormat(WAVEFORMATEX& waveFormat)
{
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
}


// setup and create a secondary sound buffer
bool SoundClass::CreateSecondaryBuffer(const WaveHeaderType& waveFileHeader, IDirectSoundBuffer8** secondaryBuffer)
{
	HRESULT hr = S_OK;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	IDirectSoundBuffer* tempBuffer = nullptr;

	/*
	now that the wave header file has been verified we can setup the secondary buffer
	we will load the audio data onto. We have to first set wave format and buffer
	description of the secondary buffer similar to how we did for the primary buffer.
	There are some changes though since this is the secondary and not primary in
	terms of the dwFlags and dwBufferBytes.
	*/

	// set the wave format of secondary buffer that this wave file will be loaded onto
	SetDefaultWaveFormat(waveFormat);

	// set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;


	/*
	Now the way to create a secondary buffer is fairly strange:

	1. First step is that you create a temporary IDirectSoundBuffer with the
	sound buffer description you setup for the secondary buffer.
	2. If this succeeds then you can use that temporary	buffer to create
	a IDirectSoundBuffer8 secondary buffer by calling QueryInterface()
	with the IID_DirectSoundBuffer8 parameter.
	3. If this succeeds then you can release
	the temporary buffer and the secondary buffer is ready for use.
	*/


	// create a temporary sound buffer with the specific buffer settings
	hr = pDirectSound_->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	Assert::NotFailed(hr, "can't create a temporary sound buffer");

	// test the buffer format against the direct sound 8 interface and create the secondary buffer
	hr = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*secondaryBuffer);
	Assert::NotFailed(hr, "can't create the secondary sound buffer");

	SafeRelease(&tempBuffer);   // release the temporary buffer

	return true;
}


/*
	Now that the secondary buffer is ready we can load in the wave data from
	the audio file. First I load it into a memory buffer so I can check and modify
	the data if I need to. Once the data is in memory you then lock the secondary
	buffer, copy the data to it using a memcpy(), and the unlock it.
	The secondary buffer is now ready for use.
	Note that locking the secondary buffer can actually take in two pointers
	and two positions to write to. This is because it is a circular buffer and if
	you start by writing to the middle of it you will need the size of the buffer
	from that point so that you don't write outside the bounds of it.
	This is useful for streaming audio and such. Here we create a buffer that
	is the same size as the audio file and write from the beginning
	to make things  simple.
*/
bool SoundClass::ReadWaveData(const WaveHeaderType& waveFileHeader, 
							  IDirectSoundBuffer8** secondaryBuffer,
							  FILE* filePtr)
{
	//UCHAR* waveData = nullptr;
	std::unique_ptr<UCHAR[]> waveData = std::make_unique<UCHAR[]>(waveFileHeader.dataSize);
	size_t count = 0;
	errno_t error = 0;
	HRESULT hr = S_OK;
	UCHAR* bufferPtr = nullptr;
	ULONG bufferSize = 0;


	// move to the beginning of the wave data which starts at the end of 
	// the data chunk header
	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	// create a temporary buffer to hold the wave file data
	//waveData = new UCHAR[waveFileHeader.dataSize];
	//Assert::True(waveData, "can't allocate memory for the wave file data");

	// read in the wave file data into the newly created buffer
	count = fread(waveData.get(), 1, waveFileHeader.dataSize, filePtr);
	Assert::True(count == waveFileHeader.dataSize, "can't read in the wave file data");

	// close the file once done reading 
	error = fclose(filePtr);
	Assert::True(error == 0, "something went wrong during closing of the file");

	// lock the secondary buffer to write wave data into it
	hr = (*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, nullptr, 0, 0);
	Assert::NotFailed(hr, "can't lock the secondary buffer");

	// copy the wave data into the buffer
	memcpy(bufferPtr, waveData.get(), waveFileHeader.dataSize);

	// unlock the secondary buffer ater the data had been written to it
	hr = (*secondaryBuffer)->Unlock((void*)bufferPtr, bufferSize, nullptr, 0);
	Assert::NotFailed(hr, "can't unlock the secondary buffer after writing to it");

	// release the wave data since it was copied into the secondary buffer
	//_DELETE(waveData);


	return true;
} // ReadWaveData()


