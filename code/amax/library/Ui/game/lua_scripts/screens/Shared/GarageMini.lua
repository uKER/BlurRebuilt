GUI =
{
	enable_respray = true,
	
	finished = false,
	
	save_on_exit = false,
	sp_restarting = false,
	
	is_multiplayer = false,
	mp_class_restriction = 0,
	is_sp_event = false,
	sp_event_id = -1,
	custom_filter = false,
	dummy_boss_car_rank = 999,
	dummy_boss_car_rank_no_mod = 888,
	
	viewing_id_old = -1,
	viewing_id = -1,
	
	current_car_selection = 0,
	
	car_icon_ids = {  },
	car_name_ids = {  },
	car_new_ids = {  },
	car_locked_ids = {  },
	car_mp_rank_ids = {  },
	car_respray_outer_ids = {  },
	car_respray_inner_ids = {  },
	
	colour_picker_id = -1,
	
	mode = 
	{
		car = 0,
		colour = 1,
	},
	
	current_mode = 0,
	
	current_palette = -1,
	palette_ids = {  },
	palette_locked_table = {  },
	
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
	
	timeout_id = -1,
}

function Init()
	AddSCUI_Elements()
	CarouselApp_SetScreenTimers()
	
	if Amax.IsGameModeMultiplayer() == true then
		GUI.enable_respray=Multiplayer.GetFeatureUnlocked(UIEnums.MPFeatures.ResprayCar)
	else
		GUI.enable_respray=true
	end
	
	StoreInfoLine()
	SetupInfoLine()
	
	UISystem.PlaySound( UIEnums.SoundEffect.GarageEnter )	
	
	GUI.is_multiplayer = ( Amax.GetGameMode() ~= UIEnums.GameMode.SinglePlayer )
	
	if GUI.is_multiplayer == true then
		-- reset
		NetRace.ChangeLocalPlayerState( Profile.GetPrimaryPad(), UIEnums.Network.PlayerStates.eNetPlayerStateChangingCar )
		UIGlobals.Multiplayer.GarageActive = true
	else
		GUI.is_sp_event = true	-- TODO - do we need it not to be anytime?
		
		if GUI.is_sp_event == true then
			GUI.sp_event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
			GUI.sp_event_info = SinglePlayer.EventInfo( GUI.sp_event_id )
			--show_table( GUI.sp_event_info )
		end
		
		GUI.sp_game_info = SinglePlayer.GameInfo()
		GUI.sp_tier_info = SinglePlayer.TierInfo()
		GUI.sp_boss_info = SinglePlayer.BossInfo()
		GUI.sp_rank_info = SinglePlayer.RankInfo()
		
		if UIGlobals.GarageMiniFromPause == true then
			Paused_EnableCarousel( false )
		else
			UIButtons.TimeLineActive( "end_alpha", true )
		end
	end
	
	if GUI.is_multiplayer == true then
		GUI.timeout_id = SCUI.name_to_id[ "countdown" ]
	end
	
	if GUI.is_multiplayer == true then
		for i, v in pairs( UIGlobals.GarageMiniClassFilter ) do
			GUI.mp_class_restriction = i	-- should only ever have one of these anyway
			break
		end
	
		GUI.viewing_id_old = Multiplayer.GetGarageCarByID( GUI.mp_class_restriction )
	else
		if GUI.is_sp_event == true then
			GUI.viewing_id_old = GUI.sp_event_info.lastVehicle
		else
			GUI.viewing_id_old = GUI.sp_game_info.lastVehicle
		end
	end
	
	GUI.viewing_id = GUI.viewing_id_old
	
	local screen_id = SCUI.name_to_id[ "screen" ]
	
	if GUI.is_multiplayer == true then
		GUI.garage = Multiplayer.GetGarage( true )
		GUI.garage.car_info = {  }
		
		for i, v in pairs( GUI.garage.cars ) do
			GUI.garage.car_info[ i ] = GameData.GetVehicle( i )
			
			if GUI.garage.car_info[ i ] == nil then
				print( "ERROR: no vehicle info for id", i )
				GUI.garage.cars[ i ] = nil
			else
				GUI.garage.cars[ i ].id = GUI.garage.car_info[ i ].id
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
		
		for i, v in pairs( GUI.garage.cars ) do
			GUI.garage.car_info[ i ] = GameData.GetVehicle( i )
			
			if GUI.garage.car_info[ i ] == nil then
				print( "ERROR: no vehicle info for id", i )
				GUI.garage.cars[ i ] = nil
			else
				if v.unlocked == true then
					GUI.garage.garage[ i ] = true
				end
				
				GUI.garage.cars[ i ].own_it = ( GUI.garage.garage[ i ] ~= nil )
				
				if Multiplayer.ShowAllGarage() == true then
					GUI.garage.cars[ i ].own_it = true
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
					
					--v.boss_state = "tier_locked"
					--v.boss_state = "event_locked"
					--v.boss_state = "car_locked"
					--v.boss_state = "car_unlocked"
				end
			end
		end
		
		if GUI.is_sp_event == true then
			GUI.custom_filter = Sp_GarageFilterCars()
		end
		
		-- TODO - recommended?
		GUI.recommended = Amax.GetRecommendedVehicles()
	end
	--show_table( GUI.garage, 1 )
	
	-- grab respray data
	GUI.resprays = GameProfile.GetResprays( GUI.is_multiplayer )
	
	GarageMini_FilterCars()
	
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
	
	table.sort( GUI.cars, sort_func )
	
	-- car list
	GUI.car_node_list_id = SCUI.name_to_id[ "car_node_list" ]
	
	for i = 1, #GUI.cars, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Shared\\GarageMini.lua", "_car_node" )
		
		GUI.car_icon_ids[ #GUI.car_icon_ids + 1 ] = UIButtons.FindChildByName( node_id, "_car_icon" )
		GUI.car_name_ids[ #GUI.car_name_ids + 1 ] = UIButtons.FindChildByName( node_id, "_car_name" )
		GUI.car_new_ids[ #GUI.car_new_ids + 1 ]  = UIButtons.FindChildByName( node_id, "_car_new" )
		GUI.car_locked_ids[ #GUI.car_locked_ids + 1 ]  = UIButtons.FindChildByName( node_id, "_car_locked_icon" )
		GUI.car_mp_rank_ids[ #GUI.car_mp_rank_ids + 1 ]  = UIButtons.FindChildByName( node_id, "_car_mp_rank_unlock" )
		GUI.car_respray_outer_ids[ #GUI.car_respray_outer_ids + 1 ]  = UIButtons.FindChildByName( node_id, "_respray_outer" )
		GUI.car_respray_inner_ids[ #GUI.car_respray_inner_ids + 1 ]  = UIButtons.FindChildByName( node_id, "_respray_inner" )
		
		UIButtons.AddListItem( GUI.car_node_list_id, node_id, i )
	end
	
	-- class names
	GUI.class_icon_names =
	{
		[ UIEnums.VehicleClass.A ] = "Class_A",
		[ UIEnums.VehicleClass.B ] = "Class_B",
		[ UIEnums.VehicleClass.C ] = "Class_C",
		[ UIEnums.VehicleClass.D ] = "Class_D",
		[ UIEnums.VehicleClass.E ] = "Class_E",
	}
	
	-- style names
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
end

function PostInit()
	GUI.colour_picker_id = SCUI.name_to_id[ "colour_picker" ]
	
	if UIGlobals.GarageMiniFromPause == true then
		UIButtons.TimeLineActive( "stage1", true )
		UIButtons.TimeLineActive( "light_fade", true )
	end
	
	UIButtons.TimeLineActive( "stage2", true )
	
	if GUI.is_multiplayer == true or UIGlobals.GarageMiniFromPause == true then
		UIButtons.SetActive( SCUI.name_to_id[ "darken" ], true )
	end
	
	if GUI.is_multiplayer == true then
		-- if in lobby and countdown has started
		if UIGlobals.Multiplayer.InLobby == true and UIGlobals.Multiplayer.CountdownStarted == true then
			UIButtons.SetActive( GUI.timeout_id, true )
			UIButtons.ChangeText( GUI.timeout_id, "MPL_TIMER_COUNTDOWN" .. UIGlobals.Multiplayer.LobbyTimer )
		end

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
			local car_challenges = Multiplayer.GetCarChallenges( v + 1, false ) -- Do we want legend and dlc cars in SS?
			
			for j, w in ipairs( car_challenges ) do
				GUI.garage.challenges[ w.id ] = w
			end
		end
	else
		-- disable car upgrades and palette unlocks
		UIButtons.SetActive( SCUI.name_to_id[ "stat_challenges_title" ], false, true )
		UIButtons.SetActive( SCUI.name_to_id[ "colour_lock" ], false )
		UIButtons.SetXtVar( GUI.colour_picker_id, "disable_additional_palettes", true )
		UIButtons.SetXtVar( GUI.colour_picker_id, "disable_respray_messages", true )
		UIButtons.SetActive( SCUI.name_to_id[ "palette_dummy" ], false, true )
	end
	
	local found = false	
	for i, v in ipairs( GUI.cars ) do
		if v == GUI.viewing_id then
			UIButtons.SetSelection( GUI.car_node_list_id, i )
			found = true
			break
		end
	end
	
	if found == false then
		if GUI.is_multiplayer == true then
			Multiplayer.SetGarageCarByID( GarageMini_GetSelectedCar().id )	-- need a default - background car is different
		else
			print( "SP current car not found - do something?" )
		end
	end

	GarageMini_UpdateCarList()
	GarageMini_CheckForCarChange()
	
	if GUI.is_sp_event == true then
		UIButtons.SetParent( SCUI.name_to_id[ "screen" ], SCUI.name_to_id[ "retry_dummy" ], UIEnums.Justify.MiddleCentre )
		UIButtons.TimeLineActive( "HelpFade", false )
	end
	
	UIGlobals.OptionsTable = Amax.Options()
	GUI.old_diff=UIGlobals.OptionsTable.difficulty
	
	-- rank icon
	UIButtons.SetActive( SCUI.name_to_id[ "unlock_rank_icon" ], GUI.is_multiplayer )
	UIButtons.SetActive( SCUI.name_to_id[ "unlock_rank_icon_sp" ], not GUI.is_multiplayer )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.current_mode == GUI.mode.car then
		if message == UIEnums.Message.MenuNext then
			local car = GarageMini_GetSelectedCar()
			
			if car.own_it == true then
				if GUI.is_multiplayer == true then
					PlaySfxNext()
					Multiplayer.SetGarageCarByID( car.id )
					Amax.LoadCar( car.id, GUI.is_multiplayer )
					GarageMini_Back()
				else
					PlaySfxNext()
					GarageMini_LoadGame()
				end
			else
				PlaySfxError()
			end
		
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			
			if GUI.old_diff ~= UIGlobals.OptionsTable.difficulty then
			
				print( "Resetting Difficulty to ", GUI.old_diff )
				
				UIGlobals.OptionsTable.difficulty = GUI.old_diff
				Amax.Options( UIGlobals.OptionsTable )
				StartAsyncSave()
			end
			
			GarageMini_Back()
		
		elseif message == UIEnums.Message.ButtonX then
			if GUI.enable_respray == false then
				return
			end
			
			local car, car_info = GarageMini_GetSelectedCar()
			
			if car.own_it == true and car_info.respray == true then
				PlaySfxNext()
				GarageMini_UpdateCurrentMode( GUI.mode.colour )
			end
		
		elseif message == UIEnums.Message.ButtonY and GUI.is_sp_event == true then
			UIGlobals.OptionsTable = Amax.Options()			
			SetupCustomPopup( UIEnums.CustomPopups.SpChangeDifficulty )	
		end
		
	elseif GUI.current_mode == GUI.mode.colour then
		if message == UIEnums.Message.MenuNext then
			if GarageMini_PaletteUnlocked() == true then
				UISystem.PlaySound( UIEnums.SoundEffect.GarageReSpray )
				GUI.save_on_exit = true
				GarageMini_RespraySetToProfile()
				GarageMini_UpdateCurrentMode( GUI.mode.car )
			else
				PlaySfxError()
			end
			
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			
			GarageMini_RespraySetFromProfile()
			GarageMini_UpdateCurrentMode( GUI.mode.car )
		
		elseif message == UIEnums.Message.ButtonX then
			UISystem.PlaySound( UIEnums.SoundEffect.GarageReSpray )
			
			GUI.save_on_exit = true
			GarageMini_ResprayRevert()
			GarageMini_UpdateCurrentMode( GUI.mode.car )
		end
	end
end

function FrameUpdate( time )
	GarageMini_CheckForCarChange()
	
	if GUI.is_multiplayer == true then
		GarageMini_CheckForPaletteChange()
	end

	-- update timer in multiplayer
	if GUI.is_multiplayer == true and UIGlobals.Multiplayer.InLobby == true and UIGlobals.Multiplayer.CountdownStarted == true then
		UIButtons.ChangeText( GUI.timeout_id, "MPL_TIMER_COUNTDOWN" .. UIGlobals.Multiplayer.LobbyTimer )
	end
end

function EnterEnd()
	UISystem.PlaySound( UIEnums.SoundEffect.GarageExit )
	RestoreInfoLine()
	
	if GUI.is_sp_event == true and GUI.sp_restarting == true then
		SetupInfoLine()
	end
	
	if UIGlobals.GarageMiniFromPause == true then
		if GUI.sp_restarting ~= true then
			UIButtons.TimeLineActive( "stage1", false )
			UIButtons.TimeLineActive( "light_fade", false )
		end
	end
	
	UIGlobals.GarageMiniFromPause = false	-- reset
end

function EndLoop( time )
	--
end

function End()
	if GUI.is_multiplayer == true then
		NetRace.ChangeLocalPlayerState( Profile.GetPrimaryPad(), UIEnums.Network.PlayerStates.eNetPlayerStateIdle )
		UIGlobals.Multiplayer.GarageActive = false
	end
end

function GarageMini_UpdateCarList()
	for i, v in ipairs( GUI.cars ) do
		local car = GUI.garage.cars[ v ]
		local car_info = GUI.garage.car_info[ v ]
		
		if car.own_it == false then
			if GUI.is_multiplayer == true then
				car.unlock_at = GUI.garage.rank[ car.id ]
				car.tease = false --( car.unlock_at <= ( Multiplayer.ProfileProgression().level + 5 ) )
			else
				car.unlock_at = GUI.garage.rank[ car.id ]
				
				if car.boss == true then
					car.tease = false --( car.boss_state ~= "none" and car.boss_state ~= "tier_locked" )
				else
					car.tease = false --( car.unlock_at <= ( GUI.sp_game_info.rank + 5 ) )
				end
			end
		end
		
		if Multiplayer.ShowAllGarage() == true then
			car.tease = false
		end
		
		UIButtons.ChangeText( GUI.car_name_ids[ i ], car_info.name_full )
		UIShape.ChangeSceneName( GUI.car_icon_ids[ i ], car_info.sheet )
		UIShape.ChangeObjectName( GUI.car_icon_ids[ i ], car_info.icon )
		
		if car.own_it == true then
			UIButtons.SetActive( GUI.car_name_ids[ i ], true )
			UIButtons.SetActive( GUI.car_icon_ids[ i ], true )
			
			if car.new == true then
				UIButtons.SetActive( GUI.car_new_ids[ i ], true )
			end
		else
			UIButtons.SetActive( GUI.car_mp_rank_ids[ i ], true )
			
			if GUI.is_multiplayer == false and ( car.unlock_at == GUI.dummy_boss_car_rank or car.unlock_at == GUI.dummy_boss_car_rank_no_mod ) then
				UIButtons.ChangeText( GUI.car_mp_rank_ids[ i ], UIText.CMN_NOWT )
			else
				if GUI.is_multiplayer == true then
					UIButtons.ChangeText( GUI.car_mp_rank_ids[ i ], "MPL_UNLOCK_RANK" .. car.unlock_at )
				else
					UIButtons.ChangeText( GUI.car_mp_rank_ids[ i ], "SPL_UNLOCK_STATUS" .. car.unlock_at )
				end
			end
			
			if car.tease == true then
				UIButtons.SetActive( GUI.car_icon_ids[ i ], true )
				UIButtons.SetActive( GUI.car_name_ids[ i ], true )
				UIButtons.ChangeColour( GUI.car_icon_ids[ i ], "Locked" )
				UIButtons.ChangeColour( GUI.car_name_ids[ i ], "Locked" )
			else
				UIButtons.SetActive( GUI.car_locked_ids[ i ], true )
			end
		end
		
		GarageMini_RespraySetCar( car.id, i )
	end
end

function GarageMini_GetSelectedCar( _GUI )
	if _GUI == nil then
		_GUI = GUI
	end
	
	local car_selected = UIButtons.GetSelection( _GUI.car_node_list_id )
	local id = _GUI.cars[ car_selected ]
	
	return _GUI.garage.cars[ id ], _GUI.garage.car_info[ id ], car_selected
end

function GarageMini_UpdateStats()
	local car, car_info = GarageMini_GetSelectedCar()
	--show_table( car )
	
	-- man badge
	--[[
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
	--]]
	
	local class_icon_id = SCUI.name_to_id[ "stat_class_icon" ]
	local car_name_id = SCUI.name_to_id[ "stat_car_name" ]
	
	UIShape.ChangeObjectName( class_icon_id, GUI.class_icon_names[ car_info.class ] )
	
	if car.own_it == true then
		UIButtons.ChangeText( car_name_id, car_info.name_full )
		UIButtons.ChangeColour( car_name_id, "Main_White" )
		UIButtons.ChangeColour( class_icon_id, GUI.primary_colour )
	elseif car.tease == true then
		UIButtons.ChangeText( car_name_id, car_info.name_full )
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
			if car_info.ability ~= -1 then
				UIShape.ChangeObjectName( SCUI.name_to_id[ "sp_boss_mod_icon" ], GUI.garage.boss_mods[ GUI.garage.boss_mods_inv[ car_info.ability ] ].info.icon_name )
			end
			--]]
		end
	end
	
	if car.own_it == true then
		UIButtons.ChangeText( SCUI.name_to_id[ "stat_style_text" ], GUI.style_names[ car_info.style ] )
		
		Amax.SetChickletsValue( SCUI.name_to_id[ "chick_acceleration" ], car_info.acceleration )
		Amax.SetChickletsValue( SCUI.name_to_id[ "chick_speed" ], car_info.speed )
		Amax.SetChickletsValue( SCUI.name_to_id[ "chick_off_road" ], car_info.stability )
		Amax.SetChickletsValue( SCUI.name_to_id[ "chick_difficulty" ], car_info.difficulty )
		Amax.SetChickletsValue( SCUI.name_to_id[ "chick_health" ], car_info.strength )
		
		if GUI.is_multiplayer == true then
			-- challenge stat icons
			local challenge = GUI.garage.challenges[ car.id ].repeat_offender
			
			for j, w in ipairs( GUI.challenge_rewards.repeat_offender ) do
				local icon_id = SCUI.name_to_id[ "repeat_offender_icon_" .. j ]
				
				if challenge.tier > j or challenge.complete == true then
					UIButtons.ChangeColour( icon_id, "Support_4" )
				else
					UIButtons.ChangeColour( icon_id, "Main_Black" )
				end
			end
			
			GarageMini_UpdateColourUnlocks()
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
	
	if GUI.is_multiplayer == false then
		-- check for recommended
		if GUI.recommended ~= nil then
			if GUI.recommended[ car.id ] ~= nil then
				UIButtons.TimeLineActive( "recommended_on", true, 0 )
			else
				UIButtons.TimeLineActive( "recommended_on", false )
			end
		end
	end
	
	GarageMini_UpdateCarInfoLines()
end

function GarageMini_UpdateCarInfoLines( mode_override, _GUI )
	if _GUI == nil then
		_GUI = GUI
	end
	
	local car, car_info = GarageMini_GetSelectedCar( _GUI )
	
	local options = {  }
	
	if car.own_it == true then
		options[ #options + 1 ] = UIText.INFO_A_SELECT
	end
	
	options[ #options + 1 ] = UIText.INFO_B_BACK
	
	if GUI.enable_respray == true and car.own_it == true and car_info.respray == true then
		options[ #options + 1 ] = UIText.INFO_SPRAY_X
	end
	
	if GUI.current_mode == GUI.mode.car and GUI.is_sp_event == true then
		options[ #options + 1 ] = UIText.INFO_Y_DIFFICULTY
	end
	
	SetupInfoLine( unpack( options ) )
end

function GarageMini_CheckForCarChange()
	if GUI.current_mode ~= GUI.mode.car then
		return
	end
	
	local car_selection = UIButtons.GetSelection( GUI.car_node_list_id )
	
	if GUI.current_car_selection ~= car_selection then
		GUI.current_car_selection = car_selection
		GarageMini_UpdateStats()
	end
end

function GarageMini_CheckForPaletteChange()
	local palette = Amax.GetColourPalette( GUI.colour_picker_id )
	
	if palette ~= GUI.current_palette then
		for i, v in pairs( GUI.palette_ids ) do
			UIButtons.SetSelected( v, ( i == palette ) )
		end
		
		GUI.current_palette = palette
		
		GarageMini_UpdateColourLocked()
	end
end

function GarageMini_LoadGame()
	local car = GarageMini_GetSelectedCar()
	
	if GUI.is_sp_event == true then
		Amax.SetCurrentCar( car.id )
		SinglePlayer.EventSetLastVehicle( GUI.sp_event_id, car.id )
	end
	
	local last_car_id = SinglePlayer.GameInfo().lastVehicle
	
	SinglePlayer.GameSetLastVehicle( car.id )
	SinglePlayer.MarkNotNew( car.id )
	
	GUI.sp_restarting = true
	
	-- load
	if GUI.is_sp_event == true then
		GameProfile.RetryingEvent()
		StopIngameMusic()
		
		Amax.SendMessage( UIEnums.GameFlowMessage.UnPause )
		Amax.SendMessage( UIEnums.GameFlowMessage.StopGameRendering )
		Amax.SendMessage( UIEnums.GameFlowMessage.StopGameUpdate )

		if last_car_id == car.id then
			UIGlobals.IsQuickRestart = true
		else
			UIGlobals.IsQuickRestart = false
		end
		
		-- update save game
		if UIGlobals.GarageMiniFromPause == true then
			SinglePlayer.AddToTotalFans()
		end
		
		UIGlobals.Ingame = {  }
		--StartGameLoad()
		GoLoadingScreen( "Loading\\LoadingSpGame.lua" )
		
		UIButtons.TimeLineActive( "start_load", true )
	end
	
	--[[
	if UIGlobals.GarageMiniFromPause == true then
		Paused_TransmitterOn()
	else	
		SpRaceAwards_TransmitterOn()
	end	
	]]--
	
	PopScreen()
	
	SetupInfoLine()
end

function GarageMini_Back()
	print( "garage should save", GUI.save_on_exit )
	
	if GUI.save_on_exit == true and UIGlobals.Multiplayer.InLobby ~= true and GUI.is_sp_event == false then
		StartAsyncSave()
	end
	
	if GUI.is_multiplayer == true then
		local car_amax_id = Multiplayer.GetGarageCarByID( GUI.mp_class_restriction )
		NetRace.ChangeLocalPlayerVehicle( Profile.GetPrimaryPad(), car_amax_id )
		UIGlobals.Multiplayer.LobbyUpdateCar = true
	end
	
	UIButtons.TimeLineActive( "stage2", false )
	
	if GUI.is_sp_event == true then
		UIButtons.TimeLineActive( "HelpFade", true )
		
		if UIGlobals.GarageMiniFromPause == true then
			Paused_EnableCarousel( true )
		else
			UIButtons.TimeLineActive( "end_alpha", false )
			SetupCustomPopup( UIEnums.CustomPopups.SpPostRaceOptions )
		end
	end
	
	PopScreen()
end

function GarageMini_UpdateCurrentMode( new_mode )
	if new_mode == GUI.current_mode then
		return
	end
	
	-- turn off current mode
	if GUI.current_mode == GUI.mode.car then
		UIButtons.SetSelected( GUI.car_node_list_id, false )
		
	elseif GUI.current_mode == GUI.mode.colour then
		UIButtons.TimeLineActive( "colour_on", false )
		UIButtons.SetSelected( GUI.colour_picker_id, false )
		RestoreInfoLine()
		GarageMini_ResprayUpdateCarSelection()
	end
	
	-- turn on new mode
	if new_mode == GUI.mode.car then
		UIButtons.SetSelected( GUI.car_node_list_id, true )
		
	elseif new_mode == GUI.mode.colour then
		UIButtons.TimeLineActive( "colour_on", true )
		UIButtons.SetSelected( GUI.colour_picker_id, true )
		StoreInfoLine()
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_REVERT_MAN_X )
		GarageMini_RespraySetFromProfile()
	end
	
	GUI.current_mode = new_mode
end

function GarageMini_RespraySetCar( id, i )
	if GUI.resprays[ id ] ~= nil then
		UIButtons.SetActive( GUI.car_respray_outer_ids[ i ], true )
		UIButtons.SetActive( GUI.car_respray_inner_ids[ i ], true )
		
		local respray = Amax.GetResprayColour( GUI.resprays[ id ].id_xt )
		
		if respray ~= nil then
			UIButtons.ChangeColour( GUI.car_respray_inner_ids[ i ], "!" .. respray )
		end
	else
		UIButtons.SetActive( GUI.car_respray_outer_ids[ i ], false )
		UIButtons.SetActive( GUI.car_respray_inner_ids[ i ], false )
	end
end

function GarageMini_ResprayUpdateCarSelection()
	local car, car_info, car_selection = GarageMini_GetSelectedCar()
	GarageMini_RespraySetCar( car.id, car_selection )
end

function GarageMini_RespraySetFromProfile()
	local car = GarageMini_GetSelectedCar()
	local respray = GUI.resprays[ car.id ]
	
	if respray == nil then
		Amax.SetColourPickerFromRespray( GUI.colour_picker_id, 0, 0, GUI.is_multiplayer )
	else
		Amax.SetColourPickerFromRespray( GUI.colour_picker_id, respray.id_xt, respray.index, GUI.is_multiplayer )
	end
end

function GarageMini_RespraySetToProfile()
	local car = GarageMini_GetSelectedCar()
	local id_xt_, index_ = Amax.ResprayCar( GUI.colour_picker_id, car.id, GUI.is_multiplayer, true )
	GUI.resprays[ car.id ] = { id_xt = id_xt_, index = index_, }
end

function GarageMini_ResprayRevert()
	local car = GarageMini_GetSelectedCar()
	Amax.ResprayCar( -1, car.id, GUI.is_multiplayer, true )
	GUI.resprays[ car.id ] = nil
end

function GarageMini_PaletteUnlocked()
	return ( GUI.is_multiplayer == false or ( GUI.is_multiplayer == true and GUI.palette_locked_table[ GUI.current_palette ] == false ) )
end

function GarageMini_UpdateColourUnlocks()
	local challenge = GUI.garage.challenges[ GarageMini_GetSelectedCar().id ].winner
	
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
	
	GarageMini_UpdateColourLocked()
end

function GarageMini_UpdateColourLocked()
	if GUI.current_palette == -1 then
		return
	end
	
	UIButtons.SetActive( SCUI.name_to_id[ "colour_lock" ], GUI.palette_locked_table[ GUI.current_palette ] )
	
	if GUI.current_mode == GUI.mode.colour then
		if GarageMini_PaletteUnlocked() == true then
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_REVERT_MAN_X )
		else
			SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_REVERT_MAN_X )
		end
	end
end

function GarageMini_FilterCars()
	GUI.cars = {  }
	
	if GUI.is_multiplayer == true then
		-- filter by class
		local IsValidClass = function( class )
			if UIGlobals.GarageMiniClassFilter[ UIEnums.VehicleClass.ANY ] ~= nil then
				return true
			end
			
			return ( UIGlobals.GarageMiniClassFilter[ class ] ~= nil )
		end
		
		for i, v in pairs( GUI.garage.cars ) do
			if IsValidClass( GUI.garage.car_info[ i ].class ) then
				GUI.cars[ #GUI.cars + 1 ] = i
			end
		end
	else
		if GUI.sp_event_info == nil then
			return
		end
		
		if GUI.sp_event_info.restriction == "none" then
			-- nothing to do
			
		elseif GUI.sp_event_info.restriction == "class" then
			for i, v in pairs( GUI.garage.cars ) do
				if GUI.sp_event_info.carClass == UIEnums.VehicleClass.ANY or GUI.garage.car_info[ i ].class == GUI.sp_event_info.carClass then
					GUI.cars[ #GUI.cars + 1 ] = i
				end
			end
			
		elseif GUI.sp_event_info.restriction == "set" then
			for i, v in pairs( GUI.garage.cars ) do
				if GUI.sp_event_info.carList[ i ] == true then
					GUI.cars[ #GUI.cars + 1 ] = i
				end
			end
			
			if #GUI.cars == 0 then
				print( "ERROR - carList restriction has no cars" )
			end
			
			return true
			
		else
			print( "WARNING - unknown car restriction" )
		end
		
		return false
	end
end
