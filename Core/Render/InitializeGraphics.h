////////////////////////////////////////////////////////////////////
// Filename:     InitializeGraphics.h
// Description:  this class is responsible for initialization all the 
//               graphics in the engine;
//
// Created:      02.12.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "../Engine/Settings.h"
#include "../Render/d3dclass.h"


namespace Core
{

class InitializeGraphics
{
public:
	InitializeGraphics();

	// initialized all the DirectX stuff
	bool InitializeDirectX(D3DClass& d3d, HWND hwnd, const Settings& settings);

private:  // restrict a copying of this class instance
	InitializeGraphics(const InitializeGraphics & obj);
	InitializeGraphics & operator=(const InitializeGraphics & obj);

};

}
