#include "UnitTestMain.h"
#include "Entity/EntityManager.h"
#include "../Engine/log.h"

// tests related stuff
#include "TestComponents.h"
#include "TestSystems.h"
#include "TestEntityMgr.h"


UnitTestMain::UnitTestMain()
{
	Log::Print();
	Log::Print("---------------  UNIT TESTS: ECS  ----------------", ConsoleColor::YELLOW);
}

UnitTestMain::~UnitTestMain()
{
	Log::Print("--------------------------------------------------", ConsoleColor::YELLOW);
	Log::Print();
}

// *********************************************************************************

void UnitTestMain::Run()
{
	TestEntityMgr testEntityMgr;
	TestComponents testComponents;  // unit tests for the ECS components
	TestSystems testSystems;

	try
	{
		// test each ECS component
		testComponents.Run();

		// test each ECS system
		testSystems.Run();

		// test the EntityManager
		Log::Print("----------------  TESTS: EntityManager -----------------", ConsoleColor::YELLOW);
		Log::Print();

		testEntityMgr.TestEntitiesCreation();
		testEntityMgr.TestSerialDeserial();

		Log::Print("");
	}
	catch (EngineException& e)
	{
		Log::Error(e, true);
		exit(-1);
	}
}

///////////////////////////////////////////////////////////
