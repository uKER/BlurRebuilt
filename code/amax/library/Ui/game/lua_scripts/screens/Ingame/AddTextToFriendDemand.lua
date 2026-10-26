GUI =
{
	finished = false,
	
	help_text_id = -1,
	help_text = 
	{
		UIText.FDE_TITLE_HELP,
		UIText.FDE_MESSAGE_HELP,
		UIText.FDE_EXTRA_CHALLENGE_HELP,
		UIText.FDE_SEND_HELP,
	},
	
	fd_node_list_id = -1,
	
	option_text_ids = {  },
	
	current_option_selection = -1,
}

function Init()
	AddSCUI_Elements()
	SetupInfoLine()
	
	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
end

function PostInit()
	local screen_id = SCUI.name_to_id[ "screen" ]
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMAND, screen_id, "message", "common_icons", 1, UIEnums.Justify.TopCentre, true, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	GUI.help_text_id, GUI.bottom_bar_id = SetupBottomHelpBar( nil, nil, 1, true )
	
	FriendDemand.Create()	-- make sure we've got latest data
	
	Fd_CreatePanel( screen_id, 4 )
	Fd_UpdatePanel()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.KeyboardFinished then
		if GUI.current_option_selection == 1 then
			FriendDemand.StoreName()
			UIButtons.ChangeText( GUI.option_text_ids[ GUI.current_option_selection ], "GAME_FRIEND_DEMAND_NAME" )
			
		elseif GUI.current_option_selection == 2 then
			FriendDemand.StoreDescription()
			UIButtons.ChangeText( GUI.option_text_ids[ GUI.current_option_selection ], "GAME_FRIEND_DEMAND_DESCRIPTION" )
		end
	
	elseif message == UIEnums.Message.MenuNext then
		if GUI.current_option_selection == 1 then
			if FriendDemand.CheckingName() == false then
				UIHardware.StartKeyboard( arg1, "GAME_FRIEND_DEMAND_NAME", UIText.FDE_FRIEND_DEMAND, UIText.FDE_ENTER_NAME, 26, UIEnums.XboxKeyboardType.StandardHighlight )
			else
				PlaySfxError()
			end
			
		elseif GUI.current_option_selection == 2 then
			if FriendDemand.CheckingDesc() == false then
				UIHardware.StartKeyboard( arg1, "GAME_FRIEND_DEMAND_DESCRIPTION", UIText.FDE_FRIEND_DEMAND, UIText.FDE_ENTER_DESCRIPTION, 52, UIEnums.XboxKeyboardType.StandardHighlight )
			else
				PlaySfxError()
			end				
			
		elseif GUI.current_option_selection == 3 then
			PlaySfxNext()
			UIButtons.PrivateTimeLineActive( GUI.bottom_bar_id, "Hide_BottomHelp", true )
			PushScreen( "Ingame\\CreateFriendDemand.lua" )
			
		elseif GUI.current_option_selection == 4 then
			
			local friend_removed=false
			for i=1,3,1 do
				if UIGlobals.FriendDemandFriends[ i ] ~= nil then
					if FriendDemand.IsFriend( UIGlobals.FriendDemandFriends[ i ] ) == false then
						friend_removed=true
						UIGlobals.FriendDemandFriends[ i ] = nil
					end
				end
			end
			
			if friend_removed == false then
				PlaySfxNext()
				UIGlobals.FriendDemandIsResend = false
				GoScreen( "Ingame\\SendingFriendDemand.lua" )
			else
				SetupCustomPopup( UIEnums.CustomPopups.FriendDemandRemovedFriend )
			end
		end
		
	elseif message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		GoScreen( "Ingame\\AddFriendsToFriendDemand.lua" )
		
	elseif message == UIEnums.Message.ButtonX then
		if UIGlobals.FriendDemandSwitchChosen > -1 then
			PlaySfxError()
			Fd_ClearExtraChallenge()			
			Fd_UpdateExtraChallenge()
			FdAddText_SetupInfoLines()
		end
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
		
		FdAddText_SetupInfoLines()
		FdAddText_SetupHelpText()
	end
	
	if UIGlobals.FdAddText_DoUpdate == true then
		UIGlobals.FdAddText_DoUpdate = false
		
		Fd_UpdateExtraChallenge()
		FdAddText_SetupInfoLines()
		
		UIButtons.PrivateTimeLineActive( GUI.bottom_bar_id, "Hide_BottomHelp", false )
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

function FdAddText_SetupInfoLines()
	if UIGlobals.FriendDemandSwitchChosen > -1 then
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_X_CLEAR_EXTRA_CHALLENGE )
	else
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
	end
end

function FdAddText_SetupHelpText()
	UIButtons.ChangeText( GUI.help_text_id, GUI.help_text[ GUI.current_option_selection ] )
	UIButtons.TimeLineActive( "HelpFade", true, 0.5 )
end
