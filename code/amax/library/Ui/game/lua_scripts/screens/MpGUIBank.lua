UISystem.LoadLuaScript( "Screens\\Multiplayer\\Shared\\MpCustomRaceHelper.lua" )

function ServiceConnectionLost()
	-- only in multiplayer and if the user hasnt signed out since you get the disconect message next when a sign out occurs
	-- the xbox live disconnection can overide the lost connection to blur servers
	if Amax.GetGameMode() ~= UIEnums.GameMode.Online or ( UIGlobals.UserKickBackMode ~= UIEnums.UserKickBackMode.XLSPConnectionLost and UIGlobals.UserKickBackMode ~= UIEnums.UserKickBackMode.None ) then
		return
	end
	
	-- disable the logic while we are loading
	if UIGlobals.IsLoading == true then
		print( "ERROR - ServiceConnectionLost while on the loading screen" )
		return
	end

	print( "Connection to service lost!" )
	Amax.KickedOutOfGame();

	ReturnToStartScreen( UIEnums.UserKickBackMode.ServiceConnectionLost )
end

function XLSPConnectionLost()
	-- The 360 can live without demonware
	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		return
	end	

	if Amax.GetGameMode() ~= UIEnums.GameMode.Online or UserKickBackActive() == true then
		return
	end

	-- disable the logic while we are loading
	if UIGlobals.IsLoading == true then
		print( "ERROR - XLSPConnectionLost while on the loading screen" )
		return
	end
	
	print( "Connection to LSP lost!" )
	Amax.KickedOutOfGame();

	ReturnToStartScreen( UIEnums.UserKickBackMode.XLSPConnectionLost )
end

function LobbyConnectionLost()
	if UserKickBackActive() == true or UIGlobals.IdlePlayerKicked == true then
		return
	end

	print( "LobbyConnectionLost!" )
	UIGlobals.Multiplayer.LobbyConnectionLost = true
	net_DisconnectFromRace()	
end

function NetworkCableRemoved()
	-- only listen in system link
	if Amax.GetGameMode() ~= UIEnums.GameMode.SystemLink or UserKickBackActive() == true then
		return
	end

	print( "NetworkCableRemoved!" )
	ReturnToStartScreen( UIEnums.UserKickBackMode.NetworkCableRemoved )
end

function MultiplayerBetaExpired()
	if UserKickBackActive() == true then
		return
	end
	
	print( "Multiplayer Beta has expired!" )
	
	ReturnToStartScreen( UIEnums.UserKickBackMode.MpBetaExpired )
end

function Mp_SetupMenuItem( uListID, sValue, uTextID, sIconName, sIconSheet )

	local root_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "node_item" )
	local text_id = UIButtons.FindChildByName( root_id, "text" )
	local icon_id = UIButtons.FindChildByName( root_id, "icon" )

	if sIconSheet ~= nil then
		UIShape.ChangeSceneName( icon_id, sIconSheet )
	end

	UIButtons.ChangeText( text_id, uTextID )
	UIShape.ChangeObjectName( icon_id, sIconName )

	UIButtons.AddListItem( uListID, root_id, sValue )

	-- managle the animation name to stop it playing. A better
	-- way might be to attach the text to the help text and have the animation in that? Another day...
	UIButtons.SetXtVar( text_id, "time_lines.0.label", "unknown" )

	return root_id
end

function Mp_SetupMenuItemHelpText( uListID, sValue, uTextID, uHelpTextID, sIconName, sIconSheet, panel )

	local root_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "node_item" )
	local help_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "helptext" )
	local text_id = UIButtons.FindChildByName( root_id, "text" )
	local icon_id = UIButtons.FindChildByName( root_id, "icon" )

	if IsString( sIconSheet ) == true then
		UIShape.ChangeSceneName( icon_id, sIconSheet )
	end

	UIButtons.SetParent( help_id, text_id, UIEnums.Justify.BottomLeft )

	--local posX, posY, posZ = UIButtons.GetPosition( icon_id ) 
	--UIButtons.ChangePosition( icon_id, posX, posY - 1, posZ )

	UIButtons.ChangeText( text_id, uTextID )
	UIButtons.ChangeText( help_id, uHelpTextID )
	UIShape.ChangeObjectName( icon_id, sIconName )

	UIButtons.AddListItem( uListID, root_id, sValue )
	
	if IsNumber( panel ) == true then
		UIButtons.ChangePanel( root_id, panel, true )
		UIButtons.ChangePanel( help_id, panel, true )
	end

	return root_id
end

function Mp_SetupMenuItemCheckbox( uListID, sValue, uTextID, sIconName, sIconSheet )

	local root_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "node_item" )
	local check_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "checkbox_item" )
	local text_id = UIButtons.FindChildByName( root_id, "text" )
	local icon_id = UIButtons.FindChildByName( root_id, "icon" )

	if sIconSheet ~= nil then
		UIShape.ChangeSceneName( icon_id, sIconSheet )
	end

	UIButtons.SetParent( check_id, root_id, UIEnums.Justify.MiddleRight )
	UIButtons.ChangeText( text_id, uTextID )
	UIShape.ChangeObjectName( icon_id, sIconName )

	UIButtons.AddListItem( uListID, root_id, sValue )
	return root_id
end

function Mp_LockMenuItem( uListID, sIndex, sUnlockRank )

	local node_id = UIButtons.GetNodeID( uListID, sIndex )
	local text_id = UIButtons.FindChildByName( node_id, "text" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )

	UIButtons.ChangeColour( text_id, "dark_white" )
	UIButtons.ChangeColour( icon_id, "dark_white" )

	-- decide weather to override the helptext with the rank
	if sUnlockRank ~= nil then
		local helptext_id = UIButtons.FindChildByName( node_id, "helptext" )

		UIButtons.ChangeColour( helptext_id, "dark_white" )
		UIButtons.ChangeText( helptext_id, "MPL_UNLOCK_AT_RANK" .. sUnlockRank )
	end

	UIButtons.SetNodeItemLocked( uListID, sIndex, true )
end

function Mp_UnlockMenuItem( uListID, sIndex )

	local node_id = UIButtons.GetNodeID( uListID, sIndex )
	local text_id = UIButtons.FindChildByName( node_id, "text" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )

	UIButtons.ChangeColour( text_id, "NonBloomWhite" )
	UIButtons.ChangeColour( icon_id, "Main_2" )

	UIButtons.SetNodeItemLocked( uListID, sIndex, false )
end

function Mp_ShowItemNew( uListID, sIndex )
	local node_id = UIButtons.GetNodeID( uListID, sIndex )

	UIButtons.SetActive( UIButtons.FindChildByName( node_id, "new" ), true )
end

function Mp_HideItemNew( uListID, sIndex )
	local node_id = UIButtons.GetNodeID( uListID, sIndex )

	UIButtons.SetActive( UIButtons.FindChildByName( node_id, "new" ), false )
end

function Mp_SetupGroupDetailItem( sList, uText, sIcon, sScene, bValue )
	local node_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "detail_node" )
	local icon_id = UIButtons.FindChildByName( node_id, "detail_icon" )

	UIShape.ChangeSceneName( icon_id, sScene )
	UIShape.ChangeObjectName( icon_id, sIcon )

	UIButtons.AddListItem( sList, node_id )

	return node_id
end

function Mp_RankIcon( id, rank, legend )

	if legend ~= nil and rank ~= nil then
		-- offset legend by max rank
		if legend > 0 then
			rank = Multiplayer.GetMaxRank() + legend - 1
		end

		local texture = 
		{
			filename = "rank_icons",
			name = "rank_icons",
			pos = { u = 0.015625 * rank, v = 0 },
			size = { u = 0.015625, v = 1 },
		}
		UIButtons.ChangeTexture( texture, 0, id )
	end
end


function Mp_RouteThumbnail( id, name, index )
	if name == nil then
		return
	end

	if index == nil then
		index = 0
	end

	local texture = 
	{
		filename = name,
		name = name,
		pos = { u = 0 , v = 0 },
		size = { u = 1, v = 1 },
	}
	UIButtons.ChangeTexture( texture, index, id )
end


function Mp_RaceTypeIcon( id, race )
	if race == nil then
		return
	end

	local icon_name = ""
	if race.type == UIEnums.MpRaceType.Racing then
		icon_name = "style_racing"
	elseif race.type == UIEnums.MpRaceType.Elimination then
		icon_name = "style_elimination"
	elseif race.type == UIEnums.MpRaceType.Points then
		icon_name = "style_elimination"
	elseif race.type == UIEnums.MpRaceType.Destruction then
		icon_name = "survivor"
	elseif race.type == UIEnums.MpRaceType.Hardcore then
		icon_name = "style_racing"	
	end
	UIShape.ChangeObjectName( id, icon_name )
end

function Mp_RaceEndIcon( id, race )
	if race == nil then
		return
	end

	local icon_name = ""
	if race.type == UIEnums.MpRaceType.Racing then
		if race.laps > 0 then
			icon_name = "lap"
		else
			icon_name = "stopwatch"
		end
	elseif race.type == UIEnums.MpRaceType.Elimination then
		icon_name = "stopwatch"
	elseif race.type == UIEnums.MpRaceType.Points then
		if race.laps > 0 then
			icon_name = "lap"
		elseif race.score_limit > 0 then
			icon_name = "style_points"
		else
			icon_name = "stopwatch"
		end
	elseif race.type == UIEnums.MpRaceType.Destruction then
		icon_name = "stopwatch"
	end
	UIShape.ChangeObjectName( id, icon_name )
end

function Mp_RaceWinIcon( id, race )
	if race == nil then
		return
	end

	UIShape.ChangeObjectName( id, "end_of_race" )
end

function Mp_ReturnToPartyLobby()
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	
	-- [WillB] - close all screens
	FlushSubscreens()

	if UIGlobals.IsIngame == true then
		UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.PartyLobby
		GoScreen( "Loading\\LoadingUi.lua" )
	else
		if UIScreen.GetCurrentScreen( UIEnums.Context.CarouselApp ) ~= GUINetwork.multiplayer_party_root then
			GoScreen( GUINetwork.multiplayer_party_root, UIEnums.Context.CarouselApp )
		end
	end
end

function Mp_RefreshRaceOverview( node_id, race_data, race_index, centre_text )

	if race_index == nil then
		race_index = Multiplayer.GetCurrentRaceIndex()
	end

	local prefix = Select( (Amax.GetPlayMode() == UIEnums.PlayMode.Playlist), "MPL_PLAYLIST_RACE" .. race_index, "MPL_CUSTOM_RACE" )

	local city_id = UIButtons.FindChildByName( node_id, "city" )
	local route_id = UIButtons.FindChildByName( node_id, "route" )

	-- setup city and route name
	UIButtons.ChangeText( city_id, prefix .. "_CITY" )
	UIButtons.ChangeText( route_id, prefix .. "_ROUTE" )

	-- game mode and car class
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "mode" ), prefix .. "_TYPE" )
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "class" ), prefix .. "_CLASS" )

	-- setup game limits( either time or laps )
	local laps_id		= UIButtons.FindChildByName( node_id, "laps" )
	local max_laps_id	= UIButtons.FindChildByName( node_id, "num_laps" )
	local time_id		= UIButtons.FindChildByName( node_id, "time" )

	if race_data.type == UIEnums.MpRaceType.Racing or race_data.type == UIEnums.MpRaceType.Hardcore then
		if laps_id ~= nil and max_laps_id ~= nil and time_id ~= nil then
			UIButtons.SetActive( laps_id, true, true )
			UIButtons.SetActive( time_id, false, true )

			UIButtons.ChangeText( max_laps_id, prefix .. "_LAPS" )
			UIButtons.ChangeText( laps_id, prefix .. "_LAP_TEXT" )
		end
	else
		if laps_id ~= nil and max_laps_id ~= nil and time_id ~= nil then
			UIButtons.SetActive( laps_id, false, true )
			UIButtons.SetActive( time_id, true, true )

			UIButtons.ChangeText( time_id, prefix .. "_TIME" )
		end
	end

	-- change icons
	local city_icon_id = UIButtons.FindChildByName( node_id, "city_icon" )
	if city_icon_id ~= nil then
		UIShape.ChangeObjectName( city_icon_id, Amax.GetCityIconName( race_data.city ) )
	end
	
	local class_icon_id = UIButtons.FindChildByName( node_id, "class_icon" )
	if class_icon_id ~= nil then
		UIShape.ChangeObjectName( class_icon_id, UIGlobals.ClassIcons[race_data.class] )
	end

	Mp_RaceTypeIcon( UIButtons.FindChildByName( node_id, "mode_icon" ), race_data )

	-- setup thumbnail image
	local thumbnail_id = UIButtons.FindChildByName( node_id, "thumbnail" )
	if thumbnail_id ~= nil then
		Mp_RouteThumbnail( thumbnail_id, race_data.thumbnail, 1 )
	end

	-- offset the city and route based on the longest item
	if centre_text ~= false then
		local city_size_x = UIButtons.GetStaticTextLength( city_id )
		local route_size_x = UIButtons.GetStaticTextLength( route_id )

		local pos_x, pos_y = UIButtons.GetPosition( city_id )

		if city_size_x > route_size_x then
			UIButtons.ChangePosition( city_id, 2.5 - ( city_size_x * 0.5 ), pos_y, 0 )
		else
			UIButtons.ChangePosition( city_id, 2.5 - ( route_size_x * 0.5 ), pos_y, 0 )
		end
	end
end

function Mp_SetupPlayerProgressBar( root_id )
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

	local size_x, size_y, size_z = UIButtons.GetSize( UIButtons.FindChildByName( root_id, "backing" ) )
	UIButtons.ChangeSize( UIButtons.FindChildByName( root_id, "progress" ), size_x * amount, size_y, size_z )
end


-- should we allow the user to bring up the guide on xbox 360 to send an invite. 
-- party may be full or host may be out of lobby.
function Mp_XenonGameInviteAvailable()
	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if NetServices.GameInviteBeingSent( ) == false then
			if NetRace.CanSendGameInvite( ) == true then
				return true				
			end
		end
	end
	
	return false
end

function Mp_XenonPartyInviteAvailable()
	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if NetServices.GameInviteBeingSent( ) == false then
			if NetParty.CanSendGameInvite( ) == true then
				return true				
			end
		end
	end
	
	return false
end

function Mp_XenonPartyActive()
	if UIEnums.CurrentPlatform ~= UIEnums.Platform.Xenon then
		return false
	end
		
	return NetServices.IsInExternalPartySession()
end

function Mp_ResetCustomRace()
	local defaults = Multiplayer.GetCustomRaceDefaults()

	UIGlobals.CustomRaceSettings = 
	{
		num_players	= nil,
		game_mode	= defaults.game_mode,
		city_id		= defaults.city,
		route_id	= defaults.route,
		laps		= defaults.laps,
		time		= defaults.time,
		class		= defaults.class,
		damage		= defaults.damage,
		respawn		= defaults.respawn,
		timeout		= defaults.timeout,
		mods		= defaults.mods,
		upgrades	= defaults.upgrades,
		lock_host	= defaults.lock_host,
		ai			= defaults.ai,
		ai_padding	= defaults.ai_padding,
		handicap	= defaults.handicap,

		powerups =
		{
			nitro	= true,
			repair	= true,
			shock	= true,
			barge	= true,
			mine	= true,
			shunt	= true,
			bolt	= true,
			shield	= true,
		},
		powerup_respawn = defaults.powerup_respawn,
		random_powerups = defaults.random_powerups,
	}
end

function Mp_ReadCustomRaceSettings()
	local settings = Multiplayer.GetCustomRaceSettings()

	UIGlobals.CustomRaceSettings = 
	{
		num_players	= Select( Amax.GetGameMode() == UIEnums.GameMode.SplitScreen, #UIGlobals.Splitscreen.players, 1 ),
		game_mode	= settings.game_mode,
		city_id		= settings.city_id,
		route_id	= settings.route_id,
		laps		= settings.laps,
		time		= settings.time,
		class		= settings.class,
		damage		= settings.damage,
		respawn		= settings.respawn,
		timeout		= settings.timeout,
		mods		= settings.mods,
		upgrades	= settings.upgrades,
		lock_host	= settings.lock_host,
		ai			= settings.ai,
		ai_padding	= settings.ai_padding,
		handicap	= settings.handicap,

		powerups		= settings.powerups,
		powerup_respawn = settings.powerup_respawn,
		random_powerups = settings.random_powerups,
	}
end

function Mp_ResetSpectateVars()
	UIGlobals.Mp_SpectateCurrentVehicle = Amax.GetPlayerVehicleIndex( 0 )
	UIGlobals.Mp_SpectateCurrentCamera = 1
end
