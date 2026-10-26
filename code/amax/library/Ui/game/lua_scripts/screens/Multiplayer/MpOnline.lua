GUI =
{
	finished = false,
	carousel_branch = "MpOnline",

	can_invite = false,

	party_running	= nil,
	party_ready		= nil,

	party_toggle	= false,
	lobby_active	= false,

	update_menu		= nil,
	
	Options =
	{
		RankedEvents	= 1,
		PrivateRace		= 2,
		ModShop			= 3,
		Showroom		= 4,
		Challenges		= 5,
		Leaderboards	= 6,
		Legend			= 7,
		Invite			= 8,
		RaceHistory		= 9,
	},

	Players			= {},
	Selection		= -1,

	table_map =
	{
		"party_running",
		"lobby_active",
		"nxe_party",
		"share",
	},

	change_table =
	{
		party_running	= nil,
		lobby_active	= nil,
		nxe_party		= nil,
		social_network	= nil,
	},

	CanExit = function( _GUI )
		if NetParty.IsRunning() == true then
			return false
		else
			-- #34678
			UIButtons.TimeLineActive( "fade_vignette", false )
			PlaySfxGraphicBack()
			return true
		end
	end,
}

function Init()	
	AddSCUI_Elements()
	DeferCam_Init( UIGlobals.CameraNames.MpAppBase )

	StoreInfoLine()
	Online_RefreshInfoLines()
	
	net_SetRichPresence( UIEnums.RichPresence.OnlineMain )
	
	UIButtons.TimeLineActive( "status_info", true )
	GUI.MenuId	= SCUI.name_to_id[ "menu" ]
	GUI.Lobby	= SCUI.name_to_id[ "lobby" ]

	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.RankedEvents,	UIText.MP_RANKED,			UIText.MP_HELP_TEXT_RANKED,			"ranked_Groups",nil,			UIEnums.Panel._3DAA_WORLD )
	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.PrivateRace,	UIText.MP_PRIVATE_RACE,		UIText.MP_HELP_TEXT_PRIVATE_RACE,	"main_groups",	nil,			UIEnums.Panel._3DAA_WORLD )
	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.ModShop,		UIText.MP_MOD_SHOP,			UIText.MP_HELP_TEXT_MOD_SHOP,		"custom_shop",	"common_icons",	UIEnums.Panel._3DAA_WORLD )
	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.Showroom,		UIText.MP_SHOWROOM,			UIText.MP_HELP_TEXT_SHOWROOM,		"garage",		"common_icons",	UIEnums.Panel._3DAA_WORLD )
	
	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.Challenges,	UIText.MP_CHALLENGES,		UIText.MP_HELP_TEXT_CHALLENGES,		"challenge",	nil,			UIEnums.Panel._3DAA_WORLD )
	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.Leaderboards,	UIText.MP_LEADERBOARD,		UIText.MP_HELP_TEXT_LEADERBOARD,	"leaderboards",	nil,			UIEnums.Panel._3DAA_WORLD )
	
	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.RaceHistory,	UIText.RBC_RACE_HISTORY,	UIText.RBU_TABHELP_RACE_HISTORY,	"Style_racing",	"common_icons",	UIEnums.Panel._3DAA_WORLD )
	
	Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.Invite,		UIText.MP_INVITE,			"LOBBY_INVITE_HELPTEXT",			"create_party",	nil,			UIEnums.Panel._3DAA_WORLD )

	
	-- Only add legend mode if its unlocked
	if Multiplayer.UnlockedLegend() == true then
		UIButtons.ChangePosition(GUI.MenuId, 0, 1, 0, true)
		Mp_SetupMenuItemHelpText( GUI.MenuId, GUI.Options.Legend, UIText.MP_LEGEND_MODE, UIText.MP_HELP_LEGEND_MODE, "legend", nil, UIEnums.Panel._3DAA_WORLD )
	end

	local modshop_unlocked = Multiplayer.GetFeatureUnlocked(UIEnums.MPFeatures.ModShop);
	
	if modshop_unlocked == false then
		local rank = Multiplayer.GetFeatureUnlockedMap()[UIEnums.MPFeatures.ModShop]

		Mp_LockMenuItem( GUI.MenuId, GUI.Options.ModShop - 1, rank )
	end

	-- add new text
	local unlock_table = Multiplayer.GetNewUnlocks()

	if unlock_table.mods == true and modshop_unlocked == true then
		Mp_ShowItemNew( GUI.MenuId, GUI.Options.ModShop - 1 )
	end
	
	if unlock_table.events == true then
		Mp_ShowItemNew( GUI.MenuId, GUI.Options.RankedEvents - 1 )
	end
	
	for i, v in pairs( Multiplayer.GetNewCarsMap() ) do
		if v == true then
			Mp_ShowItemNew( GUI.MenuId, GUI.Options.Showroom - 1 )
			break
		end
	end
	
	if unlock_table.challenges == true then
		Mp_ShowItemNew( GUI.MenuId, GUI.Options.Challenges - 1 )
	end

	-- Set gamer name and rank icon
	local pad_index = Profile.GetPrimaryPad()
	GUI.Progression	= Multiplayer.ProfileProgression()

	UIButtons.ChangeText( SCUI.name_to_id[ "player_gamertag" ], "PROFILE_PAD".. pad_index .. "_NAME" )

	LocalGamerPicture( SCUI.name_to_id[ "gamerpic" ], pad_index )
	Mp_RankIcon( SCUI.name_to_id[ "player_rank" ], GUI.Progression.level - 1, GUI.Progression.legend )
	
	-- calculate size
	local size_x, size_y, size_z = UIButtons.GetSize( SCUI.name_to_id[ "backing" ] )
	
	-- progress bar data
	GUI.Progression		= Multiplayer.ProfileProgression()
	local level_data	= Multiplayer.LevelProgressionData( GUI.Progression.level )
	
	local previous		= level_data.fans_previous
	local target_fans	= level_data.fans_needed

	local range = target_fans - previous
	local amount = 1
	
	if GUI.Progression.maxed == false then
		amount = ( GUI.Progression.fans - previous ) / range
	end

	UIButtons.ChangeSize( SCUI.name_to_id[ "progress" ], size_x * amount, size_y, size_z )

	-- setup stats
	local stats_left = 
	{
		{
			stat = "MPL_STATS_RATING",
			help = UIText.MP_STAT_HELP_RATING,
			icon = "points_target",
			file = "common_icons",
		},
		{
			stat = "MPL_STATS_POWERUP_RATIO",
			help = UIText.MP_STAT_HELP_POWERUPS,
			icon = "Perk_Hit",
			file = "fe_icons",
		},
		{
			stat = "MPL_STATS_TOTAL_PLAYTIME",
			help = UIText.MP_STAT_HELP_PLAYTIME,
			icon = "stopwatch",
			file = "common_icons",
		},
	}

	local stats_right = 
	{
		{
			stat = "MPL_STATS_TOTAL_WINS",
			help = UIText.MP_STAT_HELP_TOTAL_WINS,
			icon = "powered_up_racing",
			file = "common_icons",
		},
		{
			stat = "MPL_STATS_TOTAL_RACES",
			help = UIText.MP_STAT_HELP_TOTAL_RACES,
			icon = "end_of_race",
			file = "common_icons",
		},
		{
			stat = "MPL_STATS_TOTAL_FANS",
			help = UIText.MP_STAT_HELP_TOTAL_FANS,
			icon = "fan",
			file = "common_icons",
		},
	}

	local y = -9
	for i, v in ipairs( stats_left ) do
		UIButtons.SetParent( SetupStat( v.stat, v.help, v.icon, v.file, 5.5, y ), SCUI.name_to_id["player_frame"], UIEnums.Justify.BottomLeft )
		y = y - 6.5
	end

	local y = -9
	for i, v in ipairs( stats_right ) do
		UIButtons.SetParent( SetupStat( v.stat, v.help, v.icon, v.file, 26, y ), SCUI.name_to_id["player_frame"], UIEnums.Justify.BottomLeft )
		y = y - 6.5
	end

	-- create the party lobby
	GUI.PlayerList = SCUI.name_to_id["player_list"]
	for i = 1, NetParty.GetMaxPartySize() do
		local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpOnline.lua", "mp_player" )

		GUI.Players[i] = SetupRacer( root_id, i-1 )
		
		UIButtons.AddListItem( GUI.PlayerList, root_id, i )
	end

	net_SetGroupPresence()
	UIGlobals.Multiplayer.InPartyLobby = true
end

function PostInit()
	GUI.help_text_id, GUI.bottom_help_id = SetupBottomHelpBar( UIText.MP_MESSAGE_OF_THE_DAY )

	UIButtons.SetActive( SCUI.name_to_id["stat"], false, true )
	UIButtons.SetActive( SCUI.name_to_id["mp_player"], false )
	
	RefreshLobby()
	RestoreScreenSelection( GUI, "MenuId" )

	-- bring the party lobby up?
	if NetParty.IsRunning() == true then
		GUI.party_running = NetParty.IsRunning()

		net_EnterPartyLobby( false )
		UIButtons.TimeLineActive( "show_party", true )
		
		if CanEnterMatchmaking() == false then
			Mp_LockMenuItem( GUI.MenuId, GUI.Options.RankedEvents - 1 )
			Mp_LockMenuItem( GUI.MenuId, GUI.Options.PrivateRace - 1 )
		end
	end
	
	-- show popup incase we have been kicked
	if UIGlobals.Multiplayer.NotEnoughPlayersKick == true then
		UIGlobals.Multiplayer.NotEnoughPlayersKick = false
		SetupCustomPopup( UIEnums.CustomPopups.MultiplayerWorldTourNotEnoughPlayers )

	elseif UIGlobals.Multiplayer.LobbyConnectionLost == true then
		EndScreen( UIEnums.Context.Blurb ) -- [Bone] #37850
		
		UIGlobals.Multiplayer.LobbyConnectionLost = false
		SetupCustomPopup( UIEnums.CustomPopups.LostLobbyConnection )
	end
	
	UpdateMpStatusBar()
	
	-- [WillB] #37785 TCR:R041 - Need to present the user with a message informing them 
	if UIGlobals.Network.VoiceChatRestricted == true then
		SetupCustomPopup( UIEnums.CustomPopups.PS3NoVoiceAllowed )
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )

	-- Safety - if context 0 is set to change screen : clear parent
	if message == UIEnums.Message.ChangeScreen and arg1 == 0 then
		-- clear incase its still around as the screen changes
		UIButtons.SetParent( SCUI.name_to_id[ "gamerpic" ], nil )
		UIButtons.SetActive( SCUI.name_to_id[ "gamerpic" ], false, true )
	end

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true then

		if GUI.lobby_active == true then
			-- [WillB] #37242 - Cant view players on PS3
			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon and GUI.Slots[ UIButtons.GetSelection( GUI.PlayerList ) ].empty == false then
				UIGlobals.Multiplayer.SelectPlayerJoinRef = GUI.Slots[ UIButtons.GetSelection( GUI.PlayerList ) ].ref
				SetupCustomPopup( UIEnums.CustomPopups.ViewPlayer )
			end
		else
			local selected = UIButtons.GetSelection( GUI.MenuId )
		
			if selected == GUI.Options.RankedEvents then
				if CanEnterMatchmaking( ) == true then
					NetParty.EnableMatchingQoS( false )
					NetParty.EnterMatchmaking()

					net_MpEnterPlayMode( UIEnums.PlayMode.Playlist, true )
					GoScreen( "Multiplayer\\Shared\\MpPlaylists.lua" )
					PlaySfxNext()
					PlaySfxGraphicNext()
				else
					PlaySfxError()
				end

			elseif selected == GUI.Options.PrivateRace then
				if CanEnterMatchmaking( ) == true then
					NetParty.EnableMatchingQoS( false )
					NetParty.EnterMatchmaking()

					GoScreen( "Multiplayer\\Shared\\MpModeSelect.lua" )
					PlaySfxNext()
					PlaySfxGraphicNext()
				else
					PlaySfxError()
				end

			elseif selected == GUI.Options.Challenges then
				GoScreen( "Multiplayer\\Challenges\\MpChallenges.lua" )
				PlaySfxNext()

			elseif selected == GUI.Options.ModShop then
				if Multiplayer.GetFeatureUnlocked(UIEnums.MPFeatures.ModShop) == true then
					GoScreen( "Multiplayer\\ModShop\\MpModShop.lua" )
					PlaySfxNext()
				else
					PlaySfxError()
				end
				
			elseif selected == GUI.Options.Showroom then
				GoScreen( "Shared\\Garage.lua" )
				UISystem.PlaySound( UIEnums.SoundEffect.GarageSelect )

			elseif selected == GUI.Options.Leaderboards then
				GoScreen( "Multiplayer\\MpChooseLeaderboard.lua" )
				PlaySfxNext()
				PlaySfxGraphicNext()

			elseif selected == GUI.Options.Invite then
				if NetParty.IsRunning() == false then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerCreateOnlineParty )
				else
					if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
						if Mp_XenonPartyActive() == true then
							UIHardware.ShowParty360( Profile.GetPrimaryPad() )
						else
							UIHardware.ShowFriends360( Profile.GetPrimaryPad() )
						end

					elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
						Amax.FakeOpenFriendsList()
					end
				end
					
			elseif selected == GUI.Options.Legend then
				SetupCustomPopup( UIEnums.CustomPopups.ConfirmLegendMode )
				
			elseif selected == GUI.Options.RaceHistory then
				GoScreen( "Shared\\RaceHistory.lua" )
				PlaySfxNext()
			end
		end

	elseif message == UIEnums.Message.ButtonX and arg2 == true then
		if Mp_XenonPartyActive() == true then
			NetServices.ShowSystemCommunityUI()
		end

	elseif message == UIEnums.Message.ButtonRight and arg2 == true then
		if NetParty.IsRunning() == true then
			ToggleLobby( true )
			UISystem.PlaySound( UIEnums.SoundEffect.Right )
		end

	elseif message == UIEnums.Message.ButtonLeft and arg2 == true then
		if NetParty.IsRunning() == true then
			ToggleLobby( false )
			UISystem.PlaySound( UIEnums.SoundEffect.Left )
		end
	
	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		if NetParty.IsRunning() == true then
			SetupCustomPopup( UIEnums.CustomPopups.ExitPartyLobby ) 
		end

	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		if Amax.CanUseShare() == true then
			SetupCustomPopup( UIEnums.CustomPopups.MPOnlineSharingOptions )
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		if arg3 == UIEnums.ShareOptions.Facebook then
			
			if UIGlobals.SharingOptionsChosen == UIEnums.MPOnlineSharingOptions.LegendRank then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.LegendRank,1,-1)
			elseif UIGlobals.SharingOptionsChosen > UIEnums.MPOnlineSharingOptions.Rank then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPBlurEventRating+UIGlobals.SharingOptionsChosen-1,1,-1)
			else
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPPlayersRank,1,-1)
			end
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpOnline.lua" )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb)
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then

			if UIGlobals.SharingOptionsChosen == UIEnums.MPOnlineSharingOptions.LegendRank then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.LegendRank,0,-1)
			elseif UIGlobals.SharingOptionsChosen > UIEnums.MPOnlineSharingOptions.Rank then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPBlurEventRating+UIGlobals.SharingOptionsChosen-1,0,-1)
			else
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPPlayersRank,0,-1)
			end
		
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpOnline.lua" )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			if UIGlobals.SharingOptionsChosen == UIEnums.MPOnlineSharingOptions.LegendRank then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.LegendRank,2,-1)
			elseif UIGlobals.SharingOptionsChosen > UIEnums.MPOnlineSharingOptions.Rank then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPBlurEventRating+UIGlobals.SharingOptionsChosen-1,2,-1)
			else
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPPlayersRank,2,-1)
			end
		end		

	end

end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate( time )
	DeferCam_Update( time )
	
	if GUI.finished == true then
		return
	end

	if Multiplayer.RefreshLobby( GUI.Lobby ) == true then
		RefreshLobby()
	end

	UpdateVoiceInfo()
	
	-- hide menu items that we cant access
	if GUI.party_running ~= NetParty.IsRunning() and UIScreen.IsPopupActive() == false then
		GUI.party_running = NetParty.IsRunning()
		UIButtons.TimeLineActive( "show_party", GUI.party_running )

		-- party has just been created
		if GUI.party_running == true and NetParty.IsHost() == true then

			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
				if Mp_XenonPartyActive() == true then
					UIHardware.ShowParty360( Profile.GetPrimaryPad() )
				else
					UIHardware.ShowFriends360( Profile.GetPrimaryPad() )
				end

			elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
				Profile.SetFriendsActive( true )
				Amax.FakeOpenFriendsList()
			end

		elseif GUI.party_running == false then
			Mp_UnlockMenuItem( GUI.MenuId, GUI.Options.RankedEvents - 1 )
			Mp_UnlockMenuItem( GUI.MenuId, GUI.Options.PrivateRace - 1 )

			ToggleLobby( false )
		end
	end

	if NetParty.IsRunning() == true then
		local party_ready = CanEnterMatchmaking()
		if GUI.party_ready ~= party_ready then
			GUI.party_ready = party_ready

			if GUI.party_ready == true then
				Mp_UnlockMenuItem( GUI.MenuId, GUI.Options.RankedEvents - 1 )
				Mp_UnlockMenuItem( GUI.MenuId, GUI.Options.PrivateRace - 1 )
			else
				Mp_LockMenuItem( GUI.MenuId, GUI.Options.RankedEvents - 1 )
				Mp_LockMenuItem( GUI.MenuId, GUI.Options.PrivateRace - 1 )
			end
		end
	end

	-- accept party race invite
	if UIGlobals.Network.RaceInviteState == UIEnums.MpRaceInviteState.Initialising and UIScreen.IsPopupActive() == false then
		UIGlobals.Network.RaceInviteState = UIEnums.MpRaceInviteState.PreProcessing
	end

	Online_RefreshInfoLines()
end

function EnterEnd()
	RestoreInfoLine()
end

function End()
	UIGlobals.Multiplayer.InPartyLobby = false

	StoreScreenSelection( GUI, "MenuId" )

	UIButtons.TimeLineActive( "status_info", false )
end


function SetupStat( stat, help_text, icon, file, x, y )
	local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpOnline.lua", "stat" )
	
	local icon_id	= UIButtons.FindChildByName( root_id, "stat_icon" )
	local help_id	= UIButtons.FindChildByName( root_id, "stat_help" )

	UIButtons.ChangeText( root_id, stat )
	UIButtons.ChangeText( help_id, help_text )

	UIShape.ChangeSceneName( icon_id, file )
	UIShape.ChangeObjectName( icon_id, icon )

	UIButtons.ChangePosition( root_id, x, y, 0 )

	return root_id
end


function SetupRacer( root_id, index )
	local name_id		= UIButtons.FindChildByName( root_id, "gamertag" )
	local rank_id		= UIButtons.FindChildByName( root_id, "rank" )

	UIButtons.ChangeText( name_id, "LOBBY_PARTY_PLAYER_NAME" .. index )
	UIButtons.ChangeText( rank_id, "LOBBY_PARTY_PLAYER_RANK" .. index )

	return root_id
end

function RefreshLobby()
	GUI.Slots			= Multiplayer.GetLobbySlots( GUI.Lobby )
	GUI.GamerPictures	= Profile.GetRemoteGamerPictureMap()

	for i, v in ipairs( GUI.Slots ) do
		local empty_id		= UIButtons.FindChildByName( GUI.Players[i], "empty" )
		local gamerpic_id	= UIButtons.FindChildByName( GUI.Players[i], "gamer_picture" )
		local rank_id		= UIButtons.FindChildByName( GUI.Players[i], "rank" )
		local rank_icon_id	= UIButtons.FindChildByName( GUI.Players[i], "rank_icon" )
		local host_id		= UIButtons.FindChildByName( GUI.Players[i], "host" )

		local name_id		= UIButtons.FindChildByName( GUI.Players[i], "gamertag" )
		local frame_id		= UIButtons.FindChildByName( GUI.Players[i], "frame" )

		-- turn on or off other gadgets
		UIButtons.SetActive( rank_icon_id, not v.empty, true, false )
		UIButtons.SetActive( empty_id, v.empty )
		UIButtons.SetActive( host_id, v.party_host )

		UIButtons.PrivateTimeLineActive( GUI.Players[i], "player_active" , not v.empty )
		UIButtons.PrivateTimeLineActive( GUI.Players[i], "player_racing" , false )

		-- lock or unlock the list
		UIButtons.SetNodeItemLocked( GUI.PlayerList, i - 1, v.empty )

		-- need to refresh the string length
		UIButtons.GetStaticTextLength( rank_id, true )

		-- setup colour
		if i == 1 then
			UIButtons.ChangeColour( name_id, "Support_4" )
			UIButtons.ChangeColour( frame_id, "Support_4" )

			LocalGamerPicture( gamerpic_id, Profile.GetPrimaryPad() )
			Mp_RankIcon( rank_icon_id, v.rank, v.legend )
		else
			-- active player
			if v.empty == false then
				RemoteGamerPicture( gamerpic_id, GUI.GamerPictures[v.ref] )
				Mp_RankIcon( rank_icon_id, v.rank, v.legend )
			else
				RemoteGamerPicture( gamerpic_id, nil ) -- default
			end
		end
	end

	if NetParty.IsHost() == true then
		UIButtons.ChangeColour( SCUI.name_to_id["party_leader"], "Support_4" )
	else
		UIButtons.ChangeColour( SCUI.name_to_id["party_leader"], "Support_1" )
	end
end


function ToggleLobby( toggle )
	GUI.lobby_active = toggle
	UIButtons.SetSelected( GUI.PlayerList, toggle )
	UIButtons.SetSelected( GUI.MenuId, not toggle )

	UIButtons.TimeLineActive( "highlight_lobby", toggle )
end


function UpdateVoiceInfo()
	local voice_info = NetServices.GetVoiceMap( true )

	for i, v in ipairs( GUI.Slots ) do

		local voice_id	= UIButtons.FindChildByName( GUI.Players[i], "voice" )

		-- turn off by default
		UIButtons.SetActive( voice_id, false )

		if v.empty == false and voice_info[v.ref] ~= nil then

			-- turn icon on for speaking players
			if voice_info[v.ref].speaking == true then
				UIButtons.SetActive( voice_id, true )
				UIButtons.ChangeTextureUV( voice_id, 0, 0.5, 0 )
				UIButtons.ChangeColour( voice_id, "Main_White" )

			-- show mute icon
			elseif voice_info[v.ref].muted == true then
				UIButtons.SetActive( voice_id, true )
				UIButtons.ChangeTextureUV( voice_id, 0, 0, 0 )
				UIButtons.ChangeColour( voice_id, "Main_White" )

			-- headset is plugged in
			elseif voice_info[v.ref].active == true then
				UIButtons.SetActive( voice_id, true )
				UIButtons.ChangeTextureUV( voice_id, 0, 0.5, 0 )
				UIButtons.ChangeColour( voice_id, "Main_Black" )
			end
		end
	end
end


function Online_RefreshInfoLines()
	local change_table =
	{
		party_running	= NetParty.IsRunning(),
		lobby_active	= GUI.lobby_active,
		nxe_party		= Mp_XenonPartyActive(),
		share			= Amax.CanUseShare(),
	}

	-- check to see if anything has changed
	local update = false
	for i, v in pairs( GUI.table_map ) do
		if change_table[v] ~= GUI.change_table[v] then
			update = true
			break
		end
	end

	-- update?
	if update == false then
		return
	end
	
	-- store current state
	GUI.change_table.party_running	= NetParty.IsRunning()
	GUI.change_table.lobby_active	= GUI.lobby_active
	GUI.change_table.nxe_party		= Mp_XenonPartyActive()
	GUI.change_table.share			= Amax.CanUseShare()

	-- change info lines
	if GUI.change_table.party_running == true then
		-- [WillB] #37242 - Cant view players on PS3
		if GUI.change_table.lobby_active == true then
			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
				SetupInfoLine( UIText.INFO_VIEW_PLAYER, UIText.INFO_LEAVE_PARTY )
			else
				SetupInfoLine( UIText.INFO_LEAVE_PARTY )
			end
		else
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_LEAVE_PARTY )
		end
	else
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
	end

	if GUI.change_table.nxe_party == true then
		PushInfoLine( UIText.INFO_X_VIEW_PARTY )
	end

	if GUI.change_table.share == true then
		PushInfoLine("GAME_SHARE_BUTTON")
	end
end


function CanEnterMatchmaking(  )
	if NetParty.IsRunning( ) == false then
		return true -- no party active so can enter matchmaking at any times.
	end	
		
	if NetRace.IsRunning( ) == false then
		if NetParty.CanEnterMatchmaking( ) == true then -- only the host console will be allowed to enter the matchmaking process.
			return true
		end
	end

	return false
end
