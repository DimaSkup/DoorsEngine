////////////////////////////////////////////////////////////////////
// Filename:     timer.cpp
// Description:  the implementation of the engine timer class
// Created:      23.12.22
// Revision:     23.12.22
////////////////////////////////////////////////////////////////////
#include "timer.h"

Timer::Timer()
{
	start_ = std::chrono::high_resolution_clock::now();
	stop_  = std::chrono::high_resolution_clock::now();
}

Timer::~Timer()
{
	Log::Debug();
}


float Timer::GetMilisecondsElapsed()
{
	if (isRunning_)
	{
		auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start_);
		return static_cast<float>(elapsed.count());
	}
	else
	{
		auto elapsed = std::chrono::duration<double, std::milli>(stop_ - start_);
		return static_cast<float>(elapsed.count());
	}
}

void Timer::Restart()
{
	isRunning_ = true;
	start_ = std::chrono::high_resolution_clock::now();
}

bool Timer::Stop()
{
	if (!isRunning_)
		return false;
	else
	{
		stop_ = std::chrono::high_resolution_clock::now();
		isRunning_ = false;
		return true;
	}
}


bool Timer::Start()
{
	if (isRunning_)
	{
		return false;
	}
	else
	{
		start_ = std::chrono::high_resolution_clock::now();
		isRunning_ = true;
		return true;
	}
}