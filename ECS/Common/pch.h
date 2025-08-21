#pragma once

#include <fstream>
#include <string>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <map>
#include <new>
#include <inttypes.h> // For PRIu32

#include "log.h"
#include "CAssert.h"
#include "EngineException.h"
#include "MathHelper.h"
#include "MemHelpers.h"
#include "StrHelper.h"
#include "cvector.h"
#include "Types.h"
#include "UtilsFilesystem.h"
#include "ECSTypes.h"
#include "DMath.h"

#pragma warning (disable : 4996)

#define NEW new(std::nothrow)
