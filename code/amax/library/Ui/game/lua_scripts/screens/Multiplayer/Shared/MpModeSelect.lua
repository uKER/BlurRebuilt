GUI =
{
	finished	= false,
	menu_id		= -1,
	selection	= 1,

	options =
	{
		playlists	= 1,
		custom_race = 2,
	},

	CanExit = function( _GUI )
		return false
	end,
}

function Init()	
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )

	GUI.helptext = 
	{
		[GUI.options.playlists] = UIText.MP_HELP_TEXT_PLAYLISTS,
		[GUI.options.custom_race] = UIText.MP_HELP_TEXT_CUSTOM_GAME,
	}

	GUI.menu_id = SCUI.name_to_id["menu"]

	Multiplayer.SplitscreenResetScores()
end

function PostInit()
	Amax.ChangeUiCamera( "Mp_3", UIGlobals.CameraLerpTime, 0 )

	GUI.node_id =
	{
		MpModeSelect_SetupItem( GUI.menu_id, UIText.MP_PLAYLISTS, GUI.options.playlists, "blur_events" ),
		MpModeSelect_SetupItem( GUI.menu_id, UIText.MP_CUSTOM_GAME, GUI.options.custom_race, "custom_game" ),
	}
	
	GUI.helpline_id, GUI.helpline_root_id = SetupBottomHelpBar( UIText.MP_HELP_TEXT_PLAYLISTS )
	
	-- Select
	UIButtons.PrivateTimeLineActive( GUI.node_id[ GUI.options.playlists ], "Selected", true, 4.2 )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		local selection = UIButtons.GetSelection( GUI.menu_id )

		if selection == GUI.options.playlists then
			PlaySfxNext()

			net_MpEnterPlayMode( UIEnums.PlayMode.Playlist )
			
			PlaySfxGraphicNext()
			GoScreen( "Multiplayer\\Shared\\MpPlaylists.lua" )

		elseif selection == GUI.options.custom_race then
			PlaySfxNext()

			Mp_ResetCustomRace()
			net_MpEnterPlayMode( UIEnums.PlayMode.CustomRace )

			Amax.SetupRace( MpModeSelect_CreateGame() )

			if Amax.GetGameMode() == UIEnums.GameMode.Online then
				SetupCustomPopup( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )
			
			elseif Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
				if NetRace.CreateLanServer( MpModeSelect_CreateGame() ) == true then
					PlaySfxGraphicNext()
					GoSubScreen( "Multiplayer\\MpLobby.lua" )
				else
					print( "LAN - Failed to create game" )
				end	

			elseif Amax.GetGameMode() == UIEnums.GameMode.SplitScreen then
				PlaySfxGraphicNext()
				GoScreen( "Multiplayer\\MpSplitscreenLobby.lua" )
			end
		end

	elseif message == UIEnums.Message.MenuBack then 
		PlaySfxBack()
		PlaySfxGraphicBack()
		
		if Amax.GetGameMode() == UIEnums.GameMode.SplitScreen then
			GoScreen( "Multiplayer\\MpSplitscreenSignIn.lua" )

		elseif Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
			GoScreen( "Multiplayer\\MpLan.lua" )

		elseif Amax.GetGameMode() == UIEnums.GameMode.Online then
			GoScreen( "Multiplayer\\MpOnline.lua" )
		end
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	local selection = UIButtons.GetSelection( GUI.menu_id )
	if selection ~= GUI.selection then
		UIButtons.PrivateTimeLineActive( GUI.node_id[ GUI.selection ], "Selected", false )

		UIButtons.ChangeText( GUI.helpline_id, GUI.helptext[selection] )
		GUI.selection = selection
		
		UIButtons.PrivateTimeLineActive( GUI.node_id[ GUI.selection ], "Selected", true, 0 )
		UIButtons.TimeLineActive( "HelpFade", true, 0.5 )
	end
end

function EnterEnd()
	RestoreInfoLine()
	
	for i, v in ipairs( GUI.node_id ) do
		UIButtons.PrivateTimeLineActive( v, "End", true )
	end
end

function End()
end

function MpModeSelect_SetupItem( menu_id, title, value, icon_name )
	local node_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "CarouselBranch" )
	local title_id = UIButtons.FindChildByName( node_id, "Title" )
	local icon_id = UIButtons.FindChildByName( node_id, "Shape" )

	UIShape.ChangeSceneName( icon_id, "fe_icons" )
	UIShape.ChangeObjectName( icon_id, icon_name )

	UIButtons.ChangeText( title_id, title )
	UIButtons.ChangeSize( title_id, 0, 5, 0 )
	UIButtons.AddListItem( menu_id, node_id, value )
	
	UIButtons.PrivateTimeLineActive( node_id, "Init", true, 0.5, true  )
	
	return node_id
end

function MpModeSelect_CreateGame()
	UIGlobals.CustomRaceSettings.num_players = Select( Amax.GetGameMode() == UIEnums.GameMode.SplitScreen, #UIGlobals.Splitscreen.players, 1 )
	
	return UIGlobals.CustomRaceSettings
end