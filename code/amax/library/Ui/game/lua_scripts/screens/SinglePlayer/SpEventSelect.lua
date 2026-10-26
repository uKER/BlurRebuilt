GUI =
{
	finished = false,

	demands = 0,
	fd_option = 0,
	
	boss_demand_ids = {  },
	boss_demand_effects = {  },
	boss_demand_effect_vars = {  },
	boss_reveal_effect_vars = { },
	allow_buttons = true,
	
	bd_timer = 0,

	event_owner_panel_nodes = {},
	event_owner_nodes = {},
}

function Init()
	UISystem.PlaySound( UIEnums.SoundEffect.GarageEnter )	-- [PR] - need for the rev car sound
	
	AddSCUI_Elements( )
	UIScreen.SetScreenTimers( 0.3, 0.6 )
	
	Amax.SetCurrentSpTier( UIGlobals.Sp.CurrentTier )
	net_SetRichPresence( UIEnums.RichPresence.SpChoosing )

	Amax.RequestRivalRemotePicture( )

	StoreInfoLine()
	SetupInfoLine()

	GUI.MenuId = SCUI.name_to_id[ "Event_menu" ]
	GUI.boss_unlocked = UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].unlocked
	GUI.boss_defeated = UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].defeated
	-- Flag for SpPopToEventSelect()
	UIGlobals.Sp.CurrentTierBossUnlocked = GUI.boss_unlocked
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Keep the centered vignette exactly viewport-sized at any aspect.
		UIButtons.ChangeSize( SCUI.name_to_id[ "Gfx_Vignette" ], SCREEN_WIDTH, 480 )
	end

	UIButtons.TimeLineActive( "Hide_TierSelect", true )
		
	GUI.list = { }
	for i,v in ipairs( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events ) do
		local event = SinglePlayer.EventInfo( v )
		local amax_event = GameData.GetEvent( event.eventId )
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "Event_Node" )
				
		GUI.event_owner_panel_nodes[i] = node_id
		
		UIButtons.SetActive( UIButtons.FindChildByName( node_id, "New" ), event.new )
		
		if event.unlocked == false then
			UIButtons.SwapColour( node_id,  "Main_Black", "!255 32 32 32" )
			UIButtons.SwapColour( node_id,  "Main_White", "!255 64 64 64" )
			UIButtons.SwapColour( node_id,  "Main_2", "!255 64 64 64" )
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "LockedIcon" ), true )
		end
		
		if Sp_CompletedEventOnHard( event.difficulty ) == true then
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "ItsHard" ), true )
		end

		if FriendDemand.NumFriendDemandsForEvent(event.eventId).total > 0 then
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "FriendDemandIcon" ), true )
			if event.unlocked == false then
				UIButtons.SwapColour( node_id,  "Support_2", "!255 64 64 64" )
			end
		end
		
		SpEventSelect_UpdatePanel( node_id, event, amax_event, i )
		
		-- Show event stars
		local star_root_id = UIButtons.FindChildByName( node_id, "Light_Root" )
		
		-- primary
		local is_boss_race = ( event.kind == "boss" )
		local num_stars_primary = SP_MaxStars( is_boss_race )
		
		for j = 1, num_stars_primary, 1 do
			local star_id = Sp_CreateStar( ( j <= Sp_EventStateToStars( event.state, is_boss_race ) ), j, event.state )
			UIButtons.SetParent( star_id, star_root_id, UIEnums.Justify.TopRight )
		end
		
		if is_boss_race == false then
			-- fan par
			local star_id = Sp_CreateStar( event.fanParComplete, 6.5, nil, true )
			UIButtons.SetParent( star_id, star_root_id, UIEnums.Justify.TopRight )
			
			-- fan_run
			local star_id = Sp_CreateStar( event.fanRunComplete, 7.5, nil, nil, true )
			UIButtons.SetParent( star_id, star_root_id, UIEnums.Justify.TopRight )
		end
		
		UIButtons.AddListItem( GUI.MenuId, node_id, i )
		if event.unlocked == false then
			UIButtons.LockNode( node_id )
		end
		-- Store for later
		GUI.list[ i ] = 
		{
			event		= event,
			amax_event	= amax_event,
			node_id		= node_id,
		}
	end
	
	if UIGlobals.SpBossDemandStore == nil then
		UIGlobals.SpBossDemandStore = {  }
	end
	
	if UIGlobals.SpBossDemandStore[ UIGlobals.Sp.CurrentTier ] == nil then
		-- store current so we can compare when we come back to the event
		UIGlobals.SpBossDemandStore[ UIGlobals.Sp.CurrentTier ] = {  }
		CloneTable( UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ], UIGlobals.SpBossDemandStore[ UIGlobals.Sp.CurrentTier ] )
	end
		
	local boss_demands_store = UIGlobals.SpBossDemandStore[ UIGlobals.Sp.CurrentTier ].demands
	local boss_num = (UIGlobals.Sp.CurrentTier-1)
	local boss = UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ]
	local parent_id = SCUI.name_to_id[ "BossDemands_Root" ]
	local num_complete = 0
	
	-- Debug
--	boss_demands_store[ 2 ].progress = 0
	
	for i, v in ipairs( boss_demands_store ) do
		GUI.boss_demand_ids[ i ] = {  }
		
		local bd_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "BossDemands_Title" )
	
		local bd = "SPL_TIER_" .. boss_num .. "BD" .. (i-1)
		UIButtons.ChangeText( bd_id, bd .. "_TITLE" )
		
		GUI.boss_demand_ids[ i ].amount = UIButtons.FindChildByName( bd_id, "Amount" )
		UIButtons.ChangeText( GUI.boss_demand_ids[ i ].amount, "GAME_PROGRESS_" .. math.floor( v.value ) .. "_" .. math.floor( v.target ) )
		
		GUI.boss_demand_ids[ i ].tick = UIButtons.FindChildByName( bd_id, "Tick" )
		UIButtons.SetActive( GUI.boss_demand_ids[ i ].tick, v.complete )
		
		UIButtons.SetParent( bd_id, parent_id, UIEnums.Justify.BottomLeft )
		parent_id = UIButtons.FindChildByName( bd_id, "Frame" )
		
		GUI.boss_demand_ids[ i ].fill = UIButtons.FindChildByName( bd_id, "Fill" )
		GUI.boss_demand_ids[ i ].fill_progress = UIButtons.FindChildByName( bd_id, "Fill_Progress" )
		
		local sx, sy, sz = UIButtons.GetSize( GUI.boss_demand_ids[ i ].fill )
		UIButtons.ChangeSize( GUI.boss_demand_ids[ i ].fill_progress, sx * v.progress, sy, sz )
		
		-- work out whether we need any effects	
		if v.progress ~= boss.demands[ i ].progress then
			GUI.boss_demand_effects[ #GUI.boss_demand_effects + 1 ] = i
		end

		if boss.demands[ i ].complete == true then
			num_complete = num_complete + 1
		end
	end
	
	 if #GUI.boss_demand_effects > 0 then
		GUI.boss_demand_effect_vars.active = true
		GUI.boss_demand_effect_vars.timer = 0
		GUI.boss_demand_effect_vars.current = 1
		
		if num_complete >= #boss_demands_store then
			GUI.allow_buttons = false
			UIButtons.SetSelected( GUI.MenuId, false )
			GUI.do_boss_reveal = true
		end
	end
	
	-- Boss backing gubbins
	if GUI.boss_unlocked ~= true then
		UIButtons.SetParent( UIButtons.CloneXtGadgetByName( "SpSCUI", "event_Boss" .. UIGlobals.Sp.CurrentTier ), SCUI.name_to_id[ "Boss_Node" ], UIEnums.Justify.None )
	end
	
	-- Restore selection
	if IsNumber( UIGlobals.Sp.CurrentEvent ) == true then
		UIButtons.SetSelection( GUI.MenuId, UIGlobals.Sp.CurrentEvent )
	else
		UIGlobals.Sp.CurrentEvent = UIButtons.GetSelection( GUI.MenuId )
	end
	
	GUI.currentEvent = -1
	SpEventSelect_UpdateInfoLine( true )

	if Sp_ReturnFromGame() == true then
		if UIGlobals.SpReturnMode ~= UIGlobals.SpSelectMode_Event then
			print("Error return mode", UIGlobals.SpReturnMode, " not currently supported")
		end
		-- Select the last event played (should we move onto the next available here)?
		UIButtons.SetSelection( GUI.MenuId, UIGlobals.Sp.CurrentEvent )
		UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_Event
		UIGlobals.SpReturnMode = nil
	end
	
	-- Do this here in case they get revealed

	-- Demands
	AIGamerPicture( SCUI.name_to_id[ "Demands_Avatar" ], UIGlobals.boss_avatar_ids[ UIGlobals.Sp.CurrentTier ] )
	UIButtons.ChangeText( SCUI.name_to_id[ "Demands_Title" ], "SPL_TIER_" .. (UIGlobals.Sp.CurrentTier-1) .. "DEMANDS"  )
	-- Bio
	AIGamerPicture( SCUI.name_to_id[ "Bio_Avatar" ], UIGlobals.boss_avatar_ids[ UIGlobals.Sp.CurrentTier ] )
	UIButtons.ChangeText( SCUI.name_to_id[ "Bio_Title" ], "SPL_TIER_" .. (UIGlobals.Sp.CurrentTier-1) .. "DEMANDS_COMPLETE"  )
	UIButtons.ChangeText( SCUI.name_to_id[ "Bio_Text1" ], UIText[ "SP_BOSS_" .. UIGlobals.Sp.CurrentTier .. "_BIO" ] )
	local sx, _, sz = UIButtons.GetSize( SCUI.name_to_id[ "BioFrame" ] )
	local sy = UIButtons.GetStaticTextHeight( SCUI.name_to_id[ "Bio_Text1" ] ) + 3 -- do one

	UIButtons.ChangeSize( SCUI.name_to_id[ "BioFrame" ], sx, sy, sz )
	UIButtons.ChangeSize( SCUI.name_to_id[ "BioInner" ], sx, sy, sz )

	-- Showing boss car
	if GUI.boss_unlocked == true then
		if GUI.boss_defeated == true then
			local at = Multiplayer.GetAbility( UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].vehicle_ability, true )
			local magic_number  = 11
			if UIGlobals.Sp.CurrentTier == #UIGlobals.Sp.TierInfo then
				print("She's a special case : Evil Shannon")
				
				UIButtons.ChangeText( SCUI.name_to_id[ "Defeated_Title" ], UIText.VEH_FULL_ASTRA_SHANNON )
				UIButtons.ChangeText( SCUI.name_to_id[ "Defeated_Text0" ], UIText.MP_MODSHOP_NAME_BOSS_SUPERMOD )
				UIButtons.ChangeText( SCUI.name_to_id[ "Defeated_Text1" ], UIText.MP_MODSHOP_DESC_BOSS_SUPERMOD )
				UIShape.ChangeObjectName( SCUI.name_to_id[ "Defeated_ModIcon" ], "shannonbossmod" )
				
				-- [Bone] #37076 - Remove icon and title for last boss
				UIButtons.SetActive( SCUI.name_to_id[ "Defeated_ModIcon" ], false )
				UIButtons.SetActive( SCUI.name_to_id[ "Defeated_Text0" ], false )
				UIButtons.SetParent( SCUI.name_to_id[ "Defeated_Text1" ], SCUI.name_to_id[ "DefeatedFrame" ], UIEnums.Justify.TopCentre )
				UIButtons.ChangePosition( SCUI.name_to_id[ "Defeated_Text1" ], nil, -0.75, nil, true )
				
				magic_number = 1.5
			elseif at ~= nil then
				UIButtons.ChangeText( SCUI.name_to_id[ "Defeated_Text0" ], at.name )
				UIButtons.ChangeText( SCUI.name_to_id[ "Defeated_Text1" ], at.desc )
				UIShape.ChangeObjectName( SCUI.name_to_id[ "Defeated_ModIcon" ], at.icon_name )

				local vehicle = GameData.GetVehicle( UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].vehicle_id )
				if IsTable( vehicle ) == true then
					UIButtons.ChangeText( SCUI.name_to_id[ "Defeated_Title" ], vehicle.name_full )
				end
			end
			-- Resize box
			local sx, _, sz = UIButtons.GetSize( SCUI.name_to_id[ "DefeatedFrame" ] )
			local sy = UIButtons.GetStaticTextHeight( SCUI.name_to_id[ "Defeated_Text1" ] ) + magic_number -- Magic number awaar
			
			UIButtons.ChangeSize( SCUI.name_to_id[ "DefeatedFrame" ], sx, sy, sz )
			UIButtons.ChangeSize( SCUI.name_to_id[ "DefeatedInner" ], sx, sy, sz )
		else
			UIButtons.SetActive( SCUI.name_to_id[ "Bio_Tick" ], true )
		end
	
		GUI.hide_boss_demands = true
	else
		UIButtons.TimeLineActive( "Hide_Bio", true )
		UIButtons.TimeLineActive( "Hide_Defeated", true )
		
		GUI.hide_boss_demands = false
	end
	
	-- DEBUG
	--[[
		GUI.allow_buttons = false
		GUI.boss_reveal_effect_vars.active = true
		GUI.boss_reveal_effect_vars.timer = 1
		GUI.boss_reveal_effect_vars.current = 1
	]]--
	-- DEBUG
	
	
	GUI.show_boss_car = GUI.boss_unlocked
	if GUI.do_boss_reveal == true then
		GUI.show_boss_car = false
	end
		
	local camera_name
	local lerp_time
	if GUI.show_boss_car == true then
		UIButtons.TimeLineActive( "Hide_Demands", true )
		
		Amax.ChangeLocationNodeActor( SCUI.name_to_id[ "WorldPos" ], "Sp_2b" )
		UIButtons.TimeLineActive( "Hide_BossDemands", true, 1.0 )
		UIButtons.SetActive( SCUI.name_to_id[ "Gfx_Vignette"], true )
		
		Amax.LoadCar( UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].vehicle_id, false )
		camera_name = "Sp_2b"
		lerp_time = 0.6
	else
		camera_name = "Sp_2"
		lerp_time = 0.3
	end
	
	DeferCam_Init( camera_name, nil, lerp_time )
	
	if GUI.show_boss_car == false then
		UIButtons.SetActive( SCUI.name_to_id[ "Demands_Node" ], true )
	elseif GUI.boss_unlocked == true then
		if GUI.boss_defeated == true then
			UIButtons.SetActive( SCUI.name_to_id[ "Defeated_Node" ],true )
		else
			UIButtons.SetActive( SCUI.name_to_id[ "Bio_Node" ],true ) 
		end
	end
	
	SpEventSelect_UpdateInfoLine( true )
end

function SpEventSelect_UpdateInfoLine( force_update )
	local selected = UIButtons.GetSelection( GUI.MenuId )
	if UIGlobals.Sp.CurrentEvent == selected and force_update ~= true then
		return false
	end
	
	UIButtons.PrivateTimeLineActive( GUI.list[ UIGlobals.Sp.CurrentEvent ].node_id, "flash", false )
	
	UIGlobals.Sp.CurrentEvent = selected
	
	UIButtons.PrivateTimeLineActive( GUI.list[ UIGlobals.Sp.CurrentEvent ].node_id, "flash", true, 0 )
	
	local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ selected ]
	local options = {  }

	if GUI.allow_buttons == true then
		local event = GUI.list[ selected ].event
		local num_stars = Sp_EventStateToStars( event.state )
		
		if event.unlocked == true then
			options[ #options + 1 ] = UIText.INFO_A_SELECT
		end
		
		options[ #options + 1 ] = UIText.INFO_B_BACK
		
		if GUI.show_boss_car == true then
			options[ #options + 1 ] = UIText.INFO_X_BOSS_DEMANDS
		end

		if FriendDemand.NumFriendDemandsForEvent(event.eventId).total > 0 and event.unlocked == true then
			options[ #options + 1 ] = UIText.INFO_LB_VIEW_FRIEND_DEMANDS
		end

		if num_stars > 0 then
			options[ #options + 1 ] = "GAME_SHARE_BUTTON"
		end
	end
	
	SetupInfoLine( unpack( options ) )
	
	return true
end

function SpEventSelect_UpdatePanel( node_id, event, amax_event, i )
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "EventNumber" ), "GAME_NUM_" .. i )
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "KindName" ), Sp_KindToText( event.kind ) )
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "CityName" ), amax_event.city_name )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "CityIcon" ), amax_event.city_icon_name )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "KindIcon" ), Sp_KindToShape( event.kind ) )

	local event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ i ]
	local eo_state = Amax.EventOwnerState( event_id )
	
	GUI.event_owner_nodes[i] = UIButtons.CloneXtGadgetByName( "SpSCUI", "Owner_Node" )
	if event.new == false and event.unlocked == true then
		if eo_state == UIEnums.EventOwnerResult.Losing or eo_state == UIEnums.EventOwnerResult.Winning then
			UIButtons.SetParent( GUI.event_owner_nodes[i], UIButtons.FindChildByName( node_id, "BoxFrame" ), UIEnums.Justify.BottomLeft )
			Sp_UpdateOwnerState( GUI.event_owner_nodes[i], eo_state, event_id )
		end
	end
end

function SpEvent_UpdateFriendDemand()

	local index=GUI.demands[GUI.fd_option+1].index

	UIButtons.ChangeText( SCUI.name_to_id[ "sender_text" ], "GAME_FRIEND_DEMAND_STORE_SENDER_" .. index )
	UIButtons.ChangeText( SCUI.name_to_id[ "date_text" ], "GAME_FRIEND_DEMAND_STORE_DATE_" .. index )

	UIButtons.ChangeText( SCUI.name_to_id[ "summary_text" ], "GAME_FRIEND_DEMAND_STORE_NAME_" .. index )
	UIButtons.ChangeText( SCUI.name_to_id[ "desc_text" ], "GAME_FRIEND_DEMAND_STORE_DESCRIPTION_" .. index )

	UIButtons.ChangeText( SCUI.name_to_id[ "demand_text" ], "GAME_FRIEND_DEMAND_STORE_THE_DEMAND_" .. index )

	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text1_h" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. index .. "_" .. 0)
	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text1_v" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_VAR_" .. index .. "_" .. 0)

	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text2_h" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. index .. "_" .. 1)
	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text2_v" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_VAR_" .. index .. "_" .. 1)

	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text3_h" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. index .. "_" .. 2)
	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text3_v" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_VAR_" .. index .. "_" .. 2)

	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text4_h" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. index .. "_" .. 3)
	UIButtons.ChangeText( SCUI.name_to_id[ "crit_text4_v" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_VAR_" .. index .. "_" .. 3)

	UIButtons.ChangeText( SCUI.name_to_id[ "param_text1_h" ], "GAME_FRIEND_DEMAND_STORE_PARAM_TITLE_" .. index .. "_" .. 0)
	UIButtons.ChangeText( SCUI.name_to_id[ "param_text1_v" ], "GAME_FRIEND_DEMAND_STORE_PARAM_VAR_" .. index .. "_" .. 0)
	UIButtons.ChangeText( SCUI.name_to_id[ "param_text1_s" ], "GAME_FRIEND_DEMAND_STORE_PARAM_SWITCH_" .. index .. "_" .. 0)

	UIButtons.ChangeText( SCUI.name_to_id[ "param_text2_h" ], "GAME_FRIEND_DEMAND_STORE_PARAM_TITLE_" .. index .. "_" .. 1)
	UIButtons.ChangeText( SCUI.name_to_id[ "param_text2_v" ], "GAME_FRIEND_DEMAND_STORE_PARAM_VAR_" .. index .. "_" .. 1)
	UIButtons.ChangeText( SCUI.name_to_id[ "param_text2_s" ], "GAME_FRIEND_DEMAND_STORE_PARAM_SWITCH_" .. index .. "_" .. 1)

	UIButtons.ChangeText( SCUI.name_to_id[ "info_text" ], "GAME_FRIEND_DEMAND_INFO_" .. GUI.fd_option + 1 .. "_" .. GUI.demands.total)

	UIButtons.ChangeText( SCUI.name_to_id[ "attempts_text1_v" ], "GAME_FRIEND_DEMAND_STORE_ATTEMPTS_" .. index)

end

function StartLoop( time )
	--
	if DeferCam_Update( time ) == true and GUI.show_boss_car == true then
		Amax.ChangeEnvironmentSettings( true )
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	-- Handle popups
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Event then
			local selected = UIButtons.GetSelection( GUI.MenuId )
			local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ selected ]
		
			if arg3 == UIEnums.ShareOptions.Facebook then
				
				Amax.CreateBlurb(UIGlobals.SharingOptionsChosen, 1, eventId)
				
				StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\SpEventSelect.lua" )
				GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
			
			elseif arg3 == UIEnums.ShareOptions.Twitter then

				Amax.CreateBlurb(UIGlobals.SharingOptionsChosen, 0, eventId)

				StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\SpEventSelect.lua" )
				GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

			elseif arg3 == UIEnums.ShareOptions.Blurb then

				Amax.CreateBlurb(UIGlobals.SharingOptionsChosen, 2, eventId)

			end
		end
	end
	
	-- [Bone] #35640 - was able to press back here when controller was locked
	if GUI.allow_buttons == false or IsControllerLocked() == true then
		return
	end
	
	-- Handle controller messages
	if message == UIEnums.Message.MenuNext and arg2 == true then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Event then
			local selection = UIButtons.GetSelection( GUI.MenuId )
			local event = GUI.list[ selection ].event
			
			if event.unlocked == true then
				UIGlobals.Sp.CurrentEvent = selection
				if IsNumber( event.eventId ) == true then
					UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_PreRace
					
					PlaySfxNext()
					UISystem.PlaySound( UIEnums.SoundEffect.GraphicForward )
					PushScreen( "SinglePlayer\\SpPreRace.lua" )
					Amax.SetCurrentEvent( event.eventId )
					Amax.ChangeEnvironmentSettings( false )
					UIButtons.SetSelected( GUI.MenuId, false )

					--Check the current progress so we can show the right videos at the right time

					local tiers = SinglePlayer.TierInfo()
					local attemptedEventCount = 0;

					for i, v in ipairs( tiers ) do
						for j, w in ipairs( v.events ) do
							if SinglePlayer.EventInfo(w).state ~= "none" then
								attemptedEventCount = attemptedEventCount + 1
							end
						end
					end

					local CurrentRaceKind = -1
					local EventSpecificTutorial = -1

					--The order of this is very important and determines the order in which the videos will display

					if event.tier == 3 and event.event == 1 and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_FAN_DEMAND ) == true then
						print("FAN DEMAND VIDEO - RHYMERS 1st RACE")	
					elseif event.kind == "race" and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_GAME_MODE_RACE ) == true then
						print("RACE VIDEO")					
					elseif event.kind == "checkpoint" and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_GAME_MODE_CHECKPOINT ) == true  then
						print("CHECKPOINT VIDEO")
					elseif event.kind == "destruction" and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_GAME_MODE_DESTRUCTION ) == true  then
						print("DESTRUCTION VIDEO")
					elseif event.kind == "boss" and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_GAME_MODE_ONEONONE ) == true then
						print("BOSS VIDEO")
					elseif attemptedEventCount >= 1 and PlaySPMovieFullScreen(UIEnums.SPVideoConfig.VIDEO_POWER_UPS) == true then
						print("POWER UPS VIDEO")
					elseif attemptedEventCount >= 4 and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_FAN_PAR ) == true then
						print("FAN_PAR_VIDEO")
					elseif attemptedEventCount >= 8 and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_FAN_RUN ) == true then
						print("FAN_RUN_VIDEO")			
					elseif attemptedEventCount >= 10 and PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_TUTORIAL_2 ) == true then
						print("TUTORIAL 2 VIDEO")
					end

				end
			else
				PlaySfxError()
			end
		end
	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Event then
			UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_Tier
			UIGlobals.Sp.CurrentEvent = nil
			UIGlobals.Sp.EnableTierSelect = true
			
			PlaySfxBack()
			UISystem.PlaySound( UIEnums.SoundEffect.GraphicBackward )
			PopScreen()
			-- UISystem.PauseMovie( UIGlobals.Sp.CurrentTier, false ) -- resume
			
			if GUI.show_boss_car == true then
				Amax.ChangeEnvironmentSettings( false )
				Amax.SendMessage(UIEnums.GameFlowMessage.LoadSPProfileCar)
				Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
			else
				Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
			end

		elseif UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_PreRace or UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_FriendDemand then
			UIButtons.SetSelected( GUI.MenuId, true )
		end
		
	elseif message == UIEnums.Message.ButtonX and arg2 == true and GUI.boss_unlocked == true then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Event then
			UISystem.PlaySound( UIEnums.SoundEffect.Filter )
			
			GUI.hide_boss_demands = not GUI.hide_boss_demands
	
			UIButtons.TimeLineActive( "Hide_Demands", GUI.hide_boss_demands )
			
			if GUI.boss_defeated == true then
				UIButtons.TimeLineActive( "Hide_Defeated", not GUI.hide_boss_demands )
			else
				UIButtons.TimeLineActive( "Hide_Bio", not GUI.hide_boss_demands )
			end
		end
		
	elseif message == UIEnums.Message.ButtonY and arg2 == true then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Event then -- [Bone] #36282
			local selection = UIButtons.GetSelection( GUI.MenuId )
			local event = GUI.list[ selection ].event
			
			if FriendDemand.NumFriendDemandsForEvent( event.eventId ).total > 0 then	
			
				if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
					
					if net_CanReconnectToDemonware() == false then
						SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
					else
						net_StartServiceConnection(true, nil, false)
					end
				elseif NetServices.UserHasAgeRestriction() == true then
					SetupCustomPopup( UIEnums.CustomPopups.FailedAgeCheck )
				elseif LSP.IsConnected() == false then
					SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
				elseif FriendDemand.HasPrivilege() == false then
					SetupCustomPopup( UIEnums.CustomPopups.FriendChallengeNoPrivilege )
				else
					UIGlobals.FriendDemandFilterEvent = event.eventId
					UIGlobals.FriendDemandFilter = 3 -- current event
					Amax.ChangeEnvironmentSettings( false )
					UIButtons.SetSelected( GUI.MenuId, false )
					
					UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_FriendDemand
					UISystem.PlaySound( UIEnums.SoundEffect.Filter )
					PushScreen( "Shared\\Blurb.lua" )
				end			
			end
		end
	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Event then
			local selection = UIButtons.GetSelection( GUI.MenuId )
			local event = GUI.list[ selection ].event

			if Sp_EventStateToStars( event.state ) > 0 then
				if Amax.CanUseShare() == true then
					SetupCustomPopup( UIEnums.CustomPopups.SpEventSelectSharingOptions )
				end
			end
		end
	elseif message == UIEnums.Message.ButtonRightShoulder and arg2 == true then
		if UIGlobals.DevMode == true and UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Event then
			-- Cheat
			UIGlobals.Sp.CurrentEvent = UIButtons.GetSelection( GUI.MenuId )
			SinglePlayer.DebugCompleteEvent( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ] )
			SinglePlayer.MarkNotNew( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ] )
			SinglePlayer.ProcessUnlocks()
			
			UIGlobals.Sp.BossInfo = SinglePlayer.BossInfo()
			
			UIScreen.SetScreenTimers( 0, 0 )
			GoScreen("SinglePlayer\\SpEventSelect.lua")
		end
	elseif message == UIEnums.MiscMessage.FriendsUpdated then
		UIGlobals.UpdateEventOwners = true
	end
end

function FrameUpdate( time )
	if DeferCam_Update( time ) == true and GUI.show_boss_car == true then
		Amax.ChangeEnvironmentSettings( true )
	end
	
	GUI.bd_timer = GUI.bd_timer + time
	
	if UIGlobals.SelectEventMenu==true then
		UIButtons.SetSelected( GUI.MenuId, true )
		UIGlobals.SelectEventMenu=nil

		for i,v in ipairs( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events ) do
			local event = SinglePlayer.EventInfo( v )

			if FriendDemand.NumFriendDemandsForEvent(event.eventId).total > 0 then
				UIButtons.SetActive( UIButtons.FindChildByName( GUI.event_owner_panel_nodes[i] , "FriendDemandIcon" ), true )
				if event.unlocked == false then
					UIButtons.SwapColour( node_id,  "Support_2", "!255 64 64 64" )
				end
			else
				UIButtons.SetActive( UIButtons.FindChildByName( GUI.event_owner_panel_nodes[i] , "FriendDemandIcon" ), false )
			end
		end
		
		SpEventSelect_UpdateInfoLine(true)
	end
	
	if UIGlobals.OnMovieScreenInEventSelect == false then
		SpEventSelect_UpdateInfoLine()
	end
	
	if UIGlobals.UpdateEventOwners == true and Amax.IsFriendCompareReading() == false then
		UIGlobals.UpdateEventOwners = false

		for i,v in ipairs( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events ) do
			
			local event = SinglePlayer.EventInfo( v )
			local event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ i ]
			local eo_state = Amax.EventOwnerState( event_id )
	
			local avatar_id=UIButtons.FindChildByName( GUI.event_owner_nodes[i], "Owner_AvatarFrame" )
			UIButtons.SetActive(avatar_id, false)
			
			if event.new == false and event.unlocked == true then
				if eo_state == UIEnums.EventOwnerResult.Losing or eo_state == UIEnums.EventOwnerResult.Winning then
					UIButtons.SetParent( GUI.event_owner_nodes[i], UIButtons.FindChildByName( GUI.event_owner_panel_nodes[i], "BoxFrame" ), UIEnums.Justify.BottomLeft )
					
					UIButtons.SetActive(avatar_id, true)
					Sp_UpdateOwnerState( GUI.event_owner_nodes[i], eo_state, event_id )
				end
			end
		end		
	end

	if UIGlobals.OnMovieScreenInEventSelect == false then -- This is a movie playing
		SpEventSelect_FrameUpdateBossReveal( time )
		SpEventSelect_FrameUpdateBossDemands( time )
	end
end

function Render()
	--
end

function EnterEnd()
	--
	RestoreInfoLine()
	
	UIButtons.TimeLineActive( "Hide_TierSelect", false )
	
	UISystem.PlaySound( UIEnums.SoundEffect.GarageExit )	-- [PR] - need for the rev car sound
end	


function EndLoop( time )
	--
end

function End()
end

function SpEventSelect_FrameUpdateBossReveal( time )
	if GUI.boss_reveal_effect_vars.active ~= true then
		return
	end
	
	GUI.boss_reveal_effect_vars.timer = GUI.boss_reveal_effect_vars.timer - time
	if GUI.boss_reveal_effect_vars.timer <= 0 then
		-- print("Go stage",GUI.boss_reveal_effect_vars.current)
		if GUI.boss_reveal_effect_vars.current == 1 then
			GUI.boss_reveal_effect_vars.timer = 1.5
			UIButtons.PrivateTimeLineActive( GUI.list[ UIGlobals.Sp.CurrentEvent ].node_id, "flash", false )
			UIGlobals.Sp.CurrentEvent = #UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events 
			UIButtons.SetSelection( GUI.MenuId, UIGlobals.Sp.CurrentEvent )
			UIButtons.PrivateTimeLineActive( GUI.list[ UIGlobals.Sp.CurrentEvent ].node_id, "flash", true, 4.3 )
			
		elseif GUI.boss_reveal_effect_vars.current == 2 then
			GUI.boss_reveal_effect_vars.timer = 0.3

			UIButtons.TimeLineActive( "Hide_Bio", true )
			UIButtons.TimeLineActive( "Hide_Demands", true )
			UIButtons.SetActive( SCUI.name_to_id[ "Demands_Node" ], false )
			UIButtons.TimeLineActive( "Hide_EventSelect", true )

		elseif GUI.boss_reveal_effect_vars.current == 3 then

			GUI.boss_reveal_effect_vars.timer = 1
			UIButtons.TimeLineActive( "Hide_EventSelect", true )
			UIButtons.SetActive( SCUI.name_to_id[ "Gfx_Vignette"], true )
			
			Amax.LoadCar( UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].vehicle_id, false )
			Amax.ChangeEnvironmentSettings( true )
		elseif GUI.boss_reveal_effect_vars.current == 4 then			
			GUI.boss_reveal_effect_vars.timer = 1
			Amax.ChangeUiSplineCamera( 1, 1, 2, "Sp_1", 6, "Sp_2" )
			
		elseif GUI.boss_reveal_effect_vars.current == 5 then
			GUI.boss_reveal_effect_vars.timer = 4
			UISystem.PlaySound( UIEnums.SoundEffect[ "UnlockBoss" .. UIGlobals.Sp.CurrentTier ] ) -- [Bone] #34745
			
		elseif GUI.boss_reveal_effect_vars.current == 6 then
			GUI.boss_reveal_effect_vars.timer = 1
			Amax.ChangeLocationNodeActor( SCUI.name_to_id[ "WorldPos" ], "Sp_2b" )
			
			Amax.ChangeUiCamera( "Sp_2b", 2, 0 )
						
			UIButtons.SetActive( SCUI.name_to_id[ "Tier_Node" ], false )
			
			UIButtons.TimeLineActive( "Hide_EventSelect", false )
			UIButtons.TimeLineActive( "Hide_Bio", false )
		else
			if UIGlobals.Sp.CurrentTier == #UIGlobals.Sp.TierInfo then
				-- Flag to update on tier select that is running in the background
				UIGlobals.Sp.RevealEvilShannon = true
			end

			UIButtons.SetActive( SCUI.name_to_id[ "Bio_Tick" ], true )
			UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "Bio_Tick" ], "demand_complete", true, 0 )
			
			UIButtons.TimeLineActive( "Hide_BossBio", false )
			GUI.boss_reveal_effect_vars.active = false
			GUI.show_boss_car = true

			GUI.allow_buttons = true
			UIButtons.SetSelected( GUI.MenuId, true )
			
			SpEventSelect_UpdateInfoLine( true )
		end
		
		GUI.boss_reveal_effect_vars.current = GUI.boss_reveal_effect_vars.current + 1
		
	end
end

function SpEventSelect_FrameUpdateBossDemands( time )
	if GUI.boss_demand_effect_vars.active ~= true then
		return
	end
	
	local time_start_wait = 1
	
	if GUI.bd_timer < time_start_wait then
		return
	end
	
	local time_fill = 1
	local time_wait = 0
	
	if GUI.bd_has_played_increment_sfx ~= true then
		UISystem.PlaySound( UIEnums.SoundEffect.BossDemandIncrement )
		GUI.bd_has_played_increment_sfx = true
	end
	
	if GUI.boss_demand_effect_vars.timer <= time_fill then
		GUI.boss_demand_effect_vars.timer = GUI.boss_demand_effect_vars.timer + time
		
		local has_completed = false
		
		for i, v in ipairs( GUI.boss_demand_effects ) do
			GUI.boss_demand_effect_vars.current = i
			
			local demand_i = GUI.boss_demand_effects[ GUI.boss_demand_effect_vars.current ]
			local demand_c = UIGlobals.SpBossDemandStore[ UIGlobals.Sp.CurrentTier ].demands[ demand_i ]
			local demand_t = UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].demands[ demand_i ]
			
			local effect_progress01 = Clamp( ( GUI.boss_demand_effect_vars.timer / time_fill ), 0, 1 )
			
			local value = demand_c.value + ( demand_t.value - demand_c.value ) * effect_progress01
			local target = demand_t.target
			
			local progress01 = value / target
			
			UIButtons.ChangeText( GUI.boss_demand_ids[ demand_i ].amount, "GAME_PROGRESS_" .. math.floor( value ) .. "_" .. math.floor( target ) )
			
			local sx, sy, sz = UIButtons.GetSize( GUI.boss_demand_ids[ demand_i ].fill )
			UIButtons.ChangeSize( GUI.boss_demand_ids[ demand_i ].fill_progress, sx * progress01, sy, sz )
			
			if GUI.boss_demand_effect_vars.timer > time_fill and progress01 == 1 then
				UIButtons.SetActive( GUI.boss_demand_ids[ demand_i ].tick, true )
				UIButtons.PrivateTimeLineActive( GUI.boss_demand_ids[ demand_i ].tick, "demand_complete", true, 0 )
				UIButtons.PrivateTimeLineActive( GUI.boss_demand_ids[ demand_i ].amount, "demand_complete", true, 0 )
				has_completed = true
			end
		end
		
		if has_completed == true then
			UISystem.PlaySound( UIEnums.SoundEffect.BossDemandComplete )
		end
	else
		GUI.boss_demand_effect_vars.timer = GUI.boss_demand_effect_vars.timer + time
		
		if GUI.boss_demand_effect_vars.timer > ( time_fill + time_wait ) then
			GUI.boss_demand_effect_vars.current = GUI.boss_demand_effect_vars.current + 1
			GUI.boss_demand_effect_vars.timer = 0
			
			--if GUI.boss_demand_effect_vars.current > #GUI.boss_demand_effects then
				GUI.boss_demand_effect_vars.active = false
				
				if GUI.do_boss_reveal == true then
					SpEventSelect_CheckBossUnlocks()
					
					GUI.boss_reveal_effect_vars.active = true
					GUI.boss_reveal_effect_vars.timer = 1
					GUI.boss_reveal_effect_vars.current = 1
				end

				-- activate screen
				--GUI.allow_buttons = true
				--UIButtons.SetSelected( GUI.MenuId, true )
				SpEventSelect_UpdateInfoLine() -- for infolines
				-- copy new table
				CloneTable( UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ], UIGlobals.SpBossDemandStore[ UIGlobals.Sp.CurrentTier ] )
			--end
		end
	end
end

function SpEventSelect_CheckBossUnlocks()

	local unlocks = Sp_UnlocksFilter( SinglePlayer.ProcessUnlocks() )
	if #unlocks > 0 then
		for i, v in ipairs( unlocks ) do
			if v.kind == "event" then
			
				--Removed as they no longer want one on one unlock videos
				--[[
				if v.tier ~= nil then
					PlaySPUnlockMovieFullScreen(UIEnums.SPVideoConfig.VIDEO_BOSS_UNLOCKS, (v.tier-1) )	
				end
				]]--
			end
		end
	end
		
end
