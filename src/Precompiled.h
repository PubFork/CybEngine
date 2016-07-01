// Precompiled.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

//#define PRECOMPILED_BASE 
//#define PRECOMPILED_RENDERER

#include <glm/mat4x4.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef NDEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <string>
#include <functional>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <strstream>

#ifdef PRECOMPILED_BASE
#include "Base/Algorithm.h"
#include "Base/Debug.h"
#include "Base/File.h"
#include "Base/Memory.h"
#include "Base/MurmurHash.h"
#include "Base/ParallelJobQueue.h"
#include "Base/Sys.h"
#include "Base/Timer.h"
#endif

#ifdef PRECOMPILED_RENDERER
#include "Renderer/Definitions.h"
#include "Renderer/Model.h"
#include "Renderer/RenderDevice.h"
#include "Renderer/Texture.h"
#endif