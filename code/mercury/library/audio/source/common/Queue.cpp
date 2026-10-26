#include "AudioPch.h"
#include "..\common\types.h"
#include "..\common\Queue.h"
#include "..\common\Util.h"

//	TODO: improve this
#include "..\..\include\audio.h"
#include "..\systemheaders.h"


//using namespace Audio;

#ifndef CFG_GOLD
U32 largestMessage = 0;
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize("", off) // AUDIO_OPTIMIZE_OFF
#endif // AUDIO_OPTIMIZE_OFF

//-----------------------------------------------------------------------------
//	Update
//-----------------------------------------------------------------------------
void Audio::Core::Queue::Update()
{
	AUDIO_PERF_EVENT("Audio.Queue.Update");

	Assert(m_paBuffer != NULL);
	Assert(m_uQueueArraySize != 0);

	#ifndef CFG_GOLD
	U32 bytesQueued = m_sWriteOffset >= m_sReadOffset ? (m_sWriteOffset - m_sReadOffset) * sizeof(U32) : (m_uQueueArraySize - (m_sReadOffset - m_sWriteOffset)) * sizeof(U32);
	m_uNumBytesQueued = Maths::Max(bytesQueued, m_uNumBytesQueued);
	m_uPeakBytesQueued = Maths::Max(m_uNumBytesQueued, m_uPeakBytesQueued);
	#endif

	while (ReceiveItem()) ;
}

//-----------------------------------------------------------------------------
//	Init
//-----------------------------------------------------------------------------
void Audio::Core::Queue::Init(U32 uNumQueueBytes)
{
	Assert(uNumQueueBytes != 0);

	m_uQueueArraySize = uNumQueueBytes  / sizeof(U32);

	if (m_uQueueArraySize == 0)	// fixup to prevent size 0 allocations
		m_uQueueArraySize = 1024 * 16;

	m_paBuffer = new U32[m_uQueueArraySize];
}

//-----------------------------------------------------------------------------
//	SendItem
//-----------------------------------------------------------------------------
bool Audio::Core::Queue::SendItem(U32 eItem, const void* pData, U32 uSize)
{
	Assert(uSize < 1024);
	Assert(m_paBuffer != NULL);
	Assert(m_uQueueArraySize != 0);

    //  packet size in U32s
    U32 uActualSize = (uSize + 3) / 4;

    //  add packet size and context ptr 
    U32 uPacketSize = uActualSize + 2;  // TODO: don't assume ptr is 32bit

	#ifndef CFG_GOLD
	if (uPacketSize > largestMessage)
		largestMessage = uPacketSize;
	#endif

	AcquireLock();

    U32 uRead = m_sReadOffset;
    U32 uWrite = m_sWriteOffset;

    if ((uRead > uWrite) && (uPacketSize > uRead - uWrite))
    {
        // contiguious but not enough space
//		if (!m_sInterval)
//		{
			
			AUDIO_SYS_WARN("Warning: Queue overflow - packet dropped");
			Assert(0);
//			m_sInterval = 100;
//		}
//		m_sInterval--;
		ReleaseLock();
        return false;
    }

	if (uPacketSize > m_uQueueArraySize - uWrite)
    {
        //  split space but 1st half (end of buffer) not large enough
        if (uWrite != m_uQueueArraySize)
            //  zero to indicate wraparound
			m_paBuffer[uWrite] = 0;	// should equate to "NoItem" in both SendQueue and ReturnQueue

        uWrite = 0;

        //  if no room in 2nd half (start of buffer), failed
        if (uPacketSize > uRead)
        {
			if (!m_sInterval)
			{
				AUDIO_SYS_DEBUG("Warning: Queue overflow - packet dropped");
				Assert(0);

#if defined (AUDIO_ENABLE_PROFILING)
				Profiler::Instance().PostErrorMessage(NULL, Profiler::eMessageQueueFull);
#endif

				m_sInterval = 100;
			}
			m_sInterval--;
			ReleaseLock();
            return false;
        }
    }

    //  write packet
    m_paBuffer[uWrite++] = (U32)eItem;
    m_paBuffer[uWrite++] = uSize;
    memcpy(&m_paBuffer[uWrite], pData, uSize);
    uWrite += uActualSize;

    //  safely inc write offset
	m_sWriteOffset += uWrite - m_sWriteOffset;
	//Increment(&m_sWriteOffset, uWrite - m_sWriteOffset);

	ReleaseLock();

    return true;
}

//-----------------------------------------------------------------------------
//	SendQueue::ReceiveItem
//	Desc: returning false will cause the queue processing to terminate for this frame
//-----------------------------------------------------------------------------
bool Audio::Core::SendQueue::ReceiveItem()
{
	Assert(m_paBuffer != NULL);
	Assert(m_uQueueArraySize != 0);

    U32 sRead = m_sReadOffset;
    U32 sWrite = m_sWriteOffset;

    //  empty
	if (sRead == sWrite)
		return false;

    //  reached end, move to start
    if ((sRead == m_uQueueArraySize) || (m_paBuffer[sRead] == eNoItem))
        sRead = 0;

    //  get packet
	SendQueueItem eItem = (SendQueueItem)m_paBuffer[sRead++];
    U32 uSize = m_paBuffer[sRead++];
    U32* pData = &m_paBuffer[sRead];
    U32 uActualSize = (uSize + 3) / 4;
    sRead += uActualSize;

	bool bRet = true;

    //  process packet
    switch (eItem)
	{
	case eObjectManager:
		{
			ObjectManager::Packet* pPacket = (Audio::ObjectManager::Packet*)pData;

			System::Get()->GetObjectManager()->ReceiveData(pData, uSize);

			if (System::Get()->StaggerInstanceCreation())
			{
				// if we just created an audio object then bail out and dont process any more queue items this audio frame.
				// this attempts to "smooth out" any big chunks of instance creation work over a few frames in order
				// to try and stop the engine framing out when it gets many consecutive instance creation requests on the queue.
				if (pPacket->uMessage == ObjectManager::eCreateInstance)
					bRet = false;
			}
		}
		break;
	case eSetMaster:
		{
			System::Get()->ReceiveSetMaster(*((InstanceComponentHandle*)pData));
		}
		break;
	case eSetMonoMaster:
		{
			System::Get()->ReceiveSetMonoMaster(*((InstanceComponentHandle*)pData));
		}
		break;
	case eSetStereoMaster:
		{
			System::Get()->ReceiveSetStereoMaster(*((InstanceComponentHandle*)pData));
		}
		break;
	case eSetSurroundMaster:
		{
			System::Get()->ReceiveSetSurroundMaster(*((InstanceComponentHandle*)pData));
		}
		break;
	case eSetSurroundLFEMaster:
		{
			System::Get()->ReceiveSetSurroundLFEMaster(*((InstanceComponentHandle*)pData));
		}
		break;

	default:
		AUDIO_SYS_ERROR("SendQueue() : unknown message packet");
		break;
	}

	#if defined(PLATFORM_PS3)
	__lwsync();
	#else
	// apparently unnecessary on pc.. jkc
//	ReleaseLockBarrier();
	#endif

	int v = m_sReadOffset + (sRead - m_sReadOffset);

    //  safely inc read offset
	Increment(&m_sReadOffset, sRead - m_sReadOffset);

	return bRet;
}

//-----------------------------------------------------------------------------
//	ReturnQueue::ReceiveItem
//-----------------------------------------------------------------------------
bool Audio::Core::ReturnQueue::ReceiveItem()
{
	Assert(m_paBuffer != NULL);
	Assert(m_uQueueArraySize != 0);

    U32 sRead = m_sReadOffset;
    U32 sWrite = m_sWriteOffset;

    //  empty
	if (sRead == sWrite)
		return false;

    //  reached end, move to start
    if ((sRead == m_uQueueArraySize) || (m_paBuffer[sRead] == eNoItem))
        sRead = 0;

    //  get packet
	ReturnQueueItem eItem = (ReturnQueueItem)m_paBuffer[sRead++];
    U32 uSize = m_paBuffer[sRead++];
    U32* pData = &m_paBuffer[sRead];
    U32 uActualSize = (uSize + 3) / 4;
    sRead += uActualSize;

    //  process packet
    switch (eItem)
	{
		case eGetValue:
		{
			GetValueQueueMessage* pMessage = (GetValueQueueMessage*)pData;

			if (pMessage->pCallback)
				pMessage->pCallback(pMessage->hDataTracker, pMessage->f32Value);
		}
		break;

		case eActionOut:
		{
			CallbackQueueMessage* pMessage = (CallbackQueueMessage*)pData;

			if (pMessage->pCallback)
				pMessage->pCallback(pMessage->u32AppData);
		}
		break;

		case eDestroyObject:
		{
			DestroyObjectQueueMessage* pMessage = (DestroyObjectQueueMessage*)pData;

			System::Get()->GetObjectManager()->DestroyImmediate(pMessage->hObject, pMessage->bReserveHandle);
		}
		break;

		default:
			AUDIO_SYS_ERROR("ReturnQueue() : unknown message packet");
			break;
	}

	#if defined(PLATFORM_PS3)
	__lwsync();
	#else
	// apparently unnecessary on pc.. jkc
//	ReleaseLockBarrier();
	#endif

	int v = m_sReadOffset + (sRead - m_sReadOffset);

    //  safely inc read offset
	Increment(&m_sReadOffset, sRead - m_sReadOffset);

	return true;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize("", on) // AUDIO_OPTIMIZE_OFF
#endif // AUDIO_OPTIMIZE_OFF




