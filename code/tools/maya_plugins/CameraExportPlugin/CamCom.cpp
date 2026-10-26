
#include "stdafx.h"
#include "CamCom.h"

C_CamCom g_CamCom;

void C_CamCom::NetworkInit()
{
	ArteryStartup(false, "CameraExportPlugin");
}

void C_CamCom::NetworkExit()
{
 	ArteryCleanup();
}

C_CamCom::C_CamCom()
{
	m_conn = new ArteryConnection();
	m_connectState = E_NotConnected;
	m_confirms = 0;
}

C_CamCom::~C_CamCom()
{
	if (m_conn)
		m_conn->Close();

	delete m_conn;
	m_conn = 0;
}

S32 C_CamCom::Connect(U32 ip)
{
	if (m_connectState != E_NotConnected)
		return 0; // Already connecting/connected

	dprintf("Connecting to %d.%d.%d.%d\n", (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);

	WsSockAddr target = WsSockAddr(ip, 6317);
	m_connectState = E_Connecting;
	m_confirms = 0;
	ArteryConnect(*m_conn, target, "Camera", StaticCallback, this);

	S32 start = (S32)Time::GetCurrentTime();
	for (;;)
	{
		if (m_connectState == E_Connected)
			break;
		if ((S32)Time::GetCurrentTime() - start >= 4000000)
			break; // Time out
		MT::Thread::Sleep(1);
	}
	
	if (m_connectState == E_NotConnected || m_connectState == E_Connecting)
		return 1;

	dprintf("Connected\n");

	return 0;
}

S32 C_CamCom::Disconnect()
{
	m_conn->Close();

	S32 start = (S32)Time::GetCurrentTime();
	for (;;)
	{
		if (m_connectState == E_NotConnected)
			break;
		if ((S32)Time::GetCurrentTime() - start >= 1000000)
			break; // Time out
		MT::Thread::Sleep(1);
	}
	
	if (m_connectState != E_NotConnected)
		return 1;

	dprintf("Disconnected\n");
	
	return 0;
}

void C_CamCom::StaticCallback(ArteryConnection con, void* self)
{
	((C_CamCom *)self)->Callback(con);
}

void C_CamCom::Callback(ArteryConnection con)
{
	m_connectState = E_Connected;

	ArteryWaitContext ctx;
	while(con.IsConnected())
		con.Wait(ctx);

	m_connectState = E_NotConnected;
}

S32 C_CamCom::Send(void *data, S32 len)
{
	if (m_conn == 0)
		return 1;

	ArteryMessage msg(data, len);
	
	m_conn->Send(msg);
	return 0;
}

void C_CamCom::Update()
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
void C_CamCom::HandleMessage(ArteryMessage& msg)
{
	U8* data = (U8 *)msg.Data();
	S32 len = msg.DataLength();
	
	if (len < 4)
		return;
	
	ArteryMessageReader reader(msg);
	U32 id = 0;
	reader.Read(id);

	if (id == Fcc('C','O','N','F'))
		m_confirms++;
}
