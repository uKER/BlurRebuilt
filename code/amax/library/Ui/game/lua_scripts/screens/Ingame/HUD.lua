GUI =
{
	finished = false,
	allow_pause = true,
	
	automation_start = true,	-- Debug
	
	num_players = 1,			-- TODO - this will come from the game
	vertical_split = false,		-- TODO - this will come from the game
	
	item_table = {  },			-- contains useful info on each of the HUD items for each player - filled in later
	vp_table = {  },			-- viewport values per players
	vp_table_prev = {  },		-- viewport values per players for the previous frame
	
	ai_hud_ids = {  },
	
	safe_2d_ids = {  },
	safe_3d_ids = {  },
	inter_2d_ids = {  },
	inter_3d_ids = {  },
	
	revs_partial_ids = {  },
	
	time_id = -1,

	going_to_pause_pad = -1,	-- set when we're going to pause menu
	
	mp_rivals_ids = {  },
	ss_timer_ids = {  },
	mp_finished_ids = {  },
	mp_eliminated_ids = {  },
	mp_wrecked_ids = {  },
	
	fan_msg_ids = {  },
	
	mp_destruction_positions = {  },
	mp_destruction_ids = {  },
	
	mp_network_elimination_on = false,
	mp_network_elimination_me = false,
	
	mp_network_current_fans = -1,
	mp_network_current_rank = -1,
	
	is_network_race = false,
	
	mp_ss_finished_ids = {  },
	mp_ss_wrecked_ids = {  },
	
	mp_ss_messages = { --[[ + player stuff here ]] },

	mp_fan_msg_root = {  },
	mp_fan_msg_rb = {  },
	mp_rank_up_ids = {  },
	mp_challenge_complete_ids = {  },
	mp_rank_or_challenge_stack = {  },
	mp_fan_msg_activate_in = {  },
	
	sp = {  },

	fan_combo_ids = {  },
	fan_combo_timer = {  },
	fan_combo_sy = -1,	
	fan_combo_py = -1,	
	fan_combo_tex = { filename = "hud_main", pos = { u = 0.3125, v = 0.125 }, size = { u = 0.0625, v = 0.125, } },
	
	LastGaspPowerupSlotID = nil,
	jump_the_gun_standalone_ID = nil,
}

function Init()
	Splitscreen_AddSplits()
	
	GUI.is_network_race = Amax.IsNetworkRace()
	
	Amax.SetGlobalFade(1.0) -- Hack, please removed
	UIScreen.SetScreenTimers( 0.3, 0.15 )
	
	net_SetRichPresence( UIEnums.RichPresence.InRace)

	GUI.num_players = Amax.GetNumViewports()
	
	-- helper functions
	UISystem.LoadLuaScript( "Screens\\Ingame\\HUD_Helpers.lua" )
	
	-- setup player viewports
	HUD.SetupPlayerViewports()

	-- pad input for splitscreen
	if IsSplitScreen() == true then
		Profile.LockToPad( -1 )	-- allow all active people to pause
		Profile.SetPrimaryPad( UIGlobals.Splitscreen.primary_pad ) -- make sure this is the correct player
	end
	
	-- Pause node attached to vehicle 0
	UIScreen.AddMessageNow( UIEnums.GameMessage.SetLocalPlayerIndex, 0 )
	
	if Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then
		-- these vars not persistent over race, init ONCE here
		UIGlobals.Ingame.last_gasp_mode_active = Amax.LastGaspModeActive( 0 )
		UIGlobals.Ingame.can_fire_last_gasp = Amax.LastGaspModeActive( 0 )
		UIGlobals.Ingame.has_last_gasp = Amax.PlayerHasLastGasp( 0 )
		UIGlobals.Ingame.has_jump_the_gun = Amax.PlayerHasJumpTheGun( 0 )
		
		if UIGlobals.Ingame.last_gasp_timer == nil or UIGlobals.Ingame.last_gasp_timer <= 0.0 then
			UIGlobals.Ingame.last_gasp_timer = Amax.GetLastGaspTime( 0 ) 
		end
	end
end

function EnableIntroItems( enable )
end

function PostInit()
	GUI.show_revs = Amax.ShowRevs()
	GUI.show_camera_helpers = Amax.ShowCameraHelpers()
	
	if IsSplitScreen() == false and Amax.IsRaceMpDestruction() == true then
		-- renderstate to stop the rival arrow showing in the mirror
		local read_on = UIButtons.CloneXtGadgetByName( "hud_objects", "Mirror_Copy_Read_On" )
	end

	for i = 1, GUI.num_players, 1 do
		local player_index = (i - 1)
				
		-- Full screen effects
		local clone_fades = 
		{
			UIButtons.CloneXtGadgetByName( "hud_objects", "FullScreen_Fade" ),
			UIButtons.CloneXtGadgetByName( "hud_objects", "FullScreen_Flash" ),
			UIButtons.CloneXtGadgetByName( "hud_objects", "FullScreen_FlashGraphic" ),
		}
		UIButtons.ReplaceTimeLineLabel( clone_fades[ 1 ], "HoG_fade", "HoG_fade_" .. player_index )
		UIButtons.ReplaceTimeLineLabel( clone_fades[ 2 ], "Flash_fade", "Flash_fade_" .. player_index )
		UIButtons.ReplaceTimeLineLabel( clone_fades[ 3 ], "FlashGraphic_fade", "FlashGraphic_fade_" .. player_index )
		
		local sl, sr, st, sb = GetScreenBounds( GUI.num_players, i, false )
		
		local sw = sr - sl
		local sh = sb - st

		for i,v in ipairs( clone_fades ) do
			UIButtons.ChangeSize( v, sw, sh )
			UIButtons.ChangePosition( v, ( sl + sw / 2 ), ( st + sh / 2 ))
		end
		
		-- Set clipping windows
		if GUI.num_players > 1 then
			local clipper_ids = 
			{  
				UIButtons.CloneXtGadgetByName( "hud_objects", "Clip_Push_41" ),
				UIButtons.CloneXtGadgetByName( "hud_objects", "Clip_Push_14" ),
				UIButtons.CloneXtGadgetByName( "hud_objects", "Clip_Push_31" ),
			}

			for i, v in ipairs( clipper_ids ) do
				UIButtons.SetXtVar( v, "render_state.clip_pos.vx",  sl )
				UIButtons.SetXtVar( v, "render_state.clip_pos.vy",  st )
				UIButtons.SetXtVar( v, "render_state.clip_size.vx", sw )
				UIButtons.SetXtVar( v, "render_state.clip_size.vy", sh )
			end
		end
		
		-- Root objects
		local clone_2d = UIButtons.CloneXtGadgetByName( "hud_objects", "2D_SAFE" )
		local inter_2d = UIButtons.FindChildByName( clone_2d, "2D_Intermediate" )
		local clone_3d = UIButtons.CloneXtGadgetByName( "hud_objects", "3D_SAFE" )
		local inter_3d = UIButtons.FindChildByName( clone_3d, "3D_Intermediate" )
		
		GUI.safe_2d_ids[ i ] = clone_2d
		GUI.safe_3d_ids[ i ] = clone_3d
		GUI.inter_2d_ids[ i ] = inter_2d
		GUI.inter_3d_ids[ i ] = inter_3d
		
		UIButtons.ChangeSize( inter_2d, UIButtons.GetSize( HUD.ViewportId[ i ] ) )
		UIButtons.SetParent( clone_2d, HUD.ViewportId[ i ], UIEnums.Justify.MiddleCentre )
				
		UIButtons.ChangeSize( inter_3d, UIButtons.GetSize( HUD.ViewportId3d[ i ] ) )
		UIButtons.SetParent( clone_3d, HUD.ViewportId3d[ i ], UIEnums.Justify.MiddleCentre )

		-- Fan messaging
		local fan_msg = UIButtons.CloneXtGadgetByName( "hud_objects", "FanMsg_Main_RB" )
		UIButtons.SetParent( fan_msg, inter_2d, UIEnums.Justify.TopCentre )
		GUI.mp_fan_msg_root[ i ] = fan_msg
		GUI.mp_fan_msg_rb[ i ] = UIButtons.FindChildByName( fan_msg, "bra_FanRoot" )
		
		if Amax.ShowAiNames() == false then
			-- shift bonus scoring
			local bonus_rb = UIButtons.FindChildByName( fan_msg, "dmy_FanBonus" )
			local x, y, z = UIButtons.GetPosition( bonus_rb )
			UIButtons.ChangePosition( bonus_rb, x, y - 15, z )
		end
		
		GUI.mp_fan_msg_activate_in[i] = -1

		-- Perks
		if IsSplitScreen() == true then
			local id = 	UIButtons.FindChildByName( inter_3d, "Perks_MainHUD" )

			-- Flatten them
			UIButtons.SetXtVar( id, "bender", 0.0 )
			-- [Bone] #36834 : Offset because the back of the car isn't central to the safe area
			if GUI.num_players == 4 then 
				-- All in 4VP (1,2,3,4
				local offset = Select( math.mod(i,2)==1, -2.2, 2.2 )
				UIButtons.ChangePosition( id, offset, nil, nil, true )
			elseif GUI.num_players > 2 and i > 1 then 
				-- Bottom 2 in 3VP (2,3)
				local offset = Select( math.mod(i,2)==0, -2.2, 2.2 )
				UIButtons.ChangePosition( id, offset, nil, nil, true )
			end			
		else
			-- Perks behind the car only exist in single viewport mode
			UIButtons.CloneXtGadgetByName( "hud_objects", "Perks_Dummy" )
			
			-- boss demand / sticker / challenge progress notifications
			local progress_id = UIButtons.CloneXtGadgetByName( "hud_objects", "ProgressMessage" )
			UIButtons.SetParent( progress_id, inter_2d, UIEnums.Justify.BottomLeft )
			
			if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer and SP_IsFriendDemand() == false then
				-- update boss pic
				local event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
				local event_info = SinglePlayer.EventInfo( event_id )
				AIGamerPicture( UIButtons.FindChildByName( progress_id, "ProgressBossPic" ), UIGlobals.boss_avatar_ids[ event_info.tier ] )				
			end
		end
		
		-- Mini map
		local map_dmy	= UIButtons.CloneXtGadgetByName( "hud_objects", "dmy_MiniMapScale" )
		local mini_map	= UIButtons.CloneXtGadgetByName( "hud_objects", "Map_Root" )
		UIButtons.SetParent( mini_map, inter_3d, UIEnums.Justify.TopRight )
		Amax.ChangeText( UIButtons.FindChildByName( mini_map, "Map_SpeedNumber" ),		"HUD_" .. player_index .. "_SPEED_MPH" )
		
		local rt_panel = UIEnums.Panel._RT0 + player_index
		UIButtons.ChangePanel( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad" ),	rt_panel )
		UIButtons.ChangePanel( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad_Alt" ), rt_panel )
				
		UIButtons.SetXtVar( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad" ), "name", "shp_MiniMapRoad" .. player_index )
		UIButtons.SetXtVar( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad_Alt" ), "name", "shp_MiniMapRoad_Alt" .. player_index )
		UIButtons.SetXtVar( UIButtons.FindChildByName( map_dmy, "dmy_MiniMapTranslate" ), "name", "dmy_MiniMapTranslate" .. player_index )
		UIButtons.SetXtVar( map_dmy, "name", "dmy_MiniMapScale" .. player_index )
		
		local mini_map_id = UIButtons.FindChildByName( mini_map, "Map_Main" )
		UIButtons.ChangeTexture( { filename = "MINIMAP_RT" .. player_index }, 5, mini_map_id )
		
		if GUI.show_revs == true then
			local revs	= UIButtons.CloneXtGadgetByName( "hud_objects", "Map_Revs" )
			UIButtons.SetParent( revs, mini_map, UIEnums.Justify.MiddleCentre )
			GUI.revs_partial_ids[ i ] = UIButtons.FindChildByName( revs, "Map_RevsPartial" )
		end

		-- Fans
		local fans = UIButtons.CloneXtGadgetByName( "hud_objects", "Fans_Icon" )
		UIButtons.SetParent( fans, mini_map, UIEnums.Justify.BottomRight )
		Amax.ChangeText( UIButtons.FindChildByName( mini_map, "Fans_Number" ), "HUD_" .. player_index .. "_FANS" )
		
		-- fans backing
		local fans_backing = UIButtons.CloneXtGadgetByName( "hud_objects", "Fans_BackingTop" )
		UIButtons.SetParent( fans_backing, inter_3d, UIEnums.Justify.TopRight )

		-- Combo
		local combo_icon_id = UIButtons.FindChildByName( fans, "Combo_Icon" )
		GUI.fan_combo_ids[ #GUI.fan_combo_ids + 1 ] = combo_icon_id
		
		GUI.fan_combo_timer[ #GUI.fan_combo_timer + 1 ] = -1
		
		local _, sy, _ = UIButtons.GetSize( combo_icon_id )
		GUI.fan_combo_sy = sy
		local _, py, _ = UIButtons.GetPosition( combo_icon_id )
		GUI.fan_combo_py = py
		
		Amax.ChangeText( UIButtons.FindChildByName( fans, "Combo_Number" ),	"HUD_" .. player_index .. "_COMBO" )

		-- Minor scoring
 		if IsSplitScreen() == false then
			local minor_fans_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Fans_MinorScore" )
			UIButtons.SetParent( minor_fans_id, inter_3d, UIEnums.Justify.TopRight )
		end
		
		local mini_data_offset = 0

		if Amax.IsRaceMpDestruction() == true then
			if NetRace.IsTeamGame() == true then
				local team = Multiplayer.GetPlayerTeam()
				
				HUD_MpDestructionSetupTeam( inter_3d, player_index, team, true )
				HUD_MpDestructionSetupTeam( inter_3d, player_index, Select( ( team == "team_a" ), "team_b", "team_a" ), false )
			else
				-- points
				local backing_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
				UIButtons.SetParent( backing_id, inter_3d, UIEnums.Justify.TopLeft )
				
				local title_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Points_Title" )
				UIButtons.SetParent( title_id, inter_3d, UIEnums.Justify.TopLeft )
				
				UIButtons.ChangeText( UIButtons.FindChildByName( title_id, "Points_Amount" ), "HUD_" .. player_index .. "_MP_CURRENT_POINTS" )
				UIButtons.SetActive( UIButtons.FindChildByName( title_id, "SpDestructionPointsSmall" ), false )
				
				-- position
				local backing_pos_id = UIButtons.CloneXtGadgetByName( "hud_objects", "SpNextBackingTop" )
				UIButtons.SetParent( backing_pos_id, inter_3d, UIEnums.Justify.TopLeft )
				UIButtons.ChangePosition( backing_pos_id, nil, 3, nil, true )
				
				UIButtons.ChangeText( UIButtons.FindChildByName( backing_pos_id, "SpNextMedalTitle" ), UIText.HUD_POS )
				UIButtons.ChangeText( UIButtons.FindChildByName( backing_pos_id, "SpNextMedalInfo" ), "HUD_" .. i - 1 .. "_MP_PLACE" )
				
				local next_light_dummy = UIButtons.FindChildByName( backing_pos_id, "SpNextLightDummy" )
				local light_next = HUD_SpCreateLight( next_light_dummy, 0 )
				
				-- don't need the lights
				UIButtons.SetActive( light_next.off, false )
				UIButtons.SetActive( light_next.on, false )
				
				-- don't need the arrow
				UIButtons.SetActive( UIButtons.FindChildByName( backing_pos_id, "SpNextArrow" ), false )
				
				GUI.mp_destruction_ids[ i ] = 
				{
					icon = UIButtons.FindChildByName( backing_pos_id, "SpNextDone" ),
					blob = light_next.blob,
				}
				
				-- rival
				local rival_root_id = UIButtons.CloneXtGadgetByName( "hud_objects", "SpBossHealthBackingTop" )
				UIButtons.SetParent( rival_root_id, inter_3d, UIEnums.Justify.TopLeft )
				
				UIButtons.ChangePosition( rival_root_id, nil, -4, nil, true )
				
				GUI.mp_rivals_ids[ i ] = 
				{
					pic = UIButtons.FindChildByName( rival_root_id, "SpBossHealthPic" ),
					name = UIButtons.FindChildByName( rival_root_id, "SpBossHealthName" ),
					health = UIButtons.FindChildByName( rival_root_id, "SpBossHealth" ),
					rival_title = UIButtons.FindChildByName( rival_root_id, "MpRivalTitle" ),
					rival_time = UIButtons.FindChildByName( rival_root_id, "MpRivalTime" ),
					blob = UIButtons.FindChildByName( rival_root_id, "SpBossHealthBlob" ),
				}
			end
			
		elseif Amax.SP_IsCheckpointRace() == true or Amax.SP_IsCheckpointFD() == true then
			HUD_SpCheckpointSetup( inter_3d, inter_2d )
			
		elseif Amax.SP_IsDestructionRace() == true or Amax.SP_IsDestructionFD() == true then
			HUD_SpDestructionSetup( inter_3d, inter_2d )
			
		elseif Amax.SP_IsFanDemandRace() == true or Amax.SP_IsFanDemandFD() == true then
			HUD_SpFanDemandSetup( inter_3d, inter_2d )
			
		else
			local pos_backing = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
			UIButtons.SetParent( pos_backing, inter_3d, UIEnums.Justify.TopLeft )
			
			-- Position indicator
			local pos = UIButtons.CloneXtGadgetByName( "hud_objects", "Position_Title" )
			UIButtons.SetParent( pos, inter_3d, UIEnums.Justify.TopLeft )
			
			local pos_amount = UIButtons.FindChildByName( pos, "Position_Amount" )
			Amax.ChangeText( pos_amount, "HUD_" .. player_index .. "_POS_NUM" )
			
			local pos_amount_of = UIButtons.FindChildByName( pos, "Position_AmountOf" )
			Amax.ChangeText( pos_amount_of, "HUD_" .. player_index .. "_POS_NUM_OF" )
			
			-- team icon
			if NetRace.IsTeamGame() == true then
				local team_icon = UIButtons.CloneXtGadgetByName( "hud_objects", "Position_Team_Icon" )
				UIButtons.SetParent( team_icon, pos, UIEnums.Justify.MiddleLeft )
				
				local team = Multiplayer.GetPlayerTeam()
				UIButtons.ChangeTextureUV( team_icon, 0, UIGlobals.TeamIcons[ team ].u, UIGlobals.TeamIcons[ team ].v )
				UIButtons.ChangeColour( team_icon, UIGlobals.TeamColours[ team ] )
				
				-- shift along
				local sx, _, _ = UIButtons.GetSize( team_icon )
				UIButtons.ChangePosition( pos, sx, nil, nil, true )
				UIButtons.ChangePosition( pos_amount, -sx, nil, nil, true )
				UIButtons.ChangePosition( pos_amount_of, -sx, nil, nil, true )
				
				-- tug of war
				local tug_of_war = UIButtons.CloneXtGadgetByName( "hud_objects", "Team_ToW" )
				UIButtons.SetParent( tug_of_war, inter_3d, UIEnums.Justify.TopLeft )
				
				mini_data_offset = mini_data_offset - 2.05
			end
			
			-- Lap counter
			local laps_title = UIButtons.CloneXtGadgetByName( "hud_objects", "Map_Laps" )
			UIButtons.SetParent( laps_title, mini_map, UIEnums.Justify.TopRight )
			
			local laps_num = UIButtons.FindChildByName( laps_title, "Map_LapsNum" )
			Amax.ChangeText( laps_num,	"HUD_" .. player_index .. "_LAP" )
			
			-- extra single player specific stuff
			if Amax.SP_IsStreetRace() == true or Amax.SP_IsStreetRaceFD() == true then
				HUD_SpStreetRaceSetup( inter_3d, inter_2d )
				
			elseif Amax.SP_IsBossRace() == true or Amax.SP_IsBossBattleFD() == true then
				HUD_SpBossSetup( inter_3d, inter_2d )
			end
		end

		-- friend demands
		if Amax.SP_IsStreetRaceFD() == true or Amax.SP_IsDestructionFD() == true or Amax.SP_IsCheckpointFD() == true or Amax.SP_IsFanDemandFD() == true or Amax.SP_IsBossBattleFD() == true then
			HUD_SpSetupFD( inter_3d, inter_2d )
		end
		
		if IsSplitScreen() == true then
			-- Don't wobble HUD
			UIButtons.SetActive( clone_2d, false )
			UIButtons.SetActive( clone_3d, false )
			
			-- Turn off fan damands root
			UIButtons.SetActive( UIButtons.FindChildByName( fan_msg, "bra_FanDemandRoot" ), false )
			UIButtons.SetActive( clone_fades[ 3 ], false )
			
			-- Race line
			local race_line = UIButtons.CloneXtGadgetByName( "hud_objects", "Ss_RaceLine_Main" )
			-- size to safe area
			local sx, sy, sz = UIButtons.GetSize( race_line )
			UIButtons.ChangeSize( race_line, Screen.safe.width, sy, sz )
			
			-- Change colours to suit VP
			local pad = UIGlobals.Splitscreen.players[ i ].pad
			local colour = UIGlobals.Splitscreen.colours[ pad ]
			--local inv_colour = UIGlobals.Splitscreen.inv_colours[ pad ]
			local inv_colour = "Support_4"
			
			UIButtons.SwapColour( inter_2d, "Main_1", colour )
			UIButtons.SwapColour( clone_3d, "Main_1", colour )
			
			-- minimap colours
			UIButtons.SetXtVar( mini_map_id, "colour_you", colour )
			UIButtons.ChangeColour( UIButtons.FindChildByName( map_dmy, "shp_MiniMapRoad" .. player_index ), inv_colour )
			
			-- move fan messaging a bit
			local x, y, z = UIButtons.GetPosition( fan_msg )
			UIButtons.ChangePosition( fan_msg, x, y - 57, z )
			
			-- add health bar
			local health_SS = UIButtons.CloneXtGadgetByName( "hud_objects", "Mirror_Root_SS" )
			UIButtons.SetParent( health_SS, inter_2d, UIEnums.Justify.TopCentre )
			
			local x, y, z = UIButtons.GetPosition( health_SS )
			UIButtons.ChangePosition( health_SS, x, y - 10, z )
			
			-- scale targeting
			local targeting = UIButtons.FindChildByName( inter_3d, "targeting" )
			UIButtons.ChangeScale( targeting, 0.725, 0.725, 1 )
			
			-- add finished stuff
			local ss_finished_rb = UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpFinished.lua", "FinishedDummy" )
			UIButtons.SetParent( ss_finished_rb, clone_2d, UIEnums.Justify.MiddleCentre )
			local x, y, z = UIButtons.GetPosition( ss_finished_rb )
			UIButtons.ChangePosition( ss_finished_rb, x - 320, y - 240, z )	-- whatever - dummy already has a parent attach
			UIButtons.ChangeLayer( ss_finished_rb, 4, true )	-- so they get clipped
			
			-- [PR] - #38066
			-- in 4:3 with more than two players, "finished" text is outside safe area
			if IsWideScreen() == false and GUI.num_players > 2 then	-- [Blur2PC] retail: IsWideScreen
				local finished_text_id = UIButtons.FindChildByName( ss_finished_rb, "Finished" )
				UIButtons.ChangeSize( finished_text_id, 375, 80, 0 )
			end
			
			-- turn off the stencil stuff
			UIButtons.SetActive( UIButtons.FindChildByName( ss_finished_rb, "StencilWrite" ), false )
			UIButtons.SetActive( UIButtons.FindChildByName( ss_finished_rb, "CSB_T" ), false )
			UIButtons.SetActive( UIButtons.FindChildByName( ss_finished_rb, "CSB_M" ), false )
			UIButtons.SetActive( UIButtons.FindChildByName( ss_finished_rb, "CSB_B" ), false )
			UIButtons.SetActive( UIButtons.FindChildByName( ss_finished_rb, "StencilWriteOff" ), false )
			
			-- turn on splitscreen finished
			UIButtons.SetActive( UIButtons.FindChildByName( ss_finished_rb, "CSB_T_SS" ), true )
			UIButtons.SetActive( UIButtons.FindChildByName( ss_finished_rb, "CSB_B_SS" ), true )
			
			local pos_dummy_id = UIButtons.FindChildByName( ss_finished_rb, "PosDummy" )
			local x, y, z = UIButtons.GetPosition( pos_dummy_id )
			UIButtons.ChangePosition( pos_dummy_id, x, y + 40, z )
			
			UIButtons.SetActive( ss_finished_rb, false )
			
			GUI.mp_ss_finished_ids[ #GUI.mp_ss_finished_ids + 1 ] = ss_finished_rb
			
			-- add wrecked stuff
			local ss_wrecked_rb = UIButtons.CloneXtGadgetByName( "Ingame\\SpWrecked.lua", "WreckedRenderBranch" )
			UIButtons.SetParent( ss_wrecked_rb, clone_2d, UIEnums.Justify.MiddleCentre )
			local x, y, z = UIButtons.GetPosition( ss_wrecked_rb )
			UIButtons.ChangePosition( ss_wrecked_rb, x - 320, y - 240, z )	-- whatever - dummy already has a parent attach
			UIButtons.ChangeLayer( ss_wrecked_rb, 4, true )	-- so they get clipped
			
			local wrecked_by_id = UIButtons.FindChildByName( ss_wrecked_rb, "Wrecked_By" )
			local x, y, z = UIButtons.GetPosition( wrecked_by_id )
			UIButtons.ChangePosition( wrecked_by_id, x, y - 10, z )
			UIButtons.ChangeText( wrecked_by_id, "MPL_WRECKED_BY" .. i - 1 )
			
			UIButtons.SetActive( UIButtons.FindChildByName( ss_wrecked_rb, "Loadout" ), false )	-- no mods in splitscreen
			
			UIButtons.SetActive( ss_wrecked_rb, false )
			
			GUI.mp_ss_wrecked_ids[ #GUI.mp_ss_wrecked_ids + 1 ] = ss_wrecked_rb
		else
			-- Add Mirror
			local mirror = UIButtons.CloneXtGadgetByName( "hud_objects", "Mirror_Root" )
			UIButtons.SetParent( mirror, inter_2d, UIEnums.Justify.TopCentre )
			
			if IsWideScreen() == false then	-- [Blur2PC] retail: IsWideScreen
				local sx, sy = UIButtons.GetScale( mirror )
				local scale = 0.75
				UIButtons.ChangeScale( mirror, sx * scale, sy * scale )
				local px, py = UIButtons.GetPosition( mirror )
				UIButtons.ChangePosition( mirror, px, py + (10 * HUD.GetScreenScale()) )
			end

			-- if in MP add last gasp 
			if Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then
				local LastGaspBranch = UIButtons.CloneXtGadgetByName( "hud_objects", "LastGaspRenderBranch" )
				GUI.LastGaspPowerupSlotID = UIButtons.CloneXtGadgetByName( "hud_objects", "LastGaspCountdownNode1" )
				UIButtons.SetParent( LastGaspBranch, inter_3d, UIEnums.Justify.TopLeft )
				SyncLastGaspHudToVars()
			end
		end
		
		if GUI.is_network_race == true or IsSplitScreen() == true then
			local timer_root_id = HUD_SpCreateTimer( inter_2d, "HUD_0_MP_TIMEOUT", i )	-- timeout is the same for all players
			
			-- shift and scale in splitscreen
			if IsSplitScreen() == true then
				UIButtons.ChangePosition( timer_root_id, nil, Select( ( GUI.num_players == 2 ), 29, 11 ), nil, true )
				UIButtons.ChangeScale( timer_root_id, 0.625, 0.625, 1 )
			end
		end
		
		if GUI.is_network_race == true and IsSplitScreen() == false then
			UIButtons.CloneXtGadgetByName( "hud_objects", "Mp_DataFullScreen" )
			
			if Amax.IsRaceMpDestruction() == false then
				local mini_data = UIButtons.CloneXtGadgetByName( "hud_objects", "Mp_DataMini" )
				UIButtons.SetParent( mini_data, inter_3d, UIEnums.Justify.TopLeft )
				UIButtons.ChangePosition( mini_data, nil, mini_data_offset, nil, true )
				
				local num_vehicles = Amax.GetNumVehicles()
				
				-- resize backing depending on number of players
				if num_vehicles < 5 then
					local diff = ( 5 - num_vehicles ) * 1.4
					
					local backing_left = UIButtons.FindChildByName( mini_data, "MiniData_BackingMiddle" )
					local backing_right = UIButtons.FindChildByName( mini_data, "MiniData_BackingMiddleR" )
					
					local sx, sy, sz = UIButtons.GetSize( backing_left )
					UIButtons.ChangeSize( backing_left, sx, sy - diff, sz )
					
					local sx, sy, sz = UIButtons.GetSize( backing_right )
					UIButtons.ChangeSize( backing_right, sx, sy - diff, sz )
				end
			end
			
			-- speaking players
			local speaking = UIButtons.CloneXtGadgetByName( "hud_objects", "Mp_Speaking" )
			UIButtons.SetParent( speaking, inter_3d, UIEnums.Justify.BottomLeft )
							
			-- rank up
			GUI.mp_rank_up_ids.root = UIButtons.CloneXtGadgetByName( "hud_objects", "mp_rank_up_dummy" )
			GUI.mp_rank_up_ids.rank = UIButtons.FindChildByName( GUI.mp_rank_up_ids.root, "mp_rank_up_rank" )
			GUI.mp_rank_up_ids.rank_num = UIButtons.FindChildByName( GUI.mp_rank_up_ids.root, "mp_rank_up_rank_num" )
			GUI.mp_rank_up_ids.rank_info = UIButtons.FindChildByName( GUI.mp_rank_up_ids.root, "mp_rank_up_rank_info" )
			UIButtons.SetParent( GUI.mp_rank_up_ids.root, inter_2d, UIEnums.Justify.TopCentre )
			
			-- challenge complete
			GUI.mp_challenge_complete_ids.root = UIButtons.CloneXtGadgetByName( "hud_objects", "mp_challenge_complete_dummy" )
			GUI.mp_challenge_complete_ids.title = UIButtons.FindChildByName( GUI.mp_challenge_complete_ids.root, "mp_challenge_complete_title" )
			GUI.mp_challenge_complete_ids.name = UIButtons.FindChildByName( GUI.mp_challenge_complete_ids.root, "mp_challenge_complete_name" )
			GUI.mp_challenge_complete_ids.desc = UIButtons.FindChildByName( GUI.mp_challenge_complete_ids.root, "mp_challenge_complete_desc" )
			GUI.mp_challenge_complete_ids.fans = UIButtons.FindChildByName( GUI.mp_challenge_complete_ids.root, "mp_challenge_complete_fans" )
			GUI.mp_challenge_complete_ids.blob = UIButtons.FindChildByName( GUI.mp_challenge_complete_ids.root, "mp_challenge_complete_fans" )
			GUI.mp_challenge_complete_ids.blob_title = UIButtons.FindChildByName( GUI.mp_challenge_complete_ids.root, "mp_challenge_complete_blob_title_c" )
			GUI.mp_challenge_complete_ids.blob_info = UIButtons.FindChildByName( GUI.mp_challenge_complete_ids.root, "mp_challenge_complete_blob_info_c" )
			UIButtons.SetParent( GUI.mp_challenge_complete_ids.root, inter_2d, UIEnums.Justify.TopCentre )
		end
		
		if GUI.is_network_race == true or IsSplitScreen() == true then
			-- special case icon for when have both JTG and last gasp (no wrecked sequence)
			if UIGlobals.Ingame.has_last_gasp and UIGlobals.Ingame.has_jump_the_gun then
				GUI.jump_the_gun_standalone_ID = UIButtons.CloneXtGadgetByName( "hud_objects", "jump_the_gun_icon_standalone" )
				UIButtons.SetParent( GUI.jump_the_gun_standalone_ID, inter_2d, UIEnums.Justify.MiddleCentre )
			end
		end
		
		-- team colours
		if NetRace.IsTeamGame() == true then
			local team = Multiplayer.GetPlayerTeam()
			UIButtons.SwapColour( inter_2d, "Main_1", UIGlobals.TeamColours[ team ] )
			UIButtons.SwapColour( inter_3d, "Main_1", UIGlobals.TeamColours[ team ] )
		end

		-- Pop clippers
		if GUI.num_players > 1 then
			UIButtons.CloneXtGadgetByName( "hud_objects", "Clip_Pop_41" )
			UIButtons.CloneXtGadgetByName( "hud_objects", "Clip_Pop_14" )
			UIButtons.CloneXtGadgetByName( "hud_objects", "Clip_Pop_31" )
		end

		GUI.ai_hud_ids[ i ] = {  }
		GUI.ai_hud_ids[ i ].id = UIButtons.FindChildByName( inter_2d, "AIHud_Main" )

		-- AI Hud item
		for j = 1, Amax.GetNumVehicles(), 1 do
			local clone_id = UIButtons.CloneXtGadgetByName( "hud_objects", "AIHud_Parent" )
		
			local ai_postfix = "_" .. player_index .. "_" .. ( j - 1 )
			
			UIButtons.ReplaceTimeLineLabel( UIButtons.FindChildByName( clone_id, "ai_targeting_left" ),		"targeted",		"targeted" .. ai_postfix )
			UIButtons.ReplaceTimeLineLabel( UIButtons.FindChildByName( clone_id, "ai_targeting_right" ),	"targeted",		"targeted" .. ai_postfix )

			GUI.ai_hud_ids[ i ][ #GUI.ai_hud_ids[ i ] + 1 ] = clone_id
			
			if IsSplitScreen() == true then
				if j <= #UIGlobals.Splitscreen.players then
					local name_id = UIButtons.FindChildByName( clone_id, "ai_name" )
					UIButtons.ChangeColour( name_id, UIGlobals.Splitscreen.colours[ UIGlobals.Splitscreen.players[ j ].pad ] )
				end
			end
		end
		
		-- GO
		if UIGlobals.Ingame.IntroHUD == true then
			local go_id = UIButtons.CloneXtGadgetByName( "hud_objects", "GoNode" )
			UIButtons.ChangePanel( UIButtons.FindChildByName( go_id, "GoText" ), UIEnums.Panel._VP0 + player_index )
		end

		-- Assign players
		UIScreen.AddMessageNow( UIEnums.GameMessage.SetLocalPlayerIndex, player_index )

		-- Debug : Turne off wobbly HUD
		if GUI.show_camera_helpers == true then
			UIButtons.SetActive( clone_2d, false )
			UIButtons.SetActive( clone_3d, false )
		end
		
		-- scan the render branches to make sure they've got all their children now
		UIButtons.RenderBranchScan( inter_2d )
		UIButtons.RenderBranchScan( inter_3d )
		
		if IsSplitScreen() == true then
			GUI.mp_ss_messages[ i ] = { finished = false, wrecked = false, respawn_timer = 0, }
		end
	end
	
	if IsSplitScreen() == false and Amax.IsRaceMpDestruction() == true then
		-- renderstates to stop the rival arrow showing in the mirror
		local state_on = UIButtons.CloneXtGadgetByName( "hud_objects", "Mirror_Copy_State_On" )
		local copy_l = UIButtons.CloneXtGadgetByName( "hud_objects", "Mirror_ImageL_Copy" )
		local copy_r = UIButtons.CloneXtGadgetByName( "hud_objects", "Mirror_ImageR_Copy" )
		local state_off = UIButtons.CloneXtGadgetByName( "hud_objects", "Mirror_Copy_State_Off" )
		
		UIButtons.SetParent( state_on, mirror, UIEnums.Justify.None )
		UIButtons.SetParent( copy_l, mirror, UIEnums.Justify.TopCentre )
		UIButtons.SetParent( copy_r, mirror, UIEnums.Justify.TopCentre )
		UIButtons.SetParent( state_off, mirror, UIEnums.Justify.None )
	end
	
	-- Scaling in animation
	if IsSplitScreen() == true then
		UIButtons.TimeLineActive( "start", true, 0.5, true )
	end
	
	-- Debug : Show camera helpers
	if GUI.show_camera_helpers == true then
		UIButtons.CloneXtGadgetByName( "hud_objects", "Debug_CameraHelpers" )
	end
	
	-- Add intro 
	if UIGlobals.Ingame.IntroHUD == true then
		Amax.SendMessage( UIEnums.GameFlowMessage.RaceStarted )
		--UISystem.PlaySound( UIEnums.SoundEffect.RaceStart )
		UIGlobals.Ingame.IntroHUD = nil
	end
	
	-- resolve ai huds
	for i, v in ipairs( GUI.ai_hud_ids ) do
		for j, w in ipairs( v ) do
			Amax.ResolveAiHud( v.id, w )
		end
	end
	
	-- set initial state
	HUD_UpdateCombo()
	
	if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
		HUD_SpUpdate( 0 )
	end
	
	-- set initial ss state
	if IsSplitScreen() == true then
		HUD_SsCheckStates( 0 )
	end
	
	-- initial state
	if ( GUI.is_network_race == true or IsSplitScreen() == true ) and Amax.IsRaceMpDestruction() == true then
		HUD_MpDestructionPositionUpdate()
		HUD_MpDestructionRivalUpdate()
	end
end

function StartLoop( time )
	if IsSplitScreen() == true then
		HUD_SsUpdateTimers( time )
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4, arg5 )
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.ControllerDisconnected then
		if UIEnums.CurrentPlatform ~= UIEnums.Platform.PS3 then
			if IsSplitScreen() == true then
				-- try and find me
				for i, v in ipairs( UIGlobals.Splitscreen.players ) do
					if v.pad == arg1 then
						PauseGame( arg1 )
						break
					end
				end
			else
				if arg1 == Profile.GetPrimaryPad() then
					PauseGame( arg1 )
				end
			end
		end
		
	elseif (message == UIEnums.GameMessage.PrimeLastGasp or message == UIEnums.GameMessage.DisarmLastGasp) and Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then 
		 RefreshLastGaspVars()
		 SyncLastGaspHudToVars()
		 
		 -- switch on JTG icon if it is time...
		 -- disarm only gets sent when the LG has been fired...
		 if message == UIEnums.GameMessage.DisarmLastGasp and UIGlobals.Ingame.has_jump_the_gun == true and UIGlobals.Ingame.has_last_gasp == true then
			-- Show the standalone icon for the duration
			UIButtons.SetActive( GUI.jump_the_gun_standalone_ID, true )
		end
	
	elseif message == UIEnums.GameMessage.DisplayECMWarning and Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then	
		HUD_ActivateECMWarning( arg1 )
	
	elseif message == UIEnums.GameMessage.RemoveECMWarning and Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then	
		HUD_DeactivateECMWarning( arg1 )
		
	elseif message == UIEnums.GameMessage.NewRivalSelected then
		HUD_MpDestructionRivalChange( arg1 + 1 )
		
	elseif message == UIEnums.GameMessage.UsePickup and Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then	
		if UIGlobals.Ingame.has_last_gasp == true and UIGlobals.Ingame.last_gasp_mode_active == true and UIGlobals.Ingame.can_fire_last_gasp == true then
			Amax.FireLastGasp( 0 )
			RefreshLastGaspVars()
			SyncLastGaspHudToVars()
		end
		
	elseif message == UIEnums.Message.ButtonStart and arg2 == true and Amax.IsGamePaused() == false then
		PlaySfxNext()
		PauseGame( arg1 )

	elseif message == UIEnums.GameFlowMessage.RaceFinished or message == UIEnums.GameFlowMessage.RaceFailed then
		UIGlobals.Ingame.RaceFinishedMsg = message
		
		if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
			Amax.SetupResults()
			
			UIScreen.SetScreenTimers( 0, 0 )
			GoScreen( "SinglePlayer\\Ingame\\SpFinished.lua" )
			
		elseif IsSplitScreen() == true then
			UIScreen.SetScreenTimers( 0, 1.5 ) -- PR - TODO - re-jig this
			GoScreen( "Multiplayer\\Ingame\\MpWinner.lua" )
			
		else
			if Multiplayer.IsEliminated() then
				NetRace.ChangeLocalPlayerState( Profile.GetPrimaryPad(), UIEnums.Network.PlayerStates.eNetPlayerStateFinished )
				if Multiplayer.RaceFinished() == true and NetRace.CanShowWinner() == true then 
					GoScreen( "Multiplayer\\Ingame\\MpWinner.lua" )
				else
					Mp_ResetSpectateVars()
					GoScreen( "Multiplayer\\Ingame\\MpSpectate.lua" )
				end
			else
				GoScreen( "Multiplayer\\Ingame\\MpFinished.lua" )
			end
		end
		
	elseif message == UIEnums.GameFlowMessage.Wrecked then
		if IsSplitScreen() == true then
			local player_index = arg1 + 1
			if player_index > 0 and player_index <= #UIGlobals.Splitscreen.players then
				UIGlobals.SplitscreenMessages[ player_index ].wrecked = true
			end
		else
			if arg1 == 0 then
				-- nuke any current challenges
				if GUI.is_network_race == true and UIButtons.IsActive( GUI.mp_challenge_complete_ids.root ) == true then
					UIButtons.TimeLineActive( "mp_challenge_complete", false )
					UIButtons.SetActive( GUI.mp_challenge_complete_ids.root, false )
					
					-- clear the stack
					GUI.mp_rank_or_challenge_stack = {  }
				end
				
				GoScreen( "Ingame\\SpWrecked.lua", UIEnums.Context.SpWrecked )
				GUI.allow_pause = false
				UIButtons.SetActive( GUI.mp_fan_msg_root[ 1 ], false )
			end
		end
		
	elseif message == UIEnums.GameFlowMessage.Respawn then
		if IsSplitScreen() == true then
			local player_index = arg1 + 1
			if player_index > 0 and player_index <= #UIGlobals.Splitscreen.players then
				UIGlobals.SplitscreenMessages[ player_index ].wrecked = false
			end
		else
			if arg1 == 0 then
				GUI.allow_pause = true
				GUI.mp_fan_msg_activate_in[ arg1 + 1 ] = 0.5
				if UIGlobals.Ingame.has_jump_the_gun == true and UIGlobals.Ingame.has_last_gasp == true then
					-- Hide the standalone icon for the duration
					UIButtons.SetActive( GUI.jump_the_gun_standalone_ID, false )
				end
			end
		end
		
	elseif message == UIEnums.GameMessage.MpRankedUp then
		HUD_MpRankUp( arg1, arg2, arg3, arg4 )
	
	elseif message == UIEnums.GameMessage.MpChallengeComplete and UIScreen.IsContextActive( UIEnums.Context.SpWrecked ) == false then
		HUD_MpChallengeComplete( arg1, arg2 )
	
	elseif message == UIEnums.GameFlowMessage.VehicleFinished and IsSplitScreen() == true then
		local player_index = arg1 + 1
		if player_index > 0 and player_index <= #UIGlobals.Splitscreen.players then
			UIGlobals.SplitscreenMessages[ player_index ].finished = true
			UIGlobals.SplitscreenMessages[ player_index ].finished_position = Multiplayer.GetSplitscreenPosition( player_index )
		end
	end
	
	-- single player
	if message == UIEnums.GameMessage.SpCheckpointPassed then
		HUD_SpCheckpointPassed()
	elseif message == UIEnums.GameMessage.SpCollectStopwatch then
		HUD_SpCheckpointCollectStopwatch()
		
	elseif message == UIEnums.GameMessage.SpDestructionPointsAdded then
		HUD_SpDestructionPointsAdded( arg1 )
	elseif message == UIEnums.GameMessage.SpDestructionBonusHitStart then
		HUD_SpDestructionBonusHitStart()
	elseif message == UIEnums.GameMessage.SpDestructionBonusHitAdded then
		HUD_SpDestructionBonusHitAdded()
	elseif message == UIEnums.GameMessage.SpDestructionBonusHitEnd then
		HUD_SpDestructionBonusHitEnd()
		
	elseif message == UIEnums.GameMessage.SpFanDemandFansAdded then
		HUD_SpFanDemandFansAdded()	
	end
end

function FrameUpdate( time )
	if GUI.finished == true or net_Disconnecting() == true then
		return
	end

	-- was the player kicked for being idle?
	if NetRace.IsPlayerInactive() == true and UIGlobals.IdlePlayerKicked == false and UIGlobals.Multiplayer.LobbyConnectionLost == false then
		print( "Idle Player Kicked" )	
		UIScreen.SetScreenTimers(0,0)
		UIGlobals.IdlePlayerKicked = true			  -- set flag so correct popup can be displayed
		
		net_DisconnectFromRace()
	end
	
	if UIGlobals.LaunchMode == UIEnums.LaunchMode.Automation then
		if GUI.automation_start == true then
			Amax.EnableAIControl( true )
			GUI.automation_start = false
			UIGlobals.automation_timer = UIGlobals.automation_length
		end
	end

	-- Catch the race finished flag and store it
	if IsSplitScreen() == false and UIGlobals.Multiplayer.RaceFinished == false and Multiplayer.RaceFinished() == true then
	
		UIScreen.SetScreenTimers(0,0) -- Delay next screen coming on for a short time
		GoScreen("Multiplayer\\Ingame\\MpFinished.lua")

		print( "RACE FINISHED" )
	end

	-- Catch the vehicle finished flag and store it
	if IsSplitScreen() == false and UIGlobals.Multiplayer.PrimaryVehicleFinished == false and Multiplayer.PrimaryVehicleFinished() == true then
		UIGlobals.Multiplayer.PrimaryVehicleFinished = true
		
		if Multiplayer.IsEliminated() then
			NetRace.ChangeLocalPlayerState( Profile.GetPrimaryPad(), UIEnums.Network.PlayerStates.eNetPlayerStateFinished )
			if Multiplayer.RaceFinished() == true and NetRace.CanShowWinner() == true then 
				GoScreen( "Multiplayer\\Ingame\\MpWinner.lua" )
			else
				Mp_ResetSpectateVars()
				GoScreen( "Multiplayer\\Ingame\\MpSpectate.lua" )
			end
		else
			GoScreen( "Multiplayer\\Ingame\\MpFinished.lua" )
		end

	 	GoScreen( "Multiplayer\\Ingame\\MpFinished.lua" )
		print( "VEHICLE FINISHED" )
	end

	if Profile.GuideActive() == true then
		PauseGame( Profile.GetPrimaryPad() )
		return
	end
	
	-- update viewports
	if GUI.show_revs == true then
		for i = 1, GUI.num_players, 1 do
			-- update revs
			local revs01 = Amax.GetRevs01( i )
			
			UIShape.SetVertexParams( GUI.revs_partial_ids[ i ], 0, revs01 )
		end
	end
	
	for i = 1, GUI.num_players, 1 do
		if GUI.mp_fan_msg_activate_in[ i ] > 0 then
			GUI.mp_fan_msg_activate_in[ i ] = GUI.mp_fan_msg_activate_in[ i ] - time

			if GUI.mp_fan_msg_activate_in[ i ] <= 0 then
				UIButtons.SetActive( GUI.mp_fan_msg_root[ i ], true )
			end
		end
	end
	
	if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
		HUD_SpUpdate( time )
	end
	
	if IsSplitScreen() == true then
		HUD_SsCheckStates( time )
		HUD_SsUpdateTimers( time )
	end
	
	HUD_UpdateCombo()
	
	-- Last Gasp update ( do we need this now.. ? )
	if Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then
		if UIGlobals.Ingame.has_last_gasp == true and UIGlobals.Ingame.last_gasp_mode_active == true then
			
			UIGlobals.Ingame.last_gasp_timer = Amax.GetLastGaspTime( 0 ) 
				
			if UIGlobals.Ingame.last_gasp_timer > 0.0 then
				UIButtons.ChangeText(  UIButtons.FindChildByName( GUI.LastGaspPowerupSlotID, "time_LastGasp_OnCar"), "MPL_TIMER_GASP_COUNTDOWN" .. UIGlobals.Ingame.last_gasp_timer )
			end
		end
	end
	
	-- mp update
	if GUI.is_network_race == true or IsSplitScreen() == true then
		local mp_timeout = Amax.MP_GetTimeout()
		
		if Amax.IsRaceMpDestruction() == true then
			if mp_timeout <= 30 then
				HUD_SpTimerUpdate( mp_timeout )
			end
			
			HUD_MpDestructionPositionUpdate()
			HUD_MpDestructionRivalUpdate()
		else
			HUD_SpTimerUpdate( mp_timeout )
		end
	end
end

function Render()
	--
end

function EnterEnd( )
	-- Fold back in
	UIButtons.TimeLineActive( "Show_Pickup", false )
end

function EndLoop( time )
	if IsSplitScreen() == true then
		HUD_SsUpdateTimers( time )
	end
end

function End()

	Camera_UseFrontend()
	
	-- cleanup
	SetNil( HUD )
	SetNil( HUD_Panels )
	SetNil( HUD_Items )
	
	-- PR - sort out splitscreen pad control after a race
	if IsSplitScreen() == true and GUI.going_to_pause_pad == -1 then
		Profile.SetPrimaryPad( UIGlobals.Splitscreen.primary_pad )
		Profile.LockToPad( UIGlobals.Splitscreen.primary_pad )
	end
end

function HUD_UpdateCombo()
	for i, v in ipairs( GUI.fan_combo_ids ) do
		local combo_timer = Amax.GetComboTimer( i - 1 )
		local combo_timer01 = combo_timer / 4	-- hardcoded from Kerbz
		
		if combo_timer ~= GUI.fan_combo_timer[ i ] then
			local new_tex = 
			{
				filename = GUI.fan_combo_tex.filename,
				pos = { u = GUI.fan_combo_tex.pos.u, v = GUI.fan_combo_tex.pos.v + ( 1 - combo_timer01 ) * GUI.fan_combo_tex.size.v, },
				size = { u = GUI.fan_combo_tex.size.u, v = GUI.fan_combo_tex.size.v * combo_timer01, },
			}
			
			UIButtons.ChangeTexture( new_tex, 0, v )
			
			local sy = GUI.fan_combo_sy * combo_timer01
			UIButtons.ChangeSize( v, GUI.fan_combo_sy, sy, 0 )
			
			local py = GUI.fan_combo_py - GUI.fan_combo_py * ( 1 - combo_timer01 ) * 0.5
			local px, _, pz = UIButtons.GetPosition( v )
			UIButtons.ChangePosition( v, px, py, pz )
			
			if GUI.fan_combo_timer[ i ] == 0 then	-- started
				UIButtons.PrivateTimeLineActive( v, "combo_pulse", true )
			end
			
			GUI.fan_combo_timer[ i ] = combo_timer
			
			if GUI.fan_combo_timer[ i ] == 0 then	-- finished
				UIButtons.PrivateTimeLineActive( v, "combo_pulse", false )
			end
		end
	end
end

function PauseGame( arg1 )	
	if GUI.allow_pause == false then
		return
	end

	GUI.going_to_pause_pad = arg1
	
	-- PR - lock pause menu to player that pressed it
	if IsSplitScreen() == true then
		Profile.SetPrimaryPad( GUI.going_to_pause_pad )
		Profile.LockToPad( GUI.going_to_pause_pad )
	end
	
	GUI.going_to_pause = true
	Amax.SendMessage(UIEnums.GameFlowMessage.Pause) 
	
	if GUI.num_players == 1 then
		UIButtons.TimeLineActive( "start", false )
	end
	
	GoScreen( "Ingame\\Paused.lua" )
end

function HUD_MpRankUp( rank, legend, fans, fans_next )
	GUI.mp_rank_or_challenge_stack[ #GUI.mp_rank_or_challenge_stack + 1 ] = { tag = "R", rank = rank, legend = legend, fans = fans, fans_next = fans_next, }
	
	if #GUI.mp_rank_or_challenge_stack == 1 then
		HUD_MpNextRankOrChallenge()
	end
end

-- called externally from a timeline
function HUD_MpRankUpFinished()
	UIButtons.SetActive( GUI.mp_rank_up_ids.root, false )
	UIButtons.SetActive( GUI.mp_fan_msg_root[ 1 ], true )
	
	table.remove( GUI.mp_rank_or_challenge_stack, 1 )
	HUD_MpNextRankOrChallenge()
end

function HUD_MpChallengeComplete( type, tier )
	GUI.mp_rank_or_challenge_stack[ #GUI.mp_rank_or_challenge_stack + 1 ] = { tag = "C", type = type, tier = tier, }
	
	if #GUI.mp_rank_or_challenge_stack == 1 then
		HUD_MpNextRankOrChallenge()
	end
end

-- called externally from a timeline
function HUD_MpChallengeCompleteFinished()
	UIButtons.SetActive( GUI.mp_challenge_complete_ids.root, false )
	UIButtons.SetActive( GUI.mp_fan_msg_root[ 1 ], true )
	
	table.remove( GUI.mp_rank_or_challenge_stack, 1 )
	HUD_MpNextRankOrChallenge()
end

-- set off the next rank or challenge message
function HUD_MpNextRankOrChallenge()
	if #GUI.mp_rank_or_challenge_stack == 0 then
		return
	end
	
	if GUI.mp_rank_or_challenge_stack[ 1 ].tag == "R" then
		-- rank up event
		local rank = GUI.mp_rank_or_challenge_stack[ 1 ].rank
		local legend = GUI.mp_rank_or_challenge_stack[ 1 ].legend
		local fans = GUI.mp_rank_or_challenge_stack[ 1 ].fans
		local fans_next = GUI.mp_rank_or_challenge_stack[ 1 ].fans_next
		
		UIButtons.SetActive( GUI.mp_rank_up_ids.root, true )
		UIButtons.SetActive( GUI.mp_fan_msg_root[ 1 ], false )
		
		Mp_RankIcon( GUI.mp_rank_up_ids.rank, rank - 1, legend )
		UIButtons.ChangeText( GUI.mp_rank_up_ids.rank_num , "HUD_0_MP_RANK_" .. rank )
		
		if fans_next == -1 then
			UIButtons.ChangeText( GUI.mp_rank_up_ids.rank_info, UIText.CMN_NOWT )
		else
			UIButtons.ChangeText( GUI.mp_rank_up_ids.rank_info, "HUD_0_MP_NEXT_RANK_IN_" .. ( fans_next - fans ) .. "_" .. ( rank + 1 ) )
		end
		
		UIButtons.TimeLineActive( "mp_rank_up", true, 0 )
		UISystem.PlaySound( UIEnums.SoundEffect.RankUp )

	elseif GUI.mp_rank_or_challenge_stack[ 1 ].tag == "C" then
		-- challenge complete event
		local type = GUI.mp_rank_or_challenge_stack[ 1 ].type
		local tier = GUI.mp_rank_or_challenge_stack[ 1 ].tier
		
		UIButtons.SetActive( GUI.mp_challenge_complete_ids.root, true )
		UIButtons.SetActive( GUI.mp_fan_msg_root[ 1 ], false )
		
		UIButtons.ChangeText( GUI.mp_challenge_complete_ids.name, "MPL_CHALLENGE" .. type .. "_" .. tier .. "_NAME" )
		UIButtons.ChangeText( GUI.mp_challenge_complete_ids.desc, "MPL_CHALLENGE" .. type .. "_" .. tier .. "_DESC" )
		UIButtons.ChangeText( GUI.mp_challenge_complete_ids.fans, "MPL_CHALLENGE" .. type .. "_" .. tier .. "_FANS" )
		
		local gap_x = 20
		
		local length_title = UIButtons.GetStaticTextLength( GUI.mp_challenge_complete_ids.title )
		local _, sy, sz = UIButtons.GetSize( GUI.mp_challenge_complete_ids.blob_title )
		UIButtons.ChangeSize( GUI.mp_challenge_complete_ids.blob_title, length_title + gap_x, sy, sz )
		
		local length_name = UIButtons.GetStaticTextLength( GUI.mp_challenge_complete_ids.name )
		local length_desc = UIButtons.GetStaticTextLength( GUI.mp_challenge_complete_ids.desc )
		local length_max = math.max( length_name, length_desc )
		local _, sy, sz = UIButtons.GetSize( GUI.mp_challenge_complete_ids.blob_info )
		UIButtons.ChangeSize( GUI.mp_challenge_complete_ids.blob_info, length_max + gap_x, sy, sz )
		
		UIButtons.TimeLineActive( "mp_challenge_complete", true, 0 )
		UISystem.PlaySound( UIEnums.SoundEffect.ChallengeComplete )
	end
end

-- splitscreen
function HUD_SsCheckStates( time )
	for i = 1, #UIGlobals.Splitscreen.players, 1 do
		-- finished
		if GUI.mp_ss_messages[ i ].finished == false and UIGlobals.SplitscreenMessages[ i ].finished == true then
			GUI.mp_ss_messages[ i ].finished = true
			GUI.mp_ss_messages[ i ].finished_position = UIGlobals.SplitscreenMessages[ i ].finished_position
			
			local finished_position = GUI.mp_ss_messages[ i ].finished_position
			
			UIButtons.SetActive( GUI.inter_2d_ids[ i ], false )
			UIButtons.SetActive( GUI.inter_3d_ids[ i ], false )
			
			local finished_id = GUI.mp_ss_finished_ids[ i ]
			UIButtons.SetActive( finished_id, true )
			
			local text_id = UIButtons.FindChildByName( finished_id, "Pos_Place" )
			UIButtons.ChangeText( text_id, "MPL_SPLITSCREEN_FINISHED_POSITION_" .. finished_position )
			
			local show_finished_ids =
			{
				UIButtons.FindChildByName( finished_id, "CSB_T_SS" ),
				UIButtons.FindChildByName( finished_id, "CSB_B_SS" ),
				UIButtons.FindChildByName( finished_id, "Finished" ),
			}
			
			local show_result_ids = 
			{
				UIButtons.FindChildByName( finished_id, "PosDummy" ),
			}
			
			local colour = GetResultColour( finished_position )
			
			if finished_position <= 3 then
				local dummy_id = UIButtons.FindChildByName( finished_id, "Dummy_" .. finished_position )
				UIButtons.SetActive( dummy_id, true )
				
				if finished_position == 3 then
					UIButtons.ChangeColour( text_id, colour )
					show_result_ids[ #show_result_ids + 1 ] = UIButtons.FindChildByName( dummy_id, "3_a" )
					show_result_ids[ #show_result_ids + 1 ] = UIButtons.FindChildByName( dummy_id, "3_b" )
					
				elseif finished_position == 2 then
					UIButtons.ChangeColour( text_id, colour )
					show_result_ids[ #show_result_ids + 1 ] = UIButtons.FindChildByName( dummy_id, "2_a" )
					show_result_ids[ #show_result_ids + 1 ] = UIButtons.FindChildByName( dummy_id, "2_b" )
					
				else
					UIButtons.ChangeColour( text_id, colour )
					show_result_ids[ #show_result_ids + 1 ] = UIButtons.FindChildByName( dummy_id, "1_a" )
					show_result_ids[ #show_result_ids + 1 ] = UIButtons.FindChildByName( dummy_id, "1_b" )
					show_result_ids[ #show_result_ids + 1 ] = UIButtons.FindChildByName( dummy_id, "1_c" )
				end
			else
				UIButtons.ChangeColour( text_id, colour )
			end
			
			local finished_timer = UIGlobals.SplitscreenMessages[ i ].finished_timer
			
			for i, v in ipairs( show_finished_ids ) do
				UIButtons.PrivateTimeLineActive( v, "show_finished", true, finished_timer, true )
			end
			
			for i, v in ipairs( show_result_ids ) do
				UIButtons.PrivateTimeLineActive( v, "show_result", true, finished_timer, true )
			end
		end
		
		-- wrecked
		if GUI.mp_ss_messages[ i ].wrecked == false and UIGlobals.SplitscreenMessages[ i ].wrecked == true then
			GUI.mp_ss_messages[ i ].wrecked = true
			
			UIButtons.SetActive( GUI.inter_2d_ids[ i ], false )
			UIButtons.SetActive( GUI.inter_3d_ids[ i ], false )
			
			local wrecked_id = GUI.mp_ss_wrecked_ids[ i ]
			UIButtons.SetActive( wrecked_id, true )
			
			local show_wrecked_ids =
			{
				UIButtons.FindChildByName( wrecked_id, "CSB_Top" ),
				UIButtons.FindChildByName( wrecked_id, "CSB_Bottom" ),
				UIButtons.FindChildByName( wrecked_id, "Wrecked" ),
				UIButtons.FindChildByName( wrecked_id, "Wrecked_By" ),
			}
			
			local wrecked_timer = UIGlobals.SplitscreenMessages[ i ].wrecked_timer
			
			for i, v in ipairs( show_wrecked_ids ) do
				UIButtons.PrivateTimeLineActive( v, "show_wrecked", true, wrecked_timer, true )
			end
		end
		
		-- respawn
		if GUI.mp_ss_messages[ i ].wrecked == true and UIGlobals.SplitscreenMessages[ i ].wrecked == false then
			GUI.mp_ss_messages[ i ].wrecked = false
			GUI.mp_ss_messages[ i ].respawn_timer = 0.5
		end
		
		if GUI.mp_ss_messages[ i ].respawn_timer > 0 then
			GUI.mp_ss_messages[ i ].respawn_timer = GUI.mp_ss_messages[ i ].respawn_timer - time
			
			if GUI.mp_ss_messages[ i ].respawn_timer < 0 then
				GUI.mp_ss_messages[ i ].respawn_timer = 0
				
				UIButtons.SetActive( GUI.mp_ss_wrecked_ids[ i ], false )
			
				UIButtons.SetActive( GUI.inter_2d_ids[ i ], true )
				UIButtons.SetActive( GUI.inter_3d_ids[ i ], true )
				
				UIGlobals.SplitscreenMessages[ i ].wrecked_timer = 0
			end
		end
	end
end

function HUD_SsUpdateTimers( time )
	for i = 1, #UIGlobals.Splitscreen.players, 1 do
		if GUI.mp_ss_messages[ i ].finished == true then
			UIGlobals.SplitscreenMessages[ i ].finished_timer = UIGlobals.SplitscreenMessages[ i ].finished_timer + time
		end
		if GUI.mp_ss_messages[ i ].wrecked == true then
			UIGlobals.SplitscreenMessages[ i ].wrecked_timer = UIGlobals.SplitscreenMessages[ i ].wrecked_timer + time
		end
	end
end

function HUD_SpCreateLight( parent, offset_mult, do_2d_checkpoint )
	local offset_extra_mult = 1.1
	offset_mult = offset_mult * offset_extra_mult
	
	local star_id, star_id_blob = Sp_CreateStar( false, offset_mult + 1, nil, nil, nil, 0.85, true, true )
	
	for i, v in ipairs( star_id ) do
		UIButtons.ChangePanel( v, UIEnums.Panel._3DAA_0, true )
		UIButtons.SetParent( v, parent, UIEnums.Justify.MiddleCentre )
	end
	
	local ids = {  }
	ids.on = star_id[ 1 ]
	ids.off = star_id[ 2 ]
	ids.blob = star_id_blob
	
	if do_2d_checkpoint == true then
		UIButtons.ChangePanel( ids.on, UIEnums.Panel._2DAA, true )
		UIButtons.ChangePanel( ids.off, UIEnums.Panel._2DAA, true )
		UIButtons.ChangePanel( ids.blob, UIEnums.Panel._2DAA, true )
		local sx, sy, sz = UIButtons.GetSize( ids.on )
		local s = 10
		local sxn = sx * s
		local syn = sy * s
		UIButtons.ChangeSize( ids.on, sxn, syn, sz )
		UIButtons.ChangeSize( ids.off, sxn, syn, sz )
		UIButtons.ChangeSize( ids.blob, sxn, syn, sz )
		
		UIButtons.ChangeJustification( ids.on, UIEnums.Justify.BottomLeft )
		
		local x, y, z = UIButtons.GetPosition( ids.on )
		UIButtons.ChangePosition( ids.on, x, y + syn, z )
	end
	
	return ids
end

function HUD_SpCreateLightPanel( inter_3d )
	local backing_lights = UIButtons.CloneXtGadgetByName( "hud_objects", "SpLightsBackingTop" )
	UIButtons.SetParent( backing_lights, inter_3d, UIEnums.Justify.TopLeft )
	
	GUI.sp.light = {  }
	local lights_dummy = UIButtons.FindChildByName( backing_lights, "SpLightsDummy" )
	
	for i = 1, 5, 1 do
		GUI.sp.light[ i ] = HUD_SpCreateLight( lights_dummy, i - 1 )
	end
end

function HUD_SpCreateNextPanel( inter_3d, next_text, best_title_text )
	if Amax.SP_IsStreetRaceFD() == true or Amax.SP_IsDestructionFD() == true or Amax.SP_IsCheckpointFD() == true or Amax.SP_IsFanDemandFD() == true or Amax.SP_IsBossBattleFD() == true then
		return
	end
	
	local backing_next = UIButtons.CloneXtGadgetByName( "hud_objects", "SpNextBackingTop" )
	UIButtons.SetParent( backing_next, inter_3d, UIEnums.Justify.TopLeft )
	
	local next_light_dummy = UIButtons.FindChildByName( backing_next, "SpNextLightDummy" )
	GUI.sp.light_next = HUD_SpCreateLight( next_light_dummy, 0 )
	
	local done_id = UIButtons.FindChildByName( backing_next, "SpNextDone" )
	GUI.sp.light_done = done_id
	
	-- don't need the off light
	UIButtons.SetActive( GUI.sp.light_next.off, false )
	
	local text_id = UIButtons.FindChildByName( backing_next, "SpNextMedalInfo" )
	Amax.ChangeText( text_id, next_text )
	
	GUI.sp.next_title = UIButtons.FindChildByName( backing_next, "SpNextMedalTitle" )
	GUI.sp.next_arrow = UIButtons.FindChildByName( backing_next, "SpNextArrow" )
	
	GUI.sp.best_title_text = UIText.HUD_BEST
	
	if best_title_text ~= nil then
		GUI.sp.best_title_text = best_title_text
	end
end

function HUD_SpUpdate( time )
	if GUI.is_street_race == true then
		HUD_SpStreetRaceUpdate( time )
		
	elseif GUI.is_checkpoint_race == true then
		HUD_SpCheckpointUpdate( time )
		
	elseif GUI.is_destruction_race == true then
		HUD_SpDestructionUpdate( time )
		
	elseif GUI.is_fan_demand_race == true then
		HUD_SpFanDemandUpdate( time )
	end
		
	if GUI.is_race_fd == true then
		HUD_SpUpdateFD( time )
	end
end

-- called externally from a timeline
function HUD_FanScoreOnOff( on )
	if on == true then
		MajorScoreOnOff( on )
		GUI.major_fan_on = true
	else
		GUI.major_fan_on = false
		MajorScoreOnOff()
	end
end

-- called externally from a timeline
function HUD_FanScoreBonusOnOff( on )
	if on == true then
		MajorScoreOnOff( on )
		GUI.major_bonus_on = true
	else
		GUI.major_bonus_on = false
		MajorScoreOnOff()
	end
end

-- called externally from a timeline
function HUD_FanDemandStart()
	-- switch off stuff
	if GUI.is_checkpoint_race == true then
		UIButtons.SetActive( GUI.sp.cp_rb, false )
		
	elseif GUI.is_destruction_race == true then
		UIButtons.SetActive( GUI.sp.destruction_rb, false )
		UIButtons.SetActive( GUI.sp.destruction_bonus_rb, false )
	end
	
	MajorScoreOnOff( true )
	GUI.major_demand_on = true
end

-- called externally from a timeline
function HUD_FanDemandEnd()
	-- switch on stuff
	if GUI.is_checkpoint_race == true then
		UIButtons.SetActive( GUI.sp.cp_rb, true )
		
	elseif GUI.is_destruction_race == true then
		UIButtons.SetActive( GUI.sp.destruction_rb, true )
		UIButtons.SetActive( GUI.sp.destruction_bonus_rb, true )
	end
	
	GUI.major_demand_on = false
	MajorScoreOnOff()
end

function MajorScoreOnOff( on )
	if on == nil then
		on = false
	end
	
	if GUI.major_fan_on ~= true and GUI.major_bonus_on ~= true and GUI.major_demand_on ~= true then
		UIButtons.TimeLineActive( "fade_hud_data", on )
	end
end

function HUD_SpStateUpdate( state, time, beating_best )
	if state ~= GUI.sp.state then
		local num_stars = Sp_EventStateToStars( state )
		local num_stars_old = Sp_EventStateToStars( GUI.sp.state )
		
		if num_stars ~= 0 then
			GUI.sp.state_change_active = true
			GUI.sp.state_change_timer = 0
			GUI.sp.state_change_timer_next = 0
			GUI.sp.state_change_current = 1
			GUI.sp.state_change_target = num_stars
			GUI.sp.state_change_state = state
			GUI.sp.state_change_down = ( num_stars < num_stars_old )
		else
			GUI.sp.state_change_active = false
		end
		
		-- current state
		for i = 1, 5, 1 do
			local active = Select( ( num_stars < num_stars_old ), false, ( i <= num_stars_old ) )
			
			UIButtons.SetActive( GUI.sp.light[ i ].on, active, true )
			UIButtons.SetActive( GUI.sp.light[ i ].off, true ) --not active )
		end
		
		if GUI.is_checkpoint_race == true then
			local cp_light_on = ( state ~= "none" )
			UIButtons.SetActive( GUI.sp.light_timer.on, cp_light_on, true )
			UIButtons.SetActive( GUI.sp.light_timer.off, true ) --not cp_light_on )
			
			if cp_light_on == true then
				SpSetStarTexture( GUI.sp.light_timer.on, true, state )
				UIButtons.ChangeColour( GUI.sp.light_timer.blob, Sp_EventStateToColour( state ) )
				UIButtons.PrivateTimeLineActive( GUI.sp.light_timer.on, "light_on", true, 0 )
				UIButtons.PrivateTimeLineActive( GUI.sp.light_timer.blob, "light_on", true, 0 )
			end
			
			-- set the light behind to the previous state
			SpSetStarTexture( GUI.sp.light_timer.off, true, Sp_EventStateToPrevState( state ) )
		end
		
		-- next state
		if GUI.sp.light_next ~= nil then
			if state == "gold" then
				UIButtons.SetActive( GUI.sp.light_done, true )
				UIButtons.SetActive( GUI.sp.light_next.on, false, true )
				UIButtons.ChangeText( GUI.sp.next_title, GUI.sp.best_title_text )
				UIButtons.SetActive( GUI.sp.next_arrow, true )
			else
				UIButtons.SetActive( GUI.sp.light_done, false )
				UIButtons.SetActive( GUI.sp.light_next.on, true, true )
				UIButtons.ChangeText( GUI.sp.next_title, UIText.HUD_NEXT )
				UIButtons.SetActive( GUI.sp.next_arrow, false )
				
				local new_state = Sp_EventStateToNextState( state )
				SpSetStarTexture( GUI.sp.light_next.on, true, new_state )
				UIButtons.ChangeColour( GUI.sp.light_next.blob, Sp_EventStateToColour( new_state ) )
			end
			
			UIButtons.PrivateTimeLineActive( GUI.sp.light_next.on, "light_on", true, 0 )
			UIButtons.PrivateTimeLineActive( GUI.sp.light_next.blob, "light_on", true, 0 )
		end
		
		GUI.sp.state = state
		
		UIButtons.TimeLineActive( "medal_state_change", true, 0 )
	end
	
	-- change state effect
	if GUI.sp.state_change_active == true then
		if GUI.sp.state_change_timer >= GUI.sp.state_change_timer_next then
			local count = Select( GUI.sp.state_change_down, GUI.sp.state_change_target, GUI.sp.state_change_current )
			
			if GUI.sp.state_change_down == true then
				UISystem.PlaySound( UIEnums.SoundEffect.MedalDecrease )
			end
			
			for i = GUI.sp.state_change_current, count, 1 do
				UIButtons.SetActive( GUI.sp.light[ i ].on, true, true )
				
				SpSetStarTexture( GUI.sp.light[ i ].on, true, GUI.sp.state_change_state )
				UIButtons.ChangeColour( GUI.sp.light[ i ].blob, Sp_EventStateToColour( GUI.sp.state_change_state ) )
				UIButtons.PrivateTimeLineActive( GUI.sp.light[ i ].on, "light_on", true, 0 )
				UIButtons.PrivateTimeLineActive( GUI.sp.light[ i ].blob, "light_on", true, 0 )
				
				GUI.sp.state_change_timer_next = GUI.sp.state_change_timer_next + 0.125
				GUI.sp.state_change_current = GUI.sp.state_change_current + 1
				
				if GUI.sp.state_change_down == false then
					UISystem.PlaySound( UIEnums.SoundEffect.MedalIncrease )
				end
			end
			
			if GUI.sp.state_change_current > GUI.sp.state_change_target then
				GUI.sp.state_change_active = false
			end
		end
		
		GUI.sp.state_change_timer = GUI.sp.state_change_timer + time
	end
	
	-- up/down arrow
	if GUI.sp.next_arrow ~= nil and state == "gold" and GUI.sp.beating_best ~= beating_best then
		if beating_best == true then
			UIButtons.ChangeColour( GUI.sp.next_arrow, "Support_0" )
			UIButtons.ChangeOrientation( GUI.sp.next_arrow, 0, 0, 0 )
		else
			UIButtons.ChangeColour( GUI.sp.next_arrow, "Support_3" )
			UIButtons.ChangeOrientation( GUI.sp.next_arrow, 0, 0, 180 )
		end
		
		GUI.sp.beating_best = beating_best
	end
end

function HUD_SpCheckpointSetup( inter_3d, inter_2d )
	GUI.is_checkpoint_race = true
	
	local backing = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
	UIButtons.SetParent( backing, inter_3d, UIEnums.Justify.TopLeft )
	
	local title = UIButtons.CloneXtGadgetByName( "hud_objects", "Checkpoint_Title" )
	UIButtons.SetParent( title, inter_3d, UIEnums.Justify.TopLeft )
	
	local checkpoints = UIButtons.CloneXtGadgetByName( "hud_objects", "SpCheckpoints" )
	
	HUD_SpCreateLightPanel( inter_3d )
	HUD_SpCreateNextPanel( inter_3d, "HUD_SP_RACE_CHECKPOINT_TIME_DIFF_TO_MEDAL", UIText.HUD_BEST_TIME )
	
	local timer_root_id = HUD_SpCreateTimer( inter_2d, "HUD_SP_RACE_CHECKPOINT_TIME_LEFT" )
	
	local time_added = UIButtons.CloneXtGadgetByName( "hud_objects", "SpCheckpointTimeAdded" )
	UIButtons.SetParent( time_added, UIButtons.FindChildByName( timer_root_id, "SpTimerScaleDummy" ), UIEnums.Justify.MiddleCentre )
	
	local bonus_rb_id = UIButtons.CloneXtGadgetByName( "hud_objects", "SpCpBonusRB" )
	UIButtons.SetParent( bonus_rb_id, inter_2d, UIEnums.Justify.TopCentre )
	
	GUI.sp.cp_rb = bonus_rb_id
	GUI.sp.cp_stopwatch_rb = UIButtons.FindChildByName( bonus_rb_id, "SpCpBonusStopwatchRB" )
	GUI.sp.cp_checkpoint_rb = UIButtons.FindChildByName( bonus_rb_id, "SpCpBonusCheckpointRB" )
	
	local gap_x = 20
	
	-- stopwatch bonus
	local blob_stopwatch = UIButtons.FindChildByName( GUI.sp.cp_stopwatch_rb, "SpCpBlobStopwatchC" )
	local icon_stopwatch = UIButtons.FindChildByName( GUI.sp.cp_stopwatch_rb, "SpCpStopwatchBonusIcon" )
	local text_stopwatch = UIButtons.FindChildByName( GUI.sp.cp_stopwatch_rb, "SpCpStopwatchBonusTitle" )
	
	local sx, _, _ = UIButtons.GetSize( icon_stopwatch )
	local px, _, _ = UIButtons.GetPosition( text_stopwatch )
	
	local length = 0
	length = length + sx
	length = length + px
	length = length + UIButtons.GetStaticTextLength( text_stopwatch )
	
	local _, sy, sz = UIButtons.GetSize( blob_stopwatch )
	UIButtons.ChangeSize( blob_stopwatch, length + gap_x, sy, sz )
	
	local _, py, pz = UIButtons.GetPosition( icon_stopwatch )
	UIButtons.ChangePosition( icon_stopwatch, -( length / 2 ), py, pz )
	
	GUI.sp.cp_checkpoint_text = UIButtons.FindChildByName( GUI.sp.cp_checkpoint_rb, "SpCpCheckpointBonusTitle" )
	GUI.sp.cp_checkpoint_blob = UIButtons.FindChildByName( GUI.sp.cp_checkpoint_rb, "SpCpBlobCheckpointC" )
	
	-- store targets
	GUI.sp.cp_bronze, GUI.sp.cp_silver, GUI.sp.cp_gold = Amax.SP_CheckpointRaceParams()
end

function HUD_SpCheckpointUpdate( time )
	local state, checkpoints_passed, time_left, beating_best = Amax.SP_CheckpointRaceState()
	
	HUD_SpStateUpdate( state, time, beating_best )
	HUD_SpTimerUpdate( time_left )
	
	if GUI.sp.checkpoint_time_left == nil then
		GUI.sp.checkpoint_time_left = time_left
	else
		if time_left > GUI.sp.checkpoint_time_left then
			UIButtons.TimeLineActive( "sp_timer_update", true )
		end
		
		GUI.sp.checkpoint_time_left = time_left
	end
end

function HUD_SpCheckpointPassed()
	local fan_rb_active = UIButtons.IsActive( GUI.mp_fan_msg_rb[ 1 ] )
	
	-- fan run active
	if fan_rb_active == false then
		UIButtons.TimeLineActive( "checkpoint_show_bonus", false, 0 )
		UIButtons.SetActive( GUI.sp.cp_stopwatch_rb, false )
		UIButtons.SetActive( GUI.sp.cp_checkpoint_rb, false )
	else
		UIButtons.TimeLineActive( "checkpoint_show_bonus", true, 0 )
		UIButtons.SetActive( GUI.sp.cp_stopwatch_rb, false )
		UIButtons.SetActive( GUI.sp.cp_checkpoint_rb, true )
	end
	
	-- resize box
	local gap_x = 20
	local length = UIButtons.GetStaticTextLength( GUI.sp.cp_checkpoint_text )
	
	local _, sy, sz = UIButtons.GetSize( GUI.sp.cp_checkpoint_blob )
	UIButtons.ChangeSize( GUI.sp.cp_checkpoint_blob, length + gap_x, sy, sz )
	
	UIButtons.TimeLineActive( "checkpoint_show_bonus_time", true, 0 )
end

function HUD_SpCheckpointCollectStopwatch()
	local fan_rb_active = UIButtons.IsActive( GUI.mp_fan_msg_rb[ 1 ] )
	
	-- fan run active
	if fan_rb_active == false then
		UIButtons.TimeLineActive( "checkpoint_show_bonus", false, 0 )
		UIButtons.SetActive( GUI.sp.cp_checkpoint_rb, false )
		UIButtons.SetActive( GUI.sp.cp_stopwatch_rb, false )
	else
		UIButtons.TimeLineActive( "checkpoint_show_bonus", true, 0 )
		UIButtons.SetActive( GUI.sp.cp_checkpoint_rb, false )
		UIButtons.SetActive( GUI.sp.cp_stopwatch_rb, true )
	end
	
	UIButtons.TimeLineActive( "checkpoint_show_bonus_time", true, 0 )
end

function HUD_SpDestructionSetup( inter_3d, inter_2d )
	GUI.is_destruction_race = true
	
	local backing = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
	UIButtons.SetParent( backing, inter_3d, UIEnums.Justify.TopLeft )
	
	local title = UIButtons.CloneXtGadgetByName( "hud_objects", "Points_Title" )
	UIButtons.SetParent( title, inter_3d, UIEnums.Justify.TopLeft )
	
	GUI.sp.destruction_points_amount = UIButtons.FindChildByName( title, "Points_Amount" )
	UIButtons.GetStaticTextLength( GUI.sp.destruction_points_amount, true )
	
	HUD_SpCreateLightPanel( inter_3d )
	HUD_SpCreateNextPanel( inter_3d, "HUD_SP_RACE_DESTRUCTION_DIFF_TO_MEDAL", UIText.HUD_BEST_POINTS )
	
	local timer_root_id = HUD_SpCreateTimer( inter_2d, "HUD_SP_RACE_DESTRUCTION_TIME_LEFT" )
	
	local time_added = UIButtons.CloneXtGadgetByName( "hud_objects", "SpDestructionTimeAdded" )
	UIButtons.SetParent( time_added, UIButtons.FindChildByName( timer_root_id, "SpTimerScaleDummy" ), UIEnums.Justify.MiddleCentre )
	
	-- points
	local points_msg_rb = UIButtons.CloneXtGadgetByName( "hud_objects", "SpDestructionPointsRB" )
	UIButtons.SetParent( points_msg_rb, inter_2d, UIEnums.Justify.TopCentre )
	
	GUI.sp.destruction_blob = UIButtons.FindChildByName( points_msg_rb, "SpDestructionBlobC" )
	GUI.sp.destruction_string = UIButtons.FindChildByName( points_msg_rb, "SpDestructionBlobString" )
	GUI.sp.destruction_points = UIButtons.FindChildByName( points_msg_rb, "SpDestructionPoints" )
	
	-- bonus points
	local bonus_points_msg_rb = UIButtons.CloneXtGadgetByName( "hud_objects", "SpDestructionBonusPointsRB" )
	UIButtons.SetParent( bonus_points_msg_rb, inter_2d, UIEnums.Justify.TopCentre )
	
	GUI.sp.destruction_bonus_blob = UIButtons.FindChildByName( bonus_points_msg_rb, "SpDestructionBonusBlobC" )
	GUI.sp.destruction_bonus_string = UIButtons.FindChildByName( bonus_points_msg_rb, "SpDestructionBonusBlobString" )
	GUI.sp.destruction_bonus_points = UIButtons.FindChildByName( bonus_points_msg_rb, "SpDestructionBonusPoints" )
	
	GUI.sp.destruction_rb = points_msg_rb
	GUI.sp.destruction_bonus_rb = bonus_points_msg_rb
end

function HUD_SpDestructionUpdate( time )
	local state, num_points, time_left, beating_best = Amax.SP_DestructionRaceState()
	
	HUD_SpStateUpdate( state, time, beating_best )
	HUD_SpTimerUpdate( time_left )
	
	if GUI.sp.destruction_time_left == nil then
		GUI.sp.destruction_time_left = time_left
	else	
		if time_left > GUI.sp.destruction_time_left then
			UIButtons.TimeLineActive( "sp_timer_update", true )
		end
	
		GUI.sp.destruction_time_left = time_left
	end
end

function HUD_SpDestructionPointsAdded( bonus )
	local fan_rb_active = UIButtons.IsActive( GUI.mp_fan_msg_rb[ 1 ] )
	
	-- fan run active
	if fan_rb_active == false then
		UIButtons.TimeLineActive( "show_destruction_points_small", true, 0 )
		UIButtons.TimeLineActive( "show_destruction_points", false, 0 )
	else		
		UIButtons.TimeLineActive( "show_destruction_points", true, 0 )
		UIButtons.TimeLineActive( "show_destruction_points_bonus", true, Select( ( bonus == true ), 0, 100 ) )
	end
	
	-- resize box	
	local gap_x = 20
	local x, _, _ = UIButtons.GetPosition( GUI.sp.destruction_points )
	
	local length = 0
	length = length + UIButtons.GetStaticTextLength( GUI.sp.destruction_string, true )
	length = length + x
	length = length + UIButtons.GetStaticTextLength( GUI.sp.destruction_points, true )
	
	local _, sy, sz = UIButtons.GetSize( GUI.sp.destruction_blob )
	UIButtons.ChangeSize( GUI.sp.destruction_blob, length + gap_x, sy, sz )
	
	local _, y, z = UIButtons.GetPosition( GUI.sp.destruction_string )
	UIButtons.ChangePosition( GUI.sp.destruction_string, -( length / 2 ), y, z )
	
	-- update length for small points
	UIButtons.GetStaticTextLength( GUI.sp.destruction_points_amount, true )
	
	UIButtons.TimeLineActive( "show_destruction_points_time", true, 0 )
end

function HUD_SpDestructionBonusHitStart()
	local fan_rb_active = UIButtons.IsActive( GUI.mp_fan_msg_rb[ 1 ] )
	
	-- fan run active
	if fan_rb_active == false then
		UIButtons.TimeLineActive( "show_destruction_bonus_points_start", false, 0 )
		UIButtons.TimeLineActive( "show_destruction_bonus_points_added", false, 0 )
	else		
		UIButtons.TimeLineActive( "show_destruction_bonus_points_start", true, 0 )
		UIButtons.TimeLineActive( "show_destruction_bonus_points_added", true, 0 )
	end
	
	-- resize box	
	local gap_x = 20
	local x, _, _ = UIButtons.GetPosition( GUI.sp.destruction_bonus_points )
	
	local length = 0
	length = length + UIButtons.GetStaticTextLength( GUI.sp.destruction_bonus_string, true )
	length = length + x
	length = length + UIButtons.GetStaticTextLength( GUI.sp.destruction_bonus_points, true )
	
	local _, sy, sz = UIButtons.GetSize( GUI.sp.destruction_bonus_blob )
	UIButtons.ChangeSize( GUI.sp.destruction_bonus_blob, length + gap_x, sy, sz )
	
	local _, y, z = UIButtons.GetPosition( GUI.sp.destruction_bonus_string )
	UIButtons.ChangePosition( GUI.sp.destruction_bonus_string, -( length / 2 ), y, z )
	
	-- update length for small points
	UIButtons.GetStaticTextLength( GUI.sp.destruction_points_amount, true )
end

function HUD_SpDestructionBonusHitAdded()
	UIButtons.TimeLineActive( "show_destruction_bonus_points_added", true, 0 )
end

function HUD_SpDestructionBonusHitEnd()
	UIButtons.TimeLineActive( "show_destruction_bonus_points_start", false )
end

function HUD_SpFanDemandFansAdded()
	UIButtons.TimeLineActive( "fan_demand_fans_added", true, 0 )
end

function HUD_SpFanDemandSetup( inter_3d, inter_2d )
	GUI.is_fan_demand_race = true
	
	local backing = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
	UIButtons.SetParent( backing, inter_3d, UIEnums.Justify.TopLeft )
	
	local title = UIButtons.CloneXtGadgetByName( "hud_objects", "FanDemand_Title" )
	UIButtons.SetParent( title, inter_3d, UIEnums.Justify.TopLeft )
	
	HUD_SpCreateLightPanel( inter_3d )
	HUD_SpCreateNextPanel( inter_3d, "HUD_SP_RACE_FAN_DEMAND_DIFF_TO_MEDAL", UIText.HUD_BEST_FANS )
	HUD_SpCreateTimer( inter_2d, "HUD_SP_RACE_FAN_DEMAND_TIME_LEFT" )
end

function HUD_SpFanDemandUpdate( time )
	local state, num_fan_demands_completed, time_left, beating_best = Amax.SP_FanDemandRaceState()
	
	HUD_SpStateUpdate( state, time, beating_best )
	HUD_SpTimerUpdate( time_left )
end

function HUD_SpBossSetup( inter_3d, inter_2d )
	GUI.is_boss_race = true
	
	local count = 0
	
	for i = 1, Amax.GetNumVehicles() - 1, 1 do	-- don't include you
		count = count + 1
		HUD_SpBossSetupCarHealth( count, i, false, inter_3d, inter_2d )
	end
	
	-- add you last
	count = count + 1
	HUD_SpBossSetupCarHealth( count, 0, true, inter_3d, inter_2d )
	
	UIButtons.TimeLineActive( "boss_show", true, 100 )
end

function HUD_SpBossSetupCarHealth( count, vehicle_index, is_player, inter_3d, inter_2d )
	GUI.sp_boss_offset = 4
	
	local health_root_id = UIButtons.CloneXtGadgetByName( "hud_objects", "SpBossHealthBackingTop" )
	UIButtons.SetParent( health_root_id, inter_3d, UIEnums.Justify.TopLeft )
	
	_, GUI.sp.boss_start_y, _ = UIButtons.GetPosition( health_root_id )
	
	local x, y, z = UIButtons.GetPosition( health_root_id )
	UIButtons.ChangePosition( health_root_id, x, y - ( GUI.sp_boss_offset * ( count - 1 ) ), z )
	
	local character_info =  Amax.SP_GetCharacterFromVehicle( vehicle_index )
	UIButtons.ChangeText( UIButtons.FindChildByName( health_root_id, "SpBossHealthName" ), character_info.name )
	
	if is_player == true then
		LocalGamerPicture( UIButtons.FindChildByName( health_root_id, "SpBossHealthPic" ), Profile.GetPrimaryPad() )
	else
		AIGamerPicture( UIButtons.FindChildByName( health_root_id, "SpBossHealthPic" ), character_info.avatar )
	end
	
	local health_id = UIButtons.FindChildByName( health_root_id, "SpBossHealth" )
	UIButtons.SetXtVar( health_id, "vehicle_index_override", vehicle_index )
	UIButtons.SetXtVar( health_id, "boss", true )
	UIButtons.SetXtVar( health_id, "always_do_effect", true )
end

function HUD_SpStreetRaceSetup( inter_3d, inter_2d )
	GUI.is_street_race = true
	
	HUD_SpCreateLightPanel( inter_3d )
	HUD_SpCreateNextPanel( inter_3d, "HUD_SP_RACE_STREET_DIFF_TO_MEDAL", UIText.HUD_BEST_FANS )
end

function HUD_SpStreetRaceUpdate( time )
	local state, position, beating_best = Amax.SP_StreetRaceState()
	
	HUD_SpStateUpdate( state, time, beating_best )
end

function HUD_SpSetupFD( inter_3d, inter_2d )
	GUI.is_race_fd = true
	
	local fd_index = UIGlobals.FriendDemandAttemptingIndex
	local fd_info = FriendDemand.GetInfo( fd_index )
	local event_info = SinglePlayer.EventInfo( fd_info.eventid_sp )	
	
	local backing_medal = UIButtons.CloneXtGadgetByName( "hud_objects", "SpMedalBackingTopFD" )
	UIButtons.SetParent( backing_medal, inter_3d, UIEnums.Justify.TopLeft )
	
	if GUI.is_boss_race == true then
		-- shift targets down
		local x, y, z = UIButtons.GetPosition( backing_medal )
		UIButtons.ChangePosition( backing_medal, x, GUI.sp.boss_start_y - ( GUI.sp_boss_offset * Amax.GetNumVehicles() ), z )
	end
	
	local light_dummy = UIButtons.FindChildByName( backing_medal, "SpLightDummyFD" )
	local light_medal = HUD_SpCreateLight( light_dummy, 0 )

	UIButtons.PrivateTimeLineActive( light_medal.on, "light_on", true, 0 )
	UIButtons.PrivateTimeLineActive( light_medal.blob, "light_on", true, 0 )
	SpSetStarTexture( light_medal.on, true, fd_info.state )
	UIButtons.ChangeColour( light_medal.blob, Sp_EventStateToColour( fd_info.state ) )
	
	GUI.sp.fd_state = fd_info.state
	
	Amax.ChangeText( UIButtons.FindChildByName( backing_medal, "SpMedalInfoFD" ), "HUD_SP_FD_RACE_MEDAL_TARGET_" .. Sp_EventStateToNumber( fd_info.state ) )
	
	GUI.sp.fd_pass = {  }
	GUI.sp.fd_pass[ 1 ] = UIButtons.FindChildByName( backing_medal, "SpMedalPassFD" )
	GUI.sp.fd_fail = {  }
	GUI.sp.fd_fail[ 1 ] = UIButtons.FindChildByName( backing_medal, "SpMedalFailFD" )
	
	-- criteria
	local backing_criteria = UIButtons.CloneXtGadgetByName( "hud_objects", "SpDummyFD2" )
	UIButtons.SetParent( backing_criteria, backing_medal, UIEnums.Justify.TopLeft )
	
	Amax.ChangeText( UIButtons.FindChildByName( backing_criteria, "SpTitleFD2" ), "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. fd_index .. "_0" )
	Amax.ChangeText( UIButtons.FindChildByName( backing_criteria, "SpInfoFD2" ), "HUD_SP_FD_RACE_CRITERIA" )
	
	GUI.sp.fd_pass[ 2 ] = UIButtons.FindChildByName( backing_medal, "SpMedalPassFD2" )
	GUI.sp.fd_fail[ 2 ] = UIButtons.FindChildByName( backing_medal, "SpMedalFailFD2" )
	
	local criteria_info = Fd_GetCriteriaIconInfo( event_info.kind )
	local criteria_icon_id = UIButtons.FindChildByName( backing_medal, "SpIconFD2" )
	
	UIButtons.ChangeTexture( criteria_info.tex, 0, criteria_icon_id )
	UIButtons.ChangeEffectIndex( criteria_icon_id, criteria_info.effect )
	
	-- optional demand target
	if fd_info.extra_modifier ~= UIEnums.FriendDemandModifer.NotUsed then
		local backing_param = UIButtons.CloneXtGadgetByName( "hud_objects", "SpDummyFD3" )
		UIButtons.SetParent( backing_param, backing_medal, UIEnums.Justify.TopLeft )
		
		Amax.ChangeText( UIButtons.FindChildByName( backing_param, "SpTitleFD3" ), "GAME_FRIEND_DEMAND_STORE_PARAM_TITLE_" .. fd_index .. "_0" )
		Amax.ChangeText( UIButtons.FindChildByName( backing_param, "SpInfoFD3" ), "HUD_SP_FD_RACE_PARAM" )
		
		GUI.sp.fd_pass[ 3 ] = UIButtons.FindChildByName( backing_medal, "SpMedalPassFD3" )
		GUI.sp.fd_fail[ 3 ] = UIButtons.FindChildByName( backing_medal, "SpMedalFailFD3" )
		
		local param_info = Fd_GetParamIconInfo( fd_info.extra_param )
		local param_icon_id = UIButtons.FindChildByName( backing_param, "SpIconFD3" )
		
		UIButtons.ChangeTexture( param_info.tex, 0, param_icon_id )
		UIButtons.ChangeEffectIndex( param_icon_id, param_info.effect )
		UIButtons.ChangeColour( param_icon_id, param_info.colour )
		
		local modifier_info = Fd_GetModifierIconInfo( fd_info.extra_modifier )
		local modifier_icon_id = UIButtons.FindChildByName( backing_param, "SpIconModifierFD3" )
		
		UIButtons.ChangeTexture( modifier_info.tex, 0, modifier_icon_id )
		UIButtons.ChangeEffectIndex( modifier_icon_id, modifier_info.effect )
	end
end

function HUD_SpUpdateFD( time )
	if GUI.is_boss_race == true then
		GUI.sp.state = Amax.SP_BossRaceState()
	end
	
	local num_current = Sp_EventStateToStars( GUI.sp.state, GUI.is_boss_race )
	local num_target = Sp_EventStateToStars( GUI.sp.fd_state, GUI.is_boss_race )
	
	local passing_medal = ( num_current >= num_target )
	
	if passing_medal ~= GUI.sp.fd_passing_medal then
		UIButtons.SetActive( GUI.sp.fd_pass[ 1 ], passing_medal )
		UIButtons.SetActive( GUI.sp.fd_fail[ 1 ], not passing_medal )
		GUI.sp.fd_passing_medal = passing_medal
	end
	
	local passing_criteria, passing_param = Amax.SP_StateFD()
	
	if passing_criteria ~= GUI.sp.passing_criteria then
		UIButtons.SetActive( GUI.sp.fd_pass[ 2 ], passing_criteria )
		UIButtons.SetActive( GUI.sp.fd_fail[ 2 ], not passing_criteria )
		GUI.sp.passing_criteria = passing_criteria
	end
	
	if passing_param ~= GUI.sp.passing_param then
		if GUI.sp.fd_pass[ 3 ] ~= nil and GUI.sp.fd_fail[ 3 ] ~= nil then
			UIButtons.SetActive( GUI.sp.fd_pass[ 3 ], passing_param )
			UIButtons.SetActive( GUI.sp.fd_fail[ 3 ], not passing_param )
		end
		GUI.sp.passing_param = passing_param
	end
end

function RefreshLastGaspVars()
	if Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then
		UIGlobals.Ingame.last_gasp_mode_active = Amax.LastGaspModeActive( 0 )
		UIGlobals.Ingame.can_fire_last_gasp = Amax.LastGaspModeActive( 0 )
		UIGlobals.Ingame.has_last_gasp = Amax.PlayerHasLastGasp( 0 )
		UIGlobals.Ingame.last_gasp_timer = Amax.GetLastGaspTime( 0 ) 
	end
end

function SyncLastGaspHudToVars()
	if Amax.IsGameModeMultiplayer() == true and IsSplitScreen() == false then
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.inter_3d_ids[ 1 ], "LastGaspRenderBranch"), UIGlobals.Ingame.last_gasp_mode_active )
		UIButtons.SetActive( GUI.LastGaspPowerupSlotID, UIGlobals.Ingame.last_gasp_mode_active, true)
		
		-- disable timer UI if not enabled
		if UIGlobals.Ingame.last_gasp_timer <= 0.0 then
			UIButtons.SetActive( UIButtons.FindChildByName( GUI.LastGaspPowerupSlotID, "time_LastGasp_OnCar"), false, true )
		else
			UIButtons.SetActive( UIButtons.FindChildByName( GUI.LastGaspPowerupSlotID, "time_LastGasp_OnCar"), true, true )
		end
		
	end
end

function HUD_ActivateECMWarning( arg1 )
	local player_index = arg1 + 1
	local targeting_bottom_ID = UIButtons.FindChildByName( GUI.inter_3d_ids[ player_index ], "targeting_bottom" )
	UIButtons.ChangeColour( targeting_bottom_ID, "Slot3" )
end

function HUD_DeactivateECMWarning( arg1 )
	local player_index = arg1 + 1
	local targeting_bottom_ID = UIButtons.FindChildByName( GUI.inter_3d_ids[ player_index ], "targeting_bottom" )
	UIButtons.ChangeColour( targeting_bottom_ID, "Support_3" )
end

function HUD_MpDestructionPositionUpdate()
	if NetRace.IsTeamGame() == true then
		return
	end
	
	for i = 1, GUI.num_players, 1 do
		local position = Multiplayer.GetSplitscreenPosition( i )
		
		if position > 3 then
			position = -1
		end
		
		if GUI.mp_destruction_positions[ i ] ~= position then
			local colour = GetResultColour( position )
			
			UIButtons.ChangeColour( GUI.mp_destruction_ids[ i ].icon, colour )
			UIButtons.ChangeColour( GUI.mp_destruction_ids[ i ].blob, colour )
			
			UIButtons.PrivateTimeLineActive( GUI.mp_destruction_ids[ i ].blob, "light_on", true, 0 )
			
			GUI.mp_destruction_positions[ i ] = position
		end
	end
end

function HUD_MpDestructionRivalUpdate()
	if NetRace.IsTeamGame() == true then
		return
	end
	
	local countdown_active, countdown_delayed = Multiplayer.GetRivalsCountdownDelayActive()
	
	if countdown_active == false and GUI.mp_rivals_active == true then
		return
	end
	
	if countdown_delayed == false and GUI.mp_rivals_on ~= true then
		-- countdown started
		UIButtons.TimeLineActive( "boss_show", true, 0.25 )
		
		for i = 1, GUI.num_players, 1 do
			UIButtons.SetActive( GUI.mp_rivals_ids[ i ].pic, false, true )
			UIButtons.SetActive( GUI.mp_rivals_ids[ i ].rival_title, true )
			UIButtons.SetActive( GUI.mp_rivals_ids[ i ].rival_time, true )
		end
		
		GUI.mp_rivals_on = true
	end
	
	if countdown_active == false and GUI.mp_rivals_active ~= true then
		-- rivals active
		UIButtons.TimeLineActive( "boss_show", true, 0 )
		
		for i = 1, GUI.num_players, 1 do
			UIButtons.SetActive( GUI.mp_rivals_ids[ i ].pic, true, true )
			UIButtons.SetActive( GUI.mp_rivals_ids[ i ].rival_title, false )
			UIButtons.SetActive( GUI.mp_rivals_ids[ i ].rival_time, false )
			
			HUD_MpDestructionRivalChange( i )
		end
		
		GUI.mp_rivals_active = true
	end
end

function HUD_MpDestructionRivalChange( player_index )
	if NetRace.IsTeamGame() == true then
		return
	end
	
	local rival_vehicle_index = Multiplayer.GetRivalIndex( player_index )
	
	-- name
	UIButtons.ChangeText( GUI.mp_rivals_ids[ player_index ].name, "MPL_SPECTATE_NAME" .. rival_vehicle_index )
	
	-- gamerpic
	if IsSplitScreen() == true then
		LocalGamerPicture( GUI.mp_rivals_ids[ player_index ].pic, UIGlobals.Splitscreen.players[ rival_vehicle_index + 1 ].pad )
	else
		local map = Profile.GetRemoteGamerPictureMap()
		local join_id = Multiplayer.VehicleIDToJoinID( rival_vehicle_index + 1 )
		RemoteGamerPicture( GUI.mp_rivals_ids[ player_index ].pic, map[ join_id ] )
	end
	
	-- health
	local health_id = GUI.mp_rivals_ids[ player_index ].health
	UIButtons.SetXtVar( health_id, "vehicle_index_override", rival_vehicle_index )
	UIButtons.SetXtVar( health_id, "boss", true )
	UIButtons.SetXtVar( health_id, "always_do_effect", true )
	
	-- effect
	UIButtons.PrivateTimeLineActive( GUI.mp_rivals_ids[ player_index ].blob, "rival_changed", true, 0 )
end

function HUD_MpDestructionSetupTeam( inter_3d, player_index, team, is_your_team )
	-- points
	local backing_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Pos_Backing" )
	UIButtons.SetParent( backing_id, inter_3d, UIEnums.Justify.TopLeft )
	
	local title_id = UIButtons.CloneXtGadgetByName( "hud_objects", "Points_Title" )
	UIButtons.SetParent( title_id, backing_id, UIEnums.Justify.TopLeft )
	UIButtons.ChangeText( title_id, Select( team == "team_a", UIText.MP_TEAM_A_NAME, UIText.MP_TEAM_B_NAME ) )
	UIButtons.ChangeColour( title_id, UIGlobals.TeamColours[ team ] )
	UIButtons.ChangePosition( title_id, nil, -0.25, nil, true )
	
	local points_id = UIButtons.FindChildByName( title_id, "Points_Amount" )
	UIButtons.ChangeText( points_id, "HUD_" .. player_index .. Select( is_your_team, "_MP_TEAM_POINTS_CURRENT", "_MP_TEAM_POINTS_OPPOSITION" ) )
	
	UIButtons.SetActive( UIButtons.FindChildByName( title_id, "SpDestructionPointsSmall" ), false )
	
	-- icon
	local team_icon = UIButtons.CloneXtGadgetByName( "hud_objects", "Position_Team_Icon" )
	UIButtons.SetParent( team_icon, title_id, UIEnums.Justify.MiddleLeft )
	
	UIButtons.ChangeTextureUV( team_icon, 0, UIGlobals.TeamIcons[ team ].u, UIGlobals.TeamIcons[ team ].v )
	UIButtons.ChangeColour( team_icon, UIGlobals.TeamColours[ team ] )
	
	-- shift along
	local sx, _, _ = UIButtons.GetSize( team_icon )
	UIButtons.ChangePosition( title_id, sx, nil, nil, true )
	UIButtons.ChangePosition( points_id, -sx, nil, nil, true )
	
	if is_your_team == false then
		local offset_y = -5.825
		UIButtons.ChangePosition( backing_id, nil, offset_y, nil, true )
		
		local scale = 0.825
		local sx, sy, sz = UIButtons.GetScale( backing_id )
		UIButtons.ChangeScale( backing_id, sx * scale, sy * scale, sz )
	end
end
