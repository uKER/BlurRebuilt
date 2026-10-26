#ifndef EDITOR_GAME_MESSAGING_H
#define EDITOR_GAME_MESSAGING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

class ArteryConnection;
class ArteryMessage;

namespace Editor
{
	class C_GameMessaging
	{
	public:
		enum E_ConnectionState
		{
			E_ConnectionState_Disconnecting,
			E_ConnectionState_Unconnected,
			E_ConnectionState_Connecting,
			E_ConnectionState_Connected
		};

		static void NetworkInit();
		static void NetworkExit();

		C_GameMessaging();
		~C_GameMessaging();

		E_ConnectionState GetConnectionState() const;

		S32 Connect(U32 ip);
		void Disconnect();
		
	private:

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_GameMessaging* pGameMessaging);
			void Connect();
			void Disconnect();
			
			void OnDgNodeTransform(const EditorLib::I_DgNode& node, EditorLib::E_DgNodeTransform e);
			void OnCoreSelectionChanged();

			FRIEND_CLASS C_GameMessaging;

		private:
			C_GameMessaging* m_pGameMessaging;
		};

		void OnDgNodeTransform(const EditorLib::I_DgNode& node, EditorLib::E_DgNodeTransform e);
		void OnCoreSelectionChanged();

		static void StaticCallback(ArteryConnection con, void* self);
		void Callback(ArteryConnection con);

		C_Slots						m_slots;

		ArteryConnection*			m_pConnection;
		E_ConnectionState			m_connectionState;
		EditorLib::C_Selection*		m_pLastSelection;

		U32							m_Ip;
	};
}

#endif