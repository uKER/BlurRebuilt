#ifndef AUDIO_AUDIOSPUPCH_H
#define AUDIO_AUDIOSPUPCH_H

//#include "../stdafx.h"

#define AUDIO_SPU_USE_MERCURY_DEBUG

#if defined( PLATFORM_SPU ) && !defined( BUILD_CONFIG_DEBUG )
#define SPU_MEMORYSAVER
#endif

// mercury stuff
#ifdef AUDIO_SPU_USE_MERCURY_DEBUG
//#define USE_MERCURY_DEBUG
#include <core/core.h>
#include <system\debuglog.h>
#endif

#define AUDIO_RESTRICT
#define AUDIO_INLINE inline

#endif // AUDIO_AUDIOSPUPCH_H
