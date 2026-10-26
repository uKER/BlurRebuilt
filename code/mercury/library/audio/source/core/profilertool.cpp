#include "AudioPch.h"
#include <core/utility/PointerSlot32.h> // [Blur2PC][x64]
#include "../core/coreheaders.h"
#include "../systemheaders.h"
#include "stdio.h"

#ifdef AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

///////////////////////////////////////////////////////////////////////////////
//	statics
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Profiler::uBuffer = 0;
Audio::Core::Lock Audio::Profiler::m_lock;
Ut::Vector<Audio::Profiler::ObjectPacket> Audio::Profiler::m_aObjectPkts[2];
Ut::Vector<Audio::Profiler::ValuePacket> Audio::Profiler::m_aValues[2];
Ut::Vector<Audio::Profiler::TriggerPacket> Audio::Profiler::m_aTriggers[2];
Ut::Vector<Audio::Profiler::EventPacket> Audio::Profiler::m_aEvents[2];
Ut::Vector<Audio::Profiler::SetParamPacket> Audio::Profiler::m_aSetParams[2];
Ut::Vector<Audio::Profiler::GetParamPacket> Audio::Profiler::m_aGetParams[2];
Ut::Vector<Audio::Profiler::AudioGraphPacket> Audio::Profiler::m_aAudioGraphs[2];
Ut::Vector<Audio::Profiler::AudioInfoPacket> Audio::Profiler::m_aAudioInfos[2];
Ut::Vector<Audio::Profiler::MutePacket> Audio::Profiler::m_aMutes[2];
Ut::Vector<Audio::Profiler::ConInfoPacket> Audio::Profiler::m_aConInfos[2];
Ut::Vector<Audio::Profiler::EventMsgPacket> Audio::Profiler::m_aEventMsgs[2];
Ut::Vector<Audio::Profiler::ErrorMsgPacket> Audio::Profiler::m_aErrorMsgs[2];
Ut::Vector<Audio::Profiler::WarningMsgPacket> Audio::Profiler::m_aWarningMsgs[2];

#ifdef AUDIO_BUILD
const bool gAudioProfilerEnabledByDefault = true;
#else
const bool gAudioProfilerEnabledByDefault = false;
#endif

namespace Audio 
{
	Debug::Bool EnableProfiling("Audio.System.Profiler.EnableProfiling", gAudioProfilerEnabledByDefault, Debug::Persist());
}

///////////////////////////////////////////////////////////////////////////////
//	InitializeComms
////////////////////////////////////////////////////////////////////////////////
bool Audio::Profiler::InitializeComms()
{
	if( !EnableProfiling )
		return true;

	bool ret = ArteryRegister("AudioProfiler", ProcessComms, 0);

	if (!ret)
	{
		AUDIO_SYS_ERROR("Cannot initialize profiler comms");

		// to prevent trying to attempt to connect forever
		if (uConnectAttempts >= 10)
			EnableProfiling = false;
	}
	else
	{
		AUDIO_SYS_INFO("Audio profiler communications initialized ok.");
	}

	//	let the profiler tool know we have restarted, if it is already connected
	Reset();

	bDoneCommsInit = ret;
	uConnectAttempts++;

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
//	Reply
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::Reply()
{
	if( !EnableProfiling )
		return;

	int iCount;
	U32 uBackBuffer = 1 - uBuffer;

	//	object list changed, force reset
	if (m_bResetInfo && m_bResetOnCreateDestroy)
	{
		m_bResetInfo = false;
		SendInitData();
	}

	ArteryMessageBuilder b;

	//	header
	b.Write("Data");

	//	Objects
	for (iCount = 0; iCount < m_aObjectPkts[uBackBuffer].Size(); iCount++)
	{
		ObjectPacket& pkt = m_aObjectPkts[uBackBuffer][iCount];
		SendObject(b, pkt.uObjHandle);
	}
	m_aObjectPkts[uBackBuffer].Clear();

	//	Values
	for (iCount = 0; iCount < m_aValues[uBackBuffer].Size(); iCount++)
	{
		ValuePacket& pkt = m_aValues[uBackBuffer][iCount];
		SendValue(b, pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle, pkt.uOutHandle);
	}
	m_aValues[uBackBuffer].Clear();

	//	Triggers
	for (iCount = 0; iCount < m_aTriggers[uBackBuffer].Size(); iCount++)
	{
		TriggerPacket& pkt = m_aTriggers[uBackBuffer][iCount];
		TriggerAction(pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle);
	}
	m_aTriggers[uBackBuffer].Clear();

	//	Events
	for (iCount = 0; iCount < m_aEvents[uBackBuffer].Size(); iCount++)
	{
		EventPacket& pkt = m_aEvents[uBackBuffer][iCount];
		SetEvents(pkt.uObjHandle, pkt.uInstHandle, pkt.uFlags);
	}
	m_aEvents[uBackBuffer].Clear();

	//	SetParam
	for (iCount = 0; iCount < m_aSetParams[uBackBuffer].Size(); iCount++)
	{
		SetParamPacket& pkt = m_aSetParams[uBackBuffer][iCount];
		SetParam(pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle, pkt.fValue, pkt.uFlags == 1);
	}
	m_aSetParams[uBackBuffer].Clear();

	//	GetParam
	for (iCount = 0; iCount < m_aGetParams[uBackBuffer].Size(); iCount++)
	{
		GetParamPacket& pkt = m_aGetParams[uBackBuffer][iCount];
		GetParam(b, pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle);
	}
	m_aGetParams[uBackBuffer].Clear();

	//	AudioGraph
	for (iCount = 0; iCount < m_aAudioGraphs[uBackBuffer].Size(); iCount++)
	{
		AudioGraphPacket& pkt = m_aAudioGraphs[uBackBuffer][iCount];
		SendAudioGraph(b, pkt.uObjHandle);
	}
	m_aAudioGraphs[uBackBuffer].Clear();

	//	AudioInfo
	for (iCount = 0; iCount < m_aAudioInfos[uBackBuffer].Size(); iCount++)
	{
		AudioInfoPacket& pkt = m_aAudioInfos[uBackBuffer][iCount];
		SendAudioInfo(b, pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle);
	}
	m_aAudioInfos[uBackBuffer].Clear();

	//	Mute
	for (iCount = 0; iCount < m_aMutes[uBackBuffer].Size(); iCount++)
	{
		MutePacket& pkt = m_aMutes[uBackBuffer][iCount];
		bool bMute = (pkt.uFlags & 1) ? true : false;
		MuteEvent(pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle, bMute);
	}
	m_aMutes[uBackBuffer].Clear();

	//	ConInfo
	for (iCount = 0; iCount < m_aConInfos[uBackBuffer].Size(); iCount++)
	{
		ConInfoPacket& pkt = m_aConInfos[uBackBuffer][iCount];
		SendConnectionInfo(b, pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle, pkt.uFlags);
	}
	m_aConInfos[uBackBuffer].Clear();

	//	EventMsg
	for (iCount = 0; iCount < m_aEventMsgs[uBackBuffer].Size(); iCount++)
	{
		EventMsgPacket& pkt = m_aEventMsgs[uBackBuffer][iCount];
		DoEventMessage(b, pkt.uObjHandle, pkt.uInstHandle, pkt.uComHandle, pkt.uMsgHandle);
	}
	m_aEventMsgs[uBackBuffer].Clear();

	//	ErrorMsg
	for (iCount = 0; iCount < m_aErrorMsgs[uBackBuffer].Size(); iCount++)
	{
		ErrorMsgPacket& pkt = m_aErrorMsgs[uBackBuffer][iCount];
		DoErrorMessage(b, pkt.uObjHandle, pkt.uInstHandle, pkt.uMsgHandle);
	}
	m_aErrorMsgs[uBackBuffer].Clear();

	//	WarningMsg
	for (iCount = 0; iCount < m_aWarningMsgs[uBackBuffer].Size(); iCount++)
	{
		WarningMsgPacket& pkt = m_aWarningMsgs[uBackBuffer][iCount];
		DoWarningMessage(b, pkt.uObjHandle, pkt.uInstHandle, pkt.uMsgHandle);
	}
	m_aWarningMsgs[uBackBuffer].Clear();

	//	footer
	b.Write("End");

	//	build msg
	ArteryMessage msg;
	if (b.Build(msg))
		m_AudioArtery.Send(msg);

	m_lock.Acquire();		//	TODO: use interlocked var
	uBuffer = uBackBuffer;
	m_lock.Release();
}

////////////////////////////////////////////////////////////////////////////////
//	ProcessComms (static)
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::ProcessComms(ArteryConnection con, void*)
{
	if( !EnableProfiling )
		return;

	Profiler::Instance().SetConnection(con);

	while (con.IsConnected())
	{
		ArteryMessage msg;

		if (con.Receive(msg))
		{
			ArteryMessageReader msgReader(msg);

			const char* msgString(NULL);

			if (!msgReader.Read(msgString))
				continue;

			//AUDIO_CORE_DEBUG("{rec} %s", msgString);

			if (!strcmp(msgString, "InitData"))
			{
				Profiler::Instance().SendInitData();
			}
			else if (!strcmp(msgString, "Object"))
			{
				ObjectPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				m_aObjectPkts[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "Value"))
			{
				ValuePacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uComHandle);
				msgReader.Read(pkt.uOutHandle);
				m_aValues[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "Trigger"))
			{
				TriggerPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uComHandle);
				m_aTriggers[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "SetParam"))
			{
				SetParamPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uComHandle);
				msgReader.Read(pkt.fValue);
				msgReader.Read(pkt.uFlags);
				m_aSetParams[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "GetParam"))
			{
				GetParamPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uComHandle);
				m_aGetParams[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "AudioGraph"))
			{
				AudioGraphPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				m_aAudioGraphs[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "AudioInfo"))
			{
				AudioInfoPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uComHandle);
				m_aAudioInfos[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "Mute"))
			{
				MutePacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uComHandle);
				msgReader.Read(pkt.uFlags);
				m_aMutes[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "ConInfo"))
			{
				ConInfoPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uComHandle);
				msgReader.Read(pkt.uFlags);
				m_aConInfos[uBuffer].PushBack(pkt);
			}
			else if (!strcmp(msgString, "Events"))
			{
				EventPacket pkt;
				msgReader.Read(pkt.uObjHandle);
				msgReader.Read(pkt.uInstHandle);
				msgReader.Read(pkt.uFlags);
				m_aEvents[uBuffer].PushBack(pkt);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::Reset()
{
	ArteryMessageBuilder b;

	//	header
	b.Write("Reset");

	ArteryMessage msg;

	if (b.Build(msg))
		m_AudioArtery.Send(msg);
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SendInitData()
{
	System::Get()->GetLock()->Acquire();

	ArteryMessageBuilder b;
	b.Write("InitData");

	int numObjs = System::Get()->GetObjectManager()->GetObjects().Size();
	b.Write((ArteryU16)numObjs);
	for (int i=0; i<numObjs; i++)
	{
		Object* pObj = System::Get()->GetObjectManager()->GetObjects().At(i);
		int numInsts = pObj->GetInstances().Size();
		b.Write(pObj->Name());
		//b.Write((ArteryU32)pObj->GetHandle().Value());
		b.Write((ArteryU32)::Core::PointerToSlot32(pObj));	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot
		b.Write((ArteryU16)numInsts);
		for (int i=0; i<numInsts; i++)
		{
			Audio::Instance* pInst = pObj->GetInstances().At(i);
			b.Write((ArteryU32)pInst->GetHandle().Value());
		}
	}

	ArteryMessage msg;

	if (b.Build(msg))
		m_AudioArtery.Send(msg);

	System::Get()->GetLock()->Release();
}

////////////////////////////////////////////////////////////////////////////////
//	SendObject
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SendObject(ArteryMessageBuilder& b, U32 uHandle)
{
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		U32 size = pObject->GetDefinition()->GetDataSize();
		void* data = pObject->GetDefinition()->Data();

		//	header
		b.Write("Object");					//	object data packet
		b.Write((ArteryU32)uHandle);		//	object handle
		b.Write((ArteryU32)size);			//	definition data size
		b.Write(data, size);				//	definition data
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SendValue
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SendValue(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle, U16 uOutHandle)
{
	if (uObjHandle == (U32)-1 && uInstHandle == (U32)-1)
	{
		//	is requesting system values

		F32 fVal = 0.f;
		if (uComHandle < 100)
		{
			//	cpu
			fVal = System::Get()->CPUUsage(uComHandle);
		}
		else if (uComHandle < 200)
		{
			//	memory
			fVal = 0.25f;
		}
		else if (uComHandle < 300)
		{
			//	stream i/o
			fVal = 0.75f;
		}
		else if (uComHandle < 400)
		{
			//	objects
			fVal = (F32)System::Get()->GetObjectManager()->GetObjects().Size();
		}
		else if (uComHandle < 500)
		{
			//	instances
			U32 uTotal = 0;
			U32 uSuspended = 0;
			U32 uMuted = 0;

			for (int i = 0; i < System::Get()->GetObjectManager()->GetObjects().Size(); i++)
			{
				Object* pObject = System::Get()->GetObjectManager()->GetObjects().At(i);

				for (int j = 0; j < pObject->GetInstances().Size(); j++)
				{
					Audio::Instance* pInstance = pObject->GetInstances().At(j);

					uTotal++;

					if (pInstance->IsSuspended())
						uSuspended++;

					if (pInstance->IsMuted())
						uMuted++;
				}
			}

			if (uComHandle == 400)
				fVal = (F32)uTotal;
			else if (uComHandle == 401)
				fVal = (F32)(uTotal - uSuspended);
			else if (uComHandle == 402)
				fVal = (F32)uMuted;
		}
		else if (uComHandle < 600)
		{
			if (uComHandle == 500)
				fVal = (F32)Core::Generator::TotalMix();
			else if (uComHandle == 501)
				fVal = (F32)Core::Generator::ActiveMix();
		}
		else if (uComHandle < 700)
		{
			if (uComHandle == 600)
				fVal = (F32)Core::Generator::TotalWave();
			else if (uComHandle == 601)
				fVal = (F32)Core::Generator::ActiveWave();
		}
		//else if (uComHandle < 800)
		//{
		//	if (uComHandle == 700)
		//		fVal = (F32)Core::IGenerator::TotalDSP();
		//	else if (uComHandle == 701)
		//		fVal = (F32)Core::IGenerator::ActiveDSP();
		//}
		else if (uComHandle < 900)
		{
			//	speakers
			if (uComHandle == 804)
				fVal = System::Get()->GetSpeakerDistance();
			else
			{
				F32* afAngles = System::Get()->GetSpeakerAngles();

				if (uComHandle == 800)
					fVal = 180.f * afAngles[4];
				else if (uComHandle == 801)
					fVal = 180.f * afAngles[1];
				else if (uComHandle == 802)
					fVal = 180.f * afAngles[3];
				else if (uComHandle == 803)
					fVal = 180.f * afAngles[2];
			}
		}
		else if (uComHandle < 1000)
		{
			//	output level
			if (uComHandle == 900)
				fVal = System::Get()->GetRendererLevel(0);
			else if (uComHandle == 901)
				fVal = System::Get()->GetRendererLevel(1);
			else if (uComHandle == 902)
				fVal = System::Get()->GetRendererLevel(2);
			else if (uComHandle == 903)
				fVal = System::Get()->GetRendererLevel(3);
			else if (uComHandle == 904)
				fVal = System::Get()->GetRendererLevel(4);
			else if (uComHandle == 905)
				fVal = System::Get()->GetRendererLevel(5);
		}

		//	header
		b.Write("Value");									//	object data packet
		b.Write((ArteryU32)System::Get()->GetFrameNum());	//	frame number
		b.Write((ArteryU32)uObjHandle);						//	object handle
		b.Write((ArteryU32)uInstHandle);					//	instance handle
		b.Write((ArteryU16)uComHandle);						//	component handle
		b.Write((ArteryU16)uOutHandle);						//	out handle
		b.Write((ArteryF32)fVal);							//	value
	}
	else
	{
		//	get object
		//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
		Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot
		
		if (pObject)
		{
			for (int i=0; i<pObject->GetInstances().Size(); i++)
			{
				//	get instance
				Audio::Instance* pInst = pObject->GetInstances().At(i);

				if (pInst->GetHandle().Value() == uInstHandle)
				{
					//	get value
					F32 fVal = pObject->GetComponentValue(uComHandle, pInst, uOutHandle);

					//	header
					b.Write("Value");									//	object data packet
					b.Write((ArteryU32)System::Get()->GetFrameNum());	//	frame number
					b.Write((ArteryU32)uObjHandle);						//	object handle
					b.Write((ArteryU32)uInstHandle);					//	instance handle
					b.Write((ArteryU16)uComHandle);						//	component handle
					b.Write((ArteryU16)uOutHandle);						//	out handle
					b.Write((ArteryF32)fVal);							//	value

					break;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	TriggerAction
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::TriggerAction(U32 uObjHandle, U32 uInstHandle, U16 uComHandle)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		for (int i=0; i<pObject->GetInstances().Size(); i++)
		{
			//	get instance
			Audio::Instance* pInst = pObject->GetInstances().At(i);

			if (pInst->GetHandle().Value() == uInstHandle)
			{
				//	get com
				Component* pCom = pObject->GetComponent(uComHandle);

				//	TODO: ensure it is an ActionIn
				ActionIn* pActionIn = (ActionIn*)pCom;

				pActionIn->DoExecute(pInst);

				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SetParam
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SetParam(U32 uObjHandle, U32 uInstHandle, U16 uComHandle, F32 fValue, bool bOverrideGame)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		for (int i=0; i<pObject->GetInstances().Size(); i++)
		{
			//	get instance
			Audio::Instance* pInst = pObject->GetInstances().At(i);

			if (pInst->GetHandle().Value() == uInstHandle)
			{
				//	get com
				Component* pCom = pObject->GetComponent(uComHandle);

				//	TODO: ensure it is an ParameterIn
				ParameterIn* pParameterIn = (ParameterIn*)pCom;

				if (bOverrideGame)
					pParameterIn->ForceSetValue(pInst, fValue);

				pParameterIn->SetDisableFlag(pInst, bOverrideGame);

				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SetParam
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::GetParam(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		for (int i=0; i<pObject->GetInstances().Size(); i++)
		{
			//	get instance
			Audio::Instance* pInst = pObject->GetInstances().At(i);

			if (pInst->GetHandle().Value() == uInstHandle)
			{
				//	get value
				F32 fVal = pObject->GetComponentValue(uComHandle, pInst, 0);

				//	header
				b.Write("GetParam");				//	get param data packet
				b.Write((ArteryU32)uObjHandle);		//	object handle
				b.Write((ArteryU32)uInstHandle);	//	instance handle
				b.Write((ArteryU16)uComHandle);		//	component handle
				b.Write((ArteryF32)fVal);			//	value

				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SendAudioGraph
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SendAudioGraph(ArteryMessageBuilder& b, U32 uObjHandle)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		//	header
		b.Write("AudioGraph");				//	audio graph data packet
		b.Write((ArteryU32)uObjHandle);		//	object handle

		//	count audio components
		U16 uCount = 0;
		for (U32 i=0; i<pObject->GetNumComponents(); i++)
		{
			//	get com
			Audio::Component* pCom = pObject->GetComponent(i);

			if (pCom->IsAudio())
				uCount++;
		}

		b.Write((ArteryU16)uCount);		//	num audio coms

		for (U32 i=0; i<pObject->GetNumComponents(); i++)
		{
			//	get com
			Audio::Component* pCom = pObject->GetComponent(i);

			if (pCom->IsAudio())
			{
				AudioComponent* pAudioCom = (AudioComponent*)pCom;

				U16 uNumInputs = pAudioCom->GetNumInputs();

				b.Write((ArteryU16)i);				//	component handle
				b.Write((ArteryU16)uNumInputs);		//	num inputs
				for (int i=0; i<uNumInputs; i++)
				{
					Com com = pAudioCom->GetInputCom(i);
					b.Write((ArteryU16)com);		//	inputs
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SendAudioInfo
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SendAudioInfo(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		for (int i=0; i<pObject->GetInstances().Size(); i++)
		{
			//	get instance
			Audio::Instance* pInst = pObject->GetInstances().At(i);

			if (pInst->GetHandle().Value() == uInstHandle)
			{
				//	get com
				Component* pCom = pObject->GetComponent(uComHandle);

				Assert( pCom->IsAudio() );

				AudioComponent* pAudioCom = (AudioComponent*)pCom;

				U16 uNumInputs = pAudioCom->GetNumInputs();

				U16 uFlags = pAudioCom->IsActive(pInst) ? 0x01 : 0x00;

				//	header
				b.Write("AudioInfo");				//	object data packet
				b.Write((ArteryU32)uObjHandle);		//	object handle
				b.Write((ArteryU32)uInstHandle);	//	instance handle
				b.Write((ArteryU16)uComHandle);		//	component handle
				b.Write((ArteryU16)uFlags);			//	flags
				b.Write((ArteryU16)uNumInputs);		//	num inputs
				for (int i=0; i<uNumInputs; i++)
				{
					Com com = pAudioCom->GetInputCom(i);
					b.Write((ArteryU16)com);		//	inputs
				}

				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	TriggerAction
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::MuteEvent(U32 uObjHandle, U32 uInstHandle, U16 uComHandle, bool bMute)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		for (int i=0; i<pObject->GetInstances().Size(); i++)
		{
			//	get instance
			Audio::Instance* pInst = pObject->GetInstances().At(i);

			if (pInst->GetHandle().Value() == uInstHandle)
			{
				//	get com
				Component* pCom = pObject->GetComponent(uComHandle);

				Assert( pCom->IsAudio() );

				AudioComponent* pAudioCom = (AudioComponent*)pCom;

				pAudioCom->SetMute(pInst, bMute);

				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SendConnectionInfo
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SendConnectionInfo(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle, U16 uFlags)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		for (int i=0; i<pObject->GetInstances().Size(); i++)
		{
			//	get instance
			Audio::Instance* pInst = pObject->GetInstances().At(i);

			if (pInst->GetHandle().Value() == uInstHandle)
			{
				U32 uOtherObjHandle = -1;
				U32 uOtherInstHandle = -1;
				U16 uOtherComHandle = -1;

				//	get com
				Component* pCom = pObject->GetComponent(uComHandle);

				Assert( pCom->IsAudio() );

				AudioComponent* pAudioCom = (AudioComponent*)pCom;

				Audio::Instance* pOtherInst = pAudioCom->GetOtherInstance(pInst);

				if (pOtherInst)
				{
					//uOtherObjHandle = pOtherInst->GetObject()->GetHandle().Value();
					uOtherObjHandle = ::Core::PointerToSlot32(pOtherInst->GetObject());	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot
					uOtherInstHandle = pOtherInst->GetHandle().Value();
					uOtherComHandle = pAudioCom->GetOtherComponent(pInst);
				}

				//	header
				b.Write("ConInfo");						//	object data packet
				b.Write((ArteryU32)uObjHandle);			//	object handle
				b.Write((ArteryU32)uInstHandle);		//	instance handle
				b.Write((ArteryU16)uComHandle);			//	component handle
				b.Write((ArteryU32)uOtherObjHandle);	//	object handle
				b.Write((ArteryU32)uOtherInstHandle);	//	instance handle
				b.Write((ArteryU16)uOtherComHandle);	//	component handle
				b.Write((ArteryU16)uFlags);				//	flags

				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SetEvents
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::SetEvents(U32 uObjHandle, U32 uInstHandle, U16 uFlags)
{
	//	get object
	//Object* pObject = System::Get()->GetObjectManager()->GetObject(uObjHandle);
	Object* pObject = (Object*)::Core::Slot32ToPointer(uObjHandle);	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot

	if (pObject)
	{
		for (int i=0; i<pObject->GetInstances().Size(); i++)
		{
			//	get instance
			Audio::Instance* pInst = pObject->GetInstances().At(i);

			if (pInst->GetHandle().Value() == uInstHandle)
			{
				pInst->SetEventLogging(uFlags == 1 ? true : false);
				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	PostEventMessage - immediate
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::PostEventMessage(Audio::Instance* pInstance, S16 sCom, S16 uMsg)
{
	if( !EnableProfiling )
		return;

	EventMsgPacket pkt;
	if (pInstance)
	{
		if (!pInstance->EventLogging())
			return;

		//pkt.uObjHandle = pInstance->GetObject()->GetHandle().Value();
		pkt.uObjHandle = ::Core::PointerToSlot32(pInstance->GetObject());	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot
		pkt.uInstHandle = pInstance->GetHandle().Value();
	}
	else
	{
		pkt.uObjHandle = (U32)-1;
		pkt.uInstHandle = (U32)-1;
	}
	pkt.uComHandle = sCom;
	pkt.uMsgHandle = uMsg;

	m_aEventMsgs[uBuffer].PushBack(pkt);
}

////////////////////////////////////////////////////////////////////////////////
//	DoEventMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::DoEventMessage(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, S16 sCom, S16 uMsg)
{

	//	header
	b.Write("Event");									//	msg/event data packet
	b.Write((ArteryU32)System::Get()->GetFrameNum());	//	frame number
	b.Write((ArteryU32)uObjHandle);						//	object handle
	b.Write((ArteryU32)uInstHandle);					//	instance handle
	b.Write((ArteryU16)sCom);							//	component handle
	b.Write((ArteryU16)uMsg);							//	msg handle
}

////////////////////////////////////////////////////////////////////////////////
//	PostErrorMessage - immediate
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::PostErrorMessage(Audio::Instance* pInstance, Errors eError)
{
	if( !EnableProfiling )
		return;

	ErrorMsgPacket pkt;
	if (pInstance)
	{
		if (!pInstance->EventLogging())
			return;

		//pkt.uObjHandle = pInstance->GetObject()->GetHandle().Value();
		pkt.uObjHandle = ::Core::PointerToSlot32(pInstance->GetObject());	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot
		pkt.uInstHandle = pInstance->GetHandle().Value();
	}
	else
	{
		pkt.uObjHandle = (U32)-1;
		pkt.uInstHandle = (U32)-1;
	}
	pkt.uMsgHandle = (S16)eError;

	m_aErrorMsgs[uBuffer].PushBack(pkt);
}


////////////////////////////////////////////////////////////////////////////////
//	DoErrorMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::DoErrorMessage(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, S16 sError)
{
	//	header
	b.Write("Error");									//	warning data packet
	b.Write((ArteryU32)System::Get()->GetFrameNum());	//	frame number
	b.Write((ArteryU32)uObjHandle);						//	object handle
	b.Write((ArteryU32)uInstHandle);					//	instance handle
	b.Write((ArteryI16)sError);							//	error handle
}

////////////////////////////////////////////////////////////////////////////////
//	PostWarningMessage - immediate
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::PostWarningMessage(Audio::Instance* pInstance, Warnings eWarning)
{
	if( !EnableProfiling )
		return;

	WarningMsgPacket pkt;
	if (pInstance)
	{
		if (!pInstance->EventLogging())
			return;

		//pkt.uObjHandle = pInstance->GetObject()->GetHandle().Value();
		pkt.uObjHandle = ::Core::PointerToSlot32(pInstance->GetObject());	// [Blur2PC][x64] 32-bit profiler id: a Core::PointerToSlot32 slot
		pkt.uInstHandle = pInstance->GetHandle().Value();
	}
	else
	{
		pkt.uObjHandle = (U32)-1;
		pkt.uInstHandle = (U32)-1;
	}
	pkt.uMsgHandle = (S16)eWarning;

	m_aWarningMsgs[uBuffer].PushBack(pkt);
}

////////////////////////////////////////////////////////////////////////////////
//	DoWarningMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Profiler::DoWarningMessage(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, S16 sWarning)
{
	//	header
	b.Write("Warning");									//	warning data packet
	b.Write((ArteryU32)System::Get()->GetFrameNum());	//	frame number
	b.Write((ArteryU32)uObjHandle);						//	object handle
	b.Write((ArteryU32)uInstHandle);					//	instance handle
	b.Write((ArteryI16)sWarning);						//	error handle
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#endif // #ifdef AUDIO_ENABLE_PROFILING

