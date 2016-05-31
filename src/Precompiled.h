// Precompiled.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef NDEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstdarg>
#include <cstdint>
#include <cinttypes>        // PRIu64 etc...

#include <string>
#include <algorithm>
#include <functional>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <atomic>
#include <fstream>
#include <strstream>