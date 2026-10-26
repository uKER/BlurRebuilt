GUI =
{
	finished = false,

	CanExit = function( _GUI )
		return false
	end,
	
	car_node_list_ids = {  },
	stat_ids = {  },
	ready_ids = {  },
	colour_ids = { {  }, },
	
	current_car_selection = {  },
	current_colour_selection = {  },
	previous_colour_selection = {  },

	ready_time		= 1,
	ready_countdown = 0,
	
	player_ready = {  },
	
	mode =
	{
		car = 0,
		colour = 1,
	},
	
	current_mode = {  },
	
	class_icon_names =
	{
		[ UIEnums.VehicleClass.A ] = "Class_A",
		[ UIEnums.VehicleClass.B ] = "Class_B",
		[ UIEnums.VehicleClass.C ] = "Class_C",
		[ UIEnums.VehicleClass.D ] = "Class_D",
		[ UIEnums.VehicleClass.E ] = "Class_E",
	},
	
	style_names =
	{
		[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
		[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
		[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
		[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
		[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
		[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
	},
}

function Init()
	Profile.LockToPad( -1 )
	
	AddSCUI_Elements()

	GUI.stencil_justify =
	{
		[SCUI.name_to_id[ "dummy_1" ]]		= UIEnums.Justify.BottomRight,
		[SCUI.name_to_id[ "dummy_2" ]]		= UIEnums.Justify.BottomLeft,
		[SCUI.name_to_id[ "dummy_3" ]]		= UIEnums.Justify.TopRight,
		[SCUI.name_to_id[ "dummy_4" ]]		= UIEnums.Justify.TopLeft,
		[SCUI.name_to_id[ "dummy_top" ]]	= UIEnums.Justify.BottomCentre,
		[SCUI.name_to_id[ "dummy_bottom" ]]	= UIEnums.Justify.TopCentre,
	}

	StoreInfoLine()
	SetupInfoLine()

	local cars = Multiplayer.GetSplitscreenVehicles()
	--show_table( GUI.cars, 1 )
	
	GUI.cars = {  }				-- sorted list
	GUI.car_info = {  }
	
	for i, v in pairs( cars ) do
		GUI.cars[ #GUI.cars + 1 ] = i
		GUI.car_info[ i ] = GameData.GetVehicle( i )
	end
	--show_table( GUI.cars, 1 )
	--show_table( GUI.car_info, 1 )
	
	-- sort by class, then by manufacturer, then by car name
	local sort_func = function( a, b )
		local car_info_a = GUI.car_info[ a ]
		local car_info_b = GUI.car_info[ b ]
		
		local class_a = car_info_a.class
		local class_b = car_info_b.class
		
		if class_a == class_b then
			local man_a = car_info_a.man_tag
			local man_b = car_info_b.man_tag
			
			if man_a == man_b then
				return ( car_info_a.tag < car_info_b.tag )
			end
			
			return ( man_a < man_b )
		end
		
		return ( class_a < class_b )
	end
	
	table.sort( GUI.cars, sort_func )
	--show_table( GUI.cars, 1 )
	
	GUI.car_colours = Multiplayer.GetSplitscreenResprays()
end

function PostInit()
	for i = 1, 4, 1 do
		UIButtons.SetActive( SCUI.name_to_id[ "dummy_" .. i ], false, true )
	end
	UIButtons.SetActive( SCUI.name_to_id[ "dummy_top" ], false, true )
	UIButtons.SetActive( SCUI.name_to_id[ "dummy_bottom" ], false, true )
	
	-- clone for each valid player
	for p, p_table in ipairs( UIGlobals.Splitscreen.players ) do
		local stat_parent_id = SCUI.name_to_id[ "stat_parent" ]

		local dummy_id = Splitscreen_GetDummyId( p )
		UIButtons.SetActive( dummy_id, true, true )

		if p ~= 1 then
			stat_parent_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpSplitscreenCarSelect.lua", "stat_parent" )
		end

		SplitscreenGarage_PushStencil( dummy_id, p )
		local car_node_list_parent_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpSplitscreenCarSelect.lua", "car_node_list_parent" )	
		local car_node_list_id	= UIButtons.FindChildByName( car_node_list_parent_id, "car_node_list" )
		SplitscreenGarage_PopStencil( dummy_id, p )

		-- cars
		for i, v in pairs( GUI.cars ) do
			local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpSplitscreenCarSelect.lua", "_car_node" )
			local item_id = UIButtons.FindChildByName( node_id, "_car_icon" )
			
			UIShape.ChangeSceneName( item_id, GUI.car_info[ v ].sheet )
			UIShape.ChangeObjectName( item_id, GUI.car_info[ v ].icon )
			
			UIButtons.AddListItem( car_node_list_id, node_id, i )
		end
		
		UIButtons.SetParent( stat_parent_id, dummy_id, UIEnums.Justify.MiddleCentre )
		UIButtons.SetParent( car_node_list_id, dummy_id, UIEnums.Justify.MiddleCentre )

		-- cache ids
		GUI.stat_ids[ p ] = 
		{
			car_name = UIButtons.FindChildByName( stat_parent_id, "stat_car_name" ),
			man_rect = UIButtons.FindChildByName( stat_parent_id, "stat_man_rect" ),
			man_square = UIButtons.FindChildByName( stat_parent_id, "stat_man_square" ),
			class_icon = UIButtons.FindChildByName( stat_parent_id, "stat_class_icon" ),
			style_text = UIButtons.FindChildByName( stat_parent_id, "stat_style_text" ),
			acceleration = UIButtons.FindChildByName( stat_parent_id, "chick_acceleration" ),
			speed = UIButtons.FindChildByName( stat_parent_id, "chick_speed" ),
			off_road = UIButtons.FindChildByName( stat_parent_id, "chick_off_road" ),
			difficulty = UIButtons.FindChildByName( stat_parent_id, "chick_difficulty" ),
			health = UIButtons.FindChildByName( stat_parent_id, "chick_health" ),
		}
		
		-- lock to pad
		UIButtons.SetPad( car_node_list_id, p_table.pad )
		
		if p_table.car_id == 0 then
			GUI.current_car_selection[ p ] = -1
			GUI.current_colour_selection[ p ] = -1
			GUI.previous_colour_selection[ p ] = -1
		else
			GUI.current_car_selection[ p ] = FindSelectionIdFromCarId( p_table.car_id )
			GUI.current_colour_selection[ p ] = FindSelectionIdFromColourId( p_table.colour_id )
			GUI.previous_colour_selection[ p ] = GUI.current_colour_selection[ p ]
			
			UIButtons.SetSelection( car_node_list_id,  GUI.current_car_selection[ p ] )
		end
				
		SplitscreenGarage_CreateColours( p )
		
		GUI.car_node_list_ids[ p ] = car_node_list_id
		GUI.player_ready[ p ] = false
		GUI.current_mode[ p ] = GUI.mode.car
		
		SplitscreenGarage_CreateReady( p )		
		SplitscreenGarage_UpdateStats( p )
	end

	Splitscreen_SetFrontendSplits()

	-- initial setup
	SplitscreenGarage_CheckForChange()
	SplitscreenGarage_UpdateColourPicker()
	SplitscreenGarage_CheckInfoLines()

	UIButtons.SetActive( SCUI.name_to_id["car_node_list_parent"], false, true )
end

function FindSelectionIdFromCarId( car_id )
	for i, v in ipairs( GUI.cars ) do
		if car_id == v then
			return i
		end
	end
	
	return -1
end

function FindSelectionIdFromColourId( colour_id )
	for i, v in ipairs( GUI.car_colours ) do
		if colour_id == v.id then
			return i
		end
	end
	
	-- default to manufacturer
	return -1
end

function FrameUpdate( time )

	SplitscreenGarage_CheckForChange()
	SplitscreenGarage_CheckInfoLines()

	if SplitscreenGarage_AllReady() == true then
		GUI.ready_countdown = GUI.ready_countdown - time

		if GUI.ready_countdown <= 0 then
			GUI.ready_countdown = GUI.ready_time -- reset it
			SplitscreenGarage_LoadRace()
		end
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	local player_index = UIGlobals.Splitscreen.pad_to_player[ arg1 ]
	
	-- host specific
	if arg1 == Profile.GetPrimaryPad() and GUI.current_mode[ player_index ] == GUI.mode.car then
		if message == UIEnums.Message.MenuBack then
			if GUI.player_ready[ player_index ] == false then
				PlaySfxBack()
				GoScreen( "Multiplayer\\MpSplitscreenLobby.lua" )
				return
			end

			GUI.ready_countdown = GUI.ready_time -- reset
		end
	end
	
	if GUI.current_mode[ player_index ] == GUI.mode.car then
		if message == UIEnums.Message.ButtonX then
			if GUI.player_ready[ player_index ] == false then
				local car_id = GUI.cars[ UIButtons.GetSelection( GUI.car_node_list_ids[ player_index ] ) ]
				local car_info = GUI.car_info[ car_id ]
				
				if car_info.respray == true then
					PlaySfxNext()
					
					GUI.current_mode[ player_index ] = GUI.mode.colour
					UIButtons.TimeLineActive( "colour_" .. Splitscreen_GetDummyName( player_index ), true )
					GUI.previous_colour_selection[ player_index ] = GUI.current_colour_selection[ player_index ]
					
					if GUI.current_colour_selection[ player_index ] == -1 then
						GUI.current_colour_selection[ player_index ] = player_index
						SplitscreenGarage_UpdateColourPicker( player_index )
					end
					
					UIButtons.SetSelected( GUI.car_node_list_ids[ player_index ], false )
				else
					PlaySfxError()
				end
			end
			
		elseif message == UIEnums.Message.MenuNext then
			if GUI.player_ready[ player_index ] == false then
				PlaySfxNext()
				
				GUI.player_ready[ player_index ] = true
				UIButtons.TimeLineActive( "ready_" .. Splitscreen_GetDummyName( player_index ), true )
				UIButtons.SetSelected( GUI.car_node_list_ids[ player_index ], false )

				if SplitscreenGarage_AllReady() == true then
					GUI.ready_countdown = GUI.ready_time -- reset
				end
			end
			
		elseif message == UIEnums.Message.MenuBack then
			if GUI.player_ready[ player_index ] == true then
				PlaySfxBack()
				
				GUI.player_ready[ player_index ] = false
				UIButtons.TimeLineActive( "ready_" .. Splitscreen_GetDummyName( player_index ), false )
				UIButtons.SetSelected( GUI.car_node_list_ids[ player_index ], true )
			end
		end
	
	elseif GUI.current_mode[ player_index ] == GUI.mode.colour then
		if message == UIEnums.Message.MenuNext then
			UISystem.PlaySound( UIEnums.SoundEffect.GarageReSpray )
			
			GUI.current_mode[ player_index ] = GUI.mode.car
			UIButtons.TimeLineActive( "colour_" .. Splitscreen_GetDummyName( player_index ), false )
			SplitscreenGarage_UpdateColour( player_index )
			UIButtons.SetSelected( GUI.car_node_list_ids[ player_index ], true )

			-- jump to ready
			GUI.player_ready[ player_index ] = true
			UIButtons.TimeLineActive( "ready_" .. Splitscreen_GetDummyName( player_index ), true )
			UIButtons.SetSelected( GUI.car_node_list_ids[ player_index ], false )
			
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			
			GUI.current_mode[ player_index ] = GUI.mode.car
			UIButtons.TimeLineActive( "colour_" .. Splitscreen_GetDummyName( player_index ), false )
			GUI.current_colour_selection[ player_index ] = GUI.previous_colour_selection[ player_index ]
			SplitscreenGarage_UpdateColourPicker( player_index, GUI.current_colour_selection[ player_index ] )
			UIButtons.SetSelected( GUI.car_node_list_ids[ player_index ], true )
			
		elseif message == UIEnums.Message.ButtonLeft then
			local old = GUI.current_colour_selection[ player_index ]
			
			SplitscreenGarage_UpdateColourPicker( player_index, GUI.current_colour_selection[ player_index ] - 1 )
			
			if GUI.current_colour_selection[ player_index ] ~= old then
				PlaySfxLeft()
			end
			
		elseif message == UIEnums.Message.ButtonRight then
			local old = GUI.current_colour_selection[ player_index ]
			
			SplitscreenGarage_UpdateColourPicker( player_index, GUI.current_colour_selection[ player_index ] + 1 )
			
			if GUI.current_colour_selection[ player_index ] ~= old then
				PlaySfxRight()
			end
		
		elseif message == UIEnums.Message.ButtonX then
			-- revert to manufacturer
			UISystem.PlaySound( UIEnums.SoundEffect.GarageReSpray )
			
			GUI.current_mode[ player_index ] = GUI.mode.car
			UIButtons.TimeLineActive( "colour_" .. Splitscreen_GetDummyName( player_index ), false )
			GUI.current_colour_selection[ player_index ] = -1
			SplitscreenGarage_UpdateColourPicker( player_index, GUI.current_colour_selection[ player_index ] )
			SplitscreenGarage_UpdateColour( player_index )
			UIButtons.SetSelected( GUI.car_node_list_ids[ player_index ], true )
		end
	end
end

function EnterEnd()
	RestoreInfoLine()
	Profile.LockToPad( Profile.GetPrimaryPad() )
end

function End()
	--
end

function SplitscreenGarage_LoadRace()
	-- set stuff for level state
	for p, p_table in ipairs( UIGlobals.Splitscreen.players ) do
		local car_id = GUI.cars[ UIButtons.GetSelection( GUI.car_node_list_ids[ p ] ) ]
		local car_info = GUI.car_info[ car_id ]
		
		p_table.car_id = car_id
		
		if GUI.current_colour_selection[ p ] == -1 or car_info.respray == false then
			p_table.colour_id = 0
		else
			p_table.colour_id = GUI.car_colours[ GUI.current_colour_selection[ p ] ].id
		end
	end

	Amax.SendMessage( UIEnums.GameFlowMessage.StopGameRendering )
	Amax.SendMessage( UIEnums.GameFlowMessage.StopGameUpdate )
			
	UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.SplitscreenLobby
	StoreScreen( UIEnums.ScreenStorage.FE_RETURN )
	
	UIGlobals.Splitscreen.primary_pad = Profile.GetPrimaryPad()
	
	UIGlobals.Multiplayer.LobbyLoad = true
	UIGlobals.Splitscreen.can_vote = ( Amax.GetPlayMode() == UIEnums.PlayMode.Playlist )
	
	PlaySfxNext()
	StopFrontendMusic()
	
	UIGlobals.IsLoading = true
	
	GoLoadingScreen( "Loading\\LoadingMpGame.lua" )
end

function SplitscreenGarage_CreateReady( player_index )
	local ready_dummy_id = SCUI.name_to_id[ "ready_dummy_" .. Splitscreen_GetDummyName( player_index ) ]
	
	local class_icon_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpSplitscreenCarSelect.lua", "_ready_class_icon" )
	UIButtons.SetParent( class_icon_id, ready_dummy_id, UIEnums.Justify.MiddleCentre )
	
	-- cache ids
	GUI.ready_ids[ player_index ] = 
	{
		class_icon = class_icon_id,
		car_icon = UIButtons.FindChildByName( class_icon_id, "_ready_car_icon" ),
		car_name = UIButtons.FindChildByName( class_icon_id, "_ready_car_name" ),
		man_rect = UIButtons.FindChildByName( class_icon_id, "_ready_man_rect" ),
		man_square = UIButtons.FindChildByName( class_icon_id, "_ready_man_square" ),
		colour_frame = UIButtons.FindChildByName( class_icon_id, "_car_colour_frame" ),
		colour_fill = UIButtons.FindChildByName( class_icon_id, "_car_colour" ),
	}

	-- change ready text to be the players colour
	local p_table = UIGlobals.Splitscreen.players[ player_index ]
	UIButtons.ChangeColour( UIButtons.FindChildByName( class_icon_id, "_ready_text" ), UIGlobals.Splitscreen.colours[ p_table.pad ] )
end

function SplitscreenGarage_CreateColours( player_index )
	local colour_square = SCUI.elements[ SCUI.name_to_index[ "_colour_square" ] ]
	local parent_id = SCUI.name_to_id[ "colour_dummy_" .. Splitscreen_GetDummyName( player_index ) ]
	
	local mult = 0.9
	local offset_x = colour_square.pos.x - ( colour_square.size.x * mult * ( #GUI.car_colours - 1 ) ) / 2
	
	GUI.colour_ids[ player_index ] = {  }
	
	for i, v in ipairs( GUI.car_colours ) do
		local pos_x_original = colour_square.pos.x
		
		colour_square.pos.x = pos_x_original + offset_x
		colour_square.colour_style = "!" .. v.colour
		
		local colour_square_id = UIButtons.AddButton( colour_square )
		UIButtons.SetParent( colour_square_id, parent_id, UIEnums.Justify.MiddleCentre )
		
		offset_x = offset_x + colour_square.size.x * mult
		
		colour_square.pos.x = pos_x_original
		
		GUI.colour_ids[ player_index ][ i ] = colour_square_id
	end
	
	-- background
	local colour_square_background = SCUI.elements[ SCUI.name_to_index[ "_colour_square_background" ] ]
	local colour_square_background_id = UIButtons.AddButton( colour_square_background )
	
	UIButtons.SetParent( colour_square_background_id, parent_id, UIEnums.Justify.MiddleCentre )
	
	GUI.colour_ids[ player_index ].background_id = colour_square_background_id
	GUI.colour_ids[ player_index ].size_x = colour_square.size.x
end

function SplitscreenGarage_CheckForChange()
	for p, p_table in ipairs( UIGlobals.Splitscreen.players ) do
		local car_selected = UIButtons.GetSelection( GUI.car_node_list_ids[ p ] )
		if GUI.current_car_selection[ p ] ~= car_selected then
			GUI.current_car_selection[ p ] = car_selected
			SplitscreenGarage_UpdateStats( p )
		end
	end
end

function SplitscreenGarage_UpdateStats( player_index )
	local car_id = GUI.cars[ UIButtons.GetSelection( GUI.car_node_list_ids[ player_index ] ) ]
	local car_info = GUI.car_info[ car_id ]
	local stat_ids = GUI.stat_ids[ player_index ]
	local ready_ids = GUI.ready_ids[ player_index ]
	
	-- man badge
	if car_info.man_texture_square == true then
		UIButtons.SetActive( stat_ids.man_square, true )
		UIButtons.SetActive( stat_ids.man_rect, false )
		UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, stat_ids.man_square )
		-- ready
		UIButtons.SetActive( ready_ids.man_square, true )
		UIButtons.SetActive( ready_ids.man_rect, false )
		UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, ready_ids.man_square )
	else
		UIButtons.SetActive( stat_ids.man_square, false )
		UIButtons.SetActive( stat_ids.man_rect, true )
		UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, stat_ids.man_rect )
		-- ready
		UIButtons.SetActive( ready_ids.man_square, false )
		UIButtons.SetActive( ready_ids.man_rect, true )
		UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, ready_ids.man_rect )
	end
	
	UIShape.ChangeObjectName( stat_ids.class_icon, GUI.class_icon_names[ car_info.class ] )
	UIShape.ChangeObjectName( ready_ids.class_icon, GUI.class_icon_names[ car_info.class ] )
	
	UIShape.ChangeSceneName( ready_ids.car_icon, car_info.sheet )
	UIShape.ChangeObjectName( ready_ids.car_icon, car_info.icon )
	
	UIButtons.ChangeText( stat_ids.car_name, car_info.name )
	UIButtons.ChangeText( ready_ids.car_name, car_info.name )
	UIButtons.ChangeText( stat_ids.style_text, GUI.style_names[ car_info.style ] )
		
	Amax.SetChickletsValue( stat_ids.acceleration, car_info.acceleration )
	Amax.SetChickletsValue( stat_ids.speed, car_info.speed )
	Amax.SetChickletsValue( stat_ids.off_road, car_info.stability )
	Amax.SetChickletsValue( stat_ids.difficulty, car_info.difficulty )
	Amax.SetChickletsValue( stat_ids.health, car_info.strength )
		
	-- respray
	local dummy_name = Splitscreen_GetDummyName( player_index )
	
	if car_info.respray == true then
		UIButtons.TimeLineActive( "respray_" .. dummy_name, true )
		SplitscreenGarage_UpdateColour( player_index )
	else
		UIButtons.TimeLineActive( "respray_" .. dummy_name, false )
		SplitscreenGarage_UpdateColour( player_index, -1 )	-- temp back to manufacturer
	end
end

function SplitscreenGarage_UpdateColourPicker( player_index, new_option )
	if player_index == nil then
		-- update all
		for p, p_table in ipairs( UIGlobals.Splitscreen.players ) do
			SplitscreenGarage_UpdateColourPicker( p )
			SplitscreenGarage_UpdateColour( p )
		end

		return
	end
	
	if new_option == nil then
		new_option = GUI.current_colour_selection[ player_index ]
	end
	
	if new_option == nil then
		new_option = player_index
	end
	
	if new_option ~= -1 then
		new_option = Clamp( new_option, 1, #GUI.car_colours )
	end
	
	GUI.current_colour_selection[ player_index ] = new_option
	
	if new_option == -1 then
		new_option = 1
	end
	
	-- update background
	local size_x = GUI.colour_ids[ player_index ].size_x
	local back_id = GUI.colour_ids[ player_index ].background_id
	
	local mult = 0.9
	local new_x = -( size_x * mult * ( #GUI.car_colours - 1 ) ) / 2 + ( size_x * mult * ( new_option - 1 ) )
	
	local x, y, z = UIButtons.GetPosition( back_id )
	UIButtons.ChangePosition( back_id, new_x, y, z )
end

function SplitscreenGarage_UpdateColour( player_index, colour_override )
	local current_colour = GUI.current_colour_selection[ player_index ]
	local ready_ids = GUI.ready_ids[ player_index ]
	
	if colour_override ~= nil then
		current_colour = colour_override
	end
	
	local colour_frame_id = SCUI.name_to_id[ "colour_frame_" .. Splitscreen_GetDummyName( player_index ) ]
	local colour_id	= SCUI.name_to_id[ "colour_" .. Splitscreen_GetDummyName( player_index ) ]
	
	if current_colour == -1 then
		-- update lobby colour
		UIButtons.SetActive( colour_frame_id, false, true )

		-- update ready colour
		UIButtons.SetActive( ready_ids.colour_frame, false, true )
	else
		-- update lobby colour
		UIButtons.SetActive( colour_frame_id, true, true )
		local colour = "!" .. GUI.car_colours[ current_colour ].colour	
		UIButtons.ChangeColour( colour_id, colour )

		-- update ready colour
		UIButtons.SetActive( ready_ids.colour_frame, true, true )
		local colour = "!" .. GUI.car_colours[ current_colour ].colour	
		UIButtons.ChangeColour( ready_ids.colour_fill, colour )
	end
end

function SplitscreenGarage_AllReady( not_ready )
	if not_ready == nil then
		not_ready = false
	end
	
	for i, v in ipairs( GUI.player_ready ) do
		if v == not_ready then
			return false
		end
	end
	
	return true
end

function SplitscreenGarage_CheckInfoLines()
	if SplitscreenGarage_AllReady() == true then
		SetupInfoLine( UIText.INFO_B_BACK )
	else
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_SPRAY_X )
	end
end

function SplitscreenGarage_PushStencil( parent, index )
	local stencil_write_on	= SCUI.elements[ SCUI.name_to_index[ "_stencil_write_on" ] ]
	local stencil_read_on	= SCUI.elements[ SCUI.name_to_index[ "_stencil_read_on" ] ]
	local stencil			= SCUI.elements[ SCUI.name_to_index[ "_stencil_" .. Splitscreen_GetDummyName( index ) ] ]

	local write_on_id = UIButtons.AddButton( stencil_write_on )
	local stencil_id = UIButtons.AddButton( stencil )
	local read_on_id = UIButtons.AddButton( stencil_read_on )

	UIButtons.SetXtVar( write_on_id, "render_state.stencil_state.ref", index )
	UIButtons.SetXtVar( read_on_id, "render_state.stencil_state.ref", index )

	local justify = GUI.stencil_justify[parent]
	UIButtons.SetParent( write_on_id, parent, justify )
	UIButtons.SetParent( stencil_id, parent, justify )
	UIButtons.SetParent( read_on_id, parent, justify )
end

function SplitscreenGarage_PopStencil( parent, index )
	local stencil_read_off	= SCUI.elements[ SCUI.name_to_index[ "_stencil_read_off" ] ]
	local read_off_id = UIButtons.AddButton( stencil_read_off )

	UIButtons.SetParent( read_off_id, parent, GUI.stencil_justify[parent] )
	UIButtons.SetXtVar( read_off_id, "render_state.stencil_state.ref", index )
end
