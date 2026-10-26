

class ArteryConnection;

class C_CamCom
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

	C_CamCom();
	~C_CamCom();
	S32 Connect(U32 ip);
	S32 Disconnect();
	S32 Send(void *data, S32 len);
	void Update();
	S32 Confirms() { return m_confirms; }

private:
	ArteryConnection* m_conn;
	E_State m_connectState;
	S32 m_confirms;

	static void StaticCallback(ArteryConnection con, void* self);
	void Callback(ArteryConnection con);
	void HandleMessage(ArteryMessage& msg);
};

extern C_CamCom g_CamCom;
