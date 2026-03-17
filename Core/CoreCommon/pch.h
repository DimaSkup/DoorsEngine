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

#include "raw_file.h"
#include "log.h"
#include "CAssert.h"
#include "engine_exception.h"
#include "mem_helpers.h"
#include "StrHelper.h"
#include "cvector.h"
#include "Types.h"
#include "file_system.h"
#include "FileSystemPaths.h"
#include "math/math_helpers.h"
#include "math/dx_math_helpers.h"

#include <math/vec_functions.h>
#include <geometry/plane3d_functions.h>
#include <geometry/rect3d_functions.h>



#pragma warning (disable : 4996)
