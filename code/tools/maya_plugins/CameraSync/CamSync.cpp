
#include "stdafx.h"
#include <winsock2.h>
#include <system/Artery/Artery.h>

#include "CamSync.h"
#include "Main.h"

C_CamSync g_CamSync;

void C_CamSync::NetworkInit()
{
	ArteryStartup(false, "CameraSync");
}

void C_CamSync::NetworkExit()
{
 	ArteryCleanup();
}

C_CamSync::C_CamSync()
{
	m_conn = new ArteryConnection();
	m_connectState = E_NotConnected;
	m_positionsReceived = m_confirms = 0;
}

C_CamSync::~C_CamSync()
{
	if (m_conn)
		m_conn->Close();

	delete m_conn;
	m_conn = 0;
}

S32 C_CamSync::Connect(U32 ip)
{
	if (m_connectState != E_NotConnected)
		return 0; // Already connecting/connected

	WsSockAddr target = WsSockAddr(ip, 6317);
	m_connectState = E_Connecting;
	m_positionsReceived = 0;
	ArteryConnect(*m_conn, target, "Camera", StaticCallback, this);
	
	WaitForConnect();
	return 0;
}

void C_CamSync::Disconnect()
{
	if (m_conn)
		m_conn->Close();
}

void C_CamSync::StaticCallback(ArteryConnection con, void* self)
{
	((C_CamSync *)self)->Callback(con);
}

void C_CamSync::Callback(ArteryConnection con)
{
	m_connectState = E_Connected;

	ArteryWaitContext ctx;
	while(con.IsConnected())
		con.Wait(ctx);

	m_connectState = E_NotConnected;
}

void C_CamSync::Update()
{
	if (m_conn == 0)
		return;

	ArteryMessage msg;
	while (m_conn->IsConnected() && m_conn->TryReceive(msg))
	{
		HandleMessage(msg);
	}
}

static inline U32 Fcc(char a, char b, char c, char d)
{
	return (a<<24) | (b<<16) | (c<<8) | d;
}

// Handle messages sent by the server
void C_CamSync::HandleMessage(ArteryMessage& msg)
{
	S32 len = msg.DataLength();
	if (len < 4)
		return;
	
	ArteryMessageReader reader(msg);
	U32 id = 0;
	reader.Read(id);

	if (id == Fcc('C','P','O','S'))
	{
		F32 x = 0.f, y = 0.f, z = 0.f, w = 0.f;
		
		// Current Position/Orientation
		reader.Read(x); reader.Read(y); reader.Read(z);
		m_gameCam.SetPosition(Maths::HVec3(x,y,z));

		reader.Read(x); reader.Read(y); reader.Read(z); reader.Read(w);
		m_gameCam.SetOrient(Maths::C_Quaternion(x,y,z,w));
		
		F32 fov = 0.f;
		reader.Read(fov);
		m_gameCam.SetFov(fov);

		m_positionsReceived++;
	}

	if (id == Fcc('C','O','N','F'))
		m_confirms++;
}

void C_CamSync::WaitForConnect()
{
	S32 start = (S32)Time::GetCurrentTime();
	
	for (;;)
	{
		if (m_connectState == E_Connected)
			break;
		if ((S32)Time::GetCurrentTime() - start >= 3000000)
			break; // Time out
		MT::Thread::Sleep(1);
	}
}

void C_CamSync::WaitForPosition()
{
	S32 start = (S32)Time::GetCurrentTime();
	S32 receivedBefore = PositionsReceived();
	
	for (;;)
	{
		Update();
		S32 rel = PositionsReceived() - receivedBefore;
		if (rel > 0)
			break;

		if ((S32)Time::GetCurrentTime() - start >= 3000000)
			break; // Time out
		MT::Thread::Sleep(1);
	}
}

void C_CamSync::WaitForConfirm()
{
	S32 start = (S32)Time::GetCurrentTime();
	S32 initial = m_confirms;
	
	for (;;)
	{
		Update();
		if ((m_confirms - initial) > 0)
			break;

		if ((S32)Time::GetCurrentTime() - start >= 3000000)
			break; // Time out
		MT::Thread::Sleep(1);
	}
}

void C_CamSync::SendPosition(const Cam2::C_Camera& camera)
{
	ArteryMessageBuilder builder;
	builder.Write(Fcc('S','P','O','S')); // Set position
	// Write camera position:
	builder.Write((F32)camera.Position().X());
	builder.Write((F32)camera.Position().Y());
	builder.Write((F32)camera.Position().Z());
	builder.Write((F32)camera.Orient().Imag().X());
	builder.Write((F32)camera.Orient().Imag().Y());
	builder.Write((F32)camera.Orient().Imag().Z());
	builder.Write((F32)camera.Orient().Real().W());
	builder.Write(camera.Fov());

	ArteryMessage msg;
	builder.Build(msg);

	if (m_conn)
		m_conn->Send(msg);
}
