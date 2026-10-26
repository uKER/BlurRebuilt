
#ifndef UI_LOBBY_H
#define UI_LOBBY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

class UILobby : public C_GadgetBase
{
public:
	struct Slot
	{
		Slot()
			: id( 0 ), empty( true ), ref( 0 ), racing( false )
		{}
		U64		id;
		bool	empty;
		U8		ref;
		bool	racing;
	};

public:
	UILobby();
	~UILobby();

	DECLARE_GADGET( E_UIGameButtonType_Lobby, "LOBBY" )

	virtual void 				Init();
	virtual void				Setup( lua_State* pState );

	virtual void 				FrameUpdate();

	const Slot&					GetSlot( const S32 sIndex ) const
								{
									return m_Slots[sIndex];
								}

	const bool					ShouldRefresh() const
								{
									return m_ShouldRefresh;
								}

	const bool					TeamDataValid() const
								{
									return m_TeamDataValid;
								}

	void						SendToLua( lua_State* pState );

// For sorcery
public:
	bool						m_PartyLobby;

private:
	void						BuildTeamView();

private: 
	static const U32			m_MaxSlots = 20;
	typedef Btl::Vector< Slot, Btl::VectorParameters_Array< m_MaxSlots > >	TSlots;

	bool						m_ShouldRefresh;
	bool						m_TeamLobby;
	F32							m_TeamUpdateTimer;

	TSlots						m_Slots;
	TSlots						m_TeamSlots;
	bool						m_TeamDataValid;
};

}; // namespace UI

#endif // UI_LOBBY_H
