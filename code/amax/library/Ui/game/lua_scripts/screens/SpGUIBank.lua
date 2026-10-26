function Sp_NewGame()
	SinglePlayer.ClearUnlocks()				-- clear previous person's unlocks
	SinglePlayer.NewSaveGame()				-- new person
	SinglePlayer.ProcessUnlocks()			-- level 1 unlocks
	SinglePlayer.ClearUnlocks()				-- clear level 1 unlocks
	UIGlobals.SpBossDemandStore = {  }		-- boss demands store for progress effects
	UIGlobals.SpReturnMode = nil
	UIGlobals.SpSelectMode = nil
	UIGlobals.Sp = nil
end

function Sp_ReturnFromGame()
	if UIGlobals.SpReturnMode == nil then
		return false
	end
	
	if IsTable( UIGlobals.Sp ) == false then
		print("Tried to return without a single player table")
		return false
	end
	
	if IsNumber( UIGlobals.Sp.CurrentTier ) == false then
		print("Tried to return to an invalid tier")
		return false
	end
	
	-- Just returning to a tier
	if UIGlobals.SpReturnMode == UIGlobals.SpSelectMode_Tier then
		return true
	end
	
	if IsNumber( UIGlobals.Sp.CurrentEvent ) == false then
		print("Tried to return to an invalid event")
		return false
	end
	
	return true
end

function Sp_GarageFilterCars()
	if GUI.sp_event_info == nil then
		return false
	end
	
	--show_table( GUI.garage.cars, 1 )
	
	if GUI.sp_event_info.restriction == "none" then
		-- nothing to do
		
	elseif GUI.sp_event_info.restriction == "class" then
		for i, v in pairs( GUI.garage.cars ) do
			if GUI.sp_event_info.carClass ~= UIEnums.VehicleClass.ANY and GUI.garage.car_info[ i ].class ~= GUI.sp_event_info.carClass then
				GUI.garage.cars[ i ] = nil
			end
		end
		
	elseif GUI.sp_event_info.restriction == "set" then
		for i, v in pairs( GUI.garage.cars ) do
			if GUI.sp_event_info.carList[ i ] ~= true then
				GUI.garage.cars[ i ] = nil
			end
		end
		
		if #GUI.garage.cars == 0 then
			print( "ERROR - carList restriction has no cars" )
		end
		
		return true
		
	else
		print( "WARNING - unknown car restriction" )
	end
	
	return false
end

function Sp_UnlocksFilter( unlocks )
	local new_unlocks = {  }
	
	for i, v in pairs( unlocks ) do
		if v.kind == "event" and v.boss == false then
			-- remove
		elseif v.kind == "boss" then
			if v.tier == 1 then
				Amax.CheckStickerProgress(UIEnums.StickerType.UnlockShannon, 1) -- boss unlock achievement for Shannon v1 only
			end		
			-- remove
		else
			new_unlocks[ #new_unlocks + 1 ] = v
		end
	end
	
	return new_unlocks

end

function SpSetStarTexture( id, is_on, state, is_fan_par, is_fan_run )
	local u = 0
	
	if is_on == true then
		if is_fan_par == true then
			u = 4/16
		elseif is_fan_run == true then
			u = 5/16
		else
			if state == "gold" then
				u = 3/16
			elseif state == "silver" then
				u = 2/16
			elseif state == "bronze" then
				u = 1/16
			else
				u = 6/16
			end
		end
	else
		if is_fan_par == true then
			u = 7/16
		elseif is_fan_run == true then
			u = 8/16
		else
			u = 6/16
		end
	end
	
	UIButtons.ChangeTextureUV( id, 0, u, 0 )
end

function Sp_CreateStar( is_on, i, state, is_fan_par, is_fan_run, scale_offset, on_off_effects, fade_in )
	if scale_offset == nil then
		scale_offset = 1.1
	else
		scale_offset = 1.1 * scale_offset
	end
	
	local light_on  = nil
	local light_off = nil
	local light_on_blob = nil
	
	if ( is_on == true or is_on == nil ) and on_off_effects ~= true then
		light_on = UIButtons.CloneXtGadgetByName( "SpSCUI", "BlurLightW_On" )
	end
	
	if ( is_on == false or is_on == nil ) and on_off_effects ~= true then
		light_off = UIButtons.CloneXtGadgetByName( "SpSCUI", "BlurLightW_Off" )
	end
	
	if on_off_effects == true then
		light_off = UIButtons.CloneXtGadgetByName( "SpSCUI", "BlurLightW_Off" )
		
		if fade_in == true then
			light_on = UIButtons.CloneXtGadgetByName( "SpSCUI", "BlurLightW_On_FadeIn" )
			light_on_blob = UIButtons.CloneXtGadgetByName( "SpSCUI", "BlurLightW_On_FadeIn_Blob" )
			UIButtons.SetParent( light_on_blob, light_off, UIEnums.Justify.MiddleCentre )
			
			if is_on == true then
				UIButtons.PrivateTimeLineActive( light_on, "light_on", true, 1 )
			end
		else
			light_on = UIButtons.CloneXtGadgetByName( "SpSCUI", "BlurLightW_On_FadeOut" )
			
			if is_on == false then
				UIButtons.PrivateTimeLineActive( light_on, "light_off", true, 1 )
			end
		end
	end
	
	-- offset
	if i ~= nil then
		if light_on ~= nil then
			local px, py, pz = UIButtons.GetPosition( light_on )
			local sx, sy, sz = UIButtons.GetSize( light_on )
			UIButtons.ChangePosition( light_on, px + sx * ( i - 1 ) * scale_offset, py, pz )
		end
		if light_off ~= nil then
			local px, py, pz = UIButtons.GetPosition( light_off )
			local sx, sy, sz = UIButtons.GetSize( light_off )
			UIButtons.ChangePosition( light_off, px + sx * ( i - 1 ) * scale_offset, py, pz )
		end
	end
	
	-- tex
	if light_on ~= nil then
		SpSetStarTexture( light_on, true, state, is_fan_par, is_fan_run )
	end
	if light_off ~= nil then
		SpSetStarTexture( light_off, false, state, is_fan_par, is_fan_run )
	end
	
	if light_on == nil then
		return light_off
	end
	
	if light_off == nil then
		return light_on
	end
	
	return { light_on, light_off }, light_on_blob
end

function Sp_ActivateStar( star_table, is_on )
	if IsTable( star_table ) == false then
		return
	end

	local a = 1
	local b = 2
	if is_on == false then
		a = 2
		b = 1
	end

	UIButtons.SetActive( star_table[ a ], true, true )
	UIButtons.SetActive( star_table[ b ], false, true )
end

function Sp_EventStateToStars( state, is_boss )
	if is_boss == true then
		return Select( ( state == "gold" ), SP_MaxStars(), 0 )
	end
	
	local state_to_stars = 
	{
		bronze = 3,
		silver = 4,
		gold = 5,
	}
	
	return ( state_to_stars[ state ] or 0 )
end

function Sp_EventStateToNextState( state )
	local state_to_next_state = 
	{
		none	= "bronze",
		bronze	= "silver",
		silver	= "gold",
		gold	= "gold",
	}
	
	return ( state_to_next_state[ state ] or "none" )
end

function Sp_EventStateToPrevState( state )
	local state_to_prev_state = 
	{
		bronze	= "none",
		silver	= "bronze",
		gold	= "silver",
	}
	
	return ( state_to_prev_state[ state ] or "none" )
end

function Sp_EventStateToColour( state )
	-- Sorry
	return GetResultColour( state )
end

function Sp_EventStateToNumber( state )
	local state_to_number = 
	{
		bronze = 3,
		silver = 2,
		gold = 1,
	}
	
	return ( state_to_number[ state ] or 4 )
end

function Sp_KindToShape( kind )
	local kind_to_shape =
	{
		boss		= "deathmatch",
		race		= "Style_racing",
		fan			= "Elimination_time",
		checkpoint	= "quick_race",
		destruction	= "survivor",
	}
	
	return ( kind_to_shape[ kind ] or 0 )
end

function Sp_KindToText( kind )
	local kind_to_text = 
	{
		boss		= UIText.SP_KIND_BOSS,
		race		= UIText.SP_KIND_RACE,
		fan			= UIText.SP_KIND_FAN,	
		checkpoint	= UIText.SP_KIND_CHECKPOINT,
		destruction	= UIText.SP_KIND_DESTRUCTION,
	}
	
	return ( kind_to_text[ kind ] or 0 )
end

function Sp_KindToHelpText( kind )
	local kind_to_text = 
	{
		boss		= UIText.SP_KIND_BOSS,
		race		= UIText.SP_KIND_RACE,
		fan			= UIText.SP_KIND_FAN,	
		checkpoint	= UIText.SP_KIND_CHECKPOINT,
		destruction	= UIText.SP_KIND_DESTRUCTION,
	}
	
	return ( kind_to_text[ kind ] or 0 )
end

function Sp_ClassToShape( class )
	local class_to_shape = 
	{
		[0]			= "class_a",
		[1]			= "class_b",
		[2]			= "class_c",
		[3]			= "class_d",
		[4]			= "class_e",
		[5]			= "class_any",
	}
	
	return ( class_to_shape[ class ] or 0 )
end

function Sp_ClassToRestrictionText( class )
	local class_to_text = 
	{
		[0]			= UIText.CMN_CLASS_A,
		[1]			= UIText.CMN_CLASS_B,
		[2]			= UIText.CMN_CLASS_C,
		[3]			= UIText.CMN_CLASS_D,
		[4]			= UIText.CMN_CLASS_E,
		[5]			= UIText.CMN_ANY,
	}
	
	return ( class_to_text[ class ] or 0 )
end


function Sp_CentreCityRoute( city_icon_id, city_id, route_id )
	local city_len = UIButtons.GetStaticTextLength( city_id )
	local route_len = UIButtons.GetStaticTextLength( route_id )
	local sx,sy = UIButtons.GetSize( city_icon_id ) 
	local len = city_len
	if len < route_len then
		len = route_len 
	end
	len = (len - sx) / 2
	UIButtons.ChangePosition( route_id, -len, 0.5 )
end

function SP_ChangeRestrictionText( title_id, text_id, event )
	if event.restriction ~= "set" or IsNumber( event.description ) == false then
		return false
	end
	
	UIButtons.SetActive( title_id, true )
	UIButtons.SetActive( text_id, true )
		
	UIButtons.ChangeText( text_id, event.description )
	
	return true
end

function SP_ChangeRestrictionIcon( icon_id, event )
	if event.restriction == "class" and IsNumber( event.carClass ) == true then
		UIButtons.SetActive( icon_id, true )
		UIShape.ChangeObjectName( icon_id, Sp_ClassToShape( event.carClass ) )
	else
		UIButtons.SetActive( icon_id, false )
	end
end

-- LEADERBOARD STUFF


function SpMiniLeaderboard_Init()
	GUI.reading_scoreboard = false
	GUI.read_cooldown = 0
	GUI.scoreboard_has_read = false
	GUI.scoreboard_offset = 0
	GUI.scoreboard_total_rows = 0
	GUI.scoreboard = 
	{
		start = 0,
		you = 0,
		friends = 1,
		top = 2,
		count = 3,
	}
	GUI.scoreboard_type = 1
	GUI.num_rows = 3
	GUI.rows_id = { }
	GUI.reading_me = false
	GUI.no_score_me = false
	GUI.row_me = 0
	GUI.rank_me = 0
	GUI.server_timeout = 0
	GUI.CurrentScoreboardType = 0
	GUI.FinishedWriting = false
	GUI.LeaderboardWriteTimer = 1

	local sbToLookAt = Amax.CheckScoreboardToLookAt()
	Amax.ScoreboardLookAt( sbToLookAt )
	UIGlobals.ScoreboardLookingAt = sbToLookAt

	GUI.CurrentScoreboardType = Amax.CheckSpScoreboardType()

	-- default type
	GUI.scoreboard_type = GUI.scoreboard.friends
	
	print("SP LB Init ...")
end

function SpMiniLeaderboard_PostInit( parent, frontend )
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and LSP.IsConnected() == false then
		return false
	end
	
	-- [Bone] #36096 Online and offline leaderboards showing on prerace
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) ~= true then
		return false
	end

	-- clone leaderboard
	local leaderboard = UIButtons.CloneXtGadgetByName( "SpSCUI", "Leaderboard_Node" )
	UIButtons.SetParent( leaderboard, parent, UIEnums.Justify.MiddleCentre )
	
	GUI.leaderboard_node_list =  UIButtons.FindChildByName( leaderboard, "node_list" )
	GUI.leaderboard_node_list_title = UIButtons.FindChildByName( leaderboard, "node_list_title" )

	GUI.reading_id = UIButtons.FindChildByName( leaderboard, "Reading" )

	SpMiniLeaderboard_Setup()
	
	if frontend == true then
		UIButtons.ChangePanel( leaderboard, UIEnums.Panel._3DAA_WORLD, true )
		UIButtons.TimeLineActive( "leaderboard_on", true, 0 )
	end
	
	print("SP LB PostInit ...")
	
	return true
end

function SpMiniLeaderboard_FrameUpdate(time, shouldWriteLeaderboard, readLeaderboard)

	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and LSP.IsConnected() == false then
		return
	end

	-- [Bone] #36096 Online and offline leaderboards showing on prerace
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) ~= true then
		return
	end
	
	if readLeaderboard == nil then
		readLeaderboard = true
	end
	
	if shouldWriteLeaderboard == nil then
		shouldWriteLeaderboard = false
	end

	local result
	
	if shouldWriteLeaderboard == true then	
		
		GUI.LeaderboardWriteTimer = GUI.LeaderboardWriteTimer - time
		
		if GUI.LeaderboardWriteTimer <= 0 then
			result = true
		end	
	else
		result = true
	end

	if result ~= true then
		return
	elseif GUI.scoreboard_has_read == false then
		SpMiniLeaderboard_StartRead( readLeaderboard )
		GUI.scoreboard_has_read = true
		
		print("SP LB Start Read ...")
	end

	if GUI.reading_scoreboard == true then
	
		print("SP LB Start Done Read ...")
		
		local complete, rows, row_me, rank_me = Amax.ScoreboardReadComplete()

		GUI.server_timeout = GUI.server_timeout + time

		if GUI.server_timeout > 15 then
			complete = true
			rows = 0
			row_me = -1
			rank_me = -1
		end
		
		GUI.scoreboard_total_rows = rows
		GUI.row_me = row_me
		GUI.rank_me = rank_me

		if complete == true then
			if GUI.scoreboard_type == GUI.scoreboard.you then
				if GUI.rank_me ~= -1 then
					if ( GUI.rank_me - GUI.num_rows / 2 ) < 0 then
						GUI.scoreboard_offset = 0	-- we're on the first page
					elseif ( GUI.rank_me + GUI.num_rows / 2 ) > GUI.scoreboard_total_rows then
						GUI.scoreboard_offset = GUI.scoreboard_total_rows - GUI.num_rows	-- we're on the last page
					else
						GUI.scoreboard_offset = GUI.rank_me - GUI.num_rows / 2 - 1
					end
				end
			end
			
			SpMiniLeaderboard_Fill( rows, 0 )
			
			GUI.read_cooldown = 0.5 -- can't start another read within 0.5 seconds - TCR
		end
	else
		GUI.server_timeout = 0
		GUI.read_cooldown = GUI.read_cooldown - time
	end
	
	return GUI.scoreboard_has_read;
	
end

function SpMiniLeaderboard_Fill( rows, offset )
	if rows > GUI.num_rows then
		rows = GUI.num_rows
	end
	
	if GUI.reading_me == true and GUI.rank_me == -1 then
		GUI.no_score_me = true
	end
	
	if GUI.no_score_me == true then
		SetupCustomPopup( UIEnums.CustomPopups.LeaderboardNoScore )
	else
		if rows == 0 then
			--Fill with dashes
			local node_list_id = GUI.leaderboard_node_list
			for i = 1 , 3, 1 do				
				SpRaceAwards_SpLeaderboard_FillLeaderboard( node_list_id, i, 0 )				
			end
		else
		
			local node_list_id = GUI.leaderboard_node_list
			local count = 0
			
			for i = 1 + offset, rows + offset, 1 do
				count = count + 1

				SpRaceAwards_SpLeaderboard_FillLeaderboard( node_list_id, count, i )
													
				local is_me = false
			
				if GUI.scoreboard_type == GUI.scoreboard.friends then
					is_me = ( ( count + GUI.scoreboard_offset ) == GUI.row_me )
				else
					is_me = ( count == GUI.row_me )
				end
				
				local colour_me = "Support_1"
				local colour_other = "Main_1"
			
				if is_me == true then
					SpMiniLeaderboard_ColourRow( count, colour_me )
				else
					SpMiniLeaderboard_ColourRow( count, colour_other )								
				end
			end	
		end
	end
	
	SpMiniLeaderboard_ShowRead( false )
end

function SpMiniLeaderboard_ShowRead( reading )
	
	local rows = GUI.scoreboard_total_rows

	if GUI.scoreboard_offset + GUI.num_rows > GUI.scoreboard_total_rows then
		rows = GUI.scoreboard_total_rows % GUI.num_rows
	end

	if rows > GUI.num_rows then
		rows = GUI.num_rows
	end
	
	if reading == true then
		UIButtons.SetActive( GUI.reading_id, true )
		--UIButtons.SetActive( size_id, false )
		--UIButtons.SetActive( total_icon_id, false )
		
		for i = 1, GUI.num_rows, 1 do

			if #GUI.rows_id > i then
				UIButtons.SetActive( GUI.rows_id[ i ].dummy, false, true )
			end

			UIButtons.SetNodeItemLocked( GUI.leaderboard_node_list, i - 1, true )
	
		end
		
	else
		UIButtons.SetActive( GUI.reading_id, false )
		--UIButtons.SetActive( size_id, true )
		--UIButtons.SetActive( total_icon_id, true )
	end
	
	--[[
	if reading == true then
		TransmitterOn( GUI.bleep_bleep_id )
	else
		TransmitterOff( GUI.bleep_bleep_id )
	end
	--]]
	
	GUI.reading_scoreboard = reading
end

function SpMiniLeaderboard_Setup()

	local node_list_id = GUI.leaderboard_node_list

	--setup the titles
	local node_title_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "_node_mini_leaderboard_title" )
	local text_title_id = UIButtons.FindChildByName( node_title_id, "_text_title_score" )
	local name_title_id = UIButtons.FindChildByName( node_title_id, "_text_title_gamertag" )
	UIButtons.ChangeText( name_title_id, UIText.MP_SCOREBOARD_NAME )

	--change the title based on what event it is
	if  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPRacesScoreboard then
		UIButtons.ChangeText( text_title_id, UIText.SP_FANS_EARNED )
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPFanDemandsScoreboard then
		UIButtons.ChangeText( text_title_id, UIText.SP_FANS_EARNED )
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPCheckpointsScoreboard then
		UIButtons.ChangeText( text_title_id, UIText.SP_TIME_LEFT )
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPDestructionScoreboard then
		UIButtons.ChangeText( text_title_id, UIText.MP_SCOREBOARD_POINTS )
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPOneOnOneScoreboard then
		UIButtons.ChangeText( text_title_id, UIText.SP_COMP_TIME )	
	end		


	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "_node_mini_leaderboard" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_score_id = UIButtons.FindChildByName( node_id, "_text_score" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, gamertag = text_gamertag_id, score = text_score_id, }
	end

	local node_list_title_id = GUI.leaderboard_node_list_title
	

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function SpRaceAwards_SpLeaderboard_FillLeaderboard( node_list_id, count, i )
	
	if i == 0 then
		UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_BLANK"  )
		UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_BLANK" )
		UIButtons.ChangeText( GUI.rows_id[ count ].score, "GAME_SCOREBOARD_ROW_BLANK" )	
	else
		
		UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_SP_SESSION_POSITION" .. i )
		UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_MINI_SCOREBOARD_ROW_GAMERTAG" .. i )

		if  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPRacesScoreboard then
			UIButtons.ChangeText( GUI.rows_id[ count ].score, "GAME_MINI_SCOREBOARD_ROW_SP_FANS_EARNED" .. i )
		elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPFanDemandsScoreboard then
			UIButtons.ChangeText( GUI.rows_id[ count ].score, "GAME_MINI_SCOREBOARD_ROW_SP_FANS_EARNED" .. i )
		elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPCheckpointsScoreboard then
			UIButtons.ChangeText( GUI.rows_id[ count ].score, "GAME_MINI_SCOREBOARD_ROW_SP_COMPLETION_TIME" .. i )
		elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPDestructionScoreboard then
			UIButtons.ChangeText( GUI.rows_id[ count ].score, "GAME_MINI_SCOREBOARD_ROW_SP_WRECKS" .. i )
		elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPOneOnOneScoreboard then
			UIButtons.ChangeText( GUI.rows_id[ count ].score, "GAME_MINI_SCOREBOARD_ROW_SP_COMPLETION_TIME" .. i )				
		end			

	end

	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )
end

function SpMiniLeaderboard_ColourRow( count, colour )
 	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].score, colour )
end										
				

---------------------------------------------

function SpMiniLeaderboard_StartRead( force_read )
	GUI.reading_me = false
	GUI.no_score_me = false
	
	if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then
		Amax.StartLobbyScoreboardRead( GUI.scoreboard_offset )
	else
		if GUI.scoreboard_type == GUI.scoreboard.friends then
			if force_read == true then
				Amax.StartFriendScoreboardRead( GUI.scoreboard_offset )
			else
				GUI.scoreboard_total_rows = Amax.ScoreboardNumCurrentRows()
				print("Num Rows", GUI.scoreboard_total_rows)
				--GUI.row_me = row_me
				--GUI.rank_me = rank_me )
				SpMiniLeaderboard_Fill( GUI.scoreboard_total_rows, GUI.scoreboard_offset )
			end
			
		elseif GUI.scoreboard_type == GUI.scoreboard.you then
			if force_read == true then
				Amax.StartYouScoreboardRead( GUI.scoreboard_offset )
				GUI.reading_me = true
			else
				Amax.StartScoreboardRead( GUI.scoreboard_offset )
			end
			
		elseif GUI.scoreboard_type == GUI.scoreboard.top then
			Amax.StartScoreboardRead( GUI.scoreboard_offset )
		end
	end
	
	if GUI.scoreboard_type ~= GUI.scoreboard.friends or force_read == true then
		SpMiniLeaderboard_ShowRead( true )
	end
end

function Sp_UpdateOwnerState( node_id, eo_state, event_id )
	local owner_text_id = UIButtons.FindChildByName( node_id, "Owner_Name" )
	
	if eo_state == UIEnums.EventOwnerResult.Losing then
		if IsNumber( owner_text_id ) == true then
			UIButtons.ChangeColour( owner_text_id, "Support_3" )
		end
		
		local gp_map=Profile.GetRemoteGamerPictureMap()
		if gp_map[0] ~= nil then
			UIButtons.SetActive(UIButtons.FindChildByName( node_id, "Owner_AvatarFrame" ), true)
			UIButtons.ChangeTexture( { filename = "REMOTE_GAMERPIC_" .. gp_map[0] }, 1, UIButtons.FindChildByName( node_id, "Owner_AvatarFrame" ) )
		else
			UIButtons.SetActive(UIButtons.FindChildByName( node_id, "Owner_AvatarFrame" ), false)
			
		end
		
	elseif eo_state == UIEnums.EventOwnerResult.Winning then
		if IsNumber( owner_text_id ) == true then
			UIButtons.ChangeColour( owner_text_id, "Support_0" )
		end
		
		UIButtons.SetActive(UIButtons.FindChildByName( node_id, "Owner_AvatarFrame" ), true)
		UIButtons.ChangeTexture( { filename = "GAMERPIC_" .. Profile.GetPrimaryPad() }, 1, UIButtons.FindChildByName( node_id, "Owner_AvatarFrame" ) )
	else
		-- No owner, bail
		return false
	end
	
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "Owner_Name" ), "SPL_EVENT_" .. event_id .. "_OWNER_NAME" )

	return true
end

function Sp_UpdateEventPreview( vista_id, event, amax_event, eventId )

	UIShape.ChangeObjectName( UIButtons.FindChildByName( vista_id, "VP_CityIcon" ), amax_event.city_icon_name )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( vista_id, "VP_EventKindName" ), Sp_KindToText( event.kind ) )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( vista_id, "VP_EventKindIcon" ), Sp_KindToShape( event.kind ) )
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( vista_id, "VP_ClassIcon" ), Sp_ClassToShape( event.carClass ) )
	UIButtons.ChangeText( UIButtons.FindChildByName( vista_id, "VP_ClassName" ), Sp_ClassToRestrictionText( event.carClass ) )

	UIButtons.ChangeText( UIButtons.FindChildByName( vista_id, "VP_CityName" ), amax_event.city_name )
	UIButtons.ChangeText( UIButtons.FindChildByName( vista_id, "VP_RouteName" ), amax_event.route_name )
--	Sp_CentreCityRoute( UIButtons.FindChildByName( vista_id, "VP_CityIcon" ), UIButtons.FindChildByName( vista_id, "VP_CityName" ),  UIButtons.FindChildByName( vista_id,  "VP_RouteName" ) )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( vista_id,  "VP_Laps" ), "SPL_EVENT_" .. eventId .. "_LAPS" )
	
	if Sp_CompletedEventOnHard( event.difficulty ) == true then
		UIButtons.SetActive( UIButtons.FindChildByName( vista_id, "VP_ItsHard" ), true )
	end
end

-- friend demands
function Fd_Get( name )
	return UIButtons.FindChildByName( GUI.fd_panel_id, name )
end

function Fd_CreatePanel( parent, num_options, no_selection, re_challenge )
	-- create panel
	GUI.fd_panel_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "fd_event_box_back_dummy" )
	UIButtons.SetParent( GUI.fd_panel_id, parent, UIEnums.Justify.MiddleCentre )
	
	-- create options
	GUI.fd_node_list_id = Fd_Get( "fd_node_list_options" )
	
	local options = { "fd_title_node", "fd_text_node", "fd_extra_node", "fd_send_node", }
	local options_text = { UIText.CMN_NOWT, UIText.CMN_NOWT, UIText.FDE_EDIT_EXTRA_CHALLENGE, UIText.FDE_SEND_DEMAND, }
	
	if re_challenge == true then
		options = { "fd_title_node", "fd_text_node", "fd_send_node", }
		options_text = { UIText.CMN_NOWT, UIText.CMN_NOWT, UIText.FDE_SEND_DEMAND, }
	end
		
	GUI.option_box_ids = {  }
	GUI.option_text_ids = {  }
	
	for i = 1, num_options, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", options[ i ] )
		local dummy = UIButtons.FindChildByName( node_id, "fd_dummy" )
		
		local size_to_parent = ( i ~= 4 )
		
		if re_challenge == true then
			size_to_parent = ( i ~= 3 )
		end
		
		local box_id, text_id = SetupSelectionBox( dummy, options_text[ i ], nil, nil, UIEnums.Justify.TopCentre, nil, UIEnums.Justify.MiddleCentre, size_to_parent, no_selection )
		
		if re_challenge == true and i == 3 then
			UIButtons.ChangePosition( box_id, nil, -1.3, nil, true )
		end
		
		GUI.option_box_ids[ i ] = box_id
		GUI.option_text_ids[ i ] = text_id		
		
		UIButtons.AddListItem( GUI.fd_node_list_id, node_id, i )
	end

	if re_challenge == true then
		UIButtons.SetNodeItemLocked( GUI.fd_node_list_id, 0, true )
	end
	
	-- create star
	GUI.fd_star_id = Sp_CreateStar( true, 1, "none" )
	UIButtons.SetParent( GUI.fd_star_id, Fd_Get( "fd_list_icon_1" ), UIEnums.Justify.MiddleCentre )
	UIButtons.ChangePanel( GUI.fd_star_id, UIEnums.Panel._3DAA_LIGHT, true )
	UIButtons.ChangeJustification( GUI.fd_star_id, UIEnums.Justify.MiddleCentre )
	UIButtons.ChangeScale( GUI.fd_star_id, 1.75, 1.75 )
end

function Fd_UpdatePanel( fd_index, server, rechallenge )
	local fd_info = FriendDemand.GetInfo( fd_index, server )
	--show_table( fd_info )
	local event_info = SinglePlayer.EventInfo( fd_info.eventid_sp )
	--show_table( event_info )
	local amax_event_info = GameData.GetEvent( fd_info.eventid )
	--show_table( amax_event_info )
	local tier_info = SinglePlayer.TierInfo()[ event_info.tier ]
	--show_table( tier_info )
	local car_info = GameData.GetVehicle( fd_info.vehicleid )
	--show_table( car_info )
	
	UIShape.ChangeObjectName( Fd_Get( "fd_event_race_icon" ), Sp_KindToShape( event_info.kind ) )
	UIShape.ChangeObjectName( Fd_Get( "fd_event_city_icon" ), amax_event_info.city_icon_name )
	
	UIButtons.ChangeText( Fd_Get( "fd_event_city_name" ), amax_event_info.city_name )
	UIButtons.ChangeText( Fd_Get( "fd_event_tier_name" ), tier_info.name )
	UIButtons.ChangeText( Fd_Get( "fd_event_event_name" ), "SPL_EVENT_" .. fd_info.eventid_sp .. "_NAME_NUM_TYPE" )
	
	UIButtons.ChangeText( Fd_Get( "fd_event_difficulty_name" ), UIText[ "CMN_DIFFICULTY_" .. ( fd_info.difficulty + 1 ) .. "_FULL" ] )
	
	
	UIButtons.ChangeText( Fd_Get( "fd_list_info_1" ), "SPL_EVENT_STAR_COUNT_" .. Sp_EventStateToStars( fd_info.state, ( event_info.kind == "boss" ) ) )
	--UIButtons.ChangeText( Fd_Get( "fd_list_info_1" ), "SPL_EVENT_" .. fd_info.eventid_sp .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( fd_info.state ) .. "_" .. fd_info.difficulty )
	
	local car_id = Fd_Get( "fd_event_car_icon" )
	UIShape.ChangeSceneName( car_id, car_info.sheet )
	UIShape.ChangeObjectName( car_id, car_info.icon )
	UIButtons.ChangeText( Fd_Get( "fd_event_car_name" ), car_info.name_full )
	
	-- update star
	SpSetStarTexture( GUI.fd_star_id, true, fd_info.state )
	
	-- update options
	if server == true then
		UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_BLURB_SERVER_MSG" .. fd_index .."_NAME" )
		UIButtons.ChangeText( GUI.option_text_ids[ 2 ], "GAME_BLURB_SERVER_MSG" .. fd_index .."_DESC" )
		UIButtons.ChangeText( Fd_Get( "fd_list_title_2" ), "GAME_FRIEND_DEMAND_MESSAGE_CRITERIA_TITLE_" .. fd_index .. "_0" )
		UIButtons.ChangeText( Fd_Get( "fd_list_info_2" ), "GAME_FRIEND_DEMAND_MESSAGE_CRITERIA_VAR_" .. fd_index .. "_0" )
	else
		if fd_index ~= nil then
			UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_BLURB_NORMAL_MSG" .. fd_index .."_NAME" )
			UIButtons.ChangeText( GUI.option_text_ids[ 2 ], "GAME_BLURB_NORMAL_MSG" .. fd_index .."_DESC" )	
			UIButtons.ChangeText( Fd_Get( "fd_list_title_2" ), "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. fd_index .. "_0" )
			UIButtons.ChangeText( Fd_Get( "fd_list_info_2" ), "GAME_FRIEND_DEMAND_STORE_CRITERIA_VAR_" .. fd_index .. "_0" )
		else
			if rechallenge == true then
				UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_BLURB_NORMAL_MSG" .. UIGlobals.FriendDemandAttemptingIndex .."_NAME" )
			else
				UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_FRIEND_DEMAND_NAME" )
			end
			
			UIButtons.ChangeText( GUI.option_text_ids[ 2 ], "GAME_FRIEND_DEMAND_DESCRIPTION" )
			
			UIButtons.ChangeText( Fd_Get( "fd_list_title_2" ), "GAME_FRIEND_DEMAND_CRITERIA_TITLE_0" )
			UIButtons.ChangeText( Fd_Get( "fd_list_info_2" ), "GAME_FRIEND_DEMAND_CRITERIA_VAR_0" )
		end
	end
	
	Fd_UpdateCriteriaIcon( fd_info, event_info )
	Fd_UpdateExtraChallenge( fd_index, server, fd_info, rechallenge )
end

function POB_Fd_CreatePanel( parent, no_selection )
	-- create panel
	GUI.fd_panel_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "pob_fd_event_box_back_dummy" )
	UIButtons.SetParent( GUI.fd_panel_id, parent, UIEnums.Justify.MiddleCentre )
	
	-- create options
	GUI.fd_node_list_id = Fd_Get( "pob_fd_node_list_options" )
	
	local options = { "fd_title_node", "fd_text_node" }
	local options_text = { UIText.CMN_NOWT, UIText.CMN_NOWT, UIText.FDE_EDIT_EXTRA_CHALLENGE, UIText.FDE_SEND_DEMAND, }
	
	GUI.option_text_ids = {  }
	
	for i = 1, #options, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", options[ i ] )
		local dummy = UIButtons.FindChildByName( node_id, "fd_dummy" )
		
		local size_to_parent = ( i ~= 4 )
		local box_id, text_id = SetupSelectionBox( dummy, options_text[ i ], nil, nil, UIEnums.Justify.TopCentre, nil, UIEnums.Justify.MiddleCentre, size_to_parent, no_selection )
		
		GUI.option_text_ids[ i ] = text_id
		
	end
	
	-- create star
	GUI.fd_star_id = Sp_CreateStar( true, 1, "none" )
	UIButtons.SetParent( GUI.fd_star_id, Fd_Get( "pob_fd_list_icon_1" ), UIEnums.Justify.MiddleCentre )
	UIButtons.ChangePanel( GUI.fd_star_id, UIEnums.Panel._3DAA_LIGHT, true )
	UIButtons.ChangeJustification( GUI.fd_star_id, UIEnums.Justify.MiddleCentre )
	UIButtons.ChangeScale( GUI.fd_star_id, 1.75, 1.75 )
end

function POB_Fd_UpdatePanel( fd_index, server, rechallenge )
	local fd_info = FriendDemand.GetInfo( fd_index, server )
	--show_table( fd_info )
	local event_info = SinglePlayer.EventInfo( fd_info.eventid_sp )
	--show_table( event_info )
	local amax_event_info = GameData.GetEvent( fd_info.eventid )
	--show_table( amax_event_info )
	local tier_info = SinglePlayer.TierInfo()[ event_info.tier ]
	--show_table( tier_info )
	local car_info = GameData.GetVehicle( fd_info.vehicleid )
	--show_table( car_info )
	
	UIShape.ChangeObjectName( Fd_Get( "pob_fd_event_race_icon" ), Sp_KindToShape( event_info.kind ) )
	UIShape.ChangeObjectName( Fd_Get( "pob_fd_event_city_icon" ), amax_event_info.city_icon_name )
	
	UIButtons.ChangeText( Fd_Get( "pob_fd_event_city_name" ), amax_event_info.city_name )
	UIButtons.ChangeText( Fd_Get( "pob_fd_event_tier_name" ), tier_info.name )
	UIButtons.ChangeText( Fd_Get( "pob_fd_event_event_name" ), "SPL_EVENT_" .. fd_info.eventid_sp .. "_NAME_NUM_TYPE" )
	
	UIButtons.ChangeText( Fd_Get( "pob_fd_event_difficulty_name" ), UIText[ "CMN_DIFFICULTY_" .. ( fd_info.difficulty + 1 ) .. "_FULL" ] )
	
	--UIButtons.ChangeText( Fd_Get( "fd_list_info_1" ), "SPL_EVENT_" .. fd_info.eventid_sp .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( fd_info.state ) )
	
	local car_id = Fd_Get( "pob_fd_event_car_icon" )
	UIShape.ChangeSceneName( car_id, car_info.sheet )
	UIShape.ChangeObjectName( car_id, car_info.icon )
	UIButtons.ChangeText( Fd_Get( "pob_fd_event_car_name" ), car_info.name_full )
	
	-- update star
	SpSetStarTexture( GUI.fd_star_id, true, fd_info.state )
	
	-- update options
	if server == true then
		UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_BLURB_SERVER_MSG" .. fd_index .."_NAME" )
		UIButtons.ChangeText( GUI.option_text_ids[ 2 ], "GAME_BLURB_SERVER_MSG" .. fd_index .."_DESC" )
	else
		if fd_index ~= nil then
			UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_BLURB_NORMAL_MSG" .. fd_index .."_NAME" )
			UIButtons.ChangeText( GUI.option_text_ids[ 2 ], "GAME_BLURB_NORMAL_MSG" .. fd_index .."_DESC" )	
			UIButtons.ChangeText( Fd_Get( "pob_fd_list_title_2" ), "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. fd_index .. "_0" )
			UIButtons.ChangeText( Fd_Get( "pob_fd_list_info_2" ), "GAME_FRIEND_DEMAND_STORE_CRITERIA_VAR_" .. fd_index .. "_0" )
		else
			if rechallenge == true then
				UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_BLURB_NORMAL_MSG" .. UIGlobals.FriendDemandAttemptingIndex .."_NAME" )
			else
				UIButtons.ChangeText( GUI.option_text_ids[ 1 ], "GAME_FRIEND_DEMAND_NAME" )
			end
			
			UIButtons.ChangeText( GUI.option_text_ids[ 2 ], "GAME_FRIEND_DEMAND_DESCRIPTION" )
			
			UIButtons.ChangeText( Fd_Get( "pob_fd_list_title_2" ), "GAME_FRIEND_DEMAND_CRITERIA_TITLE_0" )
			UIButtons.ChangeText( Fd_Get( "pob_fd_list_info_2" ), "GAME_FRIEND_DEMAND_CRITERIA_VAR_0" )
		end
	end
	
end

function Fd_GetCriteriaShape( kind )
	local kind_to_shape = 
	{
		race		= "fan",
		destruction = "style_points",
		checkpoint	= "stopwatch",
		fan			= "fan",
		boss		= "stopwatch",
	}
	
	return ( kind_to_shape[ kind ] or --[[ default ]] "square" )
end

function Fd_GetCriteriaIconInfo( kind )
	local kind_to_info = 
	{
		race		= { tex = { filename = "hud_main",				pos = { u = 0.625, v = 0, },	size = { u = 0.125, v = 0.25, }, },		effect = UIEnums.EffectIndex.MultiTexture, },
		destruction	= { tex = { filename = "friend_demand_icons",	pos = { u = 0.875, v = 0, },	size = { u = 0.0625, v = 1, }, },		effect = UIEnums.EffectIndex.IntensityAsAlpha, },
		checkpoint	= { tex = { filename = "hud_main",				pos = { u = 0.625, v = 0.5, },	size = { u = 0.125, v = 0.25, }, },		effect = UIEnums.EffectIndex.MultiTexture, },
		fan			= { tex = { filename = "hud_main",				pos = { u = 0.625, v = 0, },	size = { u = 0.125, v = 0.25, }, },		effect = UIEnums.EffectIndex.MultiTexture, },	-- might need to change if this is revived
		boss		= { tex = { filename = "hud_main",				pos = { u = 0.625, v = 0.5, },	size = { u = 0.125, v = 0.25, }, },		effect = UIEnums.EffectIndex.MultiTexture, },
	}
	
	return ( kind_to_info[ kind ] or --[[ default ]] kind_to_info[ "race" ] )
end

function Fd_UpdateCriteriaIcon( fd_info, event_info )
	local info = Fd_GetCriteriaIconInfo( event_info.kind )
	local icon_id = Fd_Get( "fd_list_icon_2" )
	
	UIButtons.ChangeTexture( info.tex, 0, icon_id )
	UIButtons.ChangeEffectIndex( icon_id, info.effect )
end

function Fd_GetModifierIconInfo( modifier )
	local modifier_to_info = 
	{
		[ UIEnums.FriendDemandModifer.LessThan ]	= { tex = { filename = "friend_demand_icons",	pos = { u = 0, v = 0, },		size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha, },
		[ UIEnums.FriendDemandModifer.GreaterThan ] = { tex = { filename = "friend_demand_icons",	pos = { u = 0.0625, v = 0, },	size = { u = -0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha, },
		[ UIEnums.FriendDemandModifer.Equals ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.0625, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha, },
	}
	
	return ( modifier_to_info[ modifier ] or --[[ default ]] modifier_to_info[ UIEnums.FriendDemandModifer.Equals ] )
end

function Fd_UpdateModifierIcon( fd_info )
	if fd_info == nil then
		fd_info = FriendDemand.GetInfo()
	end
	
	local info = Fd_GetModifierIconInfo( fd_info.extra_modifier )
	local icon_id = Fd_Get( "fd_list_icon_full_modifier_3" )
	
	UIButtons.ChangeTexture( info.tex, 0, icon_id )
	UIButtons.ChangeEffectIndex( icon_id, info.effect )
end

function Fd_GetParamIconInfo( param )
	local param_to_info = 
	{
		[ UIEnums.FriendDemandType.ShuntsHit ]			= { tex = { filename = "friend_demand_icons",	pos = { u = 0.125, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Shunt_Icon", },
		[ UIEnums.FriendDemandType.BargesHit ]			= { tex = { filename = "friend_demand_icons",	pos = { u = 0.1875, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Barge_Icon", },
		[ UIEnums.FriendDemandType.MinesHit ]			= { tex = { filename = "friend_demand_icons",	pos = { u = 0.25, v = 0, },		size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Flash_Icon", },
		[ UIEnums.FriendDemandType.BlindfiresHit ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.3125, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Blindfire_Icon", },
		[ UIEnums.FriendDemandType.NitrousUsed ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.375, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Nitro_Icon", },
		[ UIEnums.FriendDemandType.FanRunScore ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.4375, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_4", },
		[ UIEnums.FriendDemandType.RaceTime ]			= { tex = { filename = "hud_main",				pos = { u = 0.625, v = 0.5, },	size = { u = 0.125, v = 0.25, }, },	effect = UIEnums.EffectIndex.MultiTexture,		colour = "Support_4", },
		[ UIEnums.FriendDemandType.FansGained ]			= { tex = { filename = "hud_main",				pos = { u = 0.625, v = 0, },	size = { u = 0.125, v = 0.25, }, },	effect = UIEnums.EffectIndex.MultiTexture,		colour = "Support_4", },
		[ UIEnums.FriendDemandType.BoltAccuracy ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.5, v = 0, },		size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Blindfire_Icon", },
		[ UIEnums.FriendDemandType.TotalWrecked ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.5625, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_4", },
		[ UIEnums.FriendDemandType.BlueWrecked ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.625, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_1", },
		[ UIEnums.FriendDemandType.RedWrecked ]			= { tex = { filename = "friend_demand_icons",	pos = { u = 0.625, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_3", },
		[ UIEnums.FriendDemandType.YellowWrecked ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.625, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Yellow", },
		[ UIEnums.FriendDemandType.GreenWrecked ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.625, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_0", },
		[ UIEnums.FriendDemandType.StopwatchesUSed ]	= { tex = { filename = "friend_demand_icons",	pos = { u = 0.6875, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_4", },
		[ UIEnums.FriendDemandType.PickupAccuracy ]		= { tex = { filename = "friend_demand_icons",	pos = { u = 0.8125, v = 0, },	size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_4", },
		[ UIEnums.FriendDemandType.PickupsHit ]			= { tex = { filename = "friend_demand_icons",	pos = { u = 0.75, v = 0, },		size = { u = 0.0625, v = 1, }, },	effect = UIEnums.EffectIndex.IntensityAsAlpha,	colour = "Support_4", },
	}
	
	return ( param_to_info[ param ] or --[[ default ]] param_to_info[ UIEnums.FriendDemandType.ShuntsHit ] )
end

function Fd_UpdateParamIcon( fd_info )
	if fd_info == nil then
		fd_info = FriendDemand.GetInfo()
	end
	
	local info = Fd_GetParamIconInfo( fd_info.extra_param )
	local icon_id = Fd_Get( "fd_list_icon_full_3" )
	
	UIButtons.ChangeTexture( info.tex, 0, icon_id )
	UIButtons.ChangeEffectIndex( icon_id, info.effect )
	UIButtons.ChangeColour( icon_id, info.colour )
end

function Fd_UpdateExtraChallenge( fd_index, server, fd_info, rechallenge )
	if fd_index ~= nil then
		local msg_prefix = Select( server, "GAME_FRIEND_DEMAND_MESSAGE", "GAME_FRIEND_DEMAND_STORE" )
		
		if fd_info.extra_modifier ~= UIEnums.FriendDemandModifer.NotUsed then
			UIButtons.SetActive( Fd_Get( "fd_list_icon_blank_3" ), false, true )
			UIButtons.SetActive( Fd_Get( "fd_list_icon_full_3" ), true, true )
			UIButtons.SetActive( Fd_Get( "fd_list_backing_3" ), true )
			
			UIButtons.ChangeText( Fd_Get( "fd_list_title_full_3" ), msg_prefix .. "_PARAM_TITLE_" .. fd_index .. "_0" )
			UIButtons.ChangeText( Fd_Get( "fd_list_info_full_amount_3" ), msg_prefix .. "_PARAM_VAR_" .. fd_index .. "_0" )
			UIButtons.ChangeText( Fd_Get( "fd_list_info_full_modifier_3" ), msg_prefix .. "_PARAM_SWITCH_" .. fd_index .. "_0" )
		else
			UIButtons.SetActive( Fd_Get( "fd_list_icon_blank_3" ), false, true )
			UIButtons.SetActive( Fd_Get( "fd_list_icon_full_3" ), false, true )
			UIButtons.SetActive( Fd_Get( "fd_list_backing_3" ), false )
		end
	else
		if rechallenge == true then
			-- rechallenge specifics
			fd_info = FriendDemand.GetInfo( UIGlobals.FriendDemandAttemptingIndex, false, true )
			
			if fd_info.extra_modifier ~= UIEnums.FriendDemandModifer.NotUsed then
				UIButtons.SetActive( Fd_Get( "fd_list_icon_blank_3" ), false, true )
				UIButtons.SetActive( Fd_Get( "fd_list_icon_full_3" ), true, true )
				UIButtons.SetActive( Fd_Get( "fd_list_backing_3" ), true )
				
				UIButtons.ChangeText( Fd_Get( "fd_list_title_full_3" ), "GAME_FRIEND_DEMAND_RECHALLENGE_SECONDARY_TITLE" )
				UIButtons.ChangeText( Fd_Get( "fd_list_info_full_amount_3" ), "GAME_FRIEND_DEMAND_RECHALLENGE_SECONDARY_VAR" )
				UIButtons.ChangeText( Fd_Get( "fd_list_info_full_modifier_3" ), "GAME_FRIEND_DEMAND_RECHALLENGE_SECONDARY_SWITCH" )
			else
				UIButtons.SetActive( Fd_Get( "fd_list_icon_blank_3" ), false, true )
				UIButtons.SetActive( Fd_Get( "fd_list_icon_full_3" ), false, true )
				UIButtons.SetActive( Fd_Get( "fd_list_backing_3" ), false )
			end			
		else
			if UIGlobals.FriendDemandSwitchChosen > -1 then
				UIButtons.SetActive( Fd_Get( "fd_list_icon_blank_3" ), false, true )
				UIButtons.SetActive( Fd_Get( "fd_list_icon_full_3" ), true, true )
				
				UIButtons.ChangeText( Fd_Get( "fd_list_title_full_3" ), "GAME_FRIEND_DEMAND_PARAM_TITLE_" .. UIGlobals.FriendDemandSwitchChosen )
				UIButtons.ChangeText( Fd_Get( "fd_list_info_full_amount_3" ), "GAME_FRIEND_DEMAND_PARAM_VAR_" .. UIGlobals.FriendDemandSwitchChosen )
				UIButtons.ChangeText( Fd_Get( "fd_list_info_full_modifier_3" ), "GAME_FRIEND_DEMAND_PARAM_SWITCH_" .. UIGlobals.FriendDemandSwitchChosen )
			else
				UIButtons.SetActive( Fd_Get( "fd_list_icon_full_3" ), false, true )
				UIButtons.SetActive( Fd_Get( "fd_list_icon_blank_3" ), true, true )
			end
		end
	end
	
	-- update extra param icons
	Fd_UpdateModifierIcon( fd_info )
	Fd_UpdateParamIcon( fd_info )
	
	-- sort out the background box
	local do_full_box = ( fd_index == nil and server == nil and rechallenge ~= true )
	local do_ui_box = ( do_full_box == false ) and ( fd_info ~= nil and fd_info.extra_modifier ~= UIEnums.FriendDemandModifer.NotUsed )
	local do_ui_box_no = ( do_full_box == false ) and ( fd_info ~= nil and fd_info.extra_modifier == UIEnums.FriendDemandModifer.NotUsed )
	
	UIButtons.SetActive( Fd_Get( "fd_info_box_back_outer" ), do_full_box )
	UIButtons.SetActive( Fd_Get( "fd_info_box_back_inner" ), do_full_box )
	
	UIButtons.SetActive( Fd_Get( "fd_info_box_back_outer_ui" ), do_ui_box )
	UIButtons.SetActive( Fd_Get( "fd_info_box_back_inner_ui" ), do_ui_box )
	
	UIButtons.SetActive( Fd_Get( "fd_info_box_back_outer_ui_no" ), do_ui_box_no )
	UIButtons.SetActive( Fd_Get( "fd_info_box_back_inner_ui_no" ), do_ui_box_no )
end

function Fd_ClearExtraChallenge()
	for i = 1, FriendDemand.GetParamCount(), 1 do
		FriendDemand.ChangeSwitch( ( i - 1 ), 0 )
	end
	
	UIGlobals.FriendDemandSwitchChosen = -1
	
	FriendDemand.Create()
end

function SP_IsFriendDemand()
	return ( Amax.SP_IsStreetRaceFD() == true or Amax.SP_IsDestructionFD() == true or Amax.SP_IsCheckpointFD() == true or Amax.SP_IsFanDemandFD() == true or Amax.SP_IsBossBattleFD() == true )
end

function Sp_CompletedEventOnHard( difficulty )
	if difficulty == "hard" or difficulty == "hardcore" then
		return true
	end
	
	return false
end

function SP_MaxStars( is_boss_race )
	return Select( is_boss_race, 8, 5, 8 )
end

function SpGoLoadingScreen()
	-- Run camera fly out animation and fade to black
	Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
	
	if UIGlobals.DoDrunkCamA ~= true then
		UIGlobals.DoDrunkCamA = true
		Amax.ChangeUiCamera( "Exit1a", 1, 0 )
	else
		UIGlobals.DoDrunkCamA = nil
		Amax.ChangeUiCamera( "Exit1b", 1, 0 )
	end
	
	UIButtons.TimeLineActive( "GoLoading", true )
	UIScreen.SetScreenTimers( 0, 1, UIEnums.Context.Main )
	
	-- Go loading
	StoreScreen( UIEnums.ScreenStorage.FE_RETURN, "SinglePlayer\\SpMain.lua" )
	GoLoadingScreen("Loading\\LoadingSpGame.lua")
end

function SpPopToEventSelect()
	UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_Event

	PopScreen()
	if UIGlobals.Sp.CurrentTierBossUnlocked == true then
		Amax.ChangeEnvironmentSettings( true )
		Amax.ChangeUiCamera( "Sp_2b", UIGlobals.CameraLerpTime, 0 )
	else
		Amax.ChangeUiCamera( "Sp_2", UIGlobals.CameraLerpTime, 0 )
	end
end

-- shared between HUD and MP spectate
function HUD_SpCreateTimer( inter_2d, time_text, player_index )
	local timer = UIButtons.CloneXtGadgetByName( "hud_objects", "SpTimerDummy" )
	UIButtons.SetParent( timer, inter_2d, UIEnums.Justify.MiddleCentre )
	
	local text_id = UIButtons.FindChildByName( timer, "SpTimer" )
	local text_id_green = UIButtons.FindChildByName( timer, "SpTimerGreen" )
	Amax.ChangeText( text_id, time_text )
	Amax.ChangeText( text_id_green, time_text )
	
	GUI.sp.timer_text_id = text_id
	GUI.sp.timer_blob_id = UIButtons.FindChildByName( timer, "SpTimerBlob" )
	GUI.sp.time_red = false
	GUI.sp.time_low = false
	GUI.sp.time_low_next = -1
	GUI.sp.time_low_next_sound = -1
	
	if GUI.is_checkpoint_race == true then
		local light_dummy_id = UIButtons.FindChildByName( timer, "SpTimerLightDummy" )
		GUI.sp.light_timer = HUD_SpCreateLight( light_dummy_id, 0, true )
	end
	
	if IsSplitScreen() == true then
		GUI.ss_timer_ids[ player_index ] = { text = GUI.sp.timer_text_id, blob = GUI.sp.timer_blob_id, }
	end
	
	return timer
end

function HUD_SpTimerDoRed( time_left )
	return ( time_left < 10 )
end

function HUD_SpTimerDoLow( time_left )
	local offset = 0.25
	
	local boundary_start = 10
	local boundary_end = 0
	local drain01 = 1
	
	if GUI.is_checkpoint_race == true then
		local medal_diff = 5
		local boundary_start_new = boundary_start
		local boundary_end_new = boundary_end
		
		if GUI.sp.state == "gold" then
			boundary_end_new = GUI.sp.cp_gold
		elseif GUI.sp.state == "silver" then
			boundary_end_new = GUI.sp.cp_silver
		elseif GUI.sp.state == "bronze" then
			boundary_end_new = GUI.sp.cp_bronze
		end
		
		if time_left < ( boundary_end_new + medal_diff ) then
			drain01 = Clamp( ( time_left - boundary_end_new ) / medal_diff, 0, 1 )
		end
		
		boundary_start_new = boundary_end_new + medal_diff
		
		if boundary_start_new > boundary_start then
			boundary_start = boundary_start_new
			boundary_end = boundary_end_new
		end
	end
	
	local boundary_start_actual = boundary_start + offset
	local boundary_end_actual = boundary_end
	
	local do_low = ( ( time_left <= boundary_start_actual ) and ( time_left > boundary_end_actual ) )
	
	return do_low, boundary_start_actual, offset, drain01
end

function HUD_SpTimerUpdate( time_left )
	if GUI.sp.time_left == nil then
		GUI.sp.time_left = time_left
	end
	
	if time_left ~= GUI.sp.time_left and GUI.sp_time_started ~= true then
		UIButtons.TimeLineActive( "sp_timer_show", true, 0 )
		GUI.sp_time_started = true
		
		if GUI.is_network_race == true or IsSplitScreen() == true then
			UIButtons.TimeLineActive( "mp_timer_show", true, 0 )	-- shift scoring down
		end
	end
	
	local time_low, boundary, offset, drain01 = HUD_SpTimerDoLow( time_left )
	
	if time_low == true and GUI.sp.time_low ~= true then
		GUI.sp.time_low = true
		GUI.sp.time_low_next = boundary
		GUI.sp.time_low_next_sound = boundary - offset
		
	elseif time_low == false and GUI.sp.time_low == true then
		GUI.sp.time_low = false
		GUI.sp.time_low_next = -1
		GUI.sp.time_low_next_sound = -1
	end
	
	local time_red = HUD_SpTimerDoRed( time_left )
	
	if time_red == true and GUI.sp.time_red ~= true then
		GUI.sp.time_red = true
		
		if IsSplitScreen() == true then
			for i = 1, GUI.num_players, 1 do
				UIButtons.ChangeColour( GUI.ss_timer_ids[ i ].text, "Support_3" )
				UIButtons.ChangeColour( GUI.ss_timer_ids[ i ].blob, "Support_3" )
			end
		else
			UIButtons.ChangeColour( GUI.sp.timer_text_id, "Support_3" )
			UIButtons.ChangeColour( GUI.sp.timer_blob_id, "Support_3" )
		end
		
	elseif time_red == false and GUI.sp.time_red == true then
		GUI.sp.time_red = false
		
		if IsSplitScreen() == true then
			for i = 1, GUI.num_players, 1 do
				UIButtons.ChangeColour( GUI.ss_timer_ids[ i ].text, "Main_0" )
				UIButtons.ChangeColour( GUI.ss_timer_ids[ i ].blob, "Main_0" )
			end
		else
			UIButtons.ChangeColour( GUI.sp.timer_text_id, "Main_0" )
			UIButtons.ChangeColour( GUI.sp.timer_blob_id, "Main_0" )
		end
	end
	
	if GUI.sp.time_low == true then
		if time_left > GUI.sp.time_low_next + 1 then
			GUI.sp.time_low_next = math.floor( time_left ) + offset			-- for if time goes back up but still low
			GUI.sp.time_low_next_sound = GUI.sp.time_low_next - offset
		end
		
		if time_left < GUI.sp.time_low_next then
			UIButtons.TimeLineActive( "sp_timer_low", true )
			GUI.sp.time_low_next = GUI.sp.time_low_next - 1
		end
		
		if time_left < GUI.sp.time_low_next_sound then
			if GUI.sp.time_red == true then
				UISystem.PlaySound( Select( ( time_left < 3 ), UIEnums.SoundEffect.CountDown03, UIEnums.SoundEffect.CountDown ) )
			end
			
			while GUI.sp.time_low_next_sound > time_left do
				GUI.sp.time_low_next_sound = GUI.sp.time_low_next_sound - 1
			end
		end
	end
	
	if GUI.is_checkpoint_race == true then
		-- timer light drain
		if drain01 ~= GUI.sp.drain01 then
			local sx, _, sz = UIButtons.GetScale( GUI.sp.light_timer.on )
			UIButtons.ChangeScale( GUI.sp.light_timer.on, sx, drain01, sz )
			UIButtons.ChangeTextureUV( GUI.sp.light_timer.on, 0, nil, ( 1 - drain01 ), nil, drain01 )
			
			GUI.sp.drain01 = drain01
			
			--UISystem.PlaySound( UIEnums.SoundEffect.MedalInThreat )
		end
	end
end
