// *********************************************************************************
// Filename:       TestSystems.h
// Description:    tests for each system of the ECS;
// 
// Created:        13.06.24
// *********************************************************************************
#pragma once

class TestSystems final
{
public:
	TestSystems() {}
	~TestSystems() {};

	void Run();

	void TestTexTransformSysUpdating();
	void TestMoveSysUpdating();
	void TestSerialDeserial();

private:
	// test serialization and deserialization functional of ECS systems
	void TestTransformSysSerialDeserial();   
	void TestNameSysSerialDeserial();
	void TestMoveSysSerialDeserial();
	void TestMeshSysSerialDeserial();
	void TestRenderedSysSerialDeserial();
};
