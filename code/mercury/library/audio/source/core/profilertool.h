#ifndef _PROFILERTOOL_H_
#define _PROFILERTOOL_H_

#pragma once

#include "core/containers.h"


#define AUDIO_CORE_PROFILE_WINDOW 256

namespace Audio
{
	extern Debug::Bool EnableProfiling;

	class Object;
	class Instance;

	class Profiler
	{
	public:
		enum Messages
		{
			eUnknownMessage = -1,
			eWaveBankLoaded,
		};

		enum Errors
		{
			eUnknownError = -1,
			eOutOfMemory,
			eMessageQueueFull,
		};

		enum Warnings
		{
			eUnknownWarning = -1,
			eWaveBankNotFound,
			eWaveNotInWavebank,
			eFrameDropped,
			eRendererOverloaded,
			eSystemMusicOverrideOn,
			eSystemMusicOverrideOff,
		};

		struct ObjectPacket
		{
			U32 uObjHandle;
		};

		struct ValuePacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
			U16 uOutHandle;
		};

		struct TriggerPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
		};

		struct EventPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uFlags;
		};

		struct SetParamPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
			F32 fValue;
			U16 uFlags;
		};

		struct GetParamPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
		};

		struct AudioGraphPacket
		{
			U32 uObjHandle;
		};

		struct AudioInfoPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
		};

		struct MutePacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
			U16 uFlags;
		};

		struct ConInfoPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
			U16 uFlags;
		};

		struct EventMsgPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uComHandle;
			U16 uMsgHandle;
		};

		struct ErrorMsgPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uMsgHandle;
		};

		struct WarningMsgPacket
		{
			U32 uObjHandle;
			U32 uInstHandle;
			U16 uMsgHandle;
		};

	public:
		static Profiler& Instance( )
		{
			static Profiler singleton;
			return singleton;
		}

	// Other non-static member functions
	private:
		Profiler()
		{
			m_bResetOnCreateDestroy = false;	//	set this to auto refresh columbo
			m_bResetInfo = false;
			bDoneCommsInit = false;
			uConnectAttempts = 0;
		};
		Profiler( const Profiler& );
		Profiler& operator = ( const Profiler& );

	public:
		bool InitializeComms();
		void Reply();
		void SetConnection(ArteryConnection con) { m_AudioArtery = con; }

		bool CommsInitialized() { return bDoneCommsInit; }

		//	called in Reply()
		void SendInitData();
		void SendObject(ArteryMessageBuilder& b, U32 uHandle);
		void SendValue(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle, U16 uOutHandle);
		void TriggerAction(U32 uObjHandle, U32 uInstHandle, U16 uComHandle);
		void SetParam(U32 uObjHandle, U32 uInstHandle, U16 uComHandle, F32 fValue, bool bOverrideGame);
		void GetParam(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle);
		void SendAudioGraph(ArteryMessageBuilder& b, U32 uObjHandle);
		void SendAudioInfo(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle);
		void MuteEvent(U32 uObjHandle, U32 uInstHandle, U16 uComHandle, bool bMute);
		void SendConnectionInfo(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, U16 uComHandle, U16 uFlags);
		void SetEvents(U32 uObjHandle, U32 uInstHandle, U16 uFlags);
		void DoEventMessage(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, S16 sCom, S16 uMsg);
		void DoErrorMessage(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, S16 sError);
		void DoWarningMessage(ArteryMessageBuilder& b, U32 uObjHandle, U32 uInstHandle, S16 sWarning);

		//	Called by audio thread
		void PostEventMessage(Audio::Instance* pInstance, S16 sCom, S16 uMsg);
		void PostErrorMessage(Audio::Instance* pInstance, Errors eError);
		void PostWarningMessage(Audio::Instance* pInstance, Warnings eWarning);

		void Reset();
		void SendStatus();
		void AddInstance(U32 uHandle);
		void RemoveInstance(U32 uHandle);
		void ResetInfo() { m_bResetInfo = true; }

		static void ProcessComms(ArteryConnection con, void*);


	private:
		ArteryConnection m_AudioArtery;

		static Audio::Core::Lock m_lock;

		bool m_bResetOnCreateDestroy;

		bool bDoneCommsInit;
		U32 uConnectAttempts;

		//	set by comms
		bool m_bResetInfo;
		Ut::Vector<Object*> m_aObjects;
		Ut::Vector<Audio::Instance*> m_aInstances;

		static U32 uBuffer;

		static Ut::Vector<ObjectPacket> m_aObjectPkts[2];
		static Ut::Vector<ValuePacket> m_aValues[2];
		static Ut::Vector<TriggerPacket> m_aTriggers[2];
		static Ut::Vector<EventPacket> m_aEvents[2];
		static Ut::Vector<SetParamPacket> m_aSetParams[2];
		static Ut::Vector<GetParamPacket> m_aGetParams[2];
		static Ut::Vector<AudioGraphPacket> m_aAudioGraphs[2];
		static Ut::Vector<AudioInfoPacket> m_aAudioInfos[2];
		static Ut::Vector<MutePacket> m_aMutes[2];
		static Ut::Vector<ConInfoPacket> m_aConInfos[2];
		static Ut::Vector<EventMsgPacket> m_aEventMsgs[2];
		static Ut::Vector<ErrorMsgPacket> m_aErrorMsgs[2];
		static Ut::Vector<WarningMsgPacket> m_aWarningMsgs[2];
	};
}

#endif // _PROFILERTOOL_H_

