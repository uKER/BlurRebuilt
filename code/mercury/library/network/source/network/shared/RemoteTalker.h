#ifndef REMOTE_TALKER_H
#define REMOTE_TALKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include <Systemprofile/SystemProfile.h>
#include "Btl/Containers/List.h"
#include "Btl/Containers/vector.h"


namespace Communicator
{
	struct PS3RemotePort 
	{
		PS3RemotePort()
		{
			m_VoiceInPort = 0;
			m_LocalPlayerConnection = 0;
			m_InPortCreated = 0;
			m_ConnectionActive = 0;
		}

		U32 m_VoiceInPort;
		U8 m_LocalPlayerConnection:6;
		U8 m_InPortCreated:1;
		U8 m_ConnectionActive:1;	
	};

	// assume maimum number of local talkers will never be greater than 1.
	// communicator lib on PS3 sonly supports a single mic / headset anyway.
	typedef Btl::Vector<PS3RemotePort, Btl::VectorParameters_Static<1> > PS3Ports;


	/// Used by Communicator::C_Core to keep track of remote players who are registed to have their voice decoded.
	class C_RemoteTalker : public Btl::ListElementDouble<C_RemoteTalker>
	{
	public:
		C_RemoteTalker() : m_LocalMuted(0), m_RemoteMuted(0), m_Priority(0), m_NeedsUpdate(false)
		{
#if defined ( PLATFORM_PS3 )
			m_Ports.Clear();
#endif
		};

		virtual ~C_RemoteTalker()
		{};

		void Destroy(void) 
		{
			// this is called then the object is removed from the linked list, required for ListElementDouble to function.
			m_LocalMuted = m_RemoteMuted = m_Priority = 0; m_NeedsUpdate = false;
#if defined ( PLATFORM_PS3 )
			m_Ports.Clear();
#endif
		} 

		SysProfile::UserIdentifier	m_UserData;
		U32							m_LocalMuted;		// we have muted him
		U32							m_RemoteMuted;		// he has muted me..
		U32							m_Priority;
		bool						m_NeedsUpdate;
#if defined ( PLATFORM_PS3 )
		PS3Ports					m_Ports;
#endif
	};

	/// to make things easier to read.
	typedef Btl::List<C_RemoteTalker>		C_RemoteTalkerList;
}


#endif
