
#include "stdafx.h"
#include <camera2/network/NetworkLink.h>

#pragma warning (push, 3)
#ifdef PLATFORM_PC
#include <Winsock2.h>
#endif
#include <System/Artery/Artery.h>
#include <system/serialise.h>
#include <system/serialise/I_FileSystem.h>
#include <system/serialise/C_MemoryStream.h>
#include <core/threads.h>
#pragma warning (pop)
#include <camera2/tv/AnimData.h>
#include <camera2/tv/AnimDataSerialise.h>
#include <camera2/tv/CameraScene.h>

namespace Cam2
{

C_NetworkLink& C_NetworkLink::Instance()
{
	static C_NetworkLink s_instance;
	return s_instance;
}

C_NetworkLink::C_NetworkLink()
#ifndef CFG_GOLD
: m_clientMutex("NetLink")
#endif
{
	
	m_notify = E_NetNotify_None;
	m_animData = 0;
}

C_NetworkLink::~C_NetworkLink()
{
	Exit();
	
#ifndef CFG_GOLD
	delete m_animData;
#endif
	m_animData = 0;
}

#pragma warning (disable: 4390) // ';' : empty controlled statement found;

void C_NetworkLink::Init()
{
#ifndef CFG_GOLD
	bool ret = ArteryRegister("Camera", StaticNetProc, this);
	if (!ret)
		assert(!"ArteryRegister failed");
	
#endif
}

void C_NetworkLink::Exit()
{
#ifndef CFG_GOLD
	m_clientMutex.Wait();

	for (S32 i = m_clients.Size() - 1; i >= 0; i--)
	{
		if (m_clients[i])
			m_clients[i]->Close();
	}

	m_clients.Clear();
	m_clientMutex.Release();
#endif
}

void C_NetworkLink::Update()
{
#ifndef CFG_GOLD
	m_notify = E_NetNotify_None;

	m_clientMutex.Wait();

	S32 i = 0;
	// Go through the list of clients:
	for (i = 0; i < m_clients.Size(); i++)
	{
		ProcessConnection(*m_clients[i]);
	}
	
	m_clientMutex.Release();
	
	m_updateCount++;
#endif
}


#ifndef CFG_GOLD
void C_NetworkLink::StaticNetProc(ArteryConnection con, void *self)
{
	((C_NetworkLink *)self)->NetProc(con);
}

void C_NetworkLink::NetProc(ArteryConnection con)
{
	m_clientMutex.Wait();

	m_clients.PushBack(&con);
	m_clientMutex.Release();
	
	ArteryWaitContext ctx;
	while(con.IsConnected())
		con.Wait(ctx);

	m_clientMutex.Wait();

	S32 i = 0;
	for (i = m_clients.Size() - 1; i >= 0; i--)
	{
		if (m_clients[i] == &con)
		{
			m_clients[i] = 0;
			m_clients.EraseByIndex(i);
		}
	}

	m_clientMutex.Release();
}

static inline U32 Fcc(char a, char b, char c, char d)
{
	return (a<<24) | (b<<16) | (c<<8) | d;
}

void C_NetworkLink::ProcessConnection(ArteryConnection conn)
{
	ArteryMessage msg;

	while (conn.IsConnected() && conn.TryReceive(msg))
	{
		HandleMessage(conn, msg);
	}

	// Send local camera position to peer:
	Maths::HVec3 pos = m_localCamera.Position();
	Maths::C_Quaternion orient = m_localCamera.Orient();
	
	ArteryMessageBuilder builder;
	builder.Write(Fcc('C','P','O','S')); // Current position
	// Write camera position:
	builder.Write((F32)pos.X());
	builder.Write((F32)pos.Y());
	builder.Write((F32)pos.Z());
	builder.Write((F32)orient.Imag().X());
	builder.Write((F32)orient.Imag().Y());
	builder.Write((F32)orient.Imag().Z());
	builder.Write((F32)orient.Real().W());
	builder.Write(m_localCamera.Fov());

	C_CamObj object = m_localCamera.Object();
	// Write object position:
	builder.Write((F32)object.position.X());
	builder.Write((F32)object.position.Y());
	builder.Write((F32)object.position.Z());
	builder.Write((F32)object.orient.Imag().X());
	builder.Write((F32)object.orient.Imag().Y());
	builder.Write((F32)object.orient.Imag().Z());
	builder.Write((F32)object.orient.Real().W());
	builder.Write(1); // Game ID
	builder.Write(0.f);

	builder.Build(msg);
	conn.Send(msg);
}

// Handle messages sent by the client
void C_NetworkLink::HandleMessage(ArteryConnection& conn, ArteryMessage& msg)
{
	ArteryMessageReader reader(msg);

	U32 id = 0;
	reader.Read(id);
	S32 len = msg.DataLength();
	U8* data = (U8 *)msg.Data();
	
	if (id == Fcc('C','S','C','E') && len > 4)
	{
		// Write camera export data to disk:
		Sys::I_FileSystem* fs = Sys::GetDefaultFileSystem();
		Sys::I_File* file = fs->Open("Cams.cse", Sys::E_FileAccess_Write);
		if (file == 0) return;

		file->Write(data + 4, len - 4);
		delete file;
		
		m_notify = E_NetNotify_Scene; // New camera scene received

		// Reply with Confirmation
		conn.Send(ArteryMessage("CONF", 4));
	}

	if (id == Fcc('C','A','N','I') && len > 4)
	{
		// Animation data:
		Sys::C_MemoryStream stream(data + 4, len - 4);

		Sys::BinaryDeserialiser<Ut::NativeToBigEndian> archive(stream);
		
		S32 fileVersion = 0, pad = 0;
		archive >> fileVersion >> pad;
		stream.SetVersionNumber(fileVersion);

		Debug::Logger& log = Debug::GetLogger("mercury.camera2");
		if (fileVersion == C_CameraScene::Version)
		{
			delete m_animData;
			m_animData = new C_AnimData;
			if (m_animData == 0)
				return;
			
			archive >> *m_animData;

			LOGINFO(log, "Camera anim '%s' (version %d/%d)", m_animData->m_name.Chars(), fileVersion, C_CameraScene::Version);
			m_notify = E_NetNotify_Animation; // New camera animation received
		}
		else
		{
			LOGWARN(log, "Camera anim incorrect version (%d/%d), so not loading", fileVersion, C_CameraScene::Version);
		}

		// Reply with Confirmation
		conn.Send(ArteryMessage("CONF", 4));
	}

	if (id == Fcc('S','P','O','S'))
	{
		F32 x=0.f,y=0.f,z=0.f,w=0.f;

		reader.Read(x);
		reader.Read(y);
		reader.Read(z);
		m_remoteCamera.SetPosition(Maths::HVec3(x,y,z));

		reader.Read(x);
		reader.Read(y);
		reader.Read(z);
		reader.Read(w);
		m_remoteCamera.SetOrient(Maths::C_Quaternion(x,y,z,w));

		reader.Read(x);
		m_remoteCamera.SetFov(x);

		m_notify = E_NetNotify_SetPos; // Camera position set

		// Reply with Confirmation
		conn.Send(ArteryMessage("CONF", 4));
	}
}

#endif // CFG_GOLD

}
