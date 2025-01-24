#pragma once

#include "../../UICommon/ICommand.h"

class ViewListener
{
public:
	virtual void Execute(ICommand* pCommand) = 0;
};