GUI =
{
	enable_respray = true,
	enable_boss_mods = false,
	
	finished = false,
	carousel_branch = "Garage",
	
	save_on_exit = false,
	
	is_multiplayer = false,
	is_team_game = false,
	local_team = nil,
	is_multiplayer_lobby = false,
	mp_class_restriction = 0,
	timeout_id = -1,
	is_sp_event = false,
	sp_event_id = -1,
	custom_filter = false,
	dummy_boss_car_rank = 999,
	dummy_boss_car_rank_no_mod = 888,
	
	viewing_id_old = -1,
	viewing_id = -1,
	
	wait_load_time = 0.5,
	wait_load_timer = 0,
	wait_respray_time = 0.825,
	wait_respray_timer = 0,
	
	boss_mod_node_list_id = -1,
	boss_mod_rb_id = -1,
	boss_mod_icon_id = -1,
	current_mod = -1,
	
	class_icon_names = {  },
	class_node_list_id = -1,
	
	colour_picker_id = -1,
	
	mode =
	{
		car = 0,
		colour = 1,
		boss_mods = 2,
	},
	
	current_mode = 0,
	
	viewing_challenges = false,
	viewing_challenges_prev = false,
	
	current_palette = -1,
	palette_ids = {  },
	palette_locked_table = {  },
	
	class = 
	{
		--[[
		{ 
			class = nil, 
			cars = {  }, 
			ids = { node = nil, icon = nil, text = nil, }, 
		}, 
		--]]
	},
	
	current =
	{
		class = nil, 
		car = {  }, 
	},
	
	-- multiplayer extras
	challenge_rewards = {  },
	reward_icons = {  },
	challenges = { "repeat_offender", "winner", },
	
	primary_colour_sp	= "Main_1",
	secondary_colour_sp = "Main_1",
	primary_colour_mp	= "Support_1",
	secondary_colour_mp	= "Support_1",	-- yes
	primary_colour		= "Main_1",
	secondary_colour	= "Main_1",
	
	--bottom_help_id = -1,
	
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	UIGlobals.SpViewingGarage = true
	
	AddSCUI_Elements()
	DeferCam_Init( UIEnums.ExtendedCameraType.IntoGarage, 0.1, 0.6 )
	-- Reset garage camera
	UIScreen.AddMessage( UIEnums.CutScene.StartGarageCamera )
	-- Don't allow user control for a bit longer due to long camera transitions
	UIScreen.SetScreenTimers( 0.6, 0.35 )
		
	StoreInfoLine()
	SetupMenuOptions()
	
	UISystem.PlaySound( UIEnums.SoundEffect.GarageEnter )
	UIButtons.TimeLineActive( "garage_start", true )
	UIButtons.TimeLineActive( "Hide_LightPaint", true )
	
	GUI.is_multiplayer = ( Amax.GetGameMode() ~= UIEnums.GameMode.SinglePlayer )
		
	if GUI.is_multiplayer == true then
		if UIGlobals.Multiplayer.InLobby == true then
			GUI.is_team_game = NetRace.IsTeamGame()
			GUI.local_team = NetRace.GetLocalPlayerTeam()
			
			GUI.enable_respray = ( GUI.enable_respray and GUI.is_team_game == false )
		
			GUI.is_multiplayer_lobby = true
			NetRace.ChangeLocalPlayerState( Profile.GetPrimaryPad(), UIEnums.Network.PlayerStates.eNetPlayerStateChangingCar )
			UIGlobals.Multiplayer.GarageActive = true
			GUI.timeout_id = SCUI.name_to_id[ "countdown" ]
		end
	end
	
	GUI.resprays = GameProfile.GetResprays( GUI.is_multiplayer )
	
	if GUI.is_multiplayer == false then
		GUI.is_sp_event = ( UIGlobals.SpSelectMode ~= nil )
		
		if GUI.is_sp_event == true then
			GUI.sp_event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
			GUI.sp_event_info = SinglePlayer.EventInfo( GUI.sp_event_id )
			--show_table( GUI.sp_event_info )
		end
		
		GUI.sp_game_info = SinglePlayer.GameInfo()
		GUI.sp_tier_info = SinglePlayer.TierInfo()
		GUI.sp_boss_info = SinglePlayer.BossInfo()
		GUI.sp_rank_info = SinglePlayer.RankInfo()
	end
	
	if (GUI.is_multiplayer == false and GUI.is_sp_event == false) or (GUI.is_multiplayer == true and GUI.is_multiplayer_lobby == false) then
		net_SetRichPresence( UIEnums.RichPresence.Showroom )
	end
	
	-- store current car in the background
	if GUI.is_multiplayer == true then
		if GUI.is_multiplayer_lobby == true then
			for i, v in pairs( UIGlobals.GarageMiniClassFilter ) do
				GUI.mp_class_restriction = i	-- should only ever have one of these anyway
				break
			end

			GUI.viewing_id_old = Multiplayer.GetGarageCarByID( GUI.mp_class_restriction )
		else
			GUI.viewing_id_old = Multiplayer.GetGarageCarByID()
		end
	else
		if GUI.is_sp_event == true then
			GUI.viewing_id_old = GUI.sp_event_info.lastVehicle
		else
			GUI.viewing_id_old = GUI.sp_game_info.lastVehicle
		end
	end
	
	GUI.viewing_id = GUI.viewing_id_old
	
	-- grab car data
	if GUI.is_multiplayer == true then
		GUI.garage = Multiplayer.GetGarage( GUI.is_multiplayer_lobby )
		GUI.garage_new = Multiplayer.GetNewCarsMap()
		GUI.garage.car_info = {  }
		
		for i, v in pairs( GUI.garage.cars ) do
			GUI.garage.car_info[ i ] = GameData.GetVehicle( i )
			
			if GUI.garage.car_info[ i ] == nil then
				print( "ERROR: no vehicle info for id", i )
				GUI.garage.cars[ i ] = nil
			else
				GUI.garage.cars[ i ].id = GUI.garage.car_info[ i ].id
			end
			
			if GUI.is_multiplayer_lobby == true then
				-- teams restrictions
				if GUI.is_team_game == true then
					GUI.garage.cars[ i ].allowed = ( GUI.garage.team_restrictions[ i ] == nil )
				end
				
				-- class filter
				if GUI.mp_class_restriction ~= UIEnums.VehicleClass.ANY and GUI.garage.car_info[ i ].class ~= GUI.mp_class_restriction then
					GUI.garage.cars[ i ] = nil
				end
			end
		end
	else
		GUI.garage = {  }
		GUI.garage.garage = {  }
		GUI.garage.rank = {  }
		GUI.garage.car_info = {  }
		GUI.garage.cars = SinglePlayer.VehicleInfo()
		GUI.garage.boss_mods = {  }
		GUI.garage.boss_mods_inv = {  }
		GUI.garage.boss_mods_unlocked = {  }
		GUI.garage.boss_mods_unlocked_inv = {  }
		
		-- allow sp mods if we've beat the last boss or we're doing a race or boss event
		if GUI.is_sp_event == false or ( --[[GUI.sp_boss_info[ #GUI.sp_boss_info ].defeated == true or]] GUI.sp_event_info.kind == "race" or GUI.sp_event_info.kind == "boss" ) then
			GUI.enable_boss_mods = true
		end
		
		for i, v in pairs( GUI.garage.cars ) do
			GUI.garage.car_info[ i ] = GameData.GetVehicle( i )
			
			if GUI.garage.car_info[ i ] == nil then
				print( "ERROR: no vehicle info for id", i )
				GUI.garage.cars[ i ] = nil
			else
				if v.unlocked == true then
					GUI.garage.garage[ i ] = true
				end
				
				-- boss mods
				local ability = GUI.garage.car_info[ i ].ability
				
				if ability ~= -1 then
					local found = false
					
					for j, w in ipairs( GUI.garage.boss_mods ) do
						if w.type == ability then
							found = true
							break
						end
					end
					
					if found == false then
						-- new mod
						GUI.garage.boss_mods[ #GUI.garage.boss_mods + 1 ] = { type = ability, info = Multiplayer.GetAbility( ability, true ), unlocked = v.unlocked, }
						GUI.garage.boss_mods_inv[ ability ] = #GUI.garage.boss_mods
						
						-- only allow mods for cars we've unlocked
						if v.unlocked == true then
							GUI.garage.boss_mods_unlocked[ #GUI.garage.boss_mods_unlocked + 1 ] = #GUI.garage.boss_mods
							GUI.garage.boss_mods_unlocked_inv[ #GUI.garage.boss_mods ] = #GUI.garage.boss_mods_unlocked
						end
					end
				end
				
				if v.method == "rank" then
					GUI.garage.rank[ i ] = GUI.garage.cars[ i ].rank
				else
					-- boss car
					v.boss = true
					
					local ability = GUI.garage.car_info[ i ].ability
					
					-- boss car unlock that doesn't have a boss mod
					if ability == -1 then
						GUI.garage.rank[ i ] = GUI.dummy_boss_car_rank_no_mod
						v.boss_no_mod = true
					else
						GUI.garage.rank[ i ] = GUI.dummy_boss_car_rank
					end
					
					-- tier this boss car is in
					local boss_tier = 1
					
					for j, w in ipairs( GUI.sp_tier_info ) do
						if v.bossId == w.bossId then
							boss_tier = j
							break
						end
					end
					
					v.boss_tier = boss_tier
					v.boss_state = "none"
					
					if GUI.sp_tier_info[ v.boss_tier ].unlocked == false then
						v.boss_state = "tier_locked"
					else
						local boss_event_unlocked = false
						
						for j, w in ipairs( GUI.sp_tier_info[ v.boss_tier ].events ) do
							local event_info = SinglePlayer.EventInfo( w )
							
							if event_info.kind == "boss" then
								boss_event_unlocked = event_info.unlocked
								break
							end
						end
						
						if boss_event_unlocked == false then
							v.boss_state = "event_locked"
						else
							if GUI.garage.garage[ i ] == nil then
								v.boss_state = "car_locked"
							else
								v.boss_state = "car_unlocked"
							end	
						end
					end
				end
			end
		end
		
		-- it's possible to beat the last boss without beating any of the rest
		if #GUI.garage.boss_mods_unlocked == 0 then
			GUI.enable_boss_mods = false
		end
		
		if GUI.enable_boss_mods == true then
			SetupScreenTitle( UIText.SP_GARAGE_MOD_LOADOUT, SCUI.name_to_id[ "mod_title_dummy" ], nil, nil, 5, nil, true, 0.75 )
			
			local boss_mod_loadout_rb = SCUI.name_to_id[ "boss_mod_loadout_rb" ]
			UIButtons.SetActive( boss_mod_loadout_rb, true )
			
			local boss_mod_box_id = SetupScreenTitle( UIText.SP_GARAGE_MOD_LOADOUT, boss_mod_loadout_rb, nil, nil, 0, UIEnums.Justify.BottomRight, true )
			GUI.boss_mod_icon_id = UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_boss_mod_loadout_icon" ] ] )
			UIButtons.SetParent( GUI.boss_mod_icon_id, boss_mod_box_id, UIEnums.Justify.BottomCentre )
			
			--GUI.help_text_id, GUI.bottom_help_id = SetupBottomHelpBar( UIText.SP_GARAGE_MOD_LOADOUT_HELP, nil, UIEnums.Layer.TOP, true, 0.1875 )
		end
		
		if GUI.is_sp_event == true then
			GUI.custom_filter = Sp_GarageFilterCars()
		end
	end
	--show_table( GUI.garage, 1 )

	-- process cars
	for i, v in pairs( GUI.garage.cars ) do
		v.own_it = ( GUI.garage.garage[ i ] ~= nil )
		v.has_parent = GUI.garage.car_info[ i ].has_parent
		
		if GUI.is_multiplayer == true then
			v.new = ( GUI.garage_new[ v.id ] == true )
			
			if v.own_it == false then
				v.unlock_at = GUI.garage.rank[ i ]
				
				if v.unlock_at == nil then
					v.unlock_at = 51
					GUI.garage.rank[ i ] = 51
				end

				v.tease = false -- ( v.unlock_at <= ( Multiplayer.ProfileProgression().level + 5 ) )
			end
		else
			if v.own_it == false then
				v.unlock_at = GUI.garage.rank[ i ]
				
				if v.boss == true then
					v.tease = false --( v.boss_state ~= "none" and v.boss_state ~= "tier_locked" )
				else
					v.tease = false --( v.unlock_at <= ( GUI.sp_game_info.rank + 5 ) )
				end
			end
		end
		
		if Multiplayer.ShowAllGarage() == true then
			v.own_it = true
			v.tease = false
		end

		-- hide legend cars we have not unlocked
		if GUI.is_multiplayer == true then
			if GUI.garage.cars[ i ].own_it == false and GUI.garage.rank[ GUI.garage.cars[ i ].id ] == 51 then
				GUI.garage.cars[ i ] = nil
			end
		end
	end
	
	-- build lists by class
	for i, v in pairs( GUI.garage.cars ) do
		local class = GUI.garage.car_info[ i ].class
		
		if GUI.custom_filter == true then
			class = 0	-- all in the same list
		end
			
		if GUI.class[ "_" .. class ] == nil then
			-- new class
			local class_table = {  }
			class_table.class = class
			class_table.cars = {  }
			class_table.cars[ #class_table.cars + 1 ] = i
				
			GUI.class[ #GUI.class + 1 ] = class_table
			GUI.class[ "_" .. class ] = #GUI.class
		else
			-- new car for this class
			local c = GUI.class[ GUI.class[ "_" .. class ] ]
			c.cars[ #c.cars + 1 ] = i
		end
	end
	--show_table( GUI.class, 1 )
	
	-- sort by class
	table.sort( GUI.class, function( a, b ) return a.class < b.class end )
	
	-- sort cars
	local sort_func = function( a, b )
		local car_a = GUI.garage.cars[ a ]
		local car_b = GUI.garage.cars[ b ]
		
		if car_a.own_it ~= car_b.own_it then
			return ( car_a.own_it )
		end
		
		local rank_a = GUI.garage.rank[ a ]
		local rank_b = GUI.garage.rank[ b ]
		
		if rank_a == nil then
			rank_a = 0
		end
		if rank_b == nil then
			rank_b = 0
		end
		
		local is_rank_a_boss = ( rank_a == GUI.dummy_boss_car_rank or rank_a == GUI.dummy_boss_car_rank_no_mod )
		local is_rank_b_boss = ( rank_b == GUI.dummy_boss_car_rank or rank_b == GUI.dummy_boss_car_rank_no_mod )
		
		if ( rank_a == GUI.dummy_boss_car_rank ) and car_a.own_it == true then
			rank_a = rank_a * -1
		end
		if ( rank_b == GUI.dummy_boss_car_rank ) and car_b.own_it == true then
			rank_b = rank_b * -1
		end
		
		if GUI.is_multiplayer == false and is_rank_a_boss == true and is_rank_b_boss == true and car_a.own_it == false and car_b.own_it == false then
			local stars_a = GUI.sp_tier_info[ car_a.boss_tier ].starRequirement
			local stars_b = GUI.sp_tier_info[ car_b.boss_tier ].starRequirement
			
			return ( stars_a < stars_b )
		end
		
		if GUI.is_multiplayer == false and rank_a == rank_b then
			local order_a = car_a.order
			local order_b = car_b.order
			
			if order_a == nil then
				order_a = 0
			end
			if order_b == nil then
				order_b = 0
			end
			
			return ( order_a < order_b )
		end
		
		return ( rank_a < rank_b )
	end
		
	for i, v in ipairs( GUI.class ) do
		table.sort( v.cars, sort_func )
	end
		
	-- store class icon names
	GUI.class_icon_names = 
	{
		[ UIEnums.VehicleClass.A ] = "class_a",
		[ UIEnums.VehicleClass.B ] = "class_b",
		[ UIEnums.VehicleClass.C ] = "class_c",
		[ UIEnums.VehicleClass.D ] = "class_d",
	}
	
	-- store style names
	GUI.style_names =
	{
		[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
		[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
		[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
		[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
		[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
		[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
	}
	
	GUI.palette_ids = 
	{
		[ UIEnums.Palette.Solid ]		= SCUI.name_to_id[ "colour_solid" ],
		[ UIEnums.Palette.Metallic ]	= SCUI.name_to_id[ "colour_metallic" ],
		[ UIEnums.Palette.Pearlecent ]	= SCUI.name_to_id[ "colour_pearl" ],
		[ UIEnums.Palette.Candy ]		= SCUI.name_to_id[ "colour_candy" ],
	}
	
	UIScreen.BlockInputToContext( true )

	-- Hijack this
	if Amax.ShowCameraHelpers() == true then
		GUI.debug_car_id = SCUI.name_to_id[ "debug_car_image" ]
		UIButtons.SetActive( GUI.debug_car_id, true )
	end
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Keep the centered vignette exactly viewport-sized at any aspect.
		UIButtons.ChangeSize( SCUI.name_to_id[ "vignette" ], SCREEN_WIDTH, 480 )
	end

	-- switch certain stuff on
	if GUI.is_multiplayer == true then
		UIButtons.SetActive( SCUI.name_to_id[ "challenges_render_branch" ], true )
		local chickX, chickY, chickZ = UIButtons.GetPosition( SCUI.name_to_id[ "chick_health" ] )
		UIButtons.ChangePosition( SCUI.name_to_id[ "chick_health" ], chickX-1, chickY, chickZ )
		UIButtons.SetXtVar( SCUI.name_to_id[ "chick_health" ], "num_segments", 8 )
		UIButtons.SetXtVar( SCUI.name_to_id[ "chick_health" ], "max_value", 8 )
	else
		UIButtons.SetActive( SCUI.name_to_id[ "sp_boss_unlocked_rb" ], true )
		UIButtons.SetActive( SCUI.name_to_id[ "sp_boss_rb" ], true )
	end
	
	GUI.colour_picker_id = SCUI.name_to_id[ "colour_picker" ]
	
	-- add list of cars for each class
	UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_class_stencil_read_on" ] ] )
	
	GUI.class_node_list_id = UIButtons.CloneXtGadgetByName( "Shared\\Garage.lua", "_class_node_list" )
	UIButtons.SetParent( GUI.class_node_list_id, SCUI.name_to_id[ "class_dummy" ], UIEnums.Justify.MiddleCentre )
	
	for i, v in ipairs( GUI.class ) do
		v.ids = {  }
		
		local class_node_id = UIButtons.CloneXtGadgetByName( "Shared\\Garage.lua", "_class_node" )
		local class_icon_id = UIButtons.FindChildByName( class_node_id, "_class_icon" )
		
		UIShape.ChangeObjectName( class_icon_id, GUI.class_icon_names[ v.class ] )
		
		UIButtons.AddListItem( GUI.class_node_list_id, class_node_id, i )
		
		if GUI.custom_filter == true then
			UIButtons.SetActive( class_node_id, false )
		end
		
		v.ids.node = class_node_id
		v.ids.icon = class_icon_id
	end
	
	UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_class_stencil_read_off" ] ] )
	
	UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_car_stencil_read_on" ] ] )
	
	for i, v in ipairs( GUI.class ) do
		-- car node list
		local node_list_id = UIButtons.CloneXtGadgetByName( "Shared\\Garage.lua", "_car_node_list" )
		UIButtons.SetParent( node_list_id, SCUI.name_to_id[ "class_dummy" ], UIEnums.Justify.TopLeft )
		v.ids.node_list = node_list_id
		
		for j, w in ipairs( v.cars ) do
			local car_node_id = UIButtons.CloneXtGadgetByName( "Shared\\Garage.lua", "_car_node" )
			local car_icon_id = UIButtons.FindChildByName( car_node_id, "_car_icon" )
			local car_icon_id_not_allowed = UIButtons.FindChildByName( car_node_id, "_car_icon_not_allowed" )
			local car_name_id = UIButtons.FindChildByName( car_node_id, "_car_name" )
			local car_new_id  = UIButtons.FindChildByName( car_node_id, "_car_new" )
			local car_lock_id = UIButtons.FindChildByName( car_node_id, "_car_locked_icon" )
			local car_unlock_id = UIButtons.FindChildByName( car_node_id, "_car_rank_unlock" )
			
			local car = GUI.garage.cars[ w ]
			local car_info = GUI.garage.car_info[ w ]

			UIShape.ChangeSceneName( car_icon_id, car_info.sheet )
			UIShape.ChangeObjectName( car_icon_id, car_info.icon )
			UIShape.ChangeSceneName( car_icon_id_not_allowed, car_info.sheet )
			UIShape.ChangeObjectName( car_icon_id_not_allowed, car_info.icon )
			UIButtons.ChangeText( car_name_id, car_info.name )
			
			UIButtons.AddListItem( node_list_id, car_node_id, j )
			
			if car.own_it == true then
				UIButtons.SetActive( car_icon_id, true )
				UIButtons.SetActive( car_name_id, true )
				
				if car.allowed == false then
					UIButtons.SetActive( car_icon_id, false )
					UIButtons.SetActive( car_icon_id_not_allowed, true )
					
					UIButtons.ChangeColour( car_name_id, "Locked" )
					
					UIButtons.SetActive( car_new_id, true )
					UIButtons.ChangeColour( car_new_id, "Support_3" )
					UIShape.ChangeObjectName( car_new_id, "unavailable" )
					
				elseif car.new == true then
					UIButtons.SetActive( car_new_id, true )
				end
			else
				UIButtons.SetActive( car_unlock_id, true )
				
				if GUI.is_multiplayer == false and ( car.unlock_at == GUI.dummy_boss_car_rank or car.unlock_at == GUI.dummy_boss_car_rank_no_mod ) then
					UIButtons.ChangeText( car_unlock_id, UIText.CMN_NOWT )
				else
					if GUI.is_multiplayer == true then
						UIButtons.ChangeText( car_unlock_id, "MPL_UNLOCK_RANK" .. car.unlock_at )
					else
						UIButtons.ChangeText( car_unlock_id, "SPL_UNLOCK_STATUS" .. car.unlock_at )
					end
				end
				
				if car.tease == true then
					UIButtons.SetActive( car_icon_id, true )
					UIButtons.SetActive( car_name_id, true )
					UIButtons.ChangeColour( car_icon_id, "Locked" )
					UIButtons.ChangeColour( car_name_id, "Locked" )
				else
					UIButtons.SetActive( car_lock_id, true )
				end
			end
			
			car.ids = {  }
			car.ids.node = car_node_id
			car.ids.icon = car_icon_id
			car.ids.name = car_name_id
			car.ids.new  = car_new_id
			car.ids.lock = car_lock_id
			car.ids.lock = car_lock_id
			car.ids.unlock = car_unlock_id
		end
		
		UIButtons.SetParent( node_list_id, v.ids.node, UIEnums.Justify.MiddleCentre )
	end
	
	UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_car_stencil_read_off" ] ] )
	
	-- set class and car defaults to the current background car
	local found = false
		
	for i, v in ipairs( GUI.class ) do
		for j, w in ipairs( v.cars ) do
			if w == GUI.viewing_id_old then
				UIButtons.SetSelection( GUI.class_node_list_id, i )
				UIButtons.SetSelection( v.ids.node_list, j )
				found = true
				break
			end
		end
		
		if found == true then
			break
		end
	end
	
	if found == false then
		if GUI.is_multiplayer_lobby == true then
			Multiplayer.SetGarageCarByID( Garage_GetSelectedCar().id )	-- need a default - background car is different
		else
			if GUI.is_multiplayer == false then
				-- find the first car we've got unlocked - TODO - do we need something other than this?
				local found = false
				
				for i, v in ipairs( GUI.class ) do
					for j, w in ipairs( v.cars ) do
						if GUI.garage.cars[ w ].unlocked == true then
							GUI.viewing_id_old = w
							UIButtons.SetSelection( GUI.class_node_list_id, i )
							UIButtons.SetSelection( v.ids.node_list, j )							
							found = true
							break
						end
					end
					
					if found == true then
						break
					end
				end
			else
				print( "current multiplayer car not found - do something?" )
			end
		end
	end

	if GUI.is_multiplayer == true then
		UIButtons.TimeLineActive( "Hide_Lobby", UIGlobals.Multiplayer.InLobby )
		UIButtons.SetActive( SCUI.name_to_id["countdown"], UIGlobals.Multiplayer.InLobby )
		
		GUI.challenge_rewards = Multiplayer.GetCarChallengeRewards()
		
		-- set reward icons
		GUI.reward_icons = 
		{
			[ UIEnums.MpChallengeRewards.HealthSlot ]	= "health",
			[ UIEnums.MpChallengeRewards.BonusFans ]	= "extra_slot", -- Need to change this to new icon name when we have one
			[ UIEnums.MpChallengeRewards.SpeedBoost ]	= "horsepower",
			[ UIEnums.MpChallengeRewards.Metallic ]		= "metallic",
			[ UIEnums.MpChallengeRewards.Pearlescent ]	= "pearl",
			[ UIEnums.MpChallengeRewards.Candy ]		= "candy",
		}
		
		for i, v in ipairs( GUI.challenge_rewards.repeat_offender ) do
			UIShape.ChangeObjectName( SCUI.name_to_id[ "repeat_offender_icon_" .. i ], GUI.reward_icons[ v ] )
		end
		
		-- build car challenges table
		GUI.garage.challenges = {  }
		
		for i, v in pairs( UIEnums.VehicleClass ) do
			local car_challenges = Multiplayer.GetCarChallenges( v + 1, true )
			
			for j, w in ipairs( car_challenges ) do
				GUI.garage.challenges[ w.id ] = w
			end
		end
	else
		-- disable car upgrades and palette unlocks
		UIButtons.SetActive( SCUI.name_to_id[ "stat_challenges_title" ], false, true )
		UIButtons.SetActive( SCUI.name_to_id[ "colour_lock" ], false )
		UIButtons.SetXtVar( GUI.colour_picker_id, "disable_additional_palettes", true )
		UIButtons.SetActive( SCUI.name_to_id[ "palette_dummy" ], false, true )
		
		-- add boss mods
		if GUI.enable_boss_mods == true and #GUI.garage.boss_mods > 0 then
			GUI.boss_mod_rb_id = SCUI.name_to_id[ "boss_mod_rb" ]
			UIButtons.SetActive( GUI.boss_mod_rb_id, true )
			
			local stencil_read_on = UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_mod_stencil_read_on" ] ] )
			UIButtons.SetParent( stencil_read_on, GUI.boss_mod_rb_id, UIEnums.Justify.MiddleCentre )
			
			GUI.boss_mod_node_list_id = UIButtons.CloneXtGadgetByName( "Shared\\Garage.lua", "_mod_node_list" )
			UIButtons.SetParent( GUI.boss_mod_node_list_id, GUI.boss_mod_rb_id, UIEnums.Justify.MiddleCentre )
			
			for i, v in ipairs( GUI.garage.boss_mods ) do
				if v.unlocked == true then
					local mod_node = UIButtons.CloneXtGadgetByName( "Shared\\Garage.lua", "_mod_node" )
					
					local icon_id = UIButtons.FindChildByName( mod_node, "_mod_icon" )
					UIShape.ChangeObjectName( icon_id, v.info.icon_name )
					
					UIButtons.AddListItem( GUI.boss_mod_node_list_id, mod_node, i )
				end
			end
			
			local mod_equipped = GameProfile.SP_EquippedAbility()
			
			if mod_equipped ~= -1 then
				UIButtons.SetSelection( GUI.boss_mod_node_list_id, GUI.garage.boss_mods_inv[ mod_equipped ] )
			end
			
			local stencil_read_off = UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_mod_stencil_read_off" ] ] )
			UIButtons.SetParent( stencil_read_off, GUI.boss_mod_rb_id, UIEnums.Justify.MiddleCentre )
		end
	end
	
	-- initial update
	Garage_CheckForModChange( true )
	Garage_UpdateModLoadoutFromProfile()
	Garage_CheckForClassChange( true )
	
	UIButtons.TimeLineActive( "Hide_Carousel", true )
	
	local car = Garage_GetSelectedCar()
	GUI.viewing_id = car.id
	GUI.wait_load_timer = GUI.wait_load_time
	GUI.wait_respray_timer = GUI.wait_load_time
	
	Amax.ResetChromeVehicleGarageCheat()
	
	-- rank icon
	UIButtons.SetActive( SCUI.name_to_id[ "unlock_rank_icon" ], GUI.is_multiplayer )
	UIButtons.SetActive( SCUI.name_to_id[ "unlock_rank_icon_sp" ], not GUI.is_multiplayer )
	
	--[[
	-- switch off boss help at start
	if GUI.bottom_help_id ~= -1 then
		UIButtons.PrivateTimeLineActive( GUI.bottom_help_id, "Hide_BottomHelp", true, 0 )
	end
	--]]
	
	-- popup if you've just unlocked a mod
	if GUI.enable_boss_mods == true and GUI.is_multiplayer == false then
		local boss_defeated = false
		
		for i, v in ipairs( GUI.sp_boss_info ) do
			if v.defeated == true then
				boss_defeated = true
				break
			end
		end
		
		if boss_defeated == true and GameProfile.SP_SeenGarageModPopup() == 0 then
			GameProfile.SP_SeenGarageModPopup( 1 )
			SetupCustomPopup( UIEnums.CustomPopups.GarageFirstModUnlocked )
		end
	end
end

function StartLoop( time )
	--
	Garage_UpdateCamera( time )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.current_mode == GUI.mode.car then
		if message == UIEnums.Message.MenuNext then
			Garage_End( true )
		elseif message == UIEnums.Message.MenuBack then
			Garage_End( false )
			
		elseif message == UIEnums.Message.ButtonRightShoulder then
			if GUI.enable_boss_mods == false then
				return
			end
			
			PlaySfxNext()
			Garage_UpdateCurrentMode( GUI.mode.boss_mods )
		
		elseif message == UIEnums.Message.ButtonX then
			if GUI.enable_respray == false or GUI.wait_respray_timer < GUI.wait_respray_time then
				return
			end
			
			local car, car_info = Garage_GetSelectedCar()
			
			if car.own_it == true and car_info.respray == true then
				PlaySfxNext()
				Garage_UpdateCurrentMode( GUI.mode.colour )
			end
		
		elseif message == UIEnums.Message.ButtonY then
			local car = Garage_GetSelectedCar()
			local allow_challenges = ( GUI.is_multiplayer == true and car.own_it == true and car.has_parent == false) --or ( GUI.is_multiplayer == false and car.own_it == true and car.boss == true and car.boss_no_mod ~= true )
			allow_challenges = allow_challenges == true and car.allowed ~= false
			local is_legend_car = (GUI.garage.rank[ car.id ] == nil) 
			
			if allow_challenges == true and is_legend_car == false then
				Garage_ChallengesOnOff( not GUI.viewing_challenges, true )
			end
		end
		
	elseif GUI.current_mode == GUI.mode.colour then
		if message == UIEnums.Message.MenuNext then
			if Garage_PaletteUnlocked() == true then
				UISystem.PlaySound( UIEnums.SoundEffect.GarageReSpray )
				GUI.save_on_exit = true
				Garage_RespraySetToProfile()
				Amax.CheckStickerProgress( UIEnums.StickerType.ResprayFirstCar )
				if GUI.current_palette ~= UIEnums.Palette.Solid  and GUI.is_multiplayer == true and IsSplitScreen() == false then -- cant unlock non solid paint jobs in single player
					Amax.CheckStickerProgress( UIEnums.StickerType.ResprayWithUnlocked )
				end
				if GUI.is_multiplayer_lobby == true then
					RestoreInfoLine()
					Garage_End( true )
				else
					Garage_UpdateCurrentMode( GUI.mode.car )
				end				
			else
				PlaySfxError()
			end
			
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			
			Garage_RespraySetFromProfile( true )
			Garage_UpdateCurrentMode( GUI.mode.car )
		
		elseif message == UIEnums.Message.ButtonX then
			UISystem.PlaySound( UIEnums.SoundEffect.GarageReSpray )
			
			GUI.save_on_exit = true
			Garage_ResprayRevert()
			Garage_UpdateCurrentMode( GUI.mode.car )
		end
		
	elseif GUI.current_mode == GUI.mode.boss_mods then
		if message == UIEnums.Message.MenuNext then
			local mod_selected_type = GUI.garage.boss_mods[ GUI.current_mod ].type
			GameProfile.SP_EquippedAbility( mod_selected_type )
			Garage_UpdateCurrentMode( GUI.mode.car )
			UISystem.PlaySound( UIEnums.SoundEffect.ApplyBossMod )
			
		elseif message == UIEnums.Message.MenuBack then
			local mod_equipped = GameProfile.SP_EquippedAbility()
			
			if mod_equipped ~= -1 then
				UIButtons.SetSelection( GUI.boss_mod_node_list_id, GUI.garage.boss_mods_inv[ mod_equipped ] )
			end
			
			Garage_UpdateCurrentMode( GUI.mode.car )
			PlaySfxBack()
		
		elseif message == UIEnums.Message.ButtonX then
			-- reset
			GameProfile.SP_EquippedAbility( -1 )
			Garage_UpdateCurrentMode( GUI.mode.car )
			PlaySfxError()
		end
	end

	if message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		local car = Garage_GetSelectedCar()
		
		if Amax.CanUseShare() == true then
			if UIGlobals.Multiplayer.InLobby == false then
				if GUI.current_mode == GUI.mode.car and car.own_it == true then
					UIGlobals.ShareFromWhatPopup=-1
					SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
				end
			end
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then
		-- store current background car for when we come back
		local car = Garage_GetSelectedCar()
		
		if GUI.wait_load_timer < GUI.wait_load_time then
			if car.id ~= GUI.viewing_id then
				Garage_LoadCar( car )
				GUI.viewing_id = car.id
			end
			
			GUI.wait_load_timer = GUI.wait_load_time
			GUI.wait_respray_timer = GUI.wait_load_time
			
			if car.new == true and car.allowed ~= false then
				Garage_ViewedCarNew( car.id, car.ids.new )
			end
		end
		
		if GUI.is_multiplayer == true then
			Multiplayer.SetGarageCarByID( car.id )
		else
			Garage_SpSetCar()
		end
			
		StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Shared\\Garage.lua" )

		if arg3 == UIEnums.ShareOptions.Facebook then
			if GUI.viewing_challenges == true then
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.MPCarChallengeComplete, 1, GUI.viewing_id )
			else
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.OwnCar, 1, GUI.viewing_id )
			end
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Twitter then
			if GUI.viewing_challenges == true then
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.MPCarChallengeComplete, 0, GUI.viewing_id )
			else
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.OwnCar, 0, GUI.viewing_id )
			end
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			if GUI.viewing_challenges == true then
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.MPCarChallengeComplete, 2, GUI.viewing_id )
			else
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.OwnCar, 2, GUI.viewing_id )
			end

		end
	end
end

function FrameUpdate( time )
	Garage_UpdateCamera( time )
	
	Garage_CheckForModChange()
	Garage_CheckForClassChange()
	Garage_CheckForCarChange()
	
	if GUI.is_multiplayer == true then
		if Amax.CheckChromeVehicleGarageCheat( Garage_GetSelectedCar().id ) == true then
			GoScreen( "Shared\\Garage.lua" ) -- reload
		end
		
		Garage_CheckForPaletteChange()
	end
	
	Garage_UpdateWaitLoadTimer( time )
end

function EndLoop( time )
	--
end

function EnterEnd()
	UISystem.PlaySound( UIEnums.SoundEffect.GarageExit )
	UIButtons.TimeLineActive( "garage_start", false )
	UIButtons.TimeLineActive( "Show_Garage", false )
	UIButtons.TimeLineActive( "Hide_LightPaint", false )
	RestoreInfoLine()
	
	if GUI.exit_to_game ~= true then
		Amax.ChangeEnvironmentSettings( false )
	end
end

function End()
	if GUI.is_multiplayer_lobby == true then
		NetRace.ChangeLocalPlayerState( Profile.GetPrimaryPad(), UIEnums.Network.PlayerStates.eNetPlayerStateIdle )
		UIGlobals.Multiplayer.GarageActive = false
	end

	if GUI.is_multiplayer == false then
		net_SetRichPresence(UIEnums.RichPresence.SpMain)
	end
	
	UIGlobals.SpViewingGarage = false
end

function Garage_CheckForModChange( force )
	if GUI.boss_mod_node_list_id == -1 then
		return
	end
	
	local mod_selection = UIButtons.GetSelection( GUI.boss_mod_node_list_id )
	
	if mod_selection ~= GUI.current_mod or force == true then
		UIButtons.ChangeText( SCUI.name_to_id[ "mod_name" ], GUI.garage.boss_mods[ mod_selection ].info.name )
		UIButtons.ChangeText( SCUI.name_to_id[ "mod_desc" ], GUI.garage.boss_mods[ mod_selection ].info.desc )
		
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "mod_arrow_left" ], "more", ( GUI.garage.boss_mods_unlocked_inv[ mod_selection ] ~= 1 ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "mod_arrow_left" ], "push", ( mod_selection < GUI.current_mod ), 0 )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "mod_arrow_right" ], "more", ( GUI.garage.boss_mods_unlocked_inv[ mod_selection ] ~= #GUI.garage.boss_mods_unlocked ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "mod_arrow_right" ], "push", ( mod_selection > GUI.current_mod ), 0 )
		
		UIButtons.TimeLineActive( "mod_change", true, 0 )
	
		GUI.current_mod = mod_selection
	end
end

function Garage_UpdateModLoadoutFromProfile()
	if GUI.enable_boss_mods == false then
		return
	end
	
	local mod_equipped = GameProfile.SP_EquippedAbility()
			
	if mod_equipped == -1 then
		UIShape.ChangeObjectName( GUI.boss_mod_icon_id, "shannonbossmod" )
		UIButtons.ChangeColour( GUI.boss_mod_icon_id, "Main_Black" )
	else
		UIShape.ChangeObjectName( GUI.boss_mod_icon_id, GUI.garage.boss_mods[ GUI.garage.boss_mods_inv[ mod_equipped ] ].info.icon_name )
		UIButtons.ChangeColour( GUI.boss_mod_icon_id, "Main_1" )
	end
end

function Garage_CheckForClassChange( force )
	local class_selection = UIButtons.GetSelection( GUI.class_node_list_id )
	
	if class_selection ~= GUI.current.class or force == true then
		for i, v in ipairs( GUI.class ) do
			UIButtons.SetSelected( v.ids.node_list, ( i == class_selection ) )
			UIButtons.SetActive( v.ids.node_list, ( ( i == class_selection ) or ( i == GUI.current.class ) ) )
		end
		
		if GUI.current.class == nil then
			GUI.current.class = class_selection
		end
		
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_top" ], "more", ( class_selection ~= 1 ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_top" ], "push", ( class_selection < GUI.current.class ), 0 )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_bottom" ], "more", ( class_selection ~= #GUI.class ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_bottom" ], "push", ( class_selection > GUI.current.class ), 0 )
		
		GUI.current.class = class_selection
		
		if force ~= true then
			GUI.wait_load_timer = 0
			GUI.wait_respray_timer = 0
		end
		
		Garage_CheckForCarChange( true )
	end
end

function Garage_CheckForCarChange( force )
	local car_selection = UIButtons.GetSelection( GUI.class[ GUI.current.class ].ids.node_list )
	
	if car_selection ~= GUI.current.car[ GUI.current.class ] or force == true then
		if GUI.current.car[ GUI.current.class ] == nil then
			GUI.current.car[ GUI.current.class ] = car_selection
		end
		
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_left" ], "more", ( car_selection ~= 1 ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_left" ], "push", ( car_selection < GUI.current.car[ GUI.current.class ] ), 0 )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_right" ], "more", ( car_selection ~= #GUI.class[ GUI.current.class ].cars ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_right" ], "push", ( car_selection > GUI.current.car[ GUI.current.class ] ), 0 )
		
		GUI.current.car[ GUI.current.class ] = car_selection
		
		if force ~= true then
			GUI.wait_load_timer = 0
			GUI.wait_respray_timer = 0
		end
		
		Garage_UpdateStats()
	end
end

function Garage_CheckForPaletteChange()
	local palette = Amax.GetColourPalette( GUI.colour_picker_id )
	
	if palette ~= GUI.current_palette then
		for i, v in pairs( GUI.palette_ids ) do
			UIButtons.SetSelected( v, ( i == palette ) )
		end
		
		GUI.current_palette = palette
		
		Garage_UpdateColourLocked()
	end
end

function Garage_GetSelectedCar()
	local id = GUI.class[ GUI.current.class ].cars[ GUI.current.car[ GUI.current.class ] ]
	return GUI.garage.cars[ id ], GUI.garage.car_info[ id ]
end

function Garage_UpdateStats()
	local car, car_info = Garage_GetSelectedCar()
	--show_table( car )
	
	-- man badge
	local man_square_id = SCUI.name_to_id[ "stat_man_square" ]
	local man_rect_id = SCUI.name_to_id[ "stat_man_rect" ]
	local man_lock_id = SCUI.name_to_id[ "stat_man_lock" ]
	
	if car.own_it == true or car.tease == true then
		UIButtons.SetActive( man_lock_id, false )
		
		if car_info.man_texture_square == true then
			UIButtons.SetActive( man_square_id, true )
			UIButtons.SetActive( man_rect_id, false )
			UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, man_square_id )
		else
			UIButtons.SetActive( man_square_id, false )
			UIButtons.SetActive( man_rect_id, true )
			UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, man_rect_id )
		end
	else
		UIButtons.SetActive( man_lock_id, true )
		UIButtons.SetActive( man_square_id, false )
		UIButtons.SetActive( man_rect_id, false )
	end
	
	local class_icon_id = SCUI.name_to_id[ "stat_class_icon" ]
	local car_name_id = SCUI.name_to_id[ "stat_car_name" ]
	
	UIShape.ChangeObjectName( class_icon_id, GUI.class_icon_names[ car_info.class ] )
	
	if car.own_it == true and car.allowed == false then
		UIButtons.ChangeText( car_name_id, car_info.name )
		UIButtons.ChangeColour( car_name_id, "Locked" )
		UIButtons.ChangeColour( class_icon_id, "Locked" )
	elseif car.own_it == true then
		UIButtons.ChangeText( car_name_id, car_info.name )
		UIButtons.ChangeColour( car_name_id, "Main_White" )
		UIButtons.ChangeColour( class_icon_id, GUI.primary_colour )
	elseif car.tease == true then
		UIButtons.ChangeText( car_name_id, car_info.name )
		UIButtons.ChangeColour( car_name_id, "Locked" )
		UIButtons.ChangeColour( class_icon_id, "Locked" )
	else
		UIButtons.ChangeText( car_name_id, UIText.RBC_GARAGE_UNKNOWN )
		UIButtons.ChangeColour( car_name_id, "Locked" )
		UIButtons.ChangeColour( class_icon_id, "Locked" )
	end
	
	--local boss_show_mod = car.boss_no_mod ~= true and ( ( car.boss == true and car.own_it == true ) or ( car.boss == true and car.boss_state == "car_locked" ) )
	--UIButtons.TimeLineActive( "boss_mod_on", boss_show_mod )
	
	if car.boss == true then
		if car.own_it == false then
			UIButtons.SetActive( SCUI.name_to_id[ "sp_boss_rb_tier_locked" ],	( car.boss_state == "tier_locked" ) )
			UIButtons.SetActive( SCUI.name_to_id[ "sp_boss_rb_event_locked" ],	( car.boss_state == "event_locked" ) )
			UIButtons.SetActive( SCUI.name_to_id[ "sp_boss_rb_car_locked" ],	( car.boss_state == "car_locked" ) )
			UIButtons.SetActive( SCUI.name_to_id[ "sp_boss_lock" ],				( car.boss_state ~= "car_unlocked" ) )
			
			AIGamerPicture( SCUI.name_to_id[ "sp_boss_pic" ], UIGlobals.boss_avatar_ids[ car.boss_tier ] )
			UIButtons.ChangeText( SCUI.name_to_id[ "sp_boss_name" ], GUI.sp_boss_info[ car.boss_tier ].name )
			
			if car.boss_state == "tier_locked" then
				UIButtons.ChangeText( SCUI.name_to_id[ "sp_boss_tier_locked_number" ], "GAME_NUM_" .. GUI.sp_tier_info[ car.boss_tier ].starRequirement )
			end
			
			if car.boss_state == "car_locked" then
				local title_id = SCUI.name_to_id[ "sp_boss_car_locked_title" ]
				--local mod_name_id = SCUI.name_to_id[ "sp_boss_car_locked_mod_name" ]
				
				if car_info.ability ~= -1 then			
					--UIShape.ChangeObjectName( SCUI.name_to_id[ "sp_boss_mod_icon" ], GUI.garage.boss_mods[ GUI.garage.boss_mods_inv[ car_info.ability ] ].info.icon_name )
					--UIButtons.SetActive( mod_name_id, true )
					--UIButtons.ChangeText( mod_name_id, GUI.garage.boss_mods[ GUI.garage.boss_mods_inv[ car_info.ability ] ].info.name )
					UIButtons.ChangeText( title_id, UIText.RBC_UNLOCK_THIS_BOSS_CAR )
				else
					--UIButtons.SetActive( mod_name_id, false )
					UIButtons.ChangeText( title_id, UIText.RBC_UNLOCK_THIS_BOSS_CAR_NO_MOD )
				end
			end
		else
			--[[
			AIGamerPicture( SCUI.name_to_id[ "sp_boss_unlocked_pic" ], UIGlobals.boss_avatar_ids[ car.boss_tier ] )
			UIButtons.ChangeText( SCUI.name_to_id[ "sp_boss_unlocked_name" ], GUI.sp_boss_info[ car.boss_tier ].name )
			
			if car_info.ability ~= -1 then
				UIShape.ChangeObjectName( SCUI.name_to_id[ "sp_boss_mod_icon" ], GUI.garage.boss_mods[ GUI.garage.boss_mods_inv[ car_info.ability ] ].info.icon_name )
				UIButtons.ChangeText( SCUI.name_to_id[ "sp_boss_unlocked_mod_name" ], GUI.garage.boss_mods[ GUI.garage.boss_mods_inv[ car_info.ability ] ].info.name )
				UIButtons.ChangeText( SCUI.name_to_id[ "sp_boss_unlocked_mod_info" ], GUI.garage.boss_mods[ GUI.garage.boss_mods_inv[ car_info.ability ] ].info.desc )
			end
			--]]
		end
	end
	
	if car.own_it == true then
		if car.allowed ~= false then
			UIButtons.ChangeText( SCUI.name_to_id[ "stat_style_text" ], GUI.style_names[ car_info.style ] )
			
			Amax.SetChickletsValue( SCUI.name_to_id[ "chick_acceleration" ], car_info.acceleration )
			Amax.SetChickletsValue( SCUI.name_to_id[ "chick_speed" ], car_info.speed )
			Amax.SetChickletsValue( SCUI.name_to_id[ "chick_off_road" ], car_info.stability )
			Amax.SetChickletsValue( SCUI.name_to_id[ "chick_difficulty" ], car_info.difficulty )
			-- Bug #35795 - Show extra health pip from completing tier 2 of repeat offender
			local chick_health_id = SCUI.name_to_id[ "chick_health" ]
			if GUI.is_multiplayer == true then
				if GUI.garage.challenges[ car.id ].repeat_offender.tier > 2 or GUI.garage.challenges[ car.id ].repeat_offender.complete then
					Amax.SetChickletsValue( chick_health_id, car_info.strength + 1)
				else
					Amax.SetChickletsValue( chick_health_id, car_info.strength)
				end
			else
				Amax.SetChickletsValue( chick_health_id, car_info.strength )
			end
		end
		
		-- challenge progress
		if GUI.is_multiplayer == true then
			for i, v in pairs( GUI.challenges ) do
				local challenge = GUI.garage.challenges[ car.id ][ v ]
				
				if i == 1 then
					-- stat icons
					for j, w in ipairs( GUI.challenge_rewards[ v ] ) do
						local icon_id = SCUI.name_to_id[ v .. "_icon_" .. j ]
						
						if challenge.tier > j or challenge.complete == true then
							UIButtons.ChangeColour( icon_id, "Support_4" )
						else
							UIButtons.ChangeColour( icon_id, "Main_Black" )
						end
					end
				end
				
				-- info
				UIButtons.ChangeText( SCUI.name_to_id[ v .. "_name" ],		"MPL_CAR_CHALLENGE" .. car.id .. "_" .. challenge.type .. "_NAME" )
				UIButtons.ChangeText( SCUI.name_to_id[ v .. "_desc" ],		"MPL_CAR_CHALLENGE" .. car.id .. "_" .. challenge.type .. "_DESC" )
				UIButtons.ChangeText( SCUI.name_to_id[ v .. "_amount" ],	"MPL_CAR_CHALLENGE" .. car.id .. "_" .. challenge.type .. "_PROGRESS" )
				UIButtons.ChangeText( SCUI.name_to_id[ v .. "_helptext" ],	"MPL_CAR_CHALLENGE" .. car.id .. "_" .. challenge.type .. "_REWARD" )
				
				-- progress bar
				local backing_id  = SCUI.name_to_id[ v .. "_backing" ]
				local progress_id = SCUI.name_to_id[ v .. "_progress" ]

				local sx, _, _  = UIButtons.GetSize( backing_id )
				local _, sy, sz = UIButtons.GetSize( progress_id )

				UIButtons.ChangeSize( progress_id, ( sx * challenge.progress ), sy, sz )
				
				if i == 2 then	-- winner
					UIButtons.SetActive( SCUI.name_to_id[ "winner_helptext" ], car_info.respray )
				end
				
				-- end icon
				local end_icon_id = SCUI.name_to_id[ v .. "_icon" ]
				local end_tick_id = SCUI.name_to_id[ v .. "_tick" ]
				
				UIButtons.SetActive( end_icon_id, ( challenge.complete == false ) and ( ( i == 1 ) or ( i == 2 and car_info.respray == true ) ) )
				UIButtons.SetActive( end_tick_id, challenge.complete )
				
				if challenge.complete == false then
					UIShape.ChangeObjectName( end_icon_id, GUI.reward_icons[ GUI.challenge_rewards[ v ][ challenge.tier ] ] )
				end
				
				-- slots
				for j, w in ipairs( GUI.challenge_rewards[ v ] ) do
					local slot_id = SCUI.name_to_id[ v .. "_" .. ( 4 - j ) ]
					
					if challenge.tier > j or challenge.complete == true then
						UIButtons.ChangeColour( slot_id, "Support_0" )
					else
						UIButtons.ChangeColour( slot_id, "Main_Black" )
					end
				end
			end
			
			Garage_UpdateColourUnlocks()
		end
		
		UIButtons.TimeLineActive( "locked_on", false )
		UIButtons.TimeLineActive( "boss_locked_on", false )
		UIButtons.TimeLineActive( "stats_off", false )
	else
		-- unlock progress
		if car.boss == true then
			UIButtons.TimeLineActive( "boss_locked_on", true )
			UIButtons.TimeLineActive( "locked_on", false )
		else
			local rank = GUI.garage.rank[ car.id ]
			
			if GUI.is_multiplayer == true then
				UIButtons.ChangeText( SCUI.name_to_id[ "unlock_text_rank" ], "MPL_UNLOCK_RANK" .. rank )
				Mp_RankIcon( SCUI.name_to_id[ "unlock_rank_icon" ], rank - 1, 0 )
			else
				UIButtons.ChangeText( SCUI.name_to_id[ "unlock_text_rank" ], "SPL_UNLOCK_FAN_STATUS" .. rank )
			end
			
			local current_fans = 0
			local fans_needed = 1
			
			if GUI.is_multiplayer == true then
				current_fans = Multiplayer.ProfileProgression().fans
				fans_needed = Multiplayer.LevelProgressionData( rank - 1 ).fans_needed
			elseif rank ~= GUI.dummy_boss_car_rank and rank ~= GUI.dummy_boss_car_rank_no_mod then
				current_fans = GUI.sp_game_info.fans
				fans_needed = GUI.sp_rank_info[ rank ].fans
			end
			
			local progress01 = current_fans / fans_needed
			
			UIButtons.ChangeText( SCUI.name_to_id[ "unlock_amount_next" ], "GAME_NUM_" .. fans_needed )
			
			if GUI.is_multiplayer == true then
				UIButtons.ChangeText( SCUI.name_to_id[ "unlock_desc" ], "MPL_LEVEL_FANS_NEEDED" .. ( fans_needed - current_fans ) .. "_" .. rank )
			else
				UIButtons.ChangeText( SCUI.name_to_id[ "unlock_desc" ], "SPL_FANS_NEEDED" .. ( fans_needed - current_fans ) .. "_" .. rank )
			end
			
			local backing_id  = SCUI.name_to_id[ "unlock_backing" ]
			local progress_id = SCUI.name_to_id[ "unlock_progress" ]

			local sx, _, _  = UIButtons.GetSize( backing_id )
			local _, sy, sz = UIButtons.GetSize( progress_id )

			UIButtons.ChangeSize( progress_id, ( sx * progress01 ), sy, sz )
			
			UIButtons.TimeLineActive( "boss_locked_on", false )
			UIButtons.TimeLineActive( "locked_on", true )
		end
		
		UIButtons.TimeLineActive( "stats_off", true )
	end
	
	if car.own_it == true and car.allowed == false then
		UIButtons.TimeLineActive( "stats_off", true )
		UIButtons.TimeLineActive( "not_allowed_on", true )
	else
		UIButtons.TimeLineActive( "not_allowed_on", false )
	end
	
	local allow_challenges = ( ( GUI.is_multiplayer == true and car.own_it == true and car.has_parent == false ) --[[or ( GUI.is_multiplayer == false and car.own_it == true and car.boss == true and car.boss_no_mod ~= true )]] or ( GUI.is_multiplayer == true and car.own_it == true and GUI.garage.rank[ car.id ] == 51 ) )
	allow_challenges = allow_challenges == true and car.allowed ~= false
	
	if allow_challenges == false and GUI.viewing_challenges == true then
		Garage_ChallengesOnOff( false )
	elseif allow_challenges == true and GUI.is_multiplayer == true and GUI.viewing_challenges_prev == true and GUI.viewing_challenges == false then
		Garage_ChallengesOnOff( true, true, true )
	end
	
	Garage_UpdateCarInfoLines()
end

function Garage_UpdateCarInfoLines()
	local car, car_info = Garage_GetSelectedCar()
	
	local options = {  }
	
	if car.own_it == true and car.allowed ~= false and ( ( GUI.is_multiplayer == false and GUI.is_sp_event == true ) or ( GUI.is_multiplayer == true and GUI.is_multiplayer_lobby == true ) ) then
		options[ #options + 1 ] = UIText.INFO_A_SELECT
	end
	
	--[[
	if car.own_it == false and car.boss == true and car.boss_state == "car_locked" then
		options[ #options + 1 ] = UIText.INFO_CHALLENGE_BOSS_A
	end
	--]]
	
	options[ #options + 1 ] = UIText.INFO_B_BACK
	
	if GUI.enable_respray == true and car.own_it == true and car_info.respray == true then
		options[ #options + 1 ] = UIText.INFO_SPRAY_X
	end
	
	if GUI.is_multiplayer == true and GUI.current_mode == GUI.mode.car then
		if GUI.viewing_challenges == true then
			options[ #options + 1 ] = UIText.INFO_STATS_Y
		elseif car.own_it == true and car.allowed ~= false and car.has_parent == false then
			if GUI.garage.rank[ car.id ] ~= nil then
				options[ #options + 1 ] = UIText.INFO_CHALLENGES_Y
			end
		end
	end
	
	--[[
	if GUI.is_multiplayer == false and GUI.current_mode == GUI.mode.car then
		if GUI.viewing_challenges == true then
			options[ #options + 1 ] = UIText.INFO_STATS_Y
		elseif car.own_it == true and car.boss == true and car.boss_no_mod ~= true then
			options[ #options + 1 ] = UIText.INFO_BOSS_INFO_Y
		end
	end
	--]]
	
	if GUI.enable_boss_mods == true then
		options[ #options + 1 ] = UIText.INFO_RB_MODS
	end

	if car.own_it == true and UIGlobals.Multiplayer.InLobby == false and Amax.CanUseShare() == true then
		options[ #options + 1 ] = "GAME_SHARE_BUTTON"
	end

	SetupInfoLine( unpack( options ) )
end

function Garage_UpdateWaitLoadTimer( time )
	if GUI.wait_respray_timer <= GUI.wait_respray_time then
		GUI.wait_respray_timer = GUI.wait_respray_timer + time
	end
	
	if GUI.wait_load_timer >= GUI.wait_load_time then
		return
	end
	
	GUI.wait_load_timer = GUI.wait_load_timer + time
	
	if GUI.wait_load_timer < GUI.wait_load_time then
		return
	end
	
	-- load in the new car
	local car = Garage_GetSelectedCar()
	
	local show_it = Garage_ShowIt( car )
	
	if car.id == GUI.viewing_id or ( show_it == false and GUI.viewing_id == -1 ) then
		return
	end
	
	if show_it == true then
		Garage_LoadCar( car )
		GUI.viewing_id = car.id
		
		if IsNumber( GUI.debug_car_id ) == true then
			local t = GameData.GetVehicle( car.id )
			print(t.tag)
			UIButtons.ChangeTexture( { filename = t.tag }, 0, GUI.debug_car_id )
		end
	else
		Amax.LoadLockedCarModel()
		GUI.viewing_id = -1
	end
	
	if car.own_it == true and car.new == true and car.allowed ~= false then
		Garage_ViewedCarNew( car.id, car.ids.new )
	end
end

function Garage_UpdateCurrentMode( new_mode )
	if new_mode == GUI.current_mode then
		return
	end
	
	-- turn off current mode
	if GUI.current_mode == GUI.mode.car then
		UIButtons.SetSelected( GUI.class_node_list_id, false )
		UIButtons.SetSelected( GUI.class[ GUI.current.class ].ids.node_list, false )
		UIButtons.TimeLineActive( "more", false ) -- arrows
			
	elseif GUI.current_mode == GUI.mode.colour then
		UIButtons.TimeLineActive( "colour_on", false )
		UIButtons.SetSelected( GUI.colour_picker_id, false )
		RestoreInfoLine()
		
	elseif GUI.current_mode == GUI.mode.boss_mods then
		UIButtons.SetSelected( GUI.boss_mod_node_list_id, false )
		UIButtons.TimeLineActive( "boss_mods_on", false )
		RestoreInfoLine()
		Garage_UpdateModLoadoutFromProfile()
		--UIButtons.PrivateTimeLineActive( GUI.bottom_help_id, "Hide_BottomHelp", true )
	end
	
	GUI.current_mode = new_mode
	
	-- turn on new mode
	if new_mode == GUI.mode.car then
		UIButtons.SetSelected( GUI.class_node_list_id, true )
		UIButtons.SetSelected( GUI.class[ GUI.current.class ].ids.node_list, true )
		Garage_CheckForClassChange( true ) -- arrows
		
	elseif new_mode == GUI.mode.colour then
		UIButtons.TimeLineActive( "colour_on", true )
		UIButtons.SetSelected( GUI.colour_picker_id, true )
		StoreInfoLine()
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_REVERT_MAN_X )
		Garage_RespraySetFromProfile()
		
	elseif new_mode == GUI.mode.boss_mods then
		UIButtons.SetSelected( GUI.boss_mod_node_list_id, true )
		UIButtons.TimeLineActive( "boss_mods_on", true )
		StoreInfoLine()
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_REMOVE_BOSS_MOD_X )
		Garage_CheckForModChange( true ) -- arrows
		--UIButtons.PrivateTimeLineActive( GUI.bottom_help_id, "Hide_BottomHelp", false )
	end
end

function Garage_End( confirm )
	local car = Garage_GetSelectedCar()
	local showroom = ( GUI.is_multiplayer == true and GUI.is_multiplayer_lobby == false ) or ( GUI.is_multiplayer == false and GUI.is_sp_event == false )
	
	if confirm == true and showroom == false then
		if car.own_it == true and car.allowed ~= false then
			UISystem.PlaySound( UIEnums.SoundEffect.GarageSelectVehicle )
			
			if GUI.is_multiplayer == true then
				Multiplayer.SetGarageCarByID( car.id )
				if GUI.is_multiplayer_lobby == true then
					UIButtons.TimeLineActive( "Hide_Lobby", false )
					Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
					Amax.ChangeUiCamera( Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 ), 0.3, 0 )
					PopScreen()
				else
					Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
					Amax.ChangeUiCamera( Amax.ChangeUiCamera( UIGlobals.CameraNames.MpOnline, UIGlobals.CameraLerpTime, 0 ), 0.3, 0 )
				end
			else
				Garage_SpSetCar()
				
				if GUI.is_sp_event == false then
					Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
					Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, 1, 0 )
					UIButtons.TimeLineActive( "Hide_Carousel", false )
					CloseCurrentApp()
				else
					SinglePlayer.MarkNotNew( GUI.sp_event_id )
				
					GUI.exit_to_game = true
					
					SpGoLoadingScreen()
					
					-- Pop the garage
					UIScreen.SetScreenTimers( 0.3, 0.3 )
					PopScreen()
					
					-- Flag to return from the game
					UIGlobals.SpReturnMode = UIGlobals.SpSelectMode_Event
				end
			end
		else
			PlaySfxError()
			
			--[[
			if car.boss == true and car.boss_state == "car_locked" then
				-- TODO
				print( "TODO - goto boss challenge" )
			else
				PlaySfxError()
			end
			--]]
		end
		
	elseif confirm == false then
		PlaySfxBack()
		
		if GUI.is_multiplayer == true then
			if GUI.is_multiplayer_lobby == true then
				UIButtons.TimeLineActive( "Hide_Lobby", false )
				Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
				Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
				PopScreen()
			else
				Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
				Amax.ChangeUiCamera( UIGlobals.CameraNames.MpOnline, UIGlobals.CameraLerpTime, 0 )
				GoScreen( "Multiplayer\\MpOnline.lua" )
			end
		else			
			if GUI.is_sp_event == false then
				UIButtons.TimeLineActive( "Hide_Carousel", false )
				Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
				Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, 1, 0 )
				CloseCurrentApp( true )
			else
				UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_PreRace

				Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
				Amax.ChangeUiCamera( "Sp_1", 0.75, 0 )
				UIButtons.TimeLineActive( "Hide_PreRace", false )
				PopScreen()
			end
		end
	end
	
	if GUI.is_multiplayer_lobby == true then
		local car_amax_id = Multiplayer.GetGarageCarByID( GUI.mp_class_restriction )
		NetRace.ChangeLocalPlayerVehicle( Profile.GetPrimaryPad(), car_amax_id )
		UIGlobals.Multiplayer.LobbyUpdateCar = true
	end
	
	if showroom == true and confirm == false and car.own_it == true then
		Multiplayer.SetGarageCarByID( car.id )	-- set background car for when coming back
	end
	
	local load_new_showroom = ( showroom == true and confirm == false and car.own_it == true and GUI.wait_load_timer < GUI.wait_load_time )
	local load_new_other	= ( showroom == false and confirm == true and car.own_it == true and GUI.wait_load_timer < GUI.wait_load_time )
	local load_new			= ( load_new_showroom or load_new_other )
	
	if load_new == true then
		if car.id ~= GUI.viewing_id then
			Garage_LoadCar( car )
			GUI.viewing_id = car.id
		end
		
		GUI.wait_load_timer = GUI.wait_load_time
		
		if car.new == true and car.allowed ~= false then
			Garage_ViewedCarNew( car.id, car.ids.new )
		end
	end
	
	local load_old_showroom = ( showroom == true and confirm == false and car.own_it == false and GUI.viewing_id ~= GUI.viewing_id_old )
	local load_old_other	= ( showroom == false and confirm == false and GUI.viewing_id ~= GUI.viewing_id_old )
	local load_old			= ( load_old_showroom or load_old_other )
	
	if load_old == true then
		Amax.LoadCar( GUI.viewing_id_old, GUI.is_multiplayer, GUI.is_team_game, GUI.local_team )
		GUI.viewing_id = GUI.viewing_id_old
	end
	
	if GUI.save_on_exit == true and GUI.is_multiplayer_lobby == false and ( confirm == false or ( confirm == true and showroom == false ) ) then
		StartAsyncSave()
	end
end

function Garage_RespraySetFromProfile( backing_out )
	local car = Garage_GetSelectedCar()
	local respray = GUI.resprays[ car.id ]
	
	if respray == nil then
		Amax.SetColourPickerFromRespray( GUI.colour_picker_id, 0, 0, GUI.is_multiplayer )
		Amax.ResprayCar( Select( backing_out, -1, GUI.colour_picker_id ) )
	else
		Amax.SetColourPickerFromRespray( GUI.colour_picker_id, respray.id_xt, respray.index, GUI.is_multiplayer )
		Amax.ResprayCar( GUI.colour_picker_id )
	end
end

function Garage_RespraySetToProfile()
	local car = Garage_GetSelectedCar()
	local id_xt_, index_ = Amax.ResprayCar( GUI.colour_picker_id, car.id, GUI.is_multiplayer )
	GUI.resprays[ car.id ] = { id_xt = id_xt_, index = index_, }
end

function Garage_ResprayRevert()
	local car = Garage_GetSelectedCar()
	Amax.ResprayCar( -1, car.id, GUI.is_multiplayer )
	GUI.resprays[ car.id ] = nil
end

function Garage_ChallengesOnOff( on, reset, force_timeline_end )
	if GUI.viewing_challenges == on then
		return
	end
	
	UISystem.PlaySound( UIEnums.SoundEffect.Filter )
	
	local timeline_name = Select( GUI.is_multiplayer, "challenges_on", "boss_unlocked_on" )
	local timeline_alpha_name = Select( GUI.is_multiplayer, "challenges_on_alpha", "boss_unlocked_on_alpha" )
	
	GUI.viewing_challenges_prev = GUI.viewing_challenges
	GUI.viewing_challenges = on
	
	UIButtons.TimeLineActive( timeline_alpha_name, on )
	
	if false then --force_timeline_end == true then	
		UIButtons.TimeLineActive( timeline_name, on, 0.25 )
	else
		UIButtons.TimeLineActive( timeline_name, on )
	end
	
	Garage_UpdateCarInfoLines()
	
	if reset == true then
		GUI.viewing_challenges_prev = GUI.viewing_challenges
	end
end

function Garage_PaletteUnlocked()
	return ( GUI.is_multiplayer == false or ( GUI.is_multiplayer == true and GUI.palette_locked_table[ GUI.current_palette ] == false ) )
end

function Garage_UpdateColourUnlocks()
	local challenge = GUI.garage.challenges[ Garage_GetSelectedCar().id ].winner
	
	for i, v in pairs( GUI.palette_ids ) do
		if i == UIEnums.Palette.Solid then
			GUI.palette_locked_table[ i ] = false
		else
			GUI.palette_locked_table[ i ] = ( i >= challenge.tier and challenge.complete == false )
		end
		
		if GUI.palette_locked_table[ i ] == true then
			UIButtons.ChangeColour( v, "Main_Black" )
		else
			UIButtons.ChangeColour( v, GUI.primary_colour )
		end
	end
	
	Amax.SetColourPaletteLocked( GUI.colour_picker_id, 
		GUI.palette_locked_table[ UIEnums.Palette.Solid ],
		GUI.palette_locked_table[ UIEnums.Palette.Metallic ],
		GUI.palette_locked_table[ UIEnums.Palette.Pearlecent ],
		GUI.palette_locked_table[ UIEnums.Palette.Candy ]
	)
	
	Garage_UpdateColourLocked()
end

function Garage_UpdateColourLocked()
	if GUI.current_palette == -1 then
		return
	end
	
	UIButtons.SetActive( SCUI.name_to_id[ "colour_lock" ], GUI.palette_locked_table[ GUI.current_palette ] )
		
	if GUI.current_mode == GUI.mode.colour then
		if Garage_PaletteUnlocked() == true then
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_REVERT_MAN_X )
		else
			SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_REVERT_MAN_X )
		end
	end
end

function Garage_ViewedCarNew( id, id_new )
	UIButtons.SetActive( id_new, false )
	
	if GUI.is_multiplayer == true then
		Multiplayer.CarViewed( id )
	else
		SinglePlayer.MarkNotNew( id )
	end
end

function Garage_SpSetCar()
	local car = Garage_GetSelectedCar()
	
	if GUI.is_sp_event == true then
		Amax.SetCurrentCar( car.id )
		SinglePlayer.EventSetLastVehicle( GUI.sp_event_id, car.id )
	end
	
	SinglePlayer.GameSetLastVehicle( car.id )
end

function Garage_ShowIt( car )
	return ( car.own_it == true or ( car.boss == true and car.boss_state == "car_locked" ) )
end

function Garage_UpdateCamera( time )
	if DeferCam_Update( time ) == true then
		local car = Garage_GetSelectedCar()
		
		if car.own_it == true then
			Garage_LoadCar( car )
		else
			-- just select the first in the list
			UIButtons.SetSelectionByIndex( GUI.class[ GUI.current.class ].ids.node_list, 0 )
			Garage_CheckForCarChange( true )
			local car = Garage_GetSelectedCar()
			
			-- [PR] #35988 - if this car is still locked then just show the question mark
			if Garage_ShowIt( car ) == false then
				Amax.LoadLockedCarModel()
				GUI.viewing_id = -1
			else
				Garage_LoadCar( car )
			end
		end
		
		Amax.ChangeEnvironmentSettings( true )
	end
end

function Garage_LoadCar( car )
	local team_game = GUI.is_team_game
	
	if team_game == true and car.allowed == false then
		team_game = false	-- default paint
	end
	
	Amax.LoadCar( car.id, GUI.is_multiplayer, team_game, GUI.local_team )
end
