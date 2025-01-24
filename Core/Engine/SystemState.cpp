///////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     SystemState.cpp
// Description:  implementation of the SystemState class
// Revising:     25.11.22
///////////////////////////////////////////////////////////////////////////////////////////////
#include "SystemState.h"
#include "EngineException.h"

SystemState* SystemState::pInstance_ = nullptr;


SystemState::SystemState()
{
	if (pInstance_ == nullptr)
	{
		pInstance_ = this;
	}
	else
	{
		throw EngineException("you can't create more than only one instance of this class");
	}
}

SystemState::~SystemState()
{
}


SystemState* SystemState::Get()
{
	// a static function for receiving a ptr to the instance of this class
	return pInstance_;
}


