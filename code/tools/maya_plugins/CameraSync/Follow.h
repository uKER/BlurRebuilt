
class C_CameraFollow
{
public:
	C_CameraFollow();
	void Start();
	void Stop();

private:
	HANDLE m_quitEvent, m_doneEvent;

	static DWORD WINAPI StaticThreadCode(LPVOID self);
	void ThreadCode();
	void UpdateMayaCamera();
};

extern C_CameraFollow g_CameraFollow;
