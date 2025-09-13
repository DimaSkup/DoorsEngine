#pragma once

#include <cassert>
#include <memory>   // for using std::construct_at
#include <utility>  // for using std::exchange
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <map>
#include <string>
#include <new>          // std::nothrow
#include <stdlib.h>
#include <inttypes.h>

#include "RawFile.h"
#include "log.h"
#include "CAssert.h"
#include "EngineException.h"
#include "MemHelpers.h"
#include "StrHelper.h"
#include "cvector.h"
#include "Types.h"
#include "FileSystem.h"
#include "FileSystemPaths.h"
#include "math/math_helpers.h"
#include "math/dx_math_helpers.h"


#pragma warning (disable : 4996)
