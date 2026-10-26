
#ifndef CAMERA2_CAPTURESERVICE_H
#define CAMERA2_CAPTURESERVICE_H

class ArteryConnection;
class ArteryMessage;

//namespace Display
//{
	//struct C_ConstLockedRect;
//}

namespace Cam2
{

class C_LockScreenInfo;
class C_MovieEncoder;

class C_CaptureService
{
public:
	C_CaptureService();
	~C_CaptureService();

	void Init();
	void Update();

	void RequestScreenshot();
	void SendLockedRect(const void* bits, S32 pitch, S32 width, S32 height, S32 format);
	bool TakePhoto() const;
	bool PhotoMovie() const;
	int ClientCount();

#ifndef CFG_GOLD
	// This callback is called when we should start capturing audio data:
	void (*m_AppAudioCapture)(C_CaptureService* self, bool start);
	void AudioCallback(F32 *audio, S32 channels, S32 len);
	bool Capturing() const { return m_Capturing; }

protected:
	virtual void SetFps(F32) { }
	virtual F32 GetFps() { return 0.f; }

private:
	class C_Client
	{
	public:
		ArteryConnection* m_conn;
		S32 m_recvCounter, m_respondTime;

		C_Client()
		{
			m_conn = 0;
			m_recvCounter = -1;
			m_respondTime = 0;
		}
	};

	Ut::Vector<C_Client> m_clients;
	MT::Mutex m_clientMutex;
	S32 m_counter;
	S32 m_screenCapture, m_movieCapture;
	C_MovieEncoder* m_MovieEncoder;
	F32 m_MovieFps, m_PrevFps;
	bool m_PhotoCapture, m_TakePhoto, m_PhotoMovie;
	bool m_Capturing;
	
	static void StaticNetProc(ArteryConnection con, void *self);
	void NetProc(ArteryConnection con);
	void MessagePump();
	void HandleMessage(C_Client& client, ArteryMessage& msg);
	void SendScreen();
	void SendToClients(const ArteryMessage& msg);
	void SendLockedScreen(const C_LockScreenInfo& si);
	void WaitForClients();
	void EchoRequest();
	
#endif
};

}

#endif
