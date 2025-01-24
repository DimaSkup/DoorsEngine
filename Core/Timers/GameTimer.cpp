////////////////////////////////////////////////////////////////////////////////////////////
// Filemane: GameTimer.cpp
// Revising: 01.08.22
////////////////////////////////////////////////////////////////////////////////////////////
#include "GameTimer.h"

GameTimer::GameTimer()
{
	Log::Debug();

	// get the frequence (counts per second) of the performance timer
	__int64 countsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)(&countsPerSec));

	// then the number of seconds (or fractions of a second) per count is just 
	// the reciprocal of the counts per second:
	secondsPerCount_ = 1.0 / (double)countsPerSec;
}



// --------------------------------------------------------------------------------- //
//                                                                                   //
//                              PUBLIC FUNCTIONS                                     // 
//                                                                                   //
// --------------------------------------------------------------------------------- //

void GameTimer::Tick()
{
	// THIS FUNCTION computes the time elapsed between the frames

	if (isStopped_)
	{
		deltaTime_ = 0.0f;
		return;
	}

	// get the time this frame
	__int64 currTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	currTime_ = currTime;

	// time difference between this frame and the previous
	deltaTime_ = (currTime - prevTime_) * secondsPerCount_;

	// prepare for the next frame
	prevTime_ = currTime;

	// force nonnegative. The DXSDK's CDXTUTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another processor, 
	// then deltaTime_ can be negative
	if (deltaTime_ < 0.0f)
	{
		deltaTime_ = 0.0f;
	}
}

///////////////////////////////////////////////////////////

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	isStopped_ = false;
	baseTime_ = currTime;
	prevTime_ = currTime;
	stopTime_ = 0;
}

///////////////////////////////////////////////////////////

void GameTimer::Stop()
{
	// If we are already stopped, then don't do anything
	if (isStopped_)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// otherwise, save the time we stoop at, and set 
		// the boolean flag indicating the timer is stopped
		stopTime_ = currTime;
		isStopped_ = true;
	}
}

///////////////////////////////////////////////////////////

void GameTimer::Start()
{
	// accumulate the time elapsed between stop and start pairs
	//
	//               |<--------d-------->|
	// ---------------1-----------------2----------> time
	// (1) stopTime_; (2) startTime

	// if we are resuming the timer from a stopped state...
	if (isStopped_)
	{
		__int64 startTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		// no longer stopped...
		isStopped_ = false;

		// since we are starting the timer back up, the current 
		// previous time is not valid, as it occured while paused.
		// So reset it to the current time
		prevTime_ = startTime;

		// reset the stop time
		stopTime_ = 0;

		// then accumulate the paused time
		pausedTime_ += (startTime - stopTime_);
	}
}

///////////////////////////////////////////////////////////

float GameTimer::GetGameTime() const
{
	// this function returns the time elapsed since Reset 
	// was called not counting paused time;

	// 1:
	// 
	// if we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance stopTime_ - baseTime_
	// includes paused time, which we do not want to count. To correct this we can subtract
	// the paused time from stopTime_:
	//
	// previous paused time
	// |<--------->|
	// ---*-----------*---------*-----> time
	// baseTime_   stopTime_  currTime_

	// 2:
	// 
	// the distance currTime_ - baseTime_ includes paused time, which we don't want to 
	// count. To correct this, we can subtract the paused time from currTime_:
	//
	// (currTime_ - pausedTime_) - baseTime_
	//
	// |<--paused time-->|
	// ---1-----------2-------------3------------4---> time
	// (1)baseTime_; (2)stopTime_; (3)startTime_; (4)currTime_

	const float subFrom = (float)((stopTime_ * (int)isStopped_) + (currTime_ * (int)(!isStopped_)));
	return (float)(subFrom - pausedTime_ - baseTime_) * (float)secondsPerCount_;

	//return (isStopped_) ?
		//static_cast<float>((stopTime_ - pausedTime_) - baseTime_) * (float)secondsPerCount_ :
		//static_cast<float>((currTime_ - pausedTime_) - baseTime_) * (float)secondsPerCount_;
}