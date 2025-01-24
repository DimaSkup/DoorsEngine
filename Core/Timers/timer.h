////////////////////////////////////////////////////////////////////
// Filename:     timer.h
// Description:  the engine timer
// Created:      23.12.22
// Revision:     23.12.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "../Common/log.h"
#include <chrono>


class Timer
{
public:
	Timer();
	~Timer();

	float GetMilisecondsElapsed();
	void Restart();
	bool Stop();
	bool Start();

private:
	bool isRunning_ = false;

#ifdef _WIN32   // for Windows
	std::chrono::time_point<std::chrono::steady_clock> start_;
	std::chrono::time_point<std::chrono::steady_clock> stop_;
#else           // for Linux
	std::chrono::time_point<std::chrono::system_clock> start_;
	std::chrono::time_point<std::chrono::system_clock> stop_;
#endif
};