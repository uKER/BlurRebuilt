GUI =
{
	finished = false,
	modes = {  },
	race_overview_id = -1,

	pad_icons_uv =
	{
		{
			filename = "xbox360_buttons",
			pos = { u = 0.75, v = 0.75 },
			size = { u = 0.125, v = 0.125 },
		},
		{
			filename = "xbox360_buttons",
			pos = { u = 0, v = 0.875 },
			size = { u = 0.125, v = 0.125 },
		},
		{
			filename = "xbox360_buttons",
			pos = { u = 0.125, v = 0.875 },
			size = { u = 0.125, v = 0.125 },
		},
		{
			filename = "xbox360_buttons",
			pos = { u = 0.875, v = 0.75 },
			size = { u = 0.125, v = 0.125 },
		},
	},
	
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
	
	if Amax.GetPlayMode() == UIEnums.PlayMode.Playlist then
		local playlist = Multiplayer.GetPlaylist()
		UIShape.ChangeObjectName( SCUI.name_to_id["playlist_icon"], playlist.icon_name )
		UIButtons.ChangeText( SCUI.name_to_id["playlist_name"], "MPL_PLAYLIST_NAME" .. Multiplayer.GetCurrentEventIndex() )
	
	elseif Amax.GetPlayMode() == UIEnums.PlayMode.CustomRace then
		UIShape.ChangeObjectName( SCUI.name_to_id["playlist_icon"], "misc" )
		UIButtons.ChangeText( SCUI.name_to_id["playlist_name"], UIText.MP_CUSTOM_GAME )
	end

	StoreInfoLine()

	if Amax.GetPlayMode() == UIEnums.PlayMode.CustomRace then
		SetupInfoLine( UIText.INFO_A_NEXT, UIText.INFO_B_BACK, UIText.INFO_RACE_SETTINGS, UIText.INFO_RESET_SCORES_Y, "GAME_SHARE_BUTTON" )
	else
		SetupInfoLine( UIText.INFO_A_NEXT, UIText.INFO_B_BACK, UIText.INFO_RESET_SCORES_Y, "GAME_SHARE_BUTTON" )
	end
end

function PostInit()

	-- setup player gadgets
	local gadgets = {}
	local pos_x = 0
	local pos_y = 0
	local pos_z = 0

	for i = 1, 4 do
		gadgets[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpSplitscreenLobby.lua", "frame" )

		UIButtons.ChangePosition( gadgets[i], pos_x, pos_y, pos_z )
		UIButtons.SetParent( gadgets[i], SCUI.name_to_id["left"], UIEnums.Justify.TopLeft )

		UIButtons.ChangeColour( UIButtons.FindChildByName( gadgets[i], "gamer_picture" ), "locked" )
		UIButtons.ChangeColour( UIButtons.FindChildByName( gadgets[i], "name" ), "locked" )
		UIButtons.ChangeColour( gadgets[i], "locked" )

		pos_y = pos_y - 7.5
	end

	-- display the players in order
	local scores = Multiplayer.GetSplitscreenScores( true )
	local sorted_scores = Splitscreen_GetSortedScores( scores )
	
	for i = 1, #sorted_scores, 1 do
		local player_index = sorted_scores[ i ].player_index
		local p_table = UIGlobals.Splitscreen.players[ player_index ]

		local name_id = UIButtons.FindChildByName( gadgets[i], "name" )
		local wins_id = UIButtons.FindChildByName( gadgets[i], "wins" )
		local points_id = UIButtons.FindChildByName( gadgets[i], "points" )
		local gamer_id =UIButtons.FindChildByName( gadgets[i], "gamer_picture" )
		local frame_id = gadgets[i]

		UIButtons.ChangeText( name_id, "PROFILE_PAD" .. ( p_table.pad ) .. "_NAME" )
		UIButtons.ChangeText( wins_id, "MPL_SPLITSCREEN" .. ( player_index - 1 ) .. "_WINS_TOTAL" )
		UIButtons.ChangeText( points_id, "MPL_SPLITSCREEN" .. ( player_index - 1 ) .. "_POINTS_TOTAL" )

		UIButtons.ChangeColour( frame_id, UIGlobals.Splitscreen.colours[ p_table.pad ] )
		UIButtons.ChangeColour( name_id, UIGlobals.Splitscreen.colours[ p_table.pad ] )
		UIButtons.ChangeColour( gamer_id, "!255 255 255 255" )

		UIButtons.SetAlphaIntensity( gamer_id, nil, 1.0 )
		UIButtons.SetAlphaIntensity( name_id, nil, 1.0 )
		UIButtons.SetAlphaIntensity( frame_id, nil, 1.0 )
		LocalGamerPicture( gamer_id, p_table.pad )

		-- show the correct pad icon
		local pad_id = UIButtons.FindChildByName( gadgets[i], "pad_icon" )
		UIButtons.ChangeTexture( GUI.pad_icons_uv[p_table.pad+1], 0, pad_id )
		UIButtons.SetActive( pad_id, true )
	end
	
	-- Setup race information page - pass in function name and call it on the return
	local race = Multiplayer.GetCurrentRace()

	GUI.race_overview_id = SCUI.name_to_id["race_overview_node"]
	Mp_RefreshRaceOverview( GUI.race_overview_id, race )

	GUI.gadgets = gadgets

	if UIGlobals.Splitscreen.can_vote == true and Multiplayer.IsVotingEnabled() == true then
		SplitscreenLobby_StartVoting()
	end

	UIButtons.SetActive( SCUI.name_to_id["frame"], false, true )
end

function FrameUpdate()
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true or SubScreenActive() == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		PlaySfxNext()
		GoScreen( "Multiplayer\\MpSplitscreenCarSelect.lua" )
		
	elseif message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		PlaySfxGraphicBack()
		GoScreen( "Multiplayer\\Shared\\MpModeSelect.lua" )

	elseif message == UIEnums.Message.ButtonX and Amax.GetPlayMode() == UIEnums.PlayMode.CustomRace then
		UISystem.PlaySound( UIEnums.SoundEffect.Filter )
		PlaySfxGraphicBack()

		StoreScreen( UIEnums.ScreenStorage.FE_BACK, UIScreen.GetCurrentScreen( UIScreen.Context() ) )
		GoScreen( "Multiplayer\\Shared\\MpCustomRace.lua" )
	
	elseif message == UIEnums.Message.ButtonY then
		SetupCustomPopup( UIEnums.CustomPopups.SplitscreenResetScores )
		
	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		if Amax.CanUseShare() == true then
			SetupCustomPopup( UIEnums.CustomPopups.SSLobbySharingOptions )
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then
	
		if arg3 == UIEnums.ShareOptions.Facebook then
		
			Amax.CreateBlurb(UIGlobals.SharingOptionsChosen,1,-1)

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpSplitscreenLobby.lua" )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb)
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then
		
			Amax.CreateBlurb(UIGlobals.SharingOptionsChosen,0,-1)

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpSplitscreenLobby.lua" )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Blurb then
		
			Amax.CreateBlurb(UIGlobals.SharingOptionsChosen,2,-1)
		
		end		
		
	end
end

function EnterEnd()
	local race = Multiplayer.GetCurrentRace()
	Amax.LoadTextureClone( race.thumbnail )	
	UIButtons.TimeLineActive( "lobby_fade", true )
	
	RestoreInfoLine()
end

function End()
	--
end

function SplitscreenLobby_DisablePopupActive( state )
	local _GUI = ContextTable[ UIEnums.Context.CarouselApp ].GUI
	if IsTable( _GUI ) == false then
		return
	end
	if IsTable( _GUI.gadgets ) == false then
		return
	end
	
	if state == false then
		for i, v in ipairs( _GUI.gadgets ) do
			local id = UIButtons.FindChildByName( v, "gamer_picture" )
			UIButtons.SetXtVar( id, "time_lines.0.label", "Popup_Active" )
			if UIScreen.IsPopupActive() == false then
				UIButtons.PrivateTimeLineActive( id, "Popup_Active", state )
			end
		end
	else
		for i, v in ipairs( _GUI.gadgets ) do
			local id = UIButtons.FindChildByName( v, "gamer_picture" )
			UIButtons.PrivateTimeLineActive( id, "Popup_Active", state )
			UIButtons.SetXtVar( id, "time_lines.0.label", "hacksaw" )
		end
	end
end

function SplitscreenLobby_StartVoting()
	-- Disable the effect
	SplitscreenLobby_DisablePopupActive( true )

	UIGlobals.Multiplayer.VotingFinished = false

	UIButtons.SetActive( SCUI.name_to_id["right"], false )

	Multiplayer.StartRaceVoting()
	PushScreen( "Multiplayer\\Shared\\MpVoting.lua" )
end

function SplitscreenLobby_EndVoting()
	-- Re-enable the effect
	SplitscreenLobby_DisablePopupActive( false )

	local _GUI = ContextTable[ ContextTable[ UIEnums.Context.Main ].GUI.active_context ].GUI
	local _SCUI = ContextTable[ ContextTable[ UIEnums.Context.Main ].GUI.active_context ].SCUI
	UIButtons.SetActive( _SCUI.name_to_id["right"], true )

	local race = Multiplayer.GetCurrentRace()
	Mp_RefreshRaceOverview( _GUI.race_overview_id, race, Multiplayer.GetCurrentRaceIndex() )

	Profile.LockToPad( Profile.GetPrimaryPad() )
	UIGlobals.Splitscreen.can_vote = false

	PopScreen()
end
