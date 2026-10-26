#ifndef AUDIO_BUFFER_SIZES_H
#define AUDIO_BUFFER_SIZES_H

#ifndef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
	#define AUDIO_ADPCM_BUFFER_SIZE_K 64
#elif PLATFORM_PC
	#define AUDIO_ADPCM_BUFFER_SIZE_K 64
#else
	#define AUDIO_ADPCM_BUFFER_SIZE_K 64
#endif
	
#define AUDIO_XMA_BUFFER_SIZE_K 128

#endif