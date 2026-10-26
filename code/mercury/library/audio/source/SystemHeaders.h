#ifndef AUDIO_SYSTEMHEADERS_H
#define AUDIO_SYSTEMHEADERS_H

#include "Core/CoreHeaders.h"

#include "Common/3dMaths.h"

#include "Common/Array.h"
#include "Common/BitLimitedPagedArray.h"
#include "Common/Queue.h"
#include "Common/Lock.h"
#include "Common\StopWatch.h"
#include "Common\WaveBank.h"
#include "Common\Allocator.h"
#include "Common/StaticBufferAllocator.h"
#include "Common/Handle.h"
#include "Common/Collection.h"
#include "WaveBankManager.h"

#ifdef PLATFORM_PS3
#include "SystemPS3shared.h"
#include <System/Scheduler/jobqueue.h>
#endif

#include "InstanceUpdate.h"
#include "Instance.h"

#include "ComponentHeaders.h"

#include "MemoryManager.h"

//#include "..\Include\Audio.h"
//
#include "..\Include\Stringhash.h"
#include "..\Include\AppInterfaceLayer.h"

#include "Definition.h"
#include "ComponentContainer.h"
#include "Object.h"
#include "ObjectManager.h"
#include "DefinitionManager.h"
#include "SystemCommon.h"
#include "System.h"
#include "DebugMessage.h"

#endif