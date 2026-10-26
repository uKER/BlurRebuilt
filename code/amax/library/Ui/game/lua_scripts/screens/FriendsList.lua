FL_GUI =
{
	update_info_lines = false,
	
	gamer_cards = false,
	revoke_request = false,
	accept_request = false,
	can_invite = false,
	can_join_in_progress = false,
	can_select = false,
}

function FL_Init()
	UIGlobals.ViewingFriendsList = true
	
	AddSCUI_Elements( SCUI_FriendsList )
	
	InfoLineLock()
	StoreInfoLine_F()
	SetupInfoLine_F()
end

function FL_PostInit()
	--
end

function FL_StartLoop( time )
	--
end

function FL_MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
end

function FL_FrameUpdate( time )
	if FL_GUI.update_info_lines == true then
		FL_GUI.update_info_lines = false
		
		local options = {  }
	
		if FL_GUI.gamer_cards == true then
			options[ #options + 1 ] = UIText.PRO_FRIENDS_INF_VIEW_GAMERCARD_A
		end
		if FL_GUI.revoke_request == true then
			options[ #options + 1 ] = UIText.PRO_FRIENDS_REVOKE_REQUEST_X
		end
		if FL_GUI.accept_request == true then
			options[ #options + 1 ] = UIText.PRO_FRIENDS_DECLINE_REQUEST_X
			options[ #options + 1 ] = UIText.PRO_FRIENDS_ACCEPT_REQUEST_Y
		end
		if FL_GUI.can_invite == true then
			options[ #options + 1 ] = UIText.PRO_FRIENDS_INF_SEND_INVITE_X
		end
		if FL_GUI.can_join_in_progress == true then
			options[ #options + 1 ] = UIText.PRO_FRIENDS_INF_JOIN_SESSION_Y
		end
				
		options[ #options + 1 ] = UIText.INFO_B_BACK
				
		SetupInfoLine_F( unpack( options ) )
	end
end

function FL_MessageUpdate( )
	--
end

function FL_Render()
	--
end

function FL_EnterEnd()
	RestoreInfoLine_F()
	InfoLineUnlock( true )
end

function FL_EndLoop( time )
	--
end

function FL_End()
	UIGlobals.ViewingFriendsList = false
end

function Friends_SetupInfoLine( gamer_cards, revoke_request, accept_request, can_invite, can_join_in_progress, can_select )
	FL_GUI.update_info_lines = true
	
	FL_GUI.gamer_cards = gamer_cards
	FL_GUI.revoke_request = revoke_request
	FL_GUI.accept_request = accept_request
	FL_GUI.can_invite = can_invite
	FL_GUI.can_join_in_progress = can_join_in_progress
	FL_GUI.can_select = can_select
end
