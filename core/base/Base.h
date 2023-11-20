#ifndef BASE_H_
#define BASE_H_

// C/C++
#include <new>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <type_traits>

#include "Logger.h"
#include "System.h"

// Bring common functions from C into global namespace
using std::memcpy;
using std::fabs;
using std::sqrt;
using std::cos;
using std::sin;
using std::tan;
using std::isspace;
using std::isdigit;
using std::toupper;
using std::tolower;
using std::size_t;
using std::min;
using std::max;
using std::modf;
using std::atoi;

// Common
#ifndef NULL
#define NULL     0
#endif

namespace mgp
{
/**
 * Print logging (implemented per platform).
 * @script{ignore}
 */
extern void print(const char* format, ...);

// Define a platform-independent case-insensitive ASCII string comparison function.
extern int strcmpnocase(const char* s1, const char* s2);
}

// Current function macro.
#ifdef WIN32
#define __current__func__ __FUNCTION__
#else
#define __current__func__ __func__
#endif

// Assert macros.
#ifdef _DEBUG
#define GP_ASSERT(expression) assert(expression)
#else
#define GP_ASSERT(expression)
#endif

#if defined(WIN32) && defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK()
#endif

// Error macro.
#ifdef GP_ERRORS_AS_WARNINGS
#define GP_ERROR GP_WARN
#else
#define GP_ERROR(...) do \
    { \
        mgp::Logger::log(mgp::Logger::LEVEL_ERROR, "%s -- ", __current__func__); \
        mgp::Logger::log(mgp::Logger::LEVEL_ERROR, __VA_ARGS__); \
        mgp::Logger::log(mgp::Logger::LEVEL_ERROR, "\n"); \
        DEBUG_BREAK(); \
        assert(0); \
        std::exit(-1); \
    } while (0)
#endif

// Warning macro.
#define GP_WARN(...) do \
    { \
        mgp::Logger::log(mgp::Logger::LEVEL_WARN, "%s -- ", __current__func__); \
        mgp::Logger::log(mgp::Logger::LEVEL_WARN, __VA_ARGS__); \
        mgp::Logger::log(mgp::Logger::LEVEL_WARN, "\n"); \
    } while (0)


#if _DEBUG
// Warning macro.
#define GP_DEBUG(...) do \
    { \
        mgp::Logger::log(mgp::Logger::LEVEL_DEBUG, "%s -- ", __current__func__); \
        mgp::Logger::log(mgp::Logger::LEVEL_DEBUG, __VA_ARGS__); \
        mgp::Logger::log(mgp::Logger::LEVEL_DEBUG, "\n"); \
    } while (0)
#else
#define GP_DEBUG(...) do{}while(0)
#endif

#if defined(WIN32)
    #pragma warning( disable : 4005 )
    #pragma warning( disable : 4172 )
    #pragma warning( disable : 4244 )
    #pragma warning( disable : 4267 )
    #pragma warning( disable : 4311 )
    #pragma warning( disable : 4316 )
    #pragma warning( disable : 4390 )
    #pragma warning( disable : 4800 )
    #pragma warning( disable : 4996 )
#endif



// Debug new for memory leak detection
#include "DebugNew.h"

// Object deletion macro
#define SAFE_DELETE(x) \
    { \
        delete x; \
        x = NULL; \
    }

// Array deletion macro
#define SAFE_DELETE_ARRAY(x) \
    { \
        delete[] x; \
        x = NULL; \
    }

// mem free macro
#define SAFE_FREE(x) \
    { \
        free(x); \
        x = NULL; \
    }

// Ref cleanup macro
#define SAFE_RELEASE(x) \
    if (x) \
    { \
        (x)->release(); \
        x = NULL; \
    }

// Vector deletion macro
#define SAFE_RELEASE_VECTOR(v) \
    { \
        for (auto it=v.begin(); it!=v.end(); ++it) {\
            it->release(); \
        }\
        v.clear();\
    }


// NOMINMAX makes sure that windef.h doesn't add macros min and max
#ifdef WIN32
    #define NOMINMAX
#endif


#define WINDOW_VSYNC        1



#endif
