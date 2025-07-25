///////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "Application.h"

int main()
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Game::App app;


    // ATTENTION: put the declation of logger before all the others; this instance is necessary to create a logger text file
    InitLogger("DoorsEngineLog.txt");

	app.Initialize();
	app.Run();
	app.Close();

	return 0;
}
