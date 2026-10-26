GUI =
{
	finished = false,

	current_city	= -1,
	route_index		= -1,
	point_to_point	= nil,
	current_mode	= -1,

	start_x			= 0,
	start_y			= -5,
	row_spacing 	= 10,

	table_toggle	= false,
	selection		= -1,

	options = 
	{
		game_mode	= 1,
		city		= 2,
		route		= 3,
		limit		= 4,
		class		= 5,
	},

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK_CONFIRM, UIText.INFO_RT_ADVANCED )

	GUI.helptext =
	{
		[GUI.options.game_mode] = UIText.MP_CRHELP_GAMEMODE,
		[GUI.options.city]		= UIText.MP_CRHELP_LOCATION,
		[GUI.options.route]		= UIText.MP_CRHELP_TRACK,
		[GUI.options.limit]		= UIText.CMN_NOWT,
		[GUI.options.class]		= UIText.MP_CRHELP_CARCLASS,
	}

	GUI.table_id = SCUI.name_to_id["table"]
	GUI.limit_data = Multiplayer.GetCustomRaceLimits()
	
	Mp_ReadCustomRaceSettings() -- get latest settings
	
	CustomRace_CreateLeftColumn()
	CustomRace_CreateCentreColumn()
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["spinner_node"], false )
	UIButtons.SetActive( SCUI.name_to_id["item_node"], false )
	UIButtons.SetActive( SCUI.name_to_id["toggle_node"], false )

	UIButtons.ChangePanel( SetupScreenTitle( UIText.MP_CUSTOM_RACE_BASIC_TITLE, SCUI.name_to_id["centre"], "race_settings" ), UIEnums.Panel._3DAA_WORLD, true )
	GUI.helpline_id = SetupBottomHelpBar( UIText.MP_CRHELP_GAMEMODE )

	CustomRace_UpdateHelptext()
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )	

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext()

		-- if confirm is hit then we want to start save the settings
		local selection = UIButtons.GetTableSelection( GUI.table_id )

		GUI.table_toggle = not GUI.table_toggle
		UIButtons.ActivateTableElement( GUI.table_id, GUI.table_toggle )

	elseif message == UIEnums.Message.ButtonRightTrigger and arg2 == true then
		if UIButtons.TableElementActivated( GUI.table_id ) == false then
			CustomRace_CustomRaceTable()
	
			UISystem.PlaySound( UIEnums.SoundEffect.StickersPage )
			GoScreen( "Multiplayer\\Shared\\MpCustomRaceAdvanced.lua" )
		end

	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		-- Splitscreen and Online lobbys are on the same camera
		PlaySfxBack()

		if UIButtons.TableElementActivated( GUI.table_id ) == true then
			GUI.table_toggle = false
			UIButtons.ActivateTableElement( GUI.table_id, false )
		else			
			CustomRace_CustomRaceTable()
			Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )

			if Amax.GetGameMode() == UIEnums.GameMode.Online or Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
				NetRace.SendNewRaceSettings()
				PopScreen()

			elseif Amax.GetGameMode() == UIEnums.GameMode.SplitScreen then
				GoScreen( "Multiplayer\\MpSplitscreenLobby.lua" )
			end
			
			PlaySfxGraphicNext()
		end
	end
end

function FrameUpdate( time )

	-- Rebuild the limit data if mode changes
	local current_mode = UIButtons.GetSelection( GUI.game_mode_id )
	if GUI.current_mode ~= current_mode then
		GUI.current_mode = current_mode

		if GUI.current_mode == UIEnums.MpRaceType.Destruction then
			CustomRace_SetupTimeLimit( GUI.limit_id )
		else
			CustomRace_SetupLapLimit( GUI.limit_id )
		end

		CustomRace_BuildCityList( GUI.city_id, current_mode )
		GUI.current_city = -1 -- force this to refresh
	end

	-- Rebuild the route list if the city list changes
	local city_id = UIButtons.GetSelection( GUI.city_id )
	if GUI.current_city ~= city_id then
		GUI.current_city = city_id
		GUI.route_index = -1 -- force this to refresh

		CustomRace_BuildRouteList( GUI.route_id, current_mode )
	end

	-- Refresh the thumbnail and route map if route changes
	local route_index = UIButtons.GetSelectionIndex( GUI.route_id ) + 1
	if GUI.route_index ~= route_index then
		GUI.route_index = route_index

		-- dont want to trash SetActive() every frame so only if it needs updating
		local point_to_point = GUI.routes[route_index].point_to_point
		if point_to_point ~= GUI.point_to_point then
			GUI.point_to_point = point_to_point
			
			-- force to 1 and lock out laps for point to points
			UIButtons.SetActive( UIButtons.GetNodeID( GUI.table_id, GUI.options.limit ), not point_to_point )
			UIButtons.SetSelection( GUI.limit_id, Select( point_to_point, 1, UIGlobals.CustomRaceSettings.laps ) )
		end

		Mp_RouteThumbnail( SCUI.name_to_id["thumbnail"], GUI.routes[route_index].thumbnail, 1 )

		CustomRace_UpdateTrackMap()
	end

	CustomRace_UpdateHelptext()
end

function EnterEnd()
	RestoreInfoLine()
end

function End()

end

function CustomRace_UpdateHelptext()
	local selection = UIButtons.GetTableSelection( GUI.table_id )
	if selection ~= GUI.selection then
		GUI.selection = selection

		-- special case for limit because we reuse the spinner
		if selection == GUI.options.limit then
			local loc_string = Select( UIButtons.GetSelection( GUI.game_mode_id ) == UIEnums.MpRaceType.Destruction, UIText.MP_CRHELP_TIMELIMIT, UIText.MP_CRHELP_LAPS )
			UIButtons.ChangeText( GUI.helpline_id, loc_string )
			UIButtons.TimeLineActive( "HelpFade", true, 0.5 )
		else
			UIButtons.ChangeText( GUI.helpline_id, GUI.helptext[selection] )
			UIButtons.TimeLineActive( "HelpFade", true, 0.5 )
		end
	end
end

function CustomRace_CreateLeftColumn( )

	UIButtons.AddTableCol( GUI.table_id )

	-- add game modes
	GUI.game_mode_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.game_mode, UIText.MP_GAME_MODE, "retail_demo", "common_icons" )
	UIButtons.AddItem( GUI.game_mode_id, UIEnums.MpRaceType.Racing,			UIText.MP_RACE_TYPE_RACING, false )
	UIButtons.AddItem( GUI.game_mode_id, UIEnums.MpRaceType.Destruction,	UIText.MP_RACE_TYPE_DESTRUCTION, false )
	UIButtons.SetSelection( GUI.game_mode_id, UIGlobals.CustomRaceSettings.game_mode )

	-- add limit
	GUI.limit_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.limit, UIText.MP_LAPS, "lap", "common_icons" )
	CustomRace_SetupLapLimit( GUI.limit_id )

	-- add car class
	GUI.class_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.class, UIText.MP_CAR_CLASS, "all_classes", "common_icons" )
	UIButtons.AddItem( GUI.class_id, UIEnums.MpVehicleClass.ClassA, UIText.MP_CLASS_A, false )
	UIButtons.AddItem( GUI.class_id, UIEnums.MpVehicleClass.ClassB, UIText.MP_CLASS_B, false )
	UIButtons.AddItem( GUI.class_id, UIEnums.MpVehicleClass.ClassC, UIText.MP_CLASS_C, false )
	UIButtons.AddItem( GUI.class_id, UIEnums.MpVehicleClass.ClassD, UIText.MP_CLASS_D, false )
	UIButtons.AddItem( GUI.class_id, UIEnums.MpVehicleClass.Any, UIText.MP_CLASS_ANY, false )
	UIButtons.SetSelection( GUI.class_id, UIGlobals.CustomRaceSettings.class )
end

function CustomRace_CreateCentreColumn( )
	
	UIButtons.AddTableCol( GUI.table_id )

	-- add cities
	GUI.city_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.city, UIText.MP_LOCATION, "location", "fe_icons" )
	CustomRace_BuildCityList( GUI.city_id, UIGlobals.CustomRaceSettings.game_mode )

	-- resize city backing
	local city_node_id = UIButtons.GetNodeID( GUI.table_id, GUI.options.city )
	UIButtons.ChangeSize( UIButtons.FindChildByName( city_node_id, "background_box" ), 48, 6, 0 )
	UIButtons.ChangeSize( UIButtons.FindChildByName( city_node_id, "background_inner_box" ), 48, 6, 0 )

	-- add routes
	GUI.routes = Multiplayer.GetCustomRaceRoutes( GUI.current_city, UIGlobals.CustomRaceSettings.game_mode )
	GUI.route_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.route, UIText.MP_TRACK, "route", "fe_icons" )

	-- resize city backing
	local route_node_id = UIButtons.GetNodeID( GUI.table_id, GUI.options.route )
	UIButtons.ChangeSize( UIButtons.FindChildByName( route_node_id, "background_box" ), 48, 6, 0 )
	UIButtons.ChangeSize( UIButtons.FindChildByName( route_node_id, "background_inner_box" ), 48, 6, 0 )

	for i, v in ipairs( GUI.routes ) do
		UIButtons.AddItem( GUI.route_id, v.id, v.name, false )
	end

	local route_index = UIButtons.GetSelectionIndex( GUI.route_id ) + 1
	if GUI.route_index ~= route_index then
		GUI.route_index = route_index
		Mp_RouteThumbnail( SCUI.name_to_id["thumbnail"], GUI.routes[route_index].thumbnail, 1 )
	end
end

function CustomRace_SetupSpinner( table_id, value, name, icon_name, sheet_name )
	local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Shared\\MpCustomRace.lua", "spinner_node" )
	local spinner_id = UIButtons.FindChildByName( node_id, "spinner" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )

	Amax.ChangeText( spinner_id, name )

	UIShape.ChangeSceneName( icon_id, sheet_name )
	UIShape.ChangeObjectName( icon_id, icon_name )
	UIButtons.AddTableRow( table_id, node_id, value )

	return spinner_id
end

function CustomRace_BuildCityList( menu_id, mode )
	GUI.cities = Multiplayer.GetCustomRaceCities( mode )
	UIButtons.ClearItems( menu_id )
	for i, v in ipairs( GUI.cities ) do
		UIButtons.AddItem( menu_id, v.id, v.name, false )
	end
	UIButtons.SetSelectionByIndex( menu_id, 0 )
	UIButtons.SetSelection( menu_id, UIGlobals.CustomRaceSettings.city_id )
	GUI.current_city = UIGlobals.CustomRaceSettings.city_id

	Amax.SliderUpdateArrows( menu_id )
end

function CustomRace_BuildRouteList( menu_id, mode )
	GUI.routes = Multiplayer.GetCustomRaceRoutes( GUI.current_city, mode )

	UIButtons.ClearItems( menu_id )
	for i, v in ipairs( GUI.routes ) do
		UIButtons.AddItem( menu_id, v.id, v.name, false )
	end
	UIButtons.SetSelectionByIndex( menu_id, 0 )
	UIButtons.SetSelection( menu_id, UIGlobals.CustomRaceSettings.route_id )

	Amax.SliderUpdateArrows( menu_id )
end

function CustomRace_SetupLapLimit( menu_id )
	local icon_id = UIButtons.FindChildByName( UIButtons.GetNodeID( GUI.table_id, GUI.options.limit ), "icon" )

	Amax.ChangeText( menu_id, UIText.MP_LAPS )
	UIShape.ChangeObjectName( icon_id, "lap" )

	-- add laps
	UIButtons.ClearItems( menu_id )
	for i = 1, GUI.limit_data.max_laps do
		UIButtons.AddItem( menu_id, i, "GAME_NUM_" .. i, true )
	end
	UIButtons.SetSelection( menu_id, UIGlobals.CustomRaceSettings.laps )
	Amax.SliderUpdateArrows( menu_id )
end

function CustomRace_SetupTimeLimit( menu_id )
	local icon_id = UIButtons.FindChildByName( UIButtons.GetNodeID( GUI.table_id, GUI.options.limit ), "icon" )

	Amax.ChangeText( menu_id, UIText.MP_TIME_LIMIT )
	UIShape.ChangeObjectName( icon_id, "stopwatch" )

	-- add time
	UIButtons.ClearItems( menu_id )
	for i = GUI.limit_data.time_step, GUI.limit_data.max_time, GUI.limit_data.time_step do
		UIButtons.AddItem( menu_id, i, "GAME_TIME_" .. i, true )
	end
	UIButtons.SetSelection( menu_id, UIGlobals.CustomRaceSettings.time )
	Amax.SliderUpdateArrows( menu_id )
end

function CustomRace_UpdateTrackMap()
	local map_route_id		= SCUI.name_to_id["map_track"]
	local map_draw_id		= SCUI.name_to_id["map_track_draw"]
	local map_startline_id	= SCUI.name_to_id["map_startline"]

	local city = GUI.cities[UIButtons.GetSelectionIndex( GUI.city_id )+1]
	local route = GUI.routes[UIButtons.GetSelectionIndex( GUI.route_id )+1]

	UIButtons.SetActive( map_route_id, false )
	UIButtons.SetActive( map_draw_id, false )
	UIButtons.SetActive( map_startline_id, false )

	-- ids of all shapes that need swapping
	local map_ids = { map_route_id, map_draw_id, }
	
	for i, v in pairs( map_ids ) do
		-- route
		if UIShape.ChangeSceneName( v, city.debug_tag .. "_ui" ) == false then
			return
		end
		if UIShape.ChangeObjectName( v, route.debug_tag .. "_ui" ) == false then
			if UIShape.ChangeObjectName( v, route.debug_tag ) == false then
				return
			end
		end

		UIButtons.SetActive( v, true )
	end

	-- start line
	if UIShape.ChangeSceneName( map_startline_id, city.debug_tag .. "_ui" ) == true and UIShape.ChangeObjectName( map_startline_id, route.debug_tag .. "_start_line" ) == true then
		UIButtons.SetActive( map_startline_id, true )
	end
end

function CustomRace_CustomRaceTable()
	-- need to invalidate laps or time based on game mode
	local mode = UIButtons.GetSelection( GUI.game_mode_id )

	UIGlobals.CustomRaceSettings.num_players	= Select( Amax.GetGameMode() == UIEnums.GameMode.SplitScreen, #UIGlobals.Splitscreen.players, 1 )
	UIGlobals.CustomRaceSettings.game_mode		= UIButtons.GetSelection( GUI.game_mode_id )
	UIGlobals.CustomRaceSettings.city_id		= UIButtons.GetSelection( GUI.city_id )
	UIGlobals.CustomRaceSettings.route_id		= UIButtons.GetSelection( GUI.route_id )
	UIGlobals.CustomRaceSettings.laps			= Select( mode == UIEnums.MpRaceType.Destruction, 1, UIButtons.GetSelection( GUI.limit_id ) )
	UIGlobals.CustomRaceSettings.time			= Select( mode == UIEnums.MpRaceType.Destruction, UIButtons.GetSelection( GUI.limit_id ), 1 )
	UIGlobals.CustomRaceSettings.class			= UIButtons.GetSelection( GUI.class_id )

	Amax.SetupRace( UIGlobals.CustomRaceSettings )
end
