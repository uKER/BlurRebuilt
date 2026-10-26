GUI =
{
	finished = false,

	help_text = 
	{
		UIText.FDE_TITLE_HELP,
		UIText.FDE_MESSAGE_HELP,
		UIText.FDE_SEND_HELP,
	},
	
	current_option_selection = -1,
}

function Init()
	AddSCUI_Elements()
	SetupInfoLine( UIText.INFO_A_SEND_FRIEND_DEMAND_RECHALLENGE, UIText.INFO_B_BACK, UIText.INFO_EDIT_RESPONSE )
	
	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
end

function PostInit()
	local screen_id = SCUI.name_to_id[ "screen" ]
	local panel_id = SCUI.name_to_id[ "panel" ]
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMAND, screen_id, "message", "common_icons", 1, UIEnums.Justify.TopCentre, true, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	GUI.help_text_id = SetupBottomHelpBar( UIText.FDE_ADD_A_RETORT, nil, 1, true )
	
	Fd_CreatePanel( panel_id, 3, false, true )
	Fd_UpdatePanel( nil, nil, true )
	
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuBack then
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_FRIEND_DEMAND ) )

	elseif message == UIEnums.Message.MenuNext then
			
		if GUI.current_option_selection == 2 then
			if FriendDemand.CheckingDesc() == false then
				UIHardware.StartKeyboard( arg1, "GAME_FRIEND_DEMAND_DESCRIPTION", UIText.FDE_FRIEND_DEMAND, UIText.FDE_ENTER_DESCRIPTION, 52, UIEnums.XboxKeyboardType.StandardHighlight )
			else
				PlaySfxError()
			end			
		elseif GUI.current_option_selection == 3 then
			FriendDemand.ReChallenge()
			
			UIGlobals.FriendDemandFriends[ 1 ] = FriendDemand.GetReChallengeFriend()
			UIGlobals.FriendDemandFriends[ 2 ] = nil
			UIGlobals.FriendDemandFriends[ 3 ] = nil
			
			UIGlobals.FriendDemandIsResend = true

			GoScreen( "Ingame\\SendingFriendDemand.lua" )
		end
		
	elseif message == UIEnums.Message.KeyboardFinished then
		FriendDemand.StoreDescription()
		UIButtons.ChangeText( GUI.option_text_ids[ 2 ], "GAME_FRIEND_DEMAND_DESCRIPTION" )
	end
end

function FrameUpdate( time )
	local current_option_selection = UIButtons.GetSelection( GUI.fd_node_list_id )
	
	if current_option_selection ~= GUI.current_option_selection then
		if GUI.current_option_selection ~= -1 then
			UIButtons.PrivateTimeLineActive( GUI.option_box_ids[ GUI.current_option_selection ], "flash", false )
		end
		
		GUI.current_option_selection = current_option_selection
		
		UIButtons.PrivateTimeLineActive( GUI.option_box_ids[ GUI.current_option_selection ], "flash", true, 0 )
		
		UIButtons.ChangeText( GUI.help_text_id, GUI.help_text[ GUI.current_option_selection ] )
	end	
		
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end
