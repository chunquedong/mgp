#pragma once

// Audio (OpenAL)
#ifdef __ANDROID__
#include <AL/al.h>
#include <AL/alc.h>
#define AL_ALEXT_PROTOTYPES
#include <AL/alext.h>
#elif WIN32
#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>
#elif __linux__
#include <AL/al.h>
#include <AL/alc.h>
#elif __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

// Compressed Media
//#include <vorbis/vorbisfile.h>

/**
 * Executes the specified AL code and checks the AL error afterwards
 * to ensure it succeeded.
 *
 * The AL_LAST_ERROR macro can be used afterwards to check whether a AL error was
 * encountered executing the specified code.
 */
#define AL_CHECK( al_code ) do \
    { \
        while (alGetError() != AL_NO_ERROR) ; \
        al_code; \
        __al_error_code = alGetError(); \
        if (__al_error_code != AL_NO_ERROR) \
        { \
            GP_ERROR(#al_code ": %d", (int)__al_error_code); \
        } \
    } while(0)

 /** Global variable to hold AL errors
  * @script{ignore} */
extern ALenum __al_error_code;

/**
 * Accesses the most recently set global AL error.
 */
#define AL_LAST_ERROR() __al_error_code