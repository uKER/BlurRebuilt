
#include <camera2/Camera.h>

class ArteryConnection;
class ArteryMessage;

class C_CamSync
{
public:
	enum E_State
	{
		E_NotConnected = 0,
		E_Connecting,
		E_Connected,
	};

	static void NetworkInit();
	static void NetworkExit();

	C_CamSync();
	~C_CamSync();
	S32 Connect(U32 ip);
	void Disconnect();
	void Update();
	S32 PositionsReceived() { return m_positionsReceived; }
	const Cam2::C_Camera& GameCam() { return m_gameCam; }
	void WaitForPosition();
	void SendPosition(const Cam2::C_Camera& camera);
	void WaitForConfirm();

private:
	ArteryConnection* m_conn;
	volatile E_State m_connectState;
	S32 m_positionsReceived, m_confirms;
	Cam2::C_Camera m_gameCam;
	
	static void StaticCallback(ArteryConnection con, void* self);
	void Callback(ArteryConnection con);
	void HandleMessage(ArteryMessage& msg);
	void WaitForConnect();
};

extern C_CamSync g_CamSync;
