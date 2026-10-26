#include "stdafx.h"
#include <system/Artery/Artery.h>

#include <EditorApp/Editor/CameraConnection.h>
#include <EditorApp/Editor/CameraController.h>


namespace Editor
{

void C_CameraConnection::NetworkInit()
{
	ArteryStartup(false, "CameraSync");
}

void C_CameraConnection::NetworkExit()
{
 	ArteryCleanup();
}

C_CameraConnection::C_CameraConnection() 
: m_connectionState(E_ConnectionState_Unconnected)
, m_pController(0)
{
	m_pConnection = new ArteryConnection();
	m_Timer.Connect(m_Timer.GetId(), wxEVT_TIMER, wxTimerEventHandler(C_CameraConnection::OnTimer), NULL, this);
}

C_CameraConnection::~C_CameraConnection()
{
	Disconnect();
	delete m_pConnection;
}

C_CameraConnection::E_ConnectionState C_CameraConnection::GetConnectionState() const
{
	return m_connectionState;
}

S32 C_CameraConnection::Connect(U32 ip)
{
	if (m_connectionState != E_ConnectionState_Unconnected)
		return 0; // Already connecting/connected

	WsSockAddr target = WsSockAddr(ip, 6317);
	m_connectionState = E_ConnectionState_Connecting;
	ArteryConnect(*m_pConnection, target, "Camera", StaticCallback, this);
	
	m_Timer.Start(40);

	return 0;
}

void C_CameraConnection::Disconnect()
{
	m_Timer.Stop();

	if (m_pConnection)
		m_pConnection->Close();
}

void C_CameraConnection::OnTimer(wxTimerEvent& e)
{
	if (m_pConnection == 0 || !m_pConnection->IsConnected() || !m_pController)
		return;

	CameraMessage cameraMessage;
	m_pController->Update();
	if (m_pController->Send(&cameraMessage))
	{
		ArteryMessageBuilder msgBuilder;
		ArteryMessage msg;

		msgBuilder.Write('SPOS');
		msgBuilder.Write(cameraMessage.x);
		msgBuilder.Write(cameraMessage.y);
		msgBuilder.Write(cameraMessage.z);
		msgBuilder.Write(cameraMessage.a);
		msgBuilder.Write(cameraMessage.b);
		msgBuilder.Write(cameraMessage.c);
		msgBuilder.Write(cameraMessage.d);
		msgBuilder.Write(cameraMessage.fov);
		msgBuilder.Build(msg);

		m_pConnection->Send(msg);
	}
	else
	{
		ArteryMessage msg;
		while (m_pConnection->IsConnected() && m_pConnection->TryReceive(msg))
		{
			//U8* data = (U8 *)msg.Data();
			S32 len = msg.DataLength();
			
			if (len < 4)
				return;
			
			ArteryMessageReader reader(msg);
			U32 id = 0;
			reader.Read(id);

			if (id=='CPOS')
			{					
				reader.Read(cameraMessage.x); reader.Read(cameraMessage.y); reader.Read(cameraMessage.z);
				reader.Read(cameraMessage.a); reader.Read(cameraMessage.b); reader.Read(cameraMessage.c); reader.Read(cameraMessage.d);
				reader.Read(cameraMessage.fov);

				m_pController->Recieve(&cameraMessage);
			}
		}
	}
}

I_CameraController* C_CameraConnection::GetController() const
{
	return m_pController;
}

void C_CameraConnection::SetController(I_CameraController* pController)
{
	m_pController = pController;
}

bool C_CameraConnection::Recieve(CameraMessage& msg, U32 timeoutms)
{
	if (m_pConnection == 0 || !m_pConnection->IsConnected())
		return false;

	ArteryMessage aMsg;
	if (m_pConnection->Receive(aMsg, timeoutms))
	{
		S32 len = aMsg.DataLength();
		
		if (len < 4)
			return false;
		
		ArteryMessageReader reader(aMsg);
		U32 id = 0;
		reader.Read(id);

		if (id=='CPOS')
		{					
			reader.Read(msg.x); reader.Read(msg.y); reader.Read(msg.z);
			reader.Read(msg.a); reader.Read(msg.b); reader.Read(msg.c); reader.Read(msg.d);
			reader.Read(msg.fov);
			return true;
		}
	}
	return false;
}

bool C_CameraConnection::Send(const CameraMessage& msg)
{
	if (m_pConnection == 0 || !m_pConnection->IsConnected())
		return false;

	ArteryMessageBuilder msgBuilder;
	ArteryMessage aMsg;

	msgBuilder.Write('SPOS');
	msgBuilder.Write(msg.x);
	msgBuilder.Write(msg.y);
	msgBuilder.Write(msg.z);
	msgBuilder.Write(msg.a);
	msgBuilder.Write(msg.b);
	msgBuilder.Write(msg.c);
	msgBuilder.Write(msg.d);
	msgBuilder.Write(msg.fov);
	msgBuilder.Build(aMsg);

	m_pConnection->Send(aMsg);

	return true;
}


void C_CameraConnection::StaticCallback(ArteryConnection con, void* self)
{
	((C_CameraConnection*)self)->Callback(con);
}

void C_CameraConnection::Callback(ArteryConnection con)
{
	m_connectionState = E_ConnectionState_Connected;

	ArteryWaitContext ctx;
	while(con.IsConnected())
		con.Wait(ctx);

	m_connectionState = E_ConnectionState_Unconnected;
}


}