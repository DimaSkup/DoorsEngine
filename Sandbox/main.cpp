///////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "Application.h"

int main()
{
    //setlocale(LC_ALL, "");

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Game::App app;

    // ATTENTION: put the declation of logger before all the others; it is necessary to create a logger text file
    InitLogger("log.txt");

	app.Init();
	app.Run();
	app.Close();

    CloseLogger();

	return 0;
}
