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
#include "engine_exception.h"
#include "mem_helpers.h"
#include "StrHelper.h"
#include "cvector.h"
#include "types.h"
#include "UtilsFilesystem.h"
#include "ECSTypes.h"
#include "math/math_helpers.h"

#include <geometry/rect3d_functions.h>

#pragma warning (disable : 4996)

#define NEW new(std::nothrow)
