
#include "base/Base.h"
//#include "platform/Toolkit.h"
//#include "script/ScriptController.h"
#include <stdarg.h>

namespace mgp
{

Logger::State Logger::_state[Logger::LEVEL_COUNT];

Logger::State::State() : logFunctionC(NULL), enabled(true)
{
}

Logger::Logger()
{
}

Logger::~Logger()
{
}

#ifdef __ANDROID__

#include <android/log.h>

extern void print(const char* format, ...)
{
    GP_ASSERT(format);
    va_list argptr;
    va_start(argptr, format);
    __android_log_vprint(ANDROID_LOG_INFO, "mgp-native-activity", format, argptr);
    va_end(argptr);
}

//#elif defined(_WIN32)
//#include <windowsx.h>
//extern void print(const char* format, ...)
//{
//    va_list argptr;
//    va_start(argptr, format);
//    int sz = vfprintf(stderr, format, argptr);
//    if (sz > 0)
//    {
//        char* buf = new char[sz + 1];
//        vsprintf(buf, format, argptr);
//        buf[sz] = 0;
//        OutputDebugStringA(buf);
//        SAFE_DELETE_ARRAY(buf);
//    }
//    va_end(argptr);
//}
#else
extern void print(const char* format, ...)
{
    GP_ASSERT(format);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}
#endif

void Logger::log(Level level, const char* message, ...)
{
    State& state = _state[level];
    if (!state.enabled)
        return;

    // Declare a moderately sized buffer on the stack that should be
    // large enough to accommodate most log requests.
    int size = 1024;
    char stackBuffer[1024];
    std::vector<char> dynamicBuffer;
    char* str = stackBuffer;
    for ( ; ; )
    {
        va_list args;
        va_start(args, message);

        // Pass one less than size to leave room for NULL terminator
        int needed = vsnprintf(str, size-1, message, args);

        // NOTE: Some platforms return -1 when vsnprintf runs out of room, while others return
        // the number of characters actually needed to fill the buffer.
        if (needed >= 0 && needed < size)
        {
            // Successfully wrote buffer. Added a NULL terminator in case it wasn't written.
            str[needed] = '\0';
            va_end(args);
            break;
        }

        size = needed > 0 ? (needed + 1) : (size * 2);
        dynamicBuffer.resize(size);
        str = &dynamicBuffer[0];

        va_end(args);
    }

    if (state.logFunctionC)
    {
        // Pass call to registered C log function
        (*state.logFunctionC)(level, str);
    }
    //else if (state.logFunctionLua)
    //{
        // Pass call to registered Lua log function
        //ScriptController::cur()->executeFunction<void>(state.logFunctionLua, "[Logger::Level]s", NULL, level, str);
    //}
    else
    {
        // Log to the default output
        mgp::print("%s", str);
    }
}

bool Logger::isEnabled(Level level)
{
    return _state[level].enabled;
}

void Logger::setEnabled(Level level, bool enabled)
{
    _state[level].enabled = enabled;
}

void Logger::set(Level level, void (*logFunction) (Level, const char*))
{
    State& state = _state[level];
    state.logFunctionC = logFunction;
    //state.logFunctionLua = NULL;
}

void Logger::set(Level level, const char* logFunction)
{
    State& state = _state[level];
    //state.logFunctionLua = logFunction;
    state.logFunctionC = NULL;
}

}
