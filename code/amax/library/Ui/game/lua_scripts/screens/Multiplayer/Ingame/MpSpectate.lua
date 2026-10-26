GUI =
{
	finished = false,
	
	current_vehicle = -1,
	current_camera = -1,
	
	max_vehicles = -1,
	picture_map = {  },
	
	current_vehicle_data = {  },
}

function Init()
	SetupFadeToBlack( nil, true )
	UIScreen.SetScreenTimers( 0.5, 0.5 )
	
	AddSCUI_Elements()
	
	Camera_UseInGame()
	
	Amax.SendMessage( UIEnums.GameFlowMessage.StartSpectatorMode )
	
	-- helper functions
	UISystem.LoadLuaScript( "Screens\\Ingame\\HUD_Helpers.lua" )
	
	-- info lines
	if NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true then
		SetupInfoLine( UIText.INFO_VIEW_GAMER_CARD, UIText.INFO_QUIT_RACE, UIText.INFO_CHANGE_CAMERA, UIText.INFO_VIEW_SCOREBOARD )
	else
		SetupInfoLine( UIText.INFO_QUIT_RACE, UIText.INFO_CHANGE_CAMERA, UIText.INFO_VIEW_SCOREBOARD )
	end
	
	-- restore state from globals
	GUI.current_vehicle = UIGlobals.Mp_SpectateCurrentVehicle
	GUI.current_camera = UIGlobals.Mp_SpectateCurrentCamera
	
	GUI.max_vehicles = Amax.GetNumVehicles()
	GUI.picture_map = Profile.GetRemoteGamerPictureMap()
	
	-- grab info
	GUI.current_vehicle_data = Amax.GetSpectateVehicleData( GUI.current_vehicle )
	
	-- set cameras
	Amax.SpectateVehicle( GUI.current_vehicle )
	Multiplayer.SwitchSpectateCam( GUI.current_camera, GUI.current_vehicle )
end

function PostInit()
	AddCutSceneBars()
	
	-- setup player viewports
	GUI.num_players = 1
	HUD.SetupPlayerViewports()
	
	local inter_2d = HUD.ViewportId[ 1 ]
	local inter_3d = HUD.ViewportId3d[ 1 ]
	
	local title_id = SetupScreenTitle( UIText.MP_SPECTATING, inter_3d, "effects", "common_icons", 1, UIEnums.Justify.TopCentre, true, nil, UIEnums.Panel._3DAA_0, nil, UIEnums.Justify.TopCentre )
	UIButtons.ChangeScale( title_id, 0.7, 0.7, 1 )
	
	-- scoreboard
	GUI.scoreboard_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Mp_DataFullScreen" )
	
	local only_show_finished_cars = true
	
	if Amax.IsRaceMpDestruction() == true then
		-- motor mash show everyone
		only_show_finished_cars = false
		
	elseif Amax.GetPlayMode() == UIEnums.PlayMode.CustomRace then
		-- if we've got respawn off show everyone
		if Multiplayer.GetCustomRaceSettings().respawn == 0 then
			only_show_finished_cars = false
		end
	end
	
	if only_show_finished_cars == true then
		UIButtons.SetXtVar( GUI.scoreboard_id, "only_show_finished_cars", true )
	end
	
	UIButtons.SetXtVar( GUI.scoreboard_id, "show_fullscreen_title", false )
	UIButtons.SetXtVar( GUI.scoreboard_id, "show_fullscreen_target", false )
	UIButtons.ChangePosition( GUI.scoreboard_id, nil, -30, nil, true )
	
	-- timer
	GUI.sp = {  }
	local timer_root_id = HUD_SpCreateTimer( inter_2d, "HUD_0_MP_TIMEOUT" ) -- timeout is the same for all players
	UIButtons.ChangePosition( timer_root_id, nil, -15, nil, true )
	
	-- intial timer state
	MpSpectate_UpdateTimeout()
	
	-- speaking players
	local speaking = UIButtons.CloneXtGadgetByName( "hud_objects", "Mp_Speaking" )
	UIButtons.SetParent( speaking, inter_3d, UIEnums.Justify.BottomLeft )
	UIButtons.ChangePosition( speaking, nil, -1, nil, true )
	
	-- minimap
	local map_dmy = UIButtons.CloneXtGadgetByName( "hud_objects", "dmy_MiniMapScale" )
	local mini_map = UIButtons.CloneXtGadgetByName( "hud_objects", "Map_Root" )
	UIButtons.SetParent( mini_map, inter_3d, UIEnums.Justify.TopRight )
	Amax.ChangeText( UIButtons.FindChildByName( mini_map, "Map_SpeedNumber" ), "HUD_" .. 0 .. "_SPEED_MPH" )
	
	local rt_panel = UIEnums.Panel._RT0 + 0
	UIButtons.ChangePanel( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad" ),	rt_panel )
	UIButtons.ChangePanel( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad_Alt" ), rt_panel )
			
	UIButtons.SetXtVar( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad" ), "name", "shp_MiniMapRoad" .. 0 )
	UIButtons.SetXtVar( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad_Alt" ), "name", "shp_MiniMapRoad_Alt" .. 0 )
	UIButtons.SetXtVar( UIButtons.FindChildByName( map_dmy, "dmy_MiniMapTranslate" ), "name", "dmy_MiniMapTranslate" .. 0 )
	UIButtons.SetXtVar( map_dmy, "name", "dmy_MiniMapScale" .. 0 )
	
	local mini_map_id = UIButtons.FindChildByName( mini_map, "Map_Main" )
	UIButtons.ChangeTexture( { filename = "MINIMAP_RT" .. 0 }, 5, mini_map_id )
	
	if Amax.IsRaceMpDestruction() == true then
		-- points
		local backing_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
		UIButtons.SetParent( backing_id, inter_3d, UIEnums.Justify.TopLeft )
		
		local title_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Points_Title" )
		UIButtons.SetParent( title_id, inter_3d, UIEnums.Justify.TopLeft )
		
		UIButtons.ChangeText( UIButtons.FindChildByName( title_id, "Points_Amount" ), "HUD_" .. 0 .. "_MP_CURRENT_POINTS" )
		UIButtons.SetActive( UIButtons.FindChildByName( title_id, "SpDestructionPointsSmall" ), false )
	else
		local pos_backing = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
		UIButtons.SetParent( pos_backing, inter_3d, UIEnums.Justify.TopLeft )
			
		-- Position indicator
		local pos = UIButtons.CloneXtGadgetByName( "hud_objects", "Position_Title" )
		UIButtons.SetParent( pos, inter_3d, UIEnums.Justify.TopLeft )
		
		local pos_amount = UIButtons.FindChildByName( pos, "Position_Amount" )
		Amax.ChangeText( pos_amount, "HUD_" .. 0 .. "_POS_NUM" )
		
		local pos_amount_of = UIButtons.FindChildByName( pos, "Position_AmountOf" )
		Amax.ChangeText( pos_amount_of, "HUD_" .. 0 .. "_POS_NUM_OF" )
		
		-- Lap counter
		local laps_title = UIButtons.CloneXtGadgetByName( "hud_objects", "Map_Laps" )
		UIButtons.SetParent( laps_title, mini_map, UIEnums.Justify.TopRight )
		
		local laps_num = UIButtons.FindChildByName( laps_title, "Map_LapsNum" )
		Amax.ChangeText( laps_num,	"HUD_" .. 0 .. "_LAP" )
	end
	
	-- must set local player to me
	UIScreen.AddMessageNow( UIEnums.GameMessage.SetLocalPlayerIndex, 0 )
	
	-- player info
	UIButtons.SetParent( SCUI.name_to_id[ "gamer_picture" ], inter_3d, UIEnums.Justify.BottomLeft )
	UIButtons.SetParent( SCUI.name_to_id[ "info_change_player" ], inter_3d, UIEnums.Justify.BottomCentre )
	
	MpSpectate_UpdatePlayerInfo()
	
	-- team colours
	if NetRace.IsTeamGame() == true then
		local team = Select( ( GUI.current_vehicle_data.team == 0 ), "team_a", "team_b" )
		UIButtons.SwapColour( inter_2d, "Main_1", UIGlobals.TeamColours[ team ] )
		UIButtons.SwapColour( inter_3d, "Main_1", UIGlobals.TeamColours[ team ] )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext and NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true then
		NetServices.ViewGamerCard( Profile.GetPrimaryPad(), GUI.current_vehicle_data.join_ref, false, true )

	elseif message == UIEnums.Message.ButtonRightTrigger then
		MpSpectate_ChangeSpectatedCar( 1 )

	elseif message == UIEnums.Message.ButtonLeftTrigger then
		MpSpectate_ChangeSpectatedCar( -1 )

	elseif message == UIEnums.Message.MenuBack then
		SetupCustomPopup( UIEnums.CustomPopups.ExitRace )
		
	elseif message == UIEnums.Message.ButtonY then
		MpSpectate_ChangeCamera()
		
	elseif message == UIEnums.MiscMessage.FinishedAnimatedCamera then
		if arg1 == 0 and arg2 == false then
			MpSpectate_ChangeCamera( true )
		end
	end
end

function FrameUpdate( time )
	-- timer
	MpSpectate_UpdateTimeout()
	
	if net_Disconnecting() == true then
		return
	end

	-- go if we're finished
	if Multiplayer.RaceFinished() == true and NetRace.CanShowWinner() == true then
		GoScreen( "Multiplayer\\Ingame\\MpWinner.lua" )
	end
	
	-- car we are looking at is wrecked/eliminated
	if Amax.SpectatedVehicleIsEliminated( GUI.current_vehicle ) == true then
		MpSpectate_ChangeSpectatedCar( 1 )
	end
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	Camera_UseFrontend()
	
	-- update globals
	UIGlobals.Mp_SpectateCurrentVehicle = GUI.current_vehicle
	UIGlobals.Mp_SpectateCurrentCamera = GUI.current_camera
end

function MpSpectate_UpdatePlayerInfo()
	local gamerpic_id = SCUI.name_to_id[ "gamer_picture" ]
	local gamertag_id = SCUI.name_to_id[ "gamertag" ]

	-- gamerpic
	if GUI.current_vehicle == 0 then
		UIButtons.ChangeColour( gamertag_id, "Support_4" )
		LocalGamerPicture( gamerpic_id, Profile.GetPrimaryPad() )

	elseif GUI.current_vehicle_data.ai_driver == true then
		UIButtons.ChangeColour( gamertag_id, "Support_2" )
		AIGamerPicture( gamerpic_id, GUI.current_vehicle_data.ai_avatar_id )
		
	else
		UIButtons.ChangeColour( gamertag_id, "Main_2" )
		RemoteGamerPicture( gamerpic_id, GUI.picture_map[ GUI.current_vehicle_data.join_ref ] )
	end
	
	-- name and rank
	UIButtons.ChangeText( gamertag_id, "MPL_SPECTATE_NAME" .. GUI.current_vehicle )
	UIButtons.ChangeText( SCUI.name_to_id[ "rank" ], "MPL_SPECTATE_RANK" .. GUI.current_vehicle )
	
	-- rank icon
	Mp_RankIcon( SCUI.name_to_id[ "rank_icon" ], GUI.current_vehicle_data.rank, GUI.current_vehicle_data.legend )
	
	-- mods
	if #GUI.current_vehicle_data.mods == 0 then
		UIButtons.SetActive( SCUI.name_to_id[ "loadout_1" ], false, true )
	else
		for i, v in ipairs( GUI.current_vehicle_data.mods ) do
			UIButtons.ChangeTextureUV( SCUI.name_to_id[ "loadout_" .. i ], 0, 0.03125 * v.type )
			UIButtons.ChangeColour( SCUI.name_to_id[ "loadout_" .. i ], UIGlobals.CategoryColour[ v.category ] )
		end
	end	
end

function MpSpectate_ChangeCamera( auto )
	GUI.current_camera = GUI.current_camera + 1
	
	if GUI.current_camera == 6 then
		GUI.current_camera = Select( auto, 1, 0 )
	end
	
	Multiplayer.SwitchSpectateCam( GUI.current_camera, GUI.current_vehicle )
end

function MpSpectate_ChangeSpectatedCar( num )
	local count = 0
	local original = GUI.current_vehicle
	
	local DoNext = function()
		local current_position = Multiplayer.GetVehiclePosition( GUI.current_vehicle )
		local new_position = current_position + num
		
		if new_position > GUI.max_vehicles then
			new_position = 1
		elseif new_position < 1 then
			new_position = GUI.max_vehicles
		end
		
		local new_vehicle_index = Multiplayer.GetVehicleIndexByPosition( new_position )	
		GUI.current_vehicle = new_vehicle_index
	end
	
	DoNext()

	while Amax.CanSpectateVehicle( GUI.current_vehicle ) == false do
		DoNext()
		count = count + 1

		if count == 20 then
			GUI.current_vehicle = original
			break
		end
	end
	
	if GUI.current_vehicle ~= original then
		GoScreen( "Multiplayer\\Ingame\\MpSpectate.lua" )	-- just full refresh
	end
end

function MpSpectate_UpdateTimeout()
	local mp_timeout = Amax.MP_GetTimeout()
	HUD_SpTimerUpdate( mp_timeout )
end
