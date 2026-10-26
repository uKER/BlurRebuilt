GUI =
{
	finished = false,

	reading_scoreboard = false,
	read_cooldown = 0,
	
	scoreboard_offset = 0,
	scoreboard_total_rows = 0,
	
	scoreboard = 
	{
		start = 0,
		you = 0,
		friends = 1,
		top = 2,
		count = 3,
	},
	
	scoreboard_type = 1,
	
	num_rows = 10,
	
	rows_id = {  },
	
	reading_me = false,
	no_score_me = false,
	row_me = -1,
	rank_me = -1,
	
	server_timeout = 0,

	CurrentScoreboardType = 0,
	
	Displaying_Lost_Connection = false,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	UIGlobals.SpViewingLeaderboard = true

	AddSCUI_Elements()
	net_SetRichPresence( UIEnums.RichPresence.Leaderboards )

	local sbToLookAt = Amax.CheckScoreboardToLookAt()
	Amax.ScoreboardLookAt( sbToLookAt )
	UIGlobals.ScoreboardLookingAt = sbToLookAt

	StoreInfoLine()
	SetupInfoLine()

	GUI.old_friends = net_LockoutFriendsOverlay( true )

	GUI.CurrentScoreboardType = Amax.CheckSpScoreboardType()

	print("Which is type: ", GUI.CurrentScoreboardType)

	if  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPRacesScoreboard then
		SpLeaderboard_SetupRaces()
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPFanDemandsScoreboard then
		SpLeaderboard_SetupFanDemands()
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPCheckpointsScoreboard then
		SpLeaderboard_SetupCheckpoints()
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPDestructionScoreboard then
		SpLeaderboard_SetupDestruction()
	elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPOneOnOneScoreboard then
		SpLeaderboard_SetupOneOnOne()
	end
	
	local icon_data = 
	{
		[ UIEnums.MpScoreboard.TotalFans ]		= { icon_sheet = "common_icons",	icon_name = "Fan", },
		[ UIEnums.MpScoreboard.TotalWins ] 		= { icon_sheet = "common_icons",	icon_name = "points_target", },
		[ UIEnums.MpScoreboard.PerkRatio ] 		= { icon_sheet = "fe_icons",		icon_name = "perk_hit", },
		[ UIEnums.MpScoreboard.Legends ]		= { icon_sheet = "fe_icons",		icon_name = "Legend", },
		[ UIEnums.MpScoreboard.Lobby ]			= { icon_sheet = "fe_icons",		icon_name = "leaderboards", },
	}
	
	--local type_icon_id = SCUI.name_to_id[ "type_icon" ]
	--UIShape.ChangeSceneName( type_icon_id, icon_data[ 0 ].icon_sheet )
	--UIShape.ChangeObjectName( type_icon_id, icon_data[ 0 ].icon_name )
	
	if UIGlobals.Sp == nil then
		GUI.boss_unlocked = false
	else
		GUI.boss_unlocked = UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].unlocked
	end
	
	-- default type
	GUI.scoreboard_type = GUI.scoreboard.friends
	
end

function PostInit()

	SetScreenPosition()
	
	local name_title_id = UIButtons.FindChildByName( SCUI.name_to_id[ "Root" ], "_text_title_gamertag" )
	UIButtons.ChangeText( name_title_id, UIText.MP_SCOREBOARD_NAME )
	
	if UIGlobals.IsIngame == true then
		UIButtons.SetParent( SCUI.name_to_id[ "Root" ], nil )
		UIButtons.ChangePanel( SCUI.name_to_id[ "Root" ], UIEnums.Panel._3DAA_0, true )
		UIButtons.ChangePosition( SCUI.name_to_id[ "Root" ], 0, 4, -54 )
	end
	

	StartBlockContext()	
		

	GUI.bleep_bleep_id = AddTransmitter( -2.5, 0, 3.5, 3.5, UIEnums.Panel._3DAA_0, 3 )
	UIButtons.SetParent( GUI.bleep_bleep_id, SCUI.name_to_id[ "Reading" ], UIEnums.Justify.MiddleLeft )
	
	SpLeaderboard_StartRead( true )

	if UIGlobals.SpLeaderboardInCarousel == true then	
		--toggle the leaderboard not selected at the beginning
		if UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
			print("leaderboard Should not be selected")
			UIButtons.SetSelected(SCUI.name_to_id[ "node_list" ], false)
		else
			UIButtons.SetSelected(SCUI.name_to_id[ "node_list" ], true)
		end
	else
		SetupScreenTitle( UIText.SP_EVENT_LEADERBOARD, SCUI.name_to_id["ScreenTitle_Dummy"], nil, nil, 4, nil, true, nil, UIEnums.Panel._3DAA_WORLD )			
	end	
	
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.reading_scoreboard == true then
		return
	end
	
	if message == UIEnums.Message.ButtonDown and UIButtons.GetSelectionIndex( SCUI.name_to_id[ "node_list" ] ) == GUI.num_rows - 1 then
		message = UIEnums.Message.ButtonRightTrigger
	elseif message == UIEnums.Message.ButtonUp and UIButtons.GetSelectionIndex( SCUI.name_to_id[ "node_list" ] ) == 0 then
		message = UIEnums.Message.ButtonLeftTrigger
	end

	if message == UIEnums.Message.MenuNext and UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if MpLeaderboard_CanViewGamerCard() == true then
			if GUI.scoreboard_type ~= GUI.scoreboard.friends then
				NetServices.ViewScoreboardGamerCard( Profile.GetPrimaryPad(), UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] ) )
			else
				NetServices.ViewScoreboardGamerCard( Profile.GetPrimaryPad(), GUI.scoreboard_offset + UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] ) )
			end
		end
		
	elseif message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		PlaySfxGraphicNext()
		--print("Leaderboard screen closing")
		PopScreen()
		net_SetRichPresence( UIEnums.RichPresence.SpChoosing )
			
		EndScreenPosition()	
			
	elseif message == UIEnums.Message.ButtonX then
		if GUI.reading_scoreboard == false and GUI.read_cooldown < 0 then
			if UIGlobals.ScoreboardLookingAt ~= UIEnums.MpScoreboard.Lobby then
				UISystem.PlaySound( UIEnums.SoundEffect.Filter )
				
				GUI.scoreboard_offset = 0
				GUI.scoreboard_type = GUI.scoreboard_type + 1

				if GUI.scoreboard_type == GUI.scoreboard.count then
					GUI.scoreboard_type = GUI.scoreboard.start
				end

				SpLeaderboard_StartRead( true )
			end
		end
		
	elseif message == UIEnums.Message.ButtonY then
		
		if UIGlobals.SpLeaderboardInCarousel == true then		
			if UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
				UIButtons.SetSelected(SCUI.name_to_id[ "node_list" ], false)
			else
				UIButtons.SetSelected(SCUI.name_to_id[ "node_list" ], true)
			end
			
			--want to re set the infoline
			MpLeadboard_SetupInfoLines()
			
			PlaySfxToggle()
		end

	elseif message == UIEnums.Message.ButtonLeftTrigger then
		if GUI.reading_scoreboard == false and GUI.read_cooldown < 0 and MpLeaderboard_CanPageUp() == true then
			UISystem.PlaySound( UIEnums.SoundEffect.Filter )
			GUI.scoreboard_offset = Clamp( GUI.scoreboard_offset - GUI.num_rows, 0, GUI.scoreboard_total_rows - 1 )
			SpLeaderboard_StartRead()
		end

	elseif message == UIEnums.Message.ButtonRightTrigger then
		if GUI.reading_scoreboard == false and GUI.read_cooldown < 0 and MpLeaderboard_CanPageDown() == true then
			UISystem.PlaySound( UIEnums.SoundEffect.Filter )
			GUI.scoreboard_offset = Clamp( GUI.scoreboard_offset + GUI.num_rows, 0, GUI.scoreboard_total_rows - GUI.num_rows )
			SpLeaderboard_StartRead()
		end
	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		local show_social=true
		if (UIGlobals.SpLeaderboardInCarousel == true and UIGlobals.SpLeaderboardCarouselFocusOnMenu == true) or GUI.rank_me == -1 then
			show_social=false
		end

		if show_social == true then
			if Amax.CanUseShare() == true then
				if UIGlobals.Multiplayer.InLobby == false then
					if GUI.scoreboard_type == GUI.scoreboard.friends then
						UIGlobals.ShareFromWhatPopup=-1
						SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
					end
				end
			end
		end
				
	elseif message == UIEnums.Message.ButtonRightShoulder and arg2 == true then
		
		local pos=GUI.scoreboard_offset + UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] )
		
		local show_social=true
		if UIGlobals.SpLeaderboardInCarousel == true and UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
			show_social=false
		end

		if show_social == true then
			if GUI.scoreboard_type == GUI.scoreboard.friends and pos ~= GUI.row_me and GUI.scoreboard_total_rows > 0 then
				UISystem.PlaySound( UIEnums.SoundEffect.Choose )
				UIGlobals.CompareToFriendRow=GUI.scoreboard_offset + UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] )
				SetupCustomPopup( UIEnums.CustomPopups.FriendCompare )
			end
		end
		
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.MultiplayerOnlineConnectionLost then
		PopScreen()
			
		EndScreenPosition()	
	
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		local pos=GUI.scoreboard_offset + UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] )

		if GUI.scoreboard_type ~= GUI.scoreboard.friends then
			pos=UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] )
		end

		if arg3 == UIEnums.ShareOptions.Facebook then
			
			if pos == GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.LeaderboardPosition, 1, pos)
			elseif pos > GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionAbove, 1, pos)
			elseif pos < GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionBelow, 1, pos)
			end

			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		elseif arg3 == UIEnums.ShareOptions.Twitter then
			
			if pos == GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.LeaderboardPosition, 0, pos)
			elseif pos > GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionAbove, 0, pos)
			elseif pos < GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionBelow, 0, pos)
			end

			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then
			
			if pos == GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.LeaderboardPosition, 2, pos)
			elseif pos > GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionAbove, 2, pos)
			elseif pos < GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionBelow, 2, pos)
			end
			
		end	
	end
end

function FrameUpdate( time )

	if GUI.Displaying_Lost_Connection == true then
		return
	end

	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
		if GUI.Displaying_Lost_Connection == false then
			SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
			GUI.Displaying_Lost_Connection = true
			return
		end
	end
	
	if GUI.reading_scoreboard == true then
		local complete, rows, row_me, rank_me = Amax.ScoreboardReadComplete()

		GUI.server_timeout = GUI.server_timeout + time

		local error = false
		if GUI.server_timeout > 15 then
			complete = true
			rows = 0
			row_me = -1
			rank_me = -1
			error = true
			SetupCustomPopup( UIEnums.CustomPopups.LeaderboardReadError ) 
		end
		GUI.scoreboard_total_rows = rows
		GUI.row_me = row_me
		GUI.rank_me = rank_me

		if complete == true then
			if GUI.scoreboard_type == GUI.scoreboard.you then
				if GUI.rank_me ~= -1 and GUI.row_me <= GUI.num_rows  then
					if ( GUI.rank_me - (GUI.num_rows / 2) ) < 0 then
						GUI.scoreboard_offset = 0	-- we're on the first page
					elseif ( GUI.rank_me + (GUI.num_rows / 2) ) > GUI.scoreboard_total_rows then
						GUI.scoreboard_offset = GUI.scoreboard_total_rows - GUI.num_rows	-- we're on the last page	
					else
						GUI.scoreboard_offset = GUI.rank_me - ((GUI.num_rows / 2) + 1)
					end
				end
			end
			
			MpLeaderboard_Fill( rows, 0 , error)
			
			-- initial selection
			if UIGlobals.SpLeaderboardInCarousel == true and UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
					UIButtons.SetSelected(SCUI.name_to_id[ "node_list" ], false)
			else			
				if GUI.scoreboard_type == GUI.scoreboard.you and GUI.rank_me ~= -1 then
					UIButtons.SetSelectionByIndex( SCUI.name_to_id[ "node_list" ], GUI.row_me - 1 )
				else
					UIButtons.SetSelectionByIndex( SCUI.name_to_id[ "node_list" ], 0 )
				end		
			end
			
			GUI.read_cooldown = 0.5 -- can't start another read within 0.5 seconds - TCR
		end
	else
		GUI.server_timeout = 0
		GUI.read_cooldown = GUI.read_cooldown - time
	end
	
	MpLeadboard_SetupInfoLines()
end

function EnterEnd()
	RestoreInfoLine()
	UIButtons.TimeLineActive( "Hide_PreRace", false )
end

function End()
	UIGlobals.SpViewingLeaderboard = false
	net_LockoutFriendsOverlay( GUI.old_friends )
	net_SetRichPresence(UIEnums.RichPresence.SpMain)
end

function MpLeaderboard_Fill( rows, offset, error)
	if rows > GUI.num_rows then
		rows = GUI.num_rows
	end
	
	
	-- only if there is no errors
	if error == false then
		if GUI.reading_me == true and GUI.rank_me == -1 then
			GUI.no_score_me = true
		end
		
		if GUI.no_score_me == true then
			SetupCustomPopup( UIEnums.CustomPopups.LeaderboardNoScore )
		else
			if rows == 0 then
				if GUI.scoreboard_type == GUI.scoreboard.top or GUI.scoreboard_type == GUI.scoreboard.you then
					SetupCustomPopup( UIEnums.CustomPopups.LeaderboardNoScore )
				elseif GUI.scoreboard_type == GUI.scoreboard.friends then
					SetupCustomPopup( UIEnums.CustomPopups.LeaderboardNoFriends )
				end
			else
				local node_list_id = SCUI.name_to_id[ "node_list" ]
				local count = 0
				
				for i = 1 + offset, rows + offset, 1 do
					count = count + 1

					if  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPRacesScoreboard then
						SpLeaderboard_FillRaces( node_list_id, count, i )
					elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPFanDemandsScoreboard then
						SpLeaderboard_FillFanDemands( node_list_id, count, i )
					elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPCheckpointsScoreboard then
						SpLeaderboard_FillCheckpoints( node_list_id, count, i )
					elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPDestructionScoreboard then
						SpLeaderboard_FillDestruction( node_list_id, count, i )
					elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPOneOnOneScoreboard then
						SpLeaderboard_FillOneOnOne( node_list_id, count, i )
					end						
				
					local is_me = false
				
					if GUI.scoreboard_type == GUI.scoreboard.friends then
						is_me = ( ( count + GUI.scoreboard_offset ) == GUI.row_me )
					else
						is_me = ( count == GUI.row_me )
					end
					
					local colour_me = "Support_4"
					local colour_other = "Main_1"
				
					if is_me == true then
						if  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPRacesScoreboard then
							SpLeaderboard_ColourRaces( count, colour_me )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPFanDemandsScoreboard then
							SpLeaderboard_ColourFanDemands( count, colour_me )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPCheckpointsScoreboard then
							SpLeaderboard_ColourCheckpoints( count, colour_me )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPDestructionScoreboard then
							SpLeaderboard_ColourDestruction( count, colour_me )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPOneOnOneScoreboard then
							SpLeaderboard_ColourOneOnOne( count, colour_me )
						end 
					else
						if  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPRacesScoreboard then
							SpLeaderboard_ColourRaces( count, colour_other )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPFanDemandsScoreboard then
							SpLeaderboard_ColourFanDemands( count, colour_other )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPCheckpointsScoreboard then
							SpLeaderboard_ColourCheckpoints( count, colour_other )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPDestructionScoreboard then
							SpLeaderboard_ColourDestruction( count, colour_other )
						elseif  GUI.CurrentScoreboardType == UIEnums.SPScoreboardTypes.SPOneOnOneScoreboard then
							SpLeaderboard_ColourOneOnOne( count, colour_other )
						end			
					end
				end	
			end
		end
	end
	
	MpLeaderboard_ShowRead( false )
	MpLeadboard_SetupInfoLines()
end

function MpLeadboard_SetupInfoLines()
	local x_text = UIText.INFO_X_SHOW_YOU
	
	if GUI.scoreboard_type == GUI.scoreboard.friends then
		x_text = UIText.INFO_X_SHOW_TOP
	elseif GUI.scoreboard_type == GUI.scoreboard.you then
		x_text = UIText.INFO_X_SHOW_FRIENDS
	end
	
	local options = {  }
	
	if MpLeaderboard_CanViewGamerCard() == true then
		options[ #options + 1 ] = UIText.INFO_VIEW_GAMER_CARD
	end
	
	options[ #options + 1 ] = UIText.INFO_B_BACK
		
	if UIGlobals.ScoreboardLookingAt ~= UIEnums.MpScoreboard.Lobby then
		options[ #options + 1 ] = x_text
	end
	
	if UIGlobals.SpLeaderboardInCarousel == true then
		options[ #options + 1 ] = UIText.INFO_SPLEADERBOARD_TOGGLE
	end
	
	local up = MpLeaderboard_CanPageUp()
	local dn = MpLeaderboard_CanPageDown()
	
	if up == true and dn == true then
		options[ #options + 1 ] = UIText.INFO_PAGE_LT_RT
	elseif up == true then
		options[ #options + 1 ] = UIText.INFO_SCOREBOARD_PREV_PAGE
	elseif dn == true then
		options[ #options + 1 ] = UIText.INFO_SCOREBOARD_NEXT_PAGE
	end

	local show_social=true
	if UIGlobals.SpLeaderboardInCarousel == true and UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
		show_social=false
	end
	
	if show_social == true then
		if GUI.scoreboard_type == GUI.scoreboard.friends and UIGlobals.Multiplayer.InLobby == false and Amax.CanUseShare() == true and GUI.rank_me ~= -1 then
			options[ #options + 1 ] = "GAME_SHARE_BUTTON"
		end

		local pos=GUI.scoreboard_offset + UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] )
		if GUI.scoreboard_type == GUI.scoreboard.friends and pos ~= GUI.row_me and GUI.scoreboard_total_rows > 0 then
			options[ #options + 1 ] = UIText.INFO_SCOREBOARD_CHOOSE_COMPARE
		end
	end

	SetupInfoLine( unpack( options ) )
end

function MpLeaderboard_ShowRead( reading )
	local reading_id = SCUI.name_to_id[ "Reading" ]
	local size_id = SCUI.name_to_id[ "ScoreboardSize" ]
	local total_icon_id = SCUI.name_to_id[ "total_players_icon" ]

	local rows = GUI.scoreboard_total_rows

	if GUI.scoreboard_offset + GUI.num_rows > GUI.scoreboard_total_rows then
		rows = GUI.scoreboard_total_rows % GUI.num_rows
	end

	if rows > GUI.num_rows then
		rows = GUI.num_rows
	end
	
	if reading == true then
		UIButtons.SetActive( reading_id, true )
		UIButtons.SetActive( size_id, false )
		UIButtons.SetActive( total_icon_id, false )
		
		for i = 1, GUI.num_rows, 1 do
			UIButtons.SetActive( GUI.rows_id[ i ].dummy, false, true )
			UIButtons.SetNodeItemLocked( SCUI.name_to_id[ "node_list" ], i - 1, true )
		end
		
		SetupInfoLine()
	else
		UIButtons.SetActive( reading_id, false )
		UIButtons.SetActive( size_id, true )
		UIButtons.SetActive( total_icon_id, true )
	end
	
	if reading == true then
		TransmitterOn( GUI.bleep_bleep_id )
	else
		TransmitterOff( GUI.bleep_bleep_id )
	end
	
	GUI.reading_scoreboard = reading
end

-------------------------------------------

function SpLeaderboard_SetupRaces()


	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_races" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		--local vehicle_icon_id	= UIButtons.FindChildByName( node_id, "_vehicle_icon" )
		local text_vehicle_id = UIButtons.FindChildByName( node_id, "_text_vehicle" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_fans_earned_id = UIButtons.FindChildByName( node_id, "_text_fans_earned" )
		local text_lap_time_id = UIButtons.FindChildByName( node_id, "_text_lap_time" )
		local text_comp_time_id = UIButtons.FindChildByName( node_id, "_text_comp_time" )
		local text_difficulty_id = UIButtons.FindChildByName( node_id, "_text_difficulty" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
		
		
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		--UIButtons.SetActive( vehicle_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )
		
		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, gamertag = text_gamertag_id, vehicle = text_vehicle_id, fans_earned = text_fans_earned_id, lap_time = text_lap_time_id, comp_time = text_comp_time_id, difficulty = text_difficulty_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_races_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function SpLeaderboard_FillRaces( node_list_id, count, i )

	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].vehicle, "GAME_SCOREBOARD_ROW_SP_VEHICLE_ID" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].fans_earned, "GAME_SCOREBOARD_ROW_SP_FANS_EARNED" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].lap_time, "GAME_SCOREBOARD_ROW_SP_LAP_TIME" .. i )	
	UIButtons.ChangeText( GUI.rows_id[ count ].comp_time, "GAME_SCOREBOARD_ROW_SP_COMPLETION_TIME" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].difficulty, "GAME_SCOREBOARD_ROW_SP_DIFFICULTY_LEVEL" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )

	local vehicleId = Amax.GetLeaderboardVehicleId(i)
	--UIShape.ChangeObjectName( GUI.rows_id[ count ].vehicle_icon, GameData.GetVehicle(vehicleId).icon )
end

function SpLeaderboard_ColourRaces( count, colour )
 
	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].vehicle, "Support_2" )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].fans_earned, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].lap_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].comp_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].difficulty, colour )
end								

---------------------------------------------

function SpLeaderboard_SetupFanDemands()


	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_fan_demands" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		--local vehicle_icon_id	= UIButtons.FindChildByName( node_id, "_vehicle_icon" )
		local text_vehicle_id = UIButtons.FindChildByName( node_id, "_text_vehicle" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_fans_earned_id = UIButtons.FindChildByName( node_id, "_text_fans_earned" )
		local text_comp_time_id = UIButtons.FindChildByName( node_id, "_text_comp_time" )
		local text_wrecks_id = UIButtons.FindChildByName( node_id, "_text_wrecks" )
		local text_difficulty_id = UIButtons.FindChildByName( node_id, "_text_difficulty" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		--UIButtons.SetActive( vehicle_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, gamertag = text_gamertag_id, vehicle = text_vehicle_id, fans_earned = text_fans_earned_id,  comp_time = text_comp_time_id, wrecks = text_wrecks_id, difficulty = text_difficulty_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_fan_demands_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function SpLeaderboard_FillFanDemands( node_list_id, count, i )

	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].vehicle, "GAME_SCOREBOARD_ROW_SP_VEHICLE_ID" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].fans_earned, "GAME_SCOREBOARD_ROW_SP_FANS_EARNED" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].comp_time, "GAME_SCOREBOARD_ROW_SP_COMPLETION_TIME" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].wrecks, "GAME_SCOREBOARD_ROW_SP_WRECKS" .. i )	
	UIButtons.ChangeText( GUI.rows_id[ count ].difficulty, "GAME_SCOREBOARD_ROW_SP_DIFFICULTY_LEVEL" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )
	
	local vehicleId = Amax.GetLeaderboardVehicleId(i)
	--UIShape.ChangeObjectName( GUI.rows_id[ count ].vehicle_icon, GameData.GetVehicle(vehicleId).icon )
end

function SpLeaderboard_ColourFanDemands( count, colour )

	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].vehicle, "Support_2" )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].fans_earned, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].comp_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].wrecks, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].difficulty, colour )
end										

---------------------------------------------

function SpLeaderboard_SetupCheckpoints()


	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_checkpoints" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		--local vehicle_icon_id	= UIButtons.FindChildByName( node_id, "_vehicle_icon" )
		local text_vehicle_id = UIButtons.FindChildByName( node_id, "_text_vehicle" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )	
		local text_comp_time_id = UIButtons.FindChildByName( node_id, "_text_comp_time" )		
		local text_lap_time_id = UIButtons.FindChildByName( node_id, "_text_lap_time" )
		local text_fans_earned_id = UIButtons.FindChildByName( node_id, "_text_fans_earned" )
		local text_difficulty_id = UIButtons.FindChildByName( node_id, "_text_difficulty" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		--UIButtons.SetActive( vehicle_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, gamertag = text_gamertag_id, vehicle = text_vehicle_id, comp_time = text_comp_time_id, lap_time = text_lap_time_id, fans_earned = text_fans_earned_id, difficulty = text_difficulty_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_checkpoints_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function SpLeaderboard_FillCheckpoints( node_list_id, count, i )

	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].vehicle, "GAME_SCOREBOARD_ROW_SP_VEHICLE_ID" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].comp_time, "GAME_SCOREBOARD_ROW_SP_COMPLETION_TIME" .. i )	
	UIButtons.ChangeText( GUI.rows_id[ count ].lap_time, "GAME_SCOREBOARD_ROW_SP_LAP_TIME" .. i )	
	UIButtons.ChangeText( GUI.rows_id[ count ].fans_earned, "GAME_SCOREBOARD_ROW_SP_FANS_EARNED" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].difficulty, "GAME_SCOREBOARD_ROW_SP_DIFFICULTY_LEVEL" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )
	
	local vehicleId = Amax.GetLeaderboardVehicleId(i)
	--UIShape.ChangeObjectName( GUI.rows_id[ count ].vehicle_icon, GameData.GetVehicle(vehicleId).icon )
end

function SpLeaderboard_ColourCheckpoints( count, colour )

	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].vehicle, "Support_2" )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].comp_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].lap_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].fans_earned, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].difficulty, colour )
end										

---------------------------------------------

function SpLeaderboard_SetupDestruction()


	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_destruction" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		--local vehicle_icon_id	= UIButtons.FindChildByName( node_id, "_vehicle_icon" )
		local text_vehicle_id = UIButtons.FindChildByName( node_id, "_text_vehicle" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_wrecks_id = UIButtons.FindChildByName( node_id, "_text_wrecks" )
		local text_lap_time_id = UIButtons.FindChildByName( node_id, "_text_lap_time" )
		local text_comp_time_id = UIButtons.FindChildByName( node_id, "_text_comp_time" )
		local text_difficulty_id = UIButtons.FindChildByName( node_id, "_text_difficulty" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		--UIButtons.SetActive( vehicle_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, gamertag = text_gamertag_id, vehicle = text_vehicle_id, wrecks = text_wrecks_id, lap_time = text_lap_time_id, comp_time = text_comp_time_id, difficulty = text_difficulty_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_destruction_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function SpLeaderboard_FillDestruction( node_list_id, count, i )

	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].vehicle, "GAME_SCOREBOARD_ROW_SP_VEHICLE_ID" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].wrecks, "GAME_SCOREBOARD_ROW_SP_WRECKS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].lap_time, "GAME_SCOREBOARD_ROW_SP_FANS_EARNED" .. i )	
	UIButtons.ChangeText( GUI.rows_id[ count ].comp_time, "GAME_SCOREBOARD_ROW_SP_COMPLETION_TIME" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].difficulty, "GAME_SCOREBOARD_ROW_SP_DIFFICULTY_LEVEL" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )

	local vehicleId = Amax.GetLeaderboardVehicleId(i)
	--UIShape.ChangeObjectName( GUI.rows_id[ count ].vehicle_icon, GameData.GetVehicle(vehicleId).icon )
end

function SpLeaderboard_ColourDestruction( count, colour )

	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].vehicle, "Support_2" )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].wrecks, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].lap_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].comp_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].difficulty, colour )
end										

---------------------------------------------

function SpLeaderboard_SetupOneOnOne()


	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_one_on_one" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		--local vehicle_icon_id	= UIButtons.FindChildByName( node_id, "_vehicle_icon" )
		local text_vehicle_id = UIButtons.FindChildByName( node_id, "_text_vehicle" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_comp_time_id = UIButtons.FindChildByName( node_id, "_text_comp_time" )
		local text_fans_earned_id = UIButtons.FindChildByName( node_id, "_text_fans_earned" )
		local text_difficulty_id = UIButtons.FindChildByName( node_id, "_text_difficulty" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		--UIButtons.SetActive( vehicle_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, gamertag = text_gamertag_id, vehicle = text_vehicle_id, comp_time = text_comp_time_id, fans_earned = text_fans_earned_id, difficulty = text_difficulty_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpShowLeaderboard.lua", "_node_one_on_one_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function SpLeaderboard_FillOneOnOne( node_list_id, count, i )

	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].vehicle, "GAME_SCOREBOARD_ROW_SP_VEHICLE_ID" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].comp_time, "GAME_SCOREBOARD_ROW_SP_COMPLETION_TIME" .. i )	
	UIButtons.ChangeText( GUI.rows_id[ count ].fans_earned, "GAME_SCOREBOARD_ROW_SP_FANS_EARNED" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].difficulty, "GAME_SCOREBOARD_ROW_SP_DIFFICULTY_LEVEL" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )

	local vehicleId = Amax.GetLeaderboardVehicleId(i)
	--UIShape.ChangeObjectName( GUI.rows_id[ count ].vehicle_icon, GameData.GetVehicle(vehicleId).icon )
end

function SpLeaderboard_ColourOneOnOne( count, colour )

	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].vehicle, "Support_2" )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].comp_time, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].fans_earned, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].difficulty, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].comp_time, colour )
end										

---------------------------------------------

function SpLeaderboard_StartRead( force_read )
	GUI.reading_me = false
	GUI.no_score_me = false
	
	if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then
		Amax.StartLobbyScoreboardRead( GUI.scoreboard_offset )
	else
		if GUI.scoreboard_type == GUI.scoreboard.friends then
			if force_read == true then
				Amax.StartFriendScoreboardRead( GUI.scoreboard_offset )
			else
				MpLeaderboard_Fill( GUI.scoreboard_total_rows, GUI.scoreboard_offset, false )
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
		MpLeaderboard_ShowRead( true )
	end
end

function MpLeaderboard_CanPageUp()
	return ( GUI.scoreboard_offset ~= 0 and GUI.no_score_me == false )
end

function MpLeaderboard_CanPageDown()
	return ( GUI.scoreboard_offset <= ( GUI.scoreboard_total_rows - GUI.num_rows - 1 ) and GUI.no_score_me == false )
end

function MpLeaderboard_CanViewGamerCard()

	if UIGlobals.SpLeaderboardInCarousel == true then	
		if UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
			return false
		end
	end
	
	return ( NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true and GUI.scoreboard_total_rows > 0 and GUI.no_score_me == false )
end

function SetScreenPosition()
	if UIGlobals.SpLeaderboardInCarousel == false then
		UIButtons.TimeLineActive( "Hide_PreRace", true )
	end
	
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
end

function EndScreenPosition()

	UIButtons.TimeLineActive( "end_alpha", false )
	
	if UIGlobals.SpLeaderboardInCarousel == true then
		
	else
		Amax.ChangeUiCamera( "Sp_1", UIGlobals.CameraLerpTime, 0 )
	end
	
end

function StartBlockContext()

	if UIGlobals.SpLeaderboardInCarousel == true then

	else
		UIScreen.BlockInputToContext( true )
	end

end

