#ifndef _CORE_HEADERS_H_
#define _CORE_HEADERS_H_

#ifdef PLATFORM_SPU
void* operator new ( size_t size ); 
void operator delete ( void* ptr ) ;
void* operator new[]( size_t size );
void operator delete[] ( void* ptr );

void ResetHeapToStoredPosition();
void StoreHeapPosition();
#endif


#if ! defined (PLATFORM_PS3) && ! defined (PLATFORM_SPU)
#define NEW_ADPCM_MEMBER_ARRAYS
#endif


#ifdef _PC
#include "float.h"
#ifdef PLATFORM_PS3 // JASOND: #ifdef PLATFORM_PS3 inside #ifdef _PC !?!?
#error "ifdef PLATFORM_PS3 inside ifdef _PC!"
#include <fastmath.h>
#else
#include <math.h>
#endif
#include "malloc.h"
#include "memory.h"
#include "xmmintrin.h"
#endif

//#if defined (USER_JONC) && defined (CFG_DEBUG) && defined (PLATFORM_SPU)
#if defined (CFG_DEBUG) && defined (PLATFORM_SPU)
//#define AUDIO_SPU_MINIMAL
#endif

//#include "../common/types.h"
#ifdef PLATFORM_SPU
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
//#include "assert.h"

//#define AUDIO_DISABLE_DSP
/* spurs */
//#include <cell/spurs.h>
//jkc	#include <cell/spurs/types.h>
//jkc	#include <cell/spurs/task.h>

//jkc	#include <System/Spu/Scheduler.h>
#endif

#define AUDIO_COMPONENT_OVERLAY_ID 0
#define AUDIO_GENERATOR_OVERLAY_ID 1

#ifdef PLATFORM_PS3
#define NO_STRING_TABLE_ON_SPU
#endif


#define STREAM_ACTIVE					0x0001
#define STREAM_PREPARING				0x0002
#define STREAM_EOF_READ					0x0004
#define STREAM_FINISHED_READING			0x0008
#define STREAM_UNDERRUN					0x0010
#define STREAM_RESET_REQUIRED			0x0020
#define STREAM_RELEASING				0x0040
#define STREAM_RELEASED					0x0080
#define STREAM_DESTROYING				0x0100
#define STREAM_BUFFERS_ALLOCATED		0x0200
#define STREAM_DATA_NOT_READY			0x0400
#define STREAM_YET_TO_UPDATE			0x0800
#define STREAM_FINISHED_WRITING			0x1000


#define ADPCM_BUFFER_ARRAY_SIZE			12


#include "../../include/Audio.h"



// needed to fit code on ps3 spu- may cause some performance hit on 360.
// could just be defined on ps3 if it's an issue
//#ifdef PLATFORM_SPU
//#define AUDIO_MIXER_PROCESS_UNTEMPLATED
//#endif

#ifdef PLATFORM_PS3
//#define PS3_OUTPUT_TO_MULTISTREAM
#ifdef PLATFORM_PS3
#include <fastmath.h>
#else
#include <math.h>
#endif
//#include <System/Spu/Scheduler.h>

#ifdef PS3_OUTPUT_TO_MULTISTREAM
#include <cell/mstream.h>
#if CELL_SDK_VERSION < 0x210001
//#include <cell/i3dl2.h>
#endif
#endif
#endif

#include "../common/Array.h"
#ifndef PLATFORM_SPU
#include "../common/Queue.h"
#endif	//	#ifndef PLATFORM_SPU

#include "..\common\types.h"
#include "../common/3dmaths.h"
#include "../common/Util.h"
#include "../common/Lock.h"

#include "audio_core.h"
#include "frame.h"

#if defined (AUDIO_ENABLE_PROFILING)
#include "System/Artery/Artery.h"
#include "profilertool.h"
#endif

#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
#include "CorePS3shared.h"
#endif

#include "dspeffect.h"
#include "dspsend.h"
#include "dspsaturator.h"
#include "dspbitcrusher.h"
#include "dspringmodulator.h"
#include "dspcompressor.h"
#include "dspclipper.h"
#include "dspdelay.h"
#include "dspconvolution.h"
#include "dspspectralreverb.h"

#include "dsppitchshift.h"

#include "dspflanger.h"
#include "dsprmslevel.h"
#include "dspfft.h"
#include "dspfirequaliser.h"
#include "dspiirlowpass1storder.h"
#include "dspiirlowpass2ndorder.h"
#include "dspiirhighpass1storder.h"
#include "dspiirpeakeq.h"
#include "generator.h"
#include "delayline.h"
#include "return.h"
#ifndef PLATFORM_SPU
#include "dspiirlowpass4thorder.h"
#include "oscillator.h"
#else
// TODO: SPU_TODO: something with these
//#define FS_STREAM_HANDLE				S32
#define AudioStreamFileHandle			S32
class FileStreamDef;
#endif	//	#ifndef PLATFORM_SPU
#include "decoder.h"
#include "decoderpcms16.h"
#include "decoderadpcm4bit.h"
#ifdef PLATFORM_PS3
//#include "MP3DECODE.H"
#endif
#ifndef PLATFORM_SPU
#include "filestream.h"
#include "filestreammanager.h"
#include "WaveServer.h"
#endif
#include "decoderxma.h"
#include "input.h"
#include "output.h"
#include "wave.h"
#include "mixer.h"
#include "overlapmixer.h"
#ifndef PLATFORM_SPU
#include "renderer.h"
#include "wavrenderer.h"
#endif	//	#ifndef PLATFORM_SPU

#ifdef PLATFORM_SPU

extern U32 g_u32CurrentCodeOverlayId;
//extern void* g_pOverlayTable;
bool CoreLoadCodeOverlay(U32 u32OverlayId);
//Spu::MemSyncDMA* GetWaveDataDmaObject();
//Spu::MemSyncDMA* GetAdpcmPrevFrameDmaObject();

#endif	//	PLATFORM_SPU

#endif	//	_CORE_HEADERS_H_
