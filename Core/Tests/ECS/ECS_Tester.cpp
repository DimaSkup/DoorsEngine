#include "ECS_Tester.h"

#include "Entity/EntityMgr.h"
#include <CoreCommon/log.h>

// tests related stuff
//#include "TestComponents.h"
//#include "TestSystems.h"
//#include "TestEntityMgr.h"

namespace Core
{

ECS_Tester::ECS_Tester()
{
    Log::Print();
    Log::Print("----------------  TESTS: ECS  ------------------", ConsoleColor::YELLOW);
}

ECS_Tester::~ECS_Tester()
{
    Log::Print("--------------------------------------------------", ConsoleColor::YELLOW);
    Log::Print();
}

// *********************************************************************************

void ECS_Tester::Run()
{
    //TestEntityMgr testEntityMgr;
    //TestComponents testComponents;  // unit tests for the ECS components
    //TestSystems testSystems;

    try
    {
        // test each ECS component
        //testComponents.Run();

        // test each ECS system
       // testSystems.Run();

        // test the EntityManager
        Log::Print("----------------  TESTS: EntityMgr  -----------------\n", ConsoleColor::YELLOW);

        //testEntityMgr.TestEntitiesCreation();
        //testEntityMgr.TestSerialDeserial();

        Log::Print("All the tests for ECS are passed!\n\n\n");
    }
    catch (EngineException& e)
    {
        Log::Error(e, true);
        exit(-1);
    }
}

} // namespace Core
