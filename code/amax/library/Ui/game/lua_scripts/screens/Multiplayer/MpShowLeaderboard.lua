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
		lobby = 1,
		friends = 2,
		top = 3,		
		count = 4,
	},
	
	scoreboard_type = 2,
	
	num_rows = 10,
	
	rows_id = {  },
	
	reading_me = false,
	no_score_me = false,
	row_me = -1,
	rank_me = -1,
	
	server_timeout = 0,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	GUI.old_friends = net_LockoutFriendsOverlay( true )
	
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine()
	
	if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalFans then
		MpLeaderboard_SetupTotalFans()
	elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalWins then
		MpLeaderboard_SetupRacePoints()
	elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.PerkRatio then
		MpLeaderboard_SetupPowerUps()
	elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Legends then
		MpLeaderboard_SetupLegends()
	elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then

		MpLeaderboard_SetupLobby()
	end
	
	local icon_data = 
	{
		[ UIEnums.MpScoreboard.TotalFans ]		= { icon_sheet = "common_icons",	icon_name = "Fan", },
		[ UIEnums.MpScoreboard.TotalWins ] 		= { icon_sheet = "common_icons",	icon_name = "points_target", },
		[ UIEnums.MpScoreboard.PerkRatio ] 		= { icon_sheet = "fe_icons",		icon_name = "perk_hit", },
		[ UIEnums.MpScoreboard.Legends ]		= { icon_sheet = "fe_icons",		icon_name = "Legend", },
		[ UIEnums.MpScoreboard.Lobby ]			= { icon_sheet = "fe_icons",		icon_name = "leaderboards", },
	}

	local title =
	{
		[ UIEnums.MpScoreboard.TotalFans ]		= "GAME_SCOREBOARD_TITLE_FANS_",
		[ UIEnums.MpScoreboard.TotalWins ] 		= "GAME_SCOREBOARD_TITLE_POINTS_",
		[ UIEnums.MpScoreboard.PerkRatio ] 		= "GAME_SCOREBOARD_TITLE_POWERUPS_",
		[ UIEnums.MpScoreboard.Legends ]		= "GAME_SCOREBOARD_TITLE_LEGENDS_",
		[ UIEnums.MpScoreboard.Lobby ]			= "GAME_SCOREBOARD_TITLE_POINTS_",
	}

	-- push in the biggest string so that the box will fit all.
	local root_id = SetupScreenTitle( title[UIGlobals.ScoreboardLookingAt].. "YOU", SCUI.name_to_id["centre"], icon_data[ UIGlobals.ScoreboardLookingAt ].icon_name, icon_data[ UIGlobals.ScoreboardLookingAt ].icon_sheet )
	GUI.title_id = UIButtons.FindChildByName( root_id, "title" )

	-- default type
	if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then
		--THIS WILL PROBABLY STAY THE SAME NOT TOO SURE
		GUI.scoreboard_type = GUI.scoreboard.top
	else
		GUI.scoreboard_type = GUI.scoreboard.friends
	end
end

function PostInit()
	GUI.bleep_bleep_id = AddTransmitter( -2.5, 0, 3.5, 3.5, 3, UIEnums.Panel._3DAA_0 )
	UIButtons.SetParent( GUI.bleep_bleep_id, SCUI.name_to_id[ "Reading" ], UIEnums.Justify.MiddleLeft )

	if UIGlobals.Multiplayer.InLobby == true then
		Amax.ChangeChildrenPanel( SCUI.name_to_id[ "screen" ], 5 )
		UIButtons.ChangePosition( SCUI.name_to_id[ "screen" ], 0, 2, -110 )

		UIButtons.SetActive( SCUI.name_to_id[ "darken" ], true )
	end
	
	MpLeaderboard_StartRead( true )
	
	local name_title_id = UIButtons.FindChildByName( SCUI.name_to_id[ "node_list_title" ], "_text_title_gamertag" )
	UIButtons.ChangeText( name_title_id, UIText.MP_SCOREBOARD_NAME )
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
		GoScreen( "Multiplayer\\MpChooseLeaderboard.lua" )

	elseif message == UIEnums.Message.ButtonX then
		if GUI.reading_scoreboard == false and GUI.read_cooldown < 0 then
			if UIGlobals.ScoreboardLookingAt ~= UIEnums.MpScoreboard.Lobby then
				UISystem.PlaySound( UIEnums.SoundEffect.Filter )
				
				GUI.scoreboard_offset = 0
				GUI.scoreboard_type = GUI.scoreboard_type + 1
				
				-- if we are not in the lobby then we want to skip the lobby filter
				if UIGlobals.Multiplayer.InLobby == false then
					if GUI.scoreboard_type == GUI.scoreboard.lobby then
						GUI.scoreboard_type = GUI.scoreboard_type + 1
					end
				end

				--Dont go over the end of the filter list - go back to the beginning
				if GUI.scoreboard_type == GUI.scoreboard.count then
						GUI.scoreboard_type = GUI.scoreboard.start
				end			

				MpLeaderboard_StartRead( true )
			end
		end

	elseif message == UIEnums.Message.ButtonLeftTrigger then
		if GUI.reading_scoreboard == false and GUI.read_cooldown < 0 and MpLeaderboard_CanPageUp() == true then
			UISystem.PlaySound( UIEnums.SoundEffect.Filter )
			GUI.scoreboard_offset = Clamp( GUI.scoreboard_offset - GUI.num_rows, 0, GUI.scoreboard_total_rows - 1 )
			MpLeaderboard_StartRead()
		end

	elseif message == UIEnums.Message.ButtonRightTrigger then
		if GUI.reading_scoreboard == false and GUI.read_cooldown < 0 and MpLeaderboard_CanPageDown() == true then
			UISystem.PlaySound( UIEnums.SoundEffect.Filter )
			GUI.scoreboard_offset = Clamp( GUI.scoreboard_offset + GUI.num_rows, 0, GUI.scoreboard_total_rows - GUI.num_rows )
			MpLeaderboard_StartRead()
		end
	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		if Amax.CanUseShare() == true then
			if UIGlobals.Multiplayer.InLobby == false then
				if GUI.scoreboard_type == GUI.scoreboard.friends and GUI.rank_me ~= -1 then
					UIGlobals.ShareFromWhatPopup=-1
					SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
				end
			end
		end

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

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpShowLeaderboard.lua" )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then
			
			if pos == GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.LeaderboardPosition, 0, pos)
			elseif pos > GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionAbove, 0, pos)
			elseif pos < GUI.row_me then
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.RelativeLeaderboardPositionBelow, 0, pos)
			end

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpShowLeaderboard.lua" )
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
				if GUI.rank_me ~= -1 and GUI.row_me <= GUI.num_rows then
					if ( GUI.rank_me - (GUI.num_rows / 2) ) < 0 then
						GUI.scoreboard_offset = 0	-- we're on the first page
					elseif ( GUI.rank_me + (GUI.num_rows / 2) ) > GUI.scoreboard_total_rows then
						GUI.scoreboard_offset = GUI.scoreboard_total_rows - GUI.num_rows	-- we're on the last page
					else
						GUI.scoreboard_offset = GUI.rank_me - ((GUI.num_rows / 2) + 1)
					end
				end
			end
			
			MpLeaderboard_Fill( rows, 0, error )
			
			-- initial selection
			if GUI.scoreboard_type == GUI.scoreboard.you and GUI.rank_me ~= -1 then
				UIButtons.SetSelectionByIndex( SCUI.name_to_id[ "node_list" ], GUI.row_me - 1 )
			else
				UIButtons.SetSelectionByIndex( SCUI.name_to_id[ "node_list" ], 0 )
			end
			
			GUI.read_cooldown = 0.5 -- can't start another read within 0.5 seconds - TCR
		end
	else
		GUI.server_timeout = 0
		GUI.read_cooldown = GUI.read_cooldown - time
	end
end

function EnterEnd()
	RestoreInfoLine()
end

function End()
	net_LockoutFriendsOverlay( GUI.old_friends )
end

function MpLeaderboard_Fill( rows, offset, error )
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

					if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalFans then	
						MpLeaderboard_FillTotalFans( node_list_id, count, i )
					elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalWins then
						MpLeaderboard_FillRacePoints( node_list_id, count, i )
					elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.PerkRatio then
						MpLeaderboard_FillPowerUps( node_list_id, count, i )
					elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Legends then
						MpLeaderboard_FillLegends( node_list_id, count, i )
					elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then
						MpLeaderboard_FillLobby( node_list_id, count, i )
					end
				
					local is_me = false
				
					if GUI.scoreboard_type == GUI.scoreboard.friends then
						is_me = ( ( count + GUI.scoreboard_offset ) == GUI.row_me )
					else
						is_me = ( count == GUI.row_me )
					end
					
					local colour_me = "Support_4"
					local colour_other = "Support_1"
				
					if is_me == true then
						if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalFans then	
							MpLeaderboard_ColourTotalFans( count, colour_me )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalWins then
							MpLeaderboard_ColourRacePoints( count, colour_me )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.PerkRatio then
							MpLeaderboard_ColourPowerUps( count, colour_me )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Legends then
							MpLeaderboard_ColourLegends( count, colour_me )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then
							MpLeaderboard_ColourLobby( count, colour_me )
						end
					else
						if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalFans then	
							MpLeaderboard_ColourTotalFans( count, colour_other )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalWins then
							MpLeaderboard_ColourRacePoints( count, colour_other )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.PerkRatio then
							MpLeaderboard_ColourPowerUps( count, colour_other )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Legends then
							MpLeaderboard_ColourLegends( count, colour_other )
						elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then
							MpLeaderboard_ColourLobby( count, colour_other )
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
	
	if MpLeaderboard_CanPageDown() == true then
		options[ #options + 1 ] = UIText.INFO_SCOREBOARD_NEXT_PAGE
	end
	
	if MpLeaderboard_CanPageUp() == true then
		options[ #options + 1 ] = UIText.INFO_SCOREBOARD_PREV_PAGE
	end

	if GUI.scoreboard_type == GUI.scoreboard.friends and UIGlobals.Multiplayer.InLobby == false and Amax.CanUseShare() == true and GUI.rank_me ~= -1 then
		options[ #options + 1 ] = "GAME_SHARE_BUTTON"
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

function MpLeaderboard_SetupTotalFans()
	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_total_fans" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "_rank_icon" )
		local text_rank_id = UIButtons.FindChildByName( node_id, "_text_rank" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_total_fans_id = UIButtons.FindChildByName( node_id, "_text_total_fans" )
		local text_total_time_played_id = UIButtons.FindChildByName( node_id, "_text_total_time_played" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		UIButtons.SetActive( rank_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )
		
		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, rank_icon = rank_icon_id, rank = text_rank_id, gamertag = text_gamertag_id, total_fans = text_total_fans_id, total_time_played = text_total_time_played_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_total_fans_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function MpLeaderboard_FillTotalFans( node_list_id, count, i )
	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].rank, "GAME_SCOREBOARD_ROW_PLAYER_RANK" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].total_fans, "GAME_SCOREBOARD_ROW_TOTAL_FANS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].total_time_played, "GAME_SCOREBOARD_ROW_TOTAL_RACE_TIME" .. i )	
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )
	Mp_RankIcon( GUI.rows_id[ count ].rank_icon, Amax.ScoreboardGetRowRank( i ), Amax.ScoreboardGetRowLegend( i ) )
end

function MpLeaderboard_ColourTotalFans( count, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].rank, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].total_fans, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].total_time_played, colour )
end

function MpLeaderboard_SetupRacePoints()
	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_race_points" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "_rank_icon" )
		local text_rank_id = UIButtons.FindChildByName( node_id, "_text_rank" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_race_points_id = UIButtons.FindChildByName( node_id, "_text_race_points" )
		local text_top_three_id = UIButtons.FindChildByName( node_id, "_text_top_three" )
		local text_total_races_id = UIButtons.FindChildByName( node_id, "_text_total_races" )
		local text_race_wins_id = UIButtons.FindChildByName( node_id, "_text_race_wins" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
		
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		UIButtons.SetActive( rank_icon_id, false )		

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, rank_icon = rank_icon_id, rank = text_rank_id, gamertag = text_gamertag_id, race_points = text_race_points_id, top_three = text_top_three_id, total_races = text_total_races_id, race_wins = text_race_wins_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_race_points_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function MpLeaderboard_FillRacePoints( node_list_id, count, i )
	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].rank, "GAME_SCOREBOARD_ROW_PLAYER_RANK" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].race_points, "GAME_SCOREBOARD_ROW_RACEPOINTS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].top_three, "GAME_SCOREBOARD_ROW_TOPTHREE" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].total_races, "GAME_SCOREBOARD_ROW_TOTALRACES" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].race_wins, "GAME_SCOREBOARD_ROW_RACEWINS" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )	
	Mp_RankIcon( GUI.rows_id[ count ].rank_icon, Amax.ScoreboardGetRowRank( i ), Amax.ScoreboardGetRowLegend( i ) )
end

function MpLeaderboard_ColourRacePoints( count, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].rank, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].race_points, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].top_three, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].total_races, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].race_wins, colour )
end

function MpLeaderboard_SetupPowerUps()
	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_power_up" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "_rank_icon" )
		local text_rank_id = UIButtons.FindChildByName( node_id, "_text_rank" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_powerup_hits_no_id = UIButtons.FindChildByName( node_id, "_text_powerup_hits_no" )
		local text_powerup_shot_no_id = UIButtons.FindChildByName( node_id, "_text_powerup_shot_no" )
		local text_cars_wrecked_no_id = UIButtons.FindChildByName( node_id, "_text_cars_wrecked_no" )
		local text_powerup_hit_ratio_id = UIButtons.FindChildByName( node_id, "_text_powerup_hit_ratio" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		UIButtons.SetActive( rank_icon_id, false )	

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, rank_icon = rank_icon_id, rank = text_rank_id, gamertag = text_gamertag_id, powerup_hits_no =text_powerup_hits_no_id, powerup_shot_no = text_powerup_shot_no_id, cars_wrecked_no = text_cars_wrecked_no_id, powerup_hit_ratio = text_powerup_hit_ratio_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_power_up_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function MpLeaderboard_FillPowerUps( node_list_id, count, i )
	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].rank, "GAME_SCOREBOARD_ROW_PLAYER_RANK" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].powerup_hits_no, "GAME_SCOREBOARD_ROW_POWERUPHITS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].powerup_shot_no, "GAME_SCOREBOARD_ROW_POWERUPSFIRED" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].cars_wrecked_no, "GAME_SCOREBOARD_ROW_CARSWRECKED" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].powerup_hit_ratio, "GAME_SCOREBOARD_ROW_POWERUP_HIT_RATIO" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )
	Mp_RankIcon( GUI.rows_id[ count ].rank_icon, Amax.ScoreboardGetRowRank( i ), Amax.ScoreboardGetRowLegend( i ) )
end

function MpLeaderboard_ColourPowerUps( count, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].rank, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].powerup_hits_no, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].powerup_shot_no, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].cars_wrecked_no, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].powerup_hit_ratio, colour )
end

function MpLeaderboard_SetupLegends()
	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_legends" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "_rank_icon" )
		local text_rank_id = UIButtons.FindChildByName( node_id, "_text_rank" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_legend_level_id = UIButtons.FindChildByName( node_id, "_text_legend_level" )
		local text_time_to_achieve_id = UIButtons.FindChildByName( node_id, "_text_time_to_achieve" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		UIButtons.SetActive( rank_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, rank_icon = rank_icon_id, rank = text_rank_id, gamertag = text_gamertag_id, legend_level = text_legend_level_id, time_to_achieve = text_time_to_achieve_id, }
	end																																				  

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_legends_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function MpLeaderboard_FillLegends( node_list_id, count, i )
	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].rank, "GAME_SCOREBOARD_ROW_PLAYER_RANK" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].legend_level, "GAME_SCOREBOARD_ROW_LEGENDLEVEL" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].time_to_achieve, "GAME_SCOREBOARD_ROW_TIMETOACHIEVELEGEND" .. i )
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )
	Mp_RankIcon( GUI.rows_id[ count ].rank_icon, Amax.ScoreboardGetRowRank( i ), Amax.ScoreboardGetRowLegend( i ) )
end

function MpLeaderboard_ColourLegends( count, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].rank, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].legend_level, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].time_to_achieve, colour )
end

function MpLeaderboard_SetupLobby()
	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_lobby" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "_rank_icon" )
		local text_rank_id = UIButtons.FindChildByName( node_id, "_text_rank" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_race_wins_no_id = UIButtons.FindChildByName( node_id, "_text_race_wins_no" )
		local text_fans_no_id = UIButtons.FindChildByName( node_id, "_text_fans_no" )
		local text_power_up_hits_no_id = UIButtons.FindChildByName( node_id, "_text_power_up_hits_no" )
		local text_total_time_played_id = UIButtons.FindChildByName( node_id, "_text_total_time_played" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		UIButtons.SetActive( rank_icon_id, false )		

		UIButtons.AddListItem( node_list_id, node_id, i )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, rank_icon = rank_icon_id, rank = text_rank_id, gamertag = text_gamertag_id, race_wins_no = text_race_wins_no_id, fans_no =  text_fans_no_id, power_up_hits_no = text_power_up_hits_no_id, total_time_played = text_total_time_played_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowLeaderboard.lua", "_node_lobby_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function MpLeaderboard_FillLobby( node_list_id, count, i )
	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_SCOREBOARD_ROW_POSITION" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].rank, "GAME_SCOREBOARD_ROW_PLAYER_RANK" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].race_wins_no, "GAME_SCOREBOARD_ROW_RACEWINS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].fans_no, "GAME_SCOREBOARD_ROW_TOTAL_FANS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].power_up_hits_no , "GAME_SCOREBOARD_ROW_POWERUPHITS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].total_time_played, "GAME_SCOREBOARD_ROW_TOTAL_RACE_TIME" .. i ) 
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )
	Mp_RankIcon( GUI.rows_id[ count ].rank_icon, Amax.ScoreboardGetRowRank( i ), Amax.ScoreboardGetRowLegend( i ) )
end

function MpLeaderboard_ColourLobby( count, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].rank, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].race_wins_no, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].fans_no, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].power_up_hits_no , colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].total_time_played, colour )
end

function MpLeaderboard_StartRead( force_read )
	GUI.reading_me = false
	GUI.no_score_me = false

	local TitleFinderText = "GAME_SCOREBOARD_TITLE_"

	if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalFans then
		TitleFinderText = TitleFinderText .. "FANS_"
	elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.TotalWins then
		TitleFinderText = TitleFinderText .. "POINTS_"
	elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.PerkRatio then
		TitleFinderText = TitleFinderText .. "POWERUPS_"
	elseif UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Legends then
		TitleFinderText = TitleFinderText .. "LEGENDS_"
	end

	if UIGlobals.ScoreboardLookingAt == UIEnums.MpScoreboard.Lobby then
		Amax.StartLobbyScoreboardRead( GUI.scoreboard_offset )
	else
		if GUI.scoreboard_type == GUI.scoreboard.friends then
			if force_read == true then
				Amax.StartFriendScoreboardRead( GUI.scoreboard_offset )
			else
				MpLeaderboard_Fill( GUI.scoreboard_total_rows, GUI.scoreboard_offset, false )
			end

			TitleFinderText = TitleFinderText .."FRIENDS"			
			
			print("VIEWING FRIENDS LEADEROARD")
		elseif GUI.scoreboard_type == GUI.scoreboard.you then
			if force_read == true then
				Amax.StartYouScoreboardRead( GUI.scoreboard_offset )
				GUI.reading_me = true
			else
				Amax.StartScoreboardRead( GUI.scoreboard_offset )
			end

			TitleFinderText = TitleFinderText .."YOU"
			print("VIEWING YOU LEADEROARD")
			
		elseif GUI.scoreboard_type == GUI.scoreboard.top then
			Amax.StartScoreboardRead( GUI.scoreboard_offset )
			TitleFinderText = TitleFinderText .."TOP"
			print("VIEWING TOP LEADEROARD")

		elseif GUI.scoreboard_type == GUI.scoreboard.lobby then
			Amax.StartLobbyScoreboardRead( GUI.scoreboard_offset )

			TitleFinderText = TitleFinderText .."LOBBY"
			print("VIEWING LOBBY LEADEROARD")

		end

		print("Changing Title of leaderboard")
		UIButtons.ChangeText( GUI.title_id, TitleFinderText )
	
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
	return ( NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true and GUI.scoreboard_total_rows > 0 and GUI.no_score_me == false )
end
