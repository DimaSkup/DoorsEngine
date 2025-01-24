// *********************************************************************************
// Filename:     MemHelpers.h
// Description:  operations with memory
// 
// Created:      13.08.24
// *********************************************************************************
#pragma once

namespace Render
{
	


template<typename T>
inline void SafeDelete(T*& p)
{
	if (p) { delete(p); p = nullptr; }
}

///////////////////////////////////////////////////////////

template<typename T>
inline void SafeDeleteArr(T*& p)
{
	if (p) { delete[](p); p = nullptr; }
}

///////////////////////////////////////////////////////////

template<class T>
inline void SafeShutdown(T** ppT)
{
	if (*ppT) { (*ppT)->Shutdown(); delete(ppT); *ppT = nullptr; }
}

///////////////////////////////////////////////////////////

template<class T>
inline void SafeRelease(T** ppT)
{
	if (*ppT) { (*ppT)->Release(); *ppT = nullptr; }
}


};  // namespace Render