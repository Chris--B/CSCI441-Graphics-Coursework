#pragma once

// All of the Utils headers.
#include "Utils/Color.hpp"
#include "Utils/DebugTools.hpp"
#include "Utils/GL_Defs.hpp"
#include "Utils/MathHelpers.hpp"
#include "Utils/PointVecBase.hpp"
#include "Utils/Logging.hpp"

// Common includes
#include <memory> // std::unique_ptr, std::make_unique

#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstring>

#ifndef M_PI
#pragma message ("warning: Your cmath is broken - no M_PI is defined.")
#define M_PI 3.141592653589793238462643383279
#endif
