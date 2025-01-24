// *********************************************************************************
// Filename:       ECS_Test_Components.h
// Description:    unit-tests for each component of the ECS;
// *********************************************************************************
#pragma once

class TestComponents
{
public:
	TestComponents() {};

	void Run();

private:

	void TestTransformComponent();
	void TestNameComponent();
	void TestMoveComponent();
	void TestMeshComponent();

	void TestRenderComponent();
	void TestTexturedComponent();
	void TestTexTransformComponent();

	void TestLightComponent();

	// light component test helpers
	void TestDirLights();
	void TestPointLights();
	void TestSpotLights();


	// texture transform component test helpers
	void TestTexTransformStatic();
	void TestTexTransformAtlasAnimation();
	void TestTexTransformRotAroundCoords();
};