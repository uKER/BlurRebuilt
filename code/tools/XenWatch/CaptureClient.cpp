
#include "stdafx.h"

#include <winsock2.h>
#include <core/containers/string.h>
#include <core/threads.h>
#include <system/Artery/Artery.h>
#include <system/time/time.h>
#include "CaptureClient.h"
#include "DiskWriter.h"

C_CaptureClient g_CaptureClient;

void C_CaptureClient::NetworkInit()
{
	ArteryStartup(false, "CaptureClient");
}

void C_CaptureClient::NetworkExit()
{
	ArteryCleanup();
}

C_CaptureClient::C_CaptureClient()
{
	m_conn = new ArteryConnection();
	m_connectState = E_NotConnected;
	m_imageWidth = m_imageHeight = 0;
	m_imageLine = 0;
	m_outLine = 0;
	m_diskWriter = new C_DiskWriter;
	m_bitmapPixels = 0;
	m_imageCaptured = 0;
	m_synchronous = 1;
}

C_CaptureClient::~C_CaptureClient()
{
	delete m_diskWriter;
	m_diskWriter = 0;

	delete[] m_imageLine;
	m_imageLine = 0;
	delete m_conn;
	m_conn = 0;
}

int C_CaptureClient::Connect(unsigned int ip)
{
	if (m_connectState)
		return 0; // Already connecting/connected

	WsSockAddr target = WsSockAddr(ip, 6317);

	m_connectState = E_Connecting;
	ArteryConnect(*m_conn, target, "Screenshot", StaticCallback, this);
	
	if (m_synchronous)
	{
		S32 start = (S32)Time::GetCurrentTime();
		for (;;)
		{
			if (m_connectState == E_Connected)
				break;
			if ((S32)Time::GetCurrentTime() - start >= 1000000)
				break; // Time out
		}
		
		if (m_connectState == E_NotConnected || m_connectState == E_Connecting)
			return 1;
		MT::Thread::Sleep(1);
	}
	
	return 0;
}

int C_CaptureClient::Disconnect()
{
	m_conn->Close();

	if (m_synchronous)
	{
		S32 start = (S32)Time::GetCurrentTime();
		for (;;)
		{
			if (m_connectState == E_NotConnected)
				break;
			if ((S32)Time::GetCurrentTime() - start >= 1000000)
				break; // Time out
		}
		
		if (m_connectState != E_NotConnected)
			return 1;

		MT::Thread::Sleep(1);
	}
	
	return 0;
}

C_CaptureClient::E_State C_CaptureClient::Status() const
{
	if (m_connectState == E_Connected && DiskWriter())
		if (DiskWriter()->m_outFile)
			return E_Working; // Capturing an image

	return m_connectState;
}

void C_CaptureClient::StaticCallback(ArteryConnection con, void* self)
{
	((C_CaptureClient *)self)->Callback(con);
}

void C_CaptureClient::Callback(ArteryConnection con)
{
	m_connectState = E_Connected;

	ArteryWaitContext ctx;
	while(con.IsConnected())
		con.Wait(ctx);

	m_connectState = E_NotConnected;
}

void C_CaptureClient::Update()
{
	ArteryMessage msg;

	while (m_conn->TryReceive(msg))
	{
		HandleMessage(msg);
	}
}

// Decompress truecolour data sent from the game
static int TrueDecompress(unsigned char *src, int srcLen, unsigned char *dest, int destMax)
{
	int bit=0,bs=0,i=0,op=0,top=0,val=0;
	
	for (i=0; i<srcLen && op<destMax; i++)
	{
		int again=0;
		
		bit=(bit<<8) | src[i];
		bs+=8;
		
		// Output bits:
		do
		{
			if (op>=destMax) break; // Limit to length of destination

			again=0;
			top=(bit>>(bs-1))&1;

			if (top==0 && bs>=3)
			{
				val=bit>>(bs-3);
				val=((val+2)&3)-2;
				if (op>=3) val+=dest[op-3];
				
				dest[op++] = (unsigned char)val;
				bs-=3; again=1;
			}
			else if (top==1 && bs>=9)
			{
				val=bit>>(bs-9);
				if (op>=3) val+=dest[op-3];
				dest[op++] = (unsigned char)val;
				bs-=9; again=1;
			}
		}
		while (again);
	}
	
	return op;
}

void C_CaptureClient::HandleMessage(ArteryMessage& msg)
{
	int len = msg.DataLength();
	unsigned char* data = (unsigned char*)msg.Data();
	
	if (len < 1)
		return;

	if (data[0] == 0x42 && len >= 8) // (B) Bitmap start
	{
		m_imageWidth  = (data[2] << 8) | data[3];
		m_imageHeight = (data[6] << 8) | data[7];
		
		if (m_imageLine)
			delete[] m_imageLine;
			
		m_imageLine = new unsigned char[m_imageWidth * 3];
		if (m_imageLine == 0)
			return;

		memset(m_imageLine, 0, m_imageWidth * 3);

		m_outLine = 0;
		DiskWriter()->BeginImage(m_imageWidth, m_imageHeight);
	}

	if (data[0] == 0x43 && len >= 2 && m_imageLine) // (C) Bitmap line
	{
		TrueDecompress(data + 2, len - 2, m_imageLine, m_imageWidth * 3);

		if (m_bitmapPixels && m_outLine < 720)
		{
			int width = m_imageWidth;
			if (width > 1280)
				width = 1280;

			// Output to bitmap:
			memcpy(m_bitmapPixels + m_outLine * 1280 * 3, m_imageLine, width * 3);
		}
		
		DiskWriter()->WriteLine(m_imageLine, m_imageWidth * 3);
		
		if (data[1] == 1)
		{
			DiskWriter()->EndImage();
			m_imageCaptured = 1;
		}
		
		m_outLine++;
	}
	
	if (data[0] == 0x44 && len >= 2) // (D) Echo request
	{
		unsigned char reply[2] = {0x45, 0};
		reply[1] = data[1];
		
		m_conn->Send(ArteryMessage(reply, sizeof(reply)));
	}
}

void C_CaptureClient::SendPing()
{
	ArteryMessage msg("P", 1); // Send ping
	
	m_conn->Send(msg);	
}

// Request for screen capture:
int C_CaptureClient::ScreenCapture(bool photo)
{
	m_imageCaptured = 0;

	unsigned char data[2] = {0x46};
	data[1] = photo ? 1 : 0;
	
	m_conn->Send(ArteryMessage(data, sizeof(data)));

	if (m_synchronous)
	{
		S32 start = (S32)Time::GetCurrentTime();
		for (;;)
		{
			Update();
			
			if (m_imageCaptured)
				break;
			if ((S32)Time::GetCurrentTime() - start >= 5000000)
				return 1; // Time out
			
			MT::Thread::Sleep(1);
		}
	}
		
	return 0;
}

static void PutInt(unsigned char* data, int v)
{
	data[0] = (v >> 24) & 0xff;
	data[1] = (v >> 16) & 0xff;
	data[2] = (v >>  8) & 0xff;
	data[3] =  v        & 0xff;
}

static void PutFloat(unsigned char* data, F32 v)
{
	PutInt(data, *(int *)&v );
}

// Request for movie capture:
void C_CaptureClient::MovieCaptureStart(F32 fps, bool photo)
{
	unsigned char data[7] = {0x47, 1};  // G
	
	PutFloat(data + 2, fps);
	data[6] = photo ? 1 : 0;

	m_conn->Send(ArteryMessage(data, sizeof(data)));
}

// Request for movie capture:
void C_CaptureClient::MovieCaptureStop()
{
	unsigned char data[2] = {0x47, 0};  // G
	
	m_conn->Send(ArteryMessage(data, sizeof(data)));
}
