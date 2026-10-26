#ifndef EDITOR_CAMERA_SYNC_H
#define EDITOR_CAMERA_SYNC_H

#ifdef _MSC_VER
#pragma once
#endif

class ArteryConnection;
class ArteryMessage;

//relative float comparison
inline bool AreRelativelyEqual(F32 x, F32 y, S32 maxUlps)
{
	S32 x1=*(S32*)&x; S32 y1=*(S32*)&y;
	if (x1<0) x1 = 0x80000000-x1;
	if (y1<0) y1 = 0x80000000-y1;

	return abs(x1-y1) < maxUlps;
}

#define MSG_EQUALITY_CHECK(msg1, msg2, e1, e2)		\
	(AreRelativelyEqual((msg1).x, (msg2).x, e1) &&	\
	 AreRelativelyEqual((msg1).y, (msg2).y, e1) &&	\
	 AreRelativelyEqual((msg1).z, (msg2).z, e1) &&	\
	 AreRelativelyEqual((msg1).a, (msg2).a, e2) &&	\
	 AreRelativelyEqual((msg1).b, (msg2).b, e2) &&	\
	 AreRelativelyEqual((msg1).c, (msg2).c, e2) &&	\
	 AreRelativelyEqual((msg1).d, (msg2).d, e2) &&	\
	 AreRelativelyEqual((msg1).fov, (msg2).fov, e1)	\
	 )

namespace Editor
{
	class I_CameraController;

	struct CameraMessage
	{
		F32 x, y, z;		//translation
		F32 a, b, c, d;		//quaternion orientation
		F32 fov;
	};

	class C_CameraConnection : public wxEvtHandler
	{
	public:
		enum E_ConnectionState
		{
			E_ConnectionState_Unconnected,
			E_ConnectionState_Connecting,
			E_ConnectionState_Connected
		};

		static void NetworkInit();
		static void NetworkExit();

		C_CameraConnection();
		~C_CameraConnection();

		E_ConnectionState GetConnectionState() const;

		S32 Connect(U32 ip);
		void Disconnect();
		void OnTimer(wxTimerEvent& e);
		
		I_CameraController* GetController() const;
		void SetController (I_CameraController* pViewport);

		bool Recieve(CameraMessage& msg, U32 timeoutms=500);
		bool Send(const CameraMessage& msg);

	private:

		static void StaticCallback(ArteryConnection con, void* self);
		void Callback(ArteryConnection con);

		ArteryConnection	*m_pConnection;
		I_CameraController	*m_pController;
		E_ConnectionState	m_connectionState;
		wxTimer				m_Timer;
	};
}

#endif