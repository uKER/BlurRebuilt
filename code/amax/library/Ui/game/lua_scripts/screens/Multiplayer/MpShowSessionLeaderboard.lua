GUI =
{
	finished = false,

	reading_scoreboard = true,
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
	
	num_rows_filled = 0,
	num_rows = 20,
	
	rows_id = {  },
	
	reading_me = false,
	no_score_me = false,
	row_me = 0,
	rank_me = 0,
	
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	PlaySfxGraphicBack()
	
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_3",UIGlobals.CameraLerpTime, 0 )
	
	StoreInfoLine()
	SetupInfoLine()

	GUI.old_friends = net_LockoutFriendsOverlay( true )
	
	MpLeaderboard_SetupTotalFans()
	
	--NEED TO CHANGE THIS
	--local icon_data = 
	--{
	--	[ UIEnums.MpScoreboard.TotalFans ]		= { icon_sheet = "common_icons",	icon_name = "Fan", },
	--	[ UIEnums.MpScoreboard.TotalWins ] 		= { icon_sheet = "common_icons",	icon_name = "points_target", },
	--	[ UIEnums.MpScoreboard.PerkRatio ] 		= { icon_sheet = "fe_icons",		icon_name = "perk_hit", },
	--	[ UIEnums.MpScoreboard.Legends ]		= { icon_sheet = "fe_icons",		icon_name = "Legend", },
	--	[ UIEnums.MpScoreboard.Lobby ]			= { icon_sheet = "fe_icons",		icon_name = "leaderboards", },
	--}
	
	--local type_icon_id = SCUI.name_to_id[ "type_icon" ]
	--UIShape.ChangeSceneName( type_icon_id, icon_data[ UIGlobals.ScoreboardLookingAt ].icon_sheet )
	--UIShape.ChangeObjectName( type_icon_id, icon_data[ UIGlobals.ScoreboardLookingAt ].icon_name )

end

function PostInit()

	local header_id = SetupScreenTitle( UIText.MP_SESSION_LEADERBOARD, SCUI.name_to_id["screen"], "points_target", "common_icons" )
	UIButtons.ChangePanel( header_id, UIEnums.Panel._3DAA_WORLD, true )
	UIButtons.ChangePosition( header_id, 0, 19, 0 )
	UIButtons.ChangeScale( header_id, 0.75, 0.75, 0 )
	
	GUI.bleep_bleep_id = AddTransmitter( -2.5, 0, 3.5, 3.5, UIEnums.Panel._3DAA_0, 3 )
	UIButtons.SetParent( GUI.bleep_bleep_id, SCUI.name_to_id[ "Reading" ], UIEnums.Justify.MiddleLeft )

	if UIGlobals.Multiplayer.InLobby == true then
		UIButtons.ChangePanel( SCUI.name_to_id[ "screen" ], UIEnums.Panel._3DAA_WORLD, true )

		-- Read the leaderboard

		GUI.num_rows_filled = Amax.UpdateSessionLeaderboard()

		UIButtons.SetActive( SCUI.name_to_id[ "darken" ], true )
	end

	for i = 1, GUI.num_rows, 1 do
		UIButtons.SetActive( GUI.rows_id[ i ].dummy, false, true )
		UIButtons.SetNodeItemLocked( SCUI.name_to_id[ "node_list" ], i - 1, true )
		print("Locked Node ", i)
	end
	
	local name_title_id = UIButtons.FindChildByName( SCUI.name_to_id[ "screen" ], "_text_title_gamertag" )
	UIButtons.ChangeText( name_title_id, UIText.MP_SCOREBOARD_NAME )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )

	if message == UIEnums.Message.ButtonDown and UIButtons.GetSelectionIndex( SCUI.name_to_id[ "node_list" ] ) == GUI.num_rows - 1 then
		message = UIEnums.Message.ButtonRightTrigger
	elseif message == UIEnums.Message.ButtonUp and UIButtons.GetSelectionIndex( SCUI.name_to_id[ "node_list" ] ) == 0 then
		message = UIEnums.Message.ButtonLeftTrigger
	end

	if message == UIEnums.Message.MenuNext and UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if MpLeaderboard_CanViewGamerCard() == true then
			if GUI.scoreboard_type ~= GUI.scoreboard.friends then
				NetServices.ViewSessionScoreboardGamerCard( Profile.GetPrimaryPad(), UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] ) )
			else
				NetServices.ViewSessionScoreboardGamerCard( Profile.GetPrimaryPad(), GUI.scoreboard_offset + UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] ) )
			end
		end
		
	elseif message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		PlaySfxGraphicNext()
		
		PopScreen()
	end
end

function FrameUpdate( time )

	if GUI.reading_scoreboard == true then

		complete = true
		rows = 0
		row_me = -1
		rank_me = -1

		GUI.scoreboard_total_rows = rows
		GUI.row_me = row_me
		GUI.rank_me = rank_me

		MpLeaderboard_Fill( GUI.num_rows_filled, 0 )
		
		-- initial selection
		if GUI.scoreboard_type == GUI.scoreboard.you and GUI.rank_me ~= -1 then
			UIButtons.SetSelectionByIndex( SCUI.name_to_id[ "node_list" ], GUI.row_me - 1 )
		else
			UIButtons.SetSelectionByIndex( SCUI.name_to_id[ "node_list" ], 0 )
		end			
	
	end
end

function EnterEnd()
	RestoreInfoLine()
	Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
end

function End()
	net_LockoutFriendsOverlay( GUI.old_friends )
end

function MpLeaderboard_Fill( rows, offset )
	if rows > GUI.num_rows then
		rows = GUI.num_rows
	end
	
	local node_list_id = SCUI.name_to_id[ "node_list" ]
	local count = 0
	local isTeamGame = NetRace.IsTeamGame()
	local colour_me = "Support_4"
	local colour_other = "Support_1"
	local team_a_colour = "team_a"
	local team_b_colour = "team_b"
	
	for i = 1 + offset, rows + offset, 1 do
	
		local is_me = Amax.SessionScoreboardIsMe( i )
	
		count = count + 1

		MpLeaderboard_FillTotalFans( node_list_id, count, i )
		
		if isTeamGame == true then
			local team = Amax.SessionScoreboardTeam( i )
		
			if is_me == true then
				MpLeaderboard_ColourTotalFans( count, colour_me )
			elseif team == 0 then --Team A
				MpLeaderboard_ColourTotalFans( count, team_a_colour )
			else --Team B
				MpLeaderboard_ColourTotalFans( count, team_b_colour )	
			end			
		
		else

			if is_me == true then			
				MpLeaderboard_ColourTotalFans( count, colour_me )
			else
				MpLeaderboard_ColourTotalFans( count, colour_other )			
			end
		end
	end		
	
	MpLeaderboard_ShowRead( false )
	MpLeadboard_SetupInfoLines()
end

function MpLeadboard_SetupInfoLines()

	local options = {  }
	
	if MpLeaderboard_CanViewGamerCard() == true then
		options[ #options + 1 ] = UIText.INFO_VIEW_GAMER_CARD
	end
	
	options[ #options + 1 ] = UIText.INFO_B_BACK
	
	SetupInfoLine( unpack( options ) )
end

function MpLeaderboard_ShowRead( reading )

	local rows = GUI.scoreboard_total_rows

	if GUI.scoreboard_offset + GUI.num_rows > GUI.scoreboard_total_rows then
		rows = GUI.scoreboard_total_rows % GUI.num_rows
	end

	if rows > GUI.num_rows then
		rows = GUI.num_rows
	end

	TransmitterOff( GUI.bleep_bleep_id )

	GUI.reading_scoreboard = false	
end

function MpLeaderboard_SetupTotalFans()
	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowSessionLeaderboard.lua", "_node_total_fans" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "_rank_icon" )
		local text_rank_id = UIButtons.FindChildByName( node_id, "_text_rank" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local text_points_id = UIButtons.FindChildByName( node_id, "_text_points" )
		local text_wins_id = UIButtons.FindChildByName( node_id, "_text_wins" )
		local text_played_id = UIButtons.FindChildByName( node_id, "_text_played" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		UIButtons.SetActive( rank_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )
		
		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, rank_icon = rank_icon_id, rank = text_rank_id, gamertag = text_gamertag_id, points = text_points_id, wins = text_wins_id, played = text_played_id,  }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowSessionLeaderboard.lua", "_node_total_fans_title" )

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function MpLeaderboard_FillTotalFans( node_list_id, count, i )
	UIButtons.ChangeText( GUI.rows_id[ count ].position, "GAME_NUM_" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].rank, "GAME_SCOREBOARD_ROW_SP_SESSION_RANK" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].gamertag, "GAME_SCOREBOARD_ROW_SP_SESSION_GAMERTAG" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].points, "GAME_SCOREBOARD_ROW_SP_SESSION_POINTS" .. i )
	UIButtons.ChangeText( GUI.rows_id[ count ].wins, "GAME_SCOREBOARD_ROW_SP_SESSION_WINS" .. i )	
	UIButtons.ChangeText( GUI.rows_id[ count ].played, "GAME_SCOREBOARD_ROW_SP_SESSION_RACES" .. i )	
	UIButtons.SetNodeItemLocked( node_list_id, count - 1, false )
	print("unLocked Node ", count)
	UIButtons.SetActive( GUI.rows_id[ count ].dummy, true, true )

	Mp_RankIcon( GUI.rows_id[ count ].rank_icon, Amax.SessionScoreboardGetRowRank( i ), Amax.SessionScoreboardGetRowLegend( i ) )

end

function MpLeaderboard_ColourTotalFans( count, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].position, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].rank, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].gamertag, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].points, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].wins, colour )
	UIButtons.ChangeColour( GUI.rows_id[ count ].played, colour )
end

function MpLeaderboard_CanViewGamerCard()
	return ( NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true and GUI.num_rows_filled > 0 and GUI.no_score_me == false )
end
