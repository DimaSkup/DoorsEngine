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
    LogMsgf(" ");
    LogMsgf("%s----------------  TESTS: ECS  ------------------", YELLOW);
}

ECS_Tester::~ECS_Tester()
{
    LogMsgf("%s--------------------------------------------------", YELLOW);
    LogMsgf(" ");
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
        LogMsgf("%s----------------  TESTS: EntityMgr  -----------------\n", YELLOW);

        //testEntityMgr.TestEntitiesCreation();
        //testEntityMgr.TestSerialDeserial();

        LogMsg("All the tests for ECS are passed!\n\n\n");
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        exit(-1);
    }
}

} // namespace Core
