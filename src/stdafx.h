// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <GL/glew.h>
#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef NDEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <cinttypes>        // PRIu64 etc...

#include <atomic>
#include <chrono>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <strstream>