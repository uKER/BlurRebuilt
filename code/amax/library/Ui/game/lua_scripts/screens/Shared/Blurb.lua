GUI =
{
	finished = false,	
	carousel_branch = "Blurb",
	
	blurbs = {  },
	
	node_list_id = -1,
	
	node_ids = {  },
	
	current_selection = -1,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	PlaySfxGraphicNext()
	
	AddSCUI_Elements()
	DeferCam_Init( "Sp_1" )

	CarouselApp_SetScreenTimers()
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
end

function PostInit()
	UIButtons.TimeLineActive( "Hide_EventSelect", true )
	
	GUI.blurbs = Amax.GetBlurbs( UIGlobals.FriendDemandFilter, Select( ( UIGlobals.FriendDemandFilter == 3 ), UIGlobals.FriendDemandFilterEvent, UIGlobals.FriendDemandFilterFriend ) )
	--show_table( GUI.blurbs, 1 )
	
	if UIGlobals.FriendDemandAttemptFromMessage == true then
		SetupCustomPopup( UIEnums.CustomPopups.FriendDemandRetrieving )
	end
	
	if #GUI.blurbs == 0 then
		UIButtons.SetActive( SCUI.name_to_id[ "black_top_background" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "friend_pic_frame" ], false, false )
		return
	end
	
	if UIGlobals.FriendDemandFilter == 3 then
		UIButtons.SetActive( SCUI.name_to_id[ "black_top_background" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "friend_pic_frame" ], false, true )
	else
		-- friend info
		UIButtons.ChangeText( SCUI.name_to_id[ "friend_type_info" ], Select( ( UIGlobals.FriendDemandFilter == 2 ), UIText.FDE_SORTING_NEW_CHALLENGES, UIText.FDE_SORTING_ACTIVE_CHALLENGES ) )
		UIButtons.ChangeText( SCUI.name_to_id[ "friend_name" ], "GAME_FRIEND_DEMAND_FRIEND_" .. UIGlobals.FriendDemandFilterFriend )
		
		local pic_id = SCUI.name_to_id[ "friend_pic" ]
		local slot = Profile.GetRemoteGamerPictureMap()[ UIGlobals.FriendDemandFilterFriend ]
		
		if slot ~= nil then
			UIButtons.ChangeTexture( { filename = "REMOTE_GAMERPIC_" .. slot }, 1, pic_id )
		else
			UIButtons.ChangeTexture( { filename = "default_gamerpic" }, 1, pic_id )
		end
	end
	
	-- add blurbs
	GUI.node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i, v in ipairs( GUI.blurbs ) do
		local is_server = ( v.type == UIEnums.FriendDemandMessageType.Server )
		local msg_prefix = Select( is_server, "GAME_BLURB_SERVER_MSG", "GAME_BLURB_NORMAL_MSG" )
		
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "blurb_node" )
		
		local fd_info = FriendDemand.GetInfo( v.index, is_server )
		--show_table( fd_info )
		local event_info = SinglePlayer.EventInfo( fd_info.eventid_sp )
		--show_table( event_info )
		
		UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "blurb_icon" ), Sp_KindToShape( event_info.kind ) )
		
		UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "blurb_event_text" ), Sp_KindToText( event_info.kind ) )
		UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "blurb_name" ), msg_prefix .. v.index .. "_NAME" )
		UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "blurb_info_time_sent" ), msg_prefix .. v.index .. "_TIME_SENT" )
		UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "blurb_info_days_left" ), msg_prefix .. v.index .. Select( is_server, "_DAYS_LEFT", "_ATTEMPTS" ) )
		
		local type_icon_id = UIButtons.FindChildByName( node_id, "blurb_type_icon" )
		UIShape.ChangeObjectName( type_icon_id, Select( is_server, "challenge_received", "challenge_open" ) )
		UIButtons.ChangeColour( type_icon_id, Select( fd_info.rechallenge, "Support_4", "Support_0" ) )
		
		if UIGlobals.FriendDemandFilter == 3 then
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "blurb_sender" ), msg_prefix .. v.index .. "_SENDER" )
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "blurb_icon" ), false )
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "blurb_event_text" ), false )
		end
		
		UIButtons.AddListItem( GUI.node_list_id, node_id, i )
		GUI.node_ids[ i ] = node_id
	end
	
	local panel_id = SCUI.name_to_id[ "panel_dummy" ]
	Fd_CreatePanel( panel_id, 2, true )
	UIButtons.ChangePanel( GUI.fd_panel_id, UIEnums.Panel._3DAA_WORLD, true )
	
	local screen_id = SCUI.name_to_id[ "screen" ]
	SetupScreenTitle( UIText.FDE_FRIEND_DEMANDS, screen_id, "message", "common_icons", 1, UIEnums.Justify.TopCentre, nil, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	GUI.help_text_id = SetupBottomHelpBar( UIText.FDE_BLURB_HELP, nil, 1, nil )
	
	ContextTable[ UIScreen.Context() ].FrameUpdate( 0 )	-- yeah
	
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.FriendDemandOptions then
		if arg3 == UIEnums.FriendDemandOptions.Attempt then
		
			if FriendDemand.IsFriend( UIGlobals.FriendDemandFilterFriend ) == false then
			
				SetupCustomPopup( UIEnums.CustomPopups.FriendDemandNotFriend )
				
			elseif UIGlobals.FriendDemandCurrentType == UIEnums.FriendDemandMessageType.Local then
				local blurb = GUI.blurbs[ GUI.current_selection ]
				AttemptFriendDemand( blurb.index )
			
			elseif UIGlobals.FriendDemandCurrentType == UIEnums.FriendDemandMessageType.Server then
				local blurb = GUI.blurbs[ GUI.current_selection ]
				UIGlobals.CurrentFriendDemandMessageIndex = blurb.index

				local free_slot = FriendDemand.GetFreeSlot()
				
				if free_slot == -1 then
					SetupCustomPopup( UIEnums.CustomPopups.FriendDemandNoFreeSlots )
				else
					UIGlobals.FriendDemandAttemptFromServer = true
					UIGlobals.CurrentFriendFreeSlot = free_slot
					SetupCustomPopup( UIEnums.CustomPopups.AcceptingFriendDemand )
				end
			end
			
		elseif arg3 == UIEnums.FriendDemandOptions.Store then
			
			if FriendDemand.IsFriend( UIGlobals.FriendDemandFilterFriend ) == false then
			
				SetupCustomPopup( UIEnums.CustomPopups.FriendDemandNotFriend )
				
			elseif UIGlobals.FriendDemandCurrentType == UIEnums.FriendDemandMessageType.Server then
				local blurb = GUI.blurbs[ GUI.current_selection ]
				UIGlobals.CurrentFriendDemandMessageIndex = blurb.index
				
				local free_slot = FriendDemand.GetFreeSlot()
				
				if free_slot == -1 then
					SetupCustomPopup( UIEnums.CustomPopups.FriendDemandNoFreeSlots )
				else
					UIGlobals.FriendDemandAttemptFromServer = false
					UIGlobals.CurrentFriendFreeSlot = free_slot
					SetupCustomPopup( UIEnums.CustomPopups.AcceptingFriendDemand )
				end
			end
		
		elseif arg3 == UIEnums.FriendDemandOptions.Delete then
			local blurb = GUI.blurbs[ GUI.current_selection ]
			UIGlobals.CurrentFriendDemandMessageIndex = blurb.index
			SetupCustomPopup( UIEnums.CustomPopups.FriendDemandDelete )
		end
		
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.FriendDemandNoBlurbs then
		if UIGlobals.FriendDemandFilter == 3 then
			SpPopToEventSelect()
		else
			GoScreen( "Shared\\BlurbMainMenu.lua" )
		end
	end
	
	if #GUI.blurbs == 0 then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		local blurb = GUI.blurbs[ GUI.current_selection ]
		UIGlobals.FriendDemandCurrentType = blurb.type
		SetupCustomPopup( UIEnums.CustomPopups.FriendDemandOptions )
		
	elseif message == UIEnums.Message.MenuBack then
		if UIGlobals.FriendDemandFilter == 3 then
			SpPopToEventSelect()
		else
			GoScreen( "Shared\\BlurbMainMenu.lua" )
		end	
		
		PlaySfxBack()
		PlaySfxGraphicBack()
	end
end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate( time )
	DeferCam_Update( time )
	
	if UIGlobals.RefreshFriendDemandScreen == true then
		if UIGlobals.FriendDemandAttemptFromMessage == true then
			UIGlobals.FriendDemandAttemptFromMessage = false

			local index, from_server=FriendDemand.GetKeyedIndex()

			if index ~= -1 then
				
				if from_server == true then
				
					UIGlobals.CurrentFriendDemandMessageIndex=index
			
					local free_slot=FriendDemand.GetFreeSlot()
					if free_slot == -1 then
						SetupCustomPopup( UIEnums.CustomPopups.FriendDemandNoFreeSlots )
					else
						UIGlobals.FriendDemandAttemptFromServer = true
						UIGlobals.CurrentFriendFreeSlot=free_slot
						SetupCustomPopup( UIEnums.CustomPopups.AcceptingFriendDemand )
					end
					
				else
					
					AttemptFriendDemand(index)
				
				end
				
			else
				
				SetupCustomPopup( UIEnums.CustomPopups.FriendDemandNotOnServer )
				
			end
				
		elseif UIGlobals.FriendDemandAttemptFromServer == true then
			UIGlobals.FriendDemandAttemptFromServer = false
			AttemptFriendDemand( UIGlobals.CurrentFriendFreeSlot )
			
		else
			GoScreen( "Shared\\Blurb.lua" )	-- just full refresh
		end
		
		UIGlobals.RefreshFriendDemandScreen = false
	end
	
	if UIGlobals.FriendDemandAttemptFromMessage == true or UIScreen.IsPopupActive() then
		return
	end
	
	if #GUI.blurbs == 0 or GUI.blurbs == nil then
		if UIGlobals.FriendDemandFilter == 3 then
			UIGlobals.SelectEventMenu=true
			SpPopToEventSelect()
		else
			GoScreen( "Shared\\BlurbMainMenu.lua" )
		end
		return
	end
	
	local current_selection = UIButtons.GetSelection( GUI.node_list_id )
	
	if current_selection ~= GUI.current_selection then
		if GUI.current_selection ~= -1 then
			UIButtons.PrivateTimeLineActive( GUI.node_ids[ GUI.current_selection ], "flash", false )
		end
		
		GUI.current_selection = current_selection
		
		UIButtons.PrivateTimeLineActive( GUI.node_ids[ GUI.current_selection ], "flash", true, 0 )
		
		local blurb = GUI.blurbs[ GUI.current_selection ]
		--show_table( blurb )
		
		if blurb.type == UIEnums.FriendDemandMessageType.Server then
			Fd_UpdatePanel( blurb.index, true )
			
		elseif blurb.type == UIEnums.FriendDemandMessageType.Local then
			Fd_UpdatePanel( blurb.index )
		end
	end
end

function EnterEnd()
	UIButtons.TimeLineActive( "Hide_EventSelect", false )
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	--
end

function AttemptFriendDemand( fd_index )
	local fd_info = FriendDemand.GetInfo( fd_index )
	show_table( fd_info )
	
	UIGlobals.FriendDemandAttemptingIndex = fd_index

	Amax.SetSPEvent( fd_info.eventid_sp )
	Amax.SetCurrentEvent( fd_info.eventid )

	Amax.SetCurrentCar( fd_info.vehicleid )
	SinglePlayer.EventSetLastVehicle( fd_info.eventid_sp , fd_info.vehicleid )
	SinglePlayer.GameSetLastVehicle( fd_info.vehicleid )

	FriendDemand.SetupRaceMode( fd_index, UIGlobals.FriendDemandFilterFriend )
	UIGlobals.ReturnToBlurb = true


	SpGoLoadingScreen()

	-- clone loading screen textures
	local event_info = SinglePlayer.EventInfo( fd_info.eventid_sp )
	local amax_event_info = GameData.GetEvent( event_info.eventId )
	
	Amax.LoadTextureClone( amax_event_info.route_tag, 0 )
end
