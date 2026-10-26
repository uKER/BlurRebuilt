
#ifndef CAMERA2_NETWORKLINK_H
#define CAMERA2_NETWORKLINK_H

#include <camera2/Camera.h>

class ArteryConnection;
class ArteryMessage;

namespace Cam2
{
class C_AnimData;

enum E_NetNotify
{
	E_NetNotify_None = 0,
	E_NetNotify_Scene = 1,
	E_NetNotify_Animation = 2,
	E_NetNotify_SetPos = 3,
};

class C_NetworkLink
{
public:
	static C_NetworkLink& Instance();
	void Init();
	void Exit();
	void Update();

	void SetLocalCamera(const C_Camera& cam) { m_localCamera = cam; }
	const C_Camera& RemoteCamera() const { return m_remoteCamera; }
	const C_AnimData* AnimData() const { return m_animData; }
	E_NetNotify Notify() const { return m_notify; }

private:
	C_NetworkLink();
	C_NetworkLink(const C_NetworkLink&);
	~C_NetworkLink();

	C_Camera m_localCamera, m_remoteCamera;
	C_AnimData* m_animData; // Animation data sent from Maya
	E_NetNotify m_notify;

#ifndef CFG_GOLD
	Ut::Vector<ArteryConnection *> m_clients;
	MT::Mutex m_clientMutex;

	static void StaticNetProc(ArteryConnection con, void *self);
	void NetProc(ArteryConnection con);
	void ProcessConnection(ArteryConnection conn);
	void HandleMessage(ArteryConnection& conn, ArteryMessage& msg);
	
	volatile U8 m_updateCount;
#endif
};

}

#endif
