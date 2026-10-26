
class ArteryConnection;
class ArteryMessage;
class C_DiskWriter;

class C_CaptureClient
{
public:

	enum E_State
	{
		E_NotConnected = 0,
		E_Connecting,
		E_Connected,
		E_Working
	};

	C_CaptureClient();
	~C_CaptureClient();

	static void NetworkInit();
	static void NetworkExit();

	int Connect(unsigned int ip);
	int Disconnect();
	E_State Status() const;
	void Update();
	C_DiskWriter* DiskWriter() { return m_diskWriter; }
	const C_DiskWriter* DiskWriter() const { return m_diskWriter; }
	void SetBitmapPixels(unsigned char* pixels) { m_bitmapPixels = pixels; }
	void SendPing();
	int ScreenCapture(bool photo);
	void MovieCaptureStart(float fps, bool photo);
	void MovieCaptureStop();
	void SetSynchronous(int sync) { m_synchronous = sync; }

	int m_imageCaptured;

protected:
	ArteryConnection* m_conn;
	E_State m_connectState; // 0 = not, 1 = connecting, 2 = connected
	int m_imageWidth, m_imageHeight;
	unsigned char* m_imageLine;
	int m_outLine;
	C_DiskWriter* m_diskWriter;
	unsigned char* m_bitmapPixels;
	int m_synchronous;

	static void StaticCallback(ArteryConnection con, void* self);
	void Callback(ArteryConnection con);
	void HandleMessage(ArteryMessage& msg);
	void SkipExisting();
};

extern C_CaptureClient g_CaptureClient;
