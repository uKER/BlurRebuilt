
#include "stdafx.h"
#include <system/time/time.h>

#pragma warning (push, 3)
#ifdef PLATFORM_PC
#include <Winsock2.h>
#endif
#include "System/Artery/Artery.h"
#include <core/threads.h>
#pragma warning (pop)

#include <camera2/capture/CaptureService.h>
#include <camera2/capture/LockScreen.h>
#include <camera2/capture/MovieEncoder.h>
#ifdef PLATFORM_PC
#include <d3d9.h>
#endif
#include <Display/Core/Includes.h>
#include <Display/Texture/TextureParams.h>

namespace Cam2
{

C_CaptureService::~C_CaptureService()
{
#ifndef CFG_GOLD
	SAFE_DELETE_PTR(m_MovieEncoder);
#endif
}

C_CaptureService::C_CaptureService()
{
#ifndef CFG_GOLD
	m_counter = 0;
	m_screenCapture = 0;
	m_movieCapture = 0;
	m_MovieEncoder = 0;
	m_MovieFps = m_PrevFps = 0.f;
	m_PhotoCapture = false;
	m_TakePhoto = false;
	m_AppAudioCapture = 0;
	m_Capturing = false;
#endif
}

#pragma warning (disable: 4390) // ';' : empty controlled statement found;

void C_CaptureService::Init()
{
#ifndef CFG_GOLD
	bool ret = ArteryRegister("Screenshot", StaticNetProc, this);
	if (!ret)
		assert(!"ArteryRegister failed");

	m_MovieEncoder = new C_MovieEncoder;
	if (m_MovieEncoder == 0)
		return;
#endif
}

void C_CaptureService::Update()
{
#ifndef CFG_GOLD
	MessagePump();

	if (!m_PhotoCapture && (m_screenCapture || m_movieCapture))
		SendScreen(); // Take the screenshot ourselves
	
	// Signal the app to take the photo mode shot:
	m_TakePhoto = m_PhotoCapture && m_screenCapture;
	m_PhotoMovie = m_PhotoCapture && m_movieCapture;

	// Figure out if we are capturing:
	m_Capturing = m_screenCapture || m_movieCapture;
	
	// Signal the app to start or stop capturing audio data:
	if (m_AppAudioCapture)
		m_AppAudioCapture(this, m_Capturing);
	
	m_screenCapture = 0;
#endif
}

void C_CaptureService::RequestScreenshot()
{
#ifndef CFG_GOLD
	m_screenCapture = true;
#endif
}

bool C_CaptureService::TakePhoto() const
{
#ifndef CFG_GOLD
	return m_TakePhoto;
#else
	return false;
#endif
}

bool C_CaptureService::PhotoMovie() const
{
#ifndef CFG_GOLD
	return m_PhotoMovie;
#else
	return false;
#endif
}

int C_CaptureService::ClientCount()
{
	int count = 0;

#ifndef CFG_GOLD
	if (m_clientMutex.Wait(1000))
	{
		count = m_clients.Size();
		m_clientMutex.Release();
	}
#endif

	return count;
}

#ifndef CFG_GOLD
void C_CaptureService::AudioCallback(F32 *audio, S32 channels, S32 len)
{
	S32 samples = channels * len;
	S32 totalLen = 8 + (samples << 1);
	ArteryMessage msg(totalLen);
	
	unsigned char *data = (unsigned char *)msg.Data();
	if (data == 0)
		return;

	memset(data, 0, totalLen); // Initalise to zero
	// 0x41 (A) 0 channels 0  2-byte length 0 0
	data[0] = 0x41;
	data[2] = (unsigned char)channels;
	data[4] = (unsigned char)(len >> 8);
	data[5] = (unsigned char) len;

	int i = 0;
	do
	{
		// Get clamped audio sample:
		F32 s = Maths::Clamped(audio[i] * 0.5f, -1.f, 1.f);

		// Send as 16-bit big-endian:
		int val = (int)(s * 32767.f);
		data[(i << 1) + 8] = (unsigned char)(val >> 8);
		data[(i << 1) + 9] = (unsigned char)val;
		
		i++;
	}
	while (i < samples);

	// Send to XenWatch:
	SendToClients(msg);
}

void C_CaptureService::StaticNetProc(ArteryConnection con, void *self)
{
	((C_CaptureService *)self)->NetProc(con);
}

void C_CaptureService::NetProc(ArteryConnection con)
{
	if (!m_clientMutex.Wait(1000))
		return;

	C_Client client;
	client.m_conn = &con;
	m_clients.PushBack(client);
	m_clientMutex.Release();

	m_movieCapture = 0;
	
	ArteryWaitContext ctx;

	while(con.IsConnected())
	{
		con.Wait(ctx);
	}

	m_movieCapture = 0;

	if (!m_clientMutex.Wait(1000))
		return;

	S32 i = 0;
	for (i = m_clients.Size() - 1; i >= 0; i--)
	{
		if (m_clients[i].m_conn == &con)
		{
			m_clients.EraseByIndex(i);
		}
	}

	m_clientMutex.Release();
}

void C_CaptureService::MessagePump()
{
	if (m_clientMutex.Wait(1000))
	{
		S32 i = 0;
		// Go through the list of clients:
		for (i = 0; i < m_clients.Size(); i++)
		{
			ArteryConnection* conn = m_clients[i].m_conn;

			ArteryMessage msg;
			while (conn->IsConnected() && conn->TryReceive(msg))
			{
				HandleMessage(m_clients[i], msg);
			}
		}
		
		m_clientMutex.Release();
	}
}

static S32 GetInt(U8* data)
{
	return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

static F32 GetFloat(U8* data)
{
	S32 val = GetInt(data);
	return *(F32 *)&val;
}

// Handle messages sent by the client
void C_CaptureService::HandleMessage(C_Client& client, ArteryMessage& msg)
{
	ArteryMessageReader reader(msg);

	U8* data = (U8 *)msg.Data();
	S32 len = msg.DataLength();
	if (len < 1)
		return;

	if (data[0] == 0x45 && len >= 2)
	{
		// Echo response
		client.m_recvCounter = data[1];
		// Client is up to date
		client.m_respondTime = (S32)Time::GetCurrentTime();
	}

	if (data[0] == 0x46)
	{
		// Screen capture request
		m_screenCapture = 1;

		bool photo = false;
		if (len >= 2)
			photo = data[1] != 0;
		m_PhotoCapture = photo;
	}

	if (data[0] == 0x47 && len >= 2)
	{
		// Movie capture request
		m_movieCapture = data[1];
		
		if (m_movieCapture)
		{
			// Start movie capture:
			m_MovieFps = 0.f;
			if (len >= 6)
				m_MovieFps = GetFloat(data + 2);

			bool photo = false;
			if (len >= 7)
				photo = data[6] != 0;

			m_PhotoCapture = photo;

			m_PrevFps = GetFps();
			
			// If the FPS is currently 0.0, then the user hasn't been using the debug sliders,
			// so we can be helpful and set the FPS for them from XenWatch:
			if (m_MovieFps && m_PrevFps == 0.0f)
				SetFps(m_MovieFps);
		}
		else
		{
			// Stop movie capture - change the FPS back (if the user hasn't changed it)
			if (m_PrevFps == 0.0f && GetFps() == m_MovieFps)
				SetFps(m_PrevFps);
		}
	}
}

// Grab a screenshot and send it to XenWatch:
void C_CaptureService::SendScreen()
{
	C_LockScreenInfo si;
	S32 ret = LockScreen(si);

	if (ret != 0)
		return;

	SendLockedScreen(si);
	UnlockScreen();
}

void C_CaptureService::SendToClients(const ArteryMessage& msg)
{
	if (m_clientMutex.Wait(1000))
	{
		S32 i = 0;
		// Go through the list of clients:
		for (i = 0; i < m_clients.Size(); i++)
		{
			m_clients[i].m_conn->Send(msg);
		}
		
	 	m_clientMutex.Release();
	}
}

static S32 TrueCompress(U8 *src, U8 *dest, S32 srcLen)
{
	S32 i = 0, bit = 0, bs = 0, op = 0;
	
	do
	{
		S32 d =src[i];

		if (i>=3) d-=src[i-3];
	
		// See if it's between -2 and +1
		if (((d+2)&0xfc)==0)
		{
			bit <<= 3;
			bit |= d&3; // 0 nnn
			bs += 3;
		}
		else
		{
			bit <<= 9;
			bit |= 0x100 | (d&0xff); // 1 nnnnnnnn
			bs += 9;
		}

		// Flush bits if we have a byte of them:
		while (bs >= 8)
		{
			bs -= 8;
			dest[op++] = (U8)(bit >> bs);
		}
		
		i++;
	}
	while (i < srcLen);

	if (bs > 0)
	{
		// Pad up to 8 bits and flush if there are any left
		bit <<= 8 - bs;
		dest[op++] = (U8)bit;
	}

	return op;
}


static inline S32 CalcAddr(S32 x, S32 y, S32 pitch)
{
	return y * pitch + (x << 2);
}

void C_CaptureService::SendLockedScreen(const C_LockScreenInfo& si)
{
	if (!si.bits)
		return;

	EchoRequest();

	// Normal screen/movie capture
	S32 rowlen = 2 + si.width * 3;
	if (rowlen<16) rowlen=16;

	S32 complen = rowlen << 1;

	U8 *row = new U8[rowlen + complen];
	
	if (row == 0)
		return;
	
	memset(row, 0, rowlen + complen);
	U8 *comp = row + rowlen;

	// Start image:
	row[0] = 0x42;
	row[2] = (U8)(si.width >> 8);
	row[3] = (U8) si.width;
	row[6] = (U8)(si.height >> 8);
	row[7] = (U8) si.height;

	ArteryMessage msg;
	msg.Set(row, 16);
	
	SendToClients(msg);
	comp[0] = 0x43; // Compressed scanline

	// Output all the lines of the texture:
	S32 y = 0;
	for (y = 0; y < si.height; y++)
	{
		if (y >= si.height - 1)
		{
			// 1 if last line
			comp[1] = 1;
		}
		
		U8 *pd = row + 2;
		U8 *source = (U8 *)si.bits;

		S32 x = 0;
		U8 *ps = 0;
		switch (si.format)
		{
			default:
				// Default: Big Endian 8,8,8
				for (x = 0; x < si.width; x++, pd+=3)
				{
					ps = source + CalcAddr(x, y, si.pitch);
					pd[2] = ps[1]; // Red
					pd[1] = ps[2]; // Green
					pd[0] = ps[3]; // Blue
				}
			break;

#ifdef PLATFORM_PC
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
				// Little Endian 8,8,8
				for (x = 0; x < si.width; x++, pd+=3)
				{
					ps = source + CalcAddr(x, y, si.pitch);
					S32 v = *(S32 *)ps;
					pd[0] = (U8) v; // Blue
					pd[1] = (U8)(v >> 8); // Green
					pd[2] = (U8)(v >> 16); // Red
				}
			break;
#endif
		}

		S32 ret = TrueCompress(row + 2, comp + 2, si.width * 3);
		msg.Set(comp, ret + 2);

		if ((y & 63) == 0)
			WaitForClients();

		SendToClients(msg);
		
		if ((y & 63) == 0)
			EchoRequest();
	}
	
	delete[] row;
}

void C_CaptureService::WaitForClients()
{
	for (;;)
	{
		MessagePump();

		S32 waiting = 0;

		if (m_clientMutex.Wait(1000))
		{
			S32 i = 0;
			// Go through the list of clients:
			for (i = 0; i < m_clients.Size(); i++)
			{
				C_Client& c = m_clients[i];
				
				if (c.m_recvCounter != m_counter)
					waiting = 1;
			}

			m_clientMutex.Release();
		}

		if (waiting == 0)
			break;

		MT::Thread::Sleep(1);
	}

	m_counter++;
	m_counter &= 0xff;
}

void C_CaptureService::EchoRequest()
{
	// Send echo request:
	U8 data[2] = {0x44, 0};
	data[1] = (U8)m_counter;
	
	ArteryMessage msg;
	msg.Set(data, sizeof(data));
	SendToClients(msg);
}

void C_CaptureService::SendLockedRect(const void* bits, S32 pitch, S32 width, S32 height, S32 format)
{
	C_LockScreenInfo si;

	si.bits = bits;
	si.pitch = pitch;
	si.width = width;
	si.height = height;
	si.format = format;
	
	SendLockedScreen(si);
}

#endif // CFG_GOLD

} // End of namespace

