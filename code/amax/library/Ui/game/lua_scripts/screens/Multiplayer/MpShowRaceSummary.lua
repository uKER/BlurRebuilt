GUI =
{
	finished = false,

	num_rows_filled = 0,
	num_rows = 20,
	
	rows_id = {  },
	
	row_me = 0,
	rank_me = 0,

	current_screen_id = 1,

	Progression = 0,

	race_summary = { },
	
	race_results = { },
	
	unlock_nodes = { },
	upcoming_unlock_nodes = {  },
	
	unlock_titles =
	{
		["car"]		= UIText.MP_RACESUMMARY_UNLOCK_TYPE_CAR,
		["pack"]	= UIText.MP_RACESUMMARY_UNLOCK_TYPE_PACK,
		["mod"]		= UIText.MP_RACESUMMARY_UNLOCK_TYPE_MOD,
		["feature"]	= UIText.MP_RACESUMMARY_UNLOCK_TYPE_FEATURE,
	},
	
	unlock_node_size_x	= 0,
	unlock_start_x		= -14,
	unlock_start_y		= -1,
	unlock_time_offset	= 0.1,
	UnlockItemTime	= 1,	-- time per item
	UnlockTime		= 3.5,	-- total time

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	PlaySfxGraphicBack()

	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
	
	StoreInfoLine()
	RaceSummary_SetupInfoLines()
		
	SetupRaceSummaryResults()
	
	SetupRivalAndPrey()
	

	-- Set progression and rank icon
	local pad_index = Profile.GetPrimaryPad()
	GUI.Progression	= Multiplayer.ProfileProgression()

	Mp_RankIcon( SCUI.name_to_id[ "player_rank" ], GUI.Progression.level - 1, GUI.Progression.legend )
	
	-- calculate size
	local size_x, size_y, size_z = UIButtons.GetSize( SCUI.name_to_id[ "backing" ] )
	
	-- progress bar data
	GUI.Progression		= Multiplayer.ProfileProgression()
	local level_data	= Multiplayer.LevelProgressionData( GUI.Progression.level )
	
	local previous		= level_data.fans_previous
	local target_fans	= level_data.fans_needed

	local range = target_fans - previous
	local amount = 1
	
	if GUI.Progression.maxed == false then
		amount = ( GUI.Progression.fans - previous ) / range
	end

	UIButtons.ChangeSize( SCUI.name_to_id[ "progress" ], size_x * amount, size_y, size_z )

	GUI.race_summary = Multiplayer.GetRaceSummary()
	
	GUI.race_results = Multiplayer.GetLobbyRaceResults()
	
end

function PostInit()	

	SetupScreenTitle( UIText.MP_RACE_SUMMERY, SCUI.name_to_id["ScreenTitle_Dummy"], nil, nil, nil, nil, true, nil, UIEnums.Panel._3DAA_WORLD )
	
	UIButtons.TimeLineActive( "Hide_Lobby", true )
		
	UIButtons.TimeLineActive( "show_left_arrow", false )
	UIButtons.SetActive( SCUI.name_to_id[ "Left_Arrow" ], false )
	
	if NetRace.IsPrivateGame() == true then
		UIButtons.SetActive( SCUI.name_to_id[ "Right_Arrow" ], false )
		UIButtons.TimeLineActive( "show_right_arrow", false )
	else 
		UIButtons.TimeLineActive( "show_right_arrow", true )
	end

	UIButtons.SetActive( SCUI.name_to_id[ "darken" ], true )

	--Fan Score Stuff
	UIButtons.ChangeText( SCUI.name_to_id[ "on_track_fans_text" ], "MPL_STATS_ON_TRACK_FANS" )
	UIButtons.ChangeText( SCUI.name_to_id[ "on_finish_bonus_position_text" ], "MPL_STATS_POSITION_FINISH_BONUS" )	
	UIButtons.ChangeText( SCUI.name_to_id[ "on_finish_bonus_text" ], "MPL_STATS_FINISH_BONUS" )
	UIButtons.ChangeText( SCUI.name_to_id[ "fan_total_text" ], "MPL_STATS_TOTAL_FANS" )

	local numFanBonuses = 0
	
	if GUI.race_summary.showy_flourish_fans > 0 then	
		numFanBonuses = numFanBonuses + 1
		UIButtons.SetActive( SCUI.name_to_id["fan_multipliers_text"], false )
		UIButtons.SetActive( SCUI.name_to_id["showy_flourish"], true )
		
		local px, py, pz = UIButtons.GetPosition( SCUI.name_to_id["showy_flourish"] )
		UIButtons.ChangePosition( SCUI.name_to_id["showy_flourish"], px, (py * numFanBonuses), pz )	
		UIButtons.ChangeText( SCUI.name_to_id["showy_flourish_amount"], "GAME_NUM_" .. GUI.race_summary.showy_flourish_fans )
		
		local length = UIButtons.GetStaticTextLength(SCUI.name_to_id["showy_flourish_text"], true)
		length = length + 1
		length = length + UIButtons.GetStaticTextLength(SCUI.name_to_id["showy_flourish_amount"])
		
		UIButtons.ChangePosition( SCUI.name_to_id["showy_flourish_text"], (-length * 0.5), nil, nil )	
		
	end
	
	if GUI.race_summary.fan_favourite_fans > 0 then
		numFanBonuses = numFanBonuses + 1
		UIButtons.SetActive( SCUI.name_to_id["fan_multipliers_text"], false )
		UIButtons.SetActive( SCUI.name_to_id["fan_favourite"], true )
		
		local px, py, pz = UIButtons.GetPosition( SCUI.name_to_id["fan_favourite"] )
		UIButtons.ChangePosition( SCUI.name_to_id["fan_favourite"], px, (py * numFanBonuses), pz )	
		UIButtons.ChangeText( SCUI.name_to_id["fan_favourite_amount"], "GAME_NUM_" .. GUI.race_summary.fan_favourite_fans )
		
		local length = UIButtons.GetStaticTextLength(SCUI.name_to_id["fan_favourite_text"], true)
		length = length + 1
		length = length + UIButtons.GetStaticTextLength(SCUI.name_to_id["fan_favourite_amount"])
		
		UIButtons.ChangePosition( SCUI.name_to_id["fan_favourite_text"], (-length * 0.5), nil, nil )
	end
	
	if GUI.race_summary.car_reward_fan_bonus > 0 then
		numFanBonuses = numFanBonuses + 1
		UIButtons.SetActive( SCUI.name_to_id["fan_multipliers_text"], false )
		UIButtons.SetActive( SCUI.name_to_id["car_bonus"], true )
		
		local px, py, pz = UIButtons.GetPosition( SCUI.name_to_id["car_bonus"] )
		UIButtons.ChangePosition( SCUI.name_to_id["car_bonus"], px, (py * numFanBonuses), pz )	
		UIButtons.ChangeText( SCUI.name_to_id["car_bonus_amount"], "GAME_NUM_" .. GUI.race_summary.car_reward_fan_bonus )
		
		local length = UIButtons.GetStaticTextLength(SCUI.name_to_id["car_bonus_text"], true)
		length = length + 1
		length = length + UIButtons.GetStaticTextLength(SCUI.name_to_id["car_bonus_amount"])
		
		UIButtons.ChangePosition( SCUI.name_to_id["car_bonus_text"], (-length * 0.5), nil, nil )
		
	end
	
	local numCompleteChallenges = Multiplayer.GetRaceChallengesComplete()	
	
	local numChallengesadded = 0
	
	if numCompleteChallenges > 0 then
	GUI.CompletedChallengeFans = Multiplayer.GetRaceChallengesCompleteFans()
	
	UIButtons.SetActive( SCUI.name_to_id["challenges_bonus_text"], false )
	
		if numCompleteChallenges > 3 then
				numChallengesadded = numChallengesadded +1
				UIButtons.SetActive( SCUI.name_to_id["challenge_bonus_multiple"], true )
		
				local px, py, pz = UIButtons.GetPosition( SCUI.name_to_id["challenge_bonus_multiple"] )
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_multiple"], px, (py * numChallengesadded), pz )	
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_multiple_amount"], "GAME_NUM_" .. GUI.CompletedChallengeFans.Total_Fans )
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_multiple_text"], "GAME_LOBBY_RACESUMMARY_CHALLENGE_MULTIPLE" .. numCompleteChallenges )	
				
				local length = UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_multiple_text"])				
				local sx, sy, sz = UIButtons.GetSize( SCUI.name_to_id["challenge_bonus_multiple_text"] )
				
				if length > sx then
					length = sx
				end			
					
				length = length + 1				
				length = length + UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_multiple_amount"])
				
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_multiple_text"], (-length * 0.5), nil, nil )	
					
		else
			if numCompleteChallenges > 0 then
				numChallengesadded = numChallengesadded +1
				UIButtons.SetActive( SCUI.name_to_id["challenge_bonus_I"], true )
		
				local px, py, pz = UIButtons.GetPosition( SCUI.name_to_id["challenge_bonus_I"] )
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_I"], px, (py * numChallengesadded), pz )	
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_I_text"], GUI.CompletedChallengeFans.ChallengeName[numChallengesadded] )	
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_I_amount"], "GAME_NUM_" .. GUI.CompletedChallengeFans.numFans[numChallengesadded] )
				
				local length = UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_I_text"], true)
				length = length + 1
				length = length + UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_I_amount"])
				
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_I_text"], (-length * 0.5), nil, nil )	
				
				
				
			end
			if numCompleteChallenges > 1 then
				numChallengesadded = numChallengesadded +1
				UIButtons.SetActive( SCUI.name_to_id["challenge_bonus_II"], true )
		
				local px, py, pz = UIButtons.GetPosition( SCUI.name_to_id["challenge_bonus_II"] )
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_II"], px, (py * numChallengesadded), pz )
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_II_text"], GUI.CompletedChallengeFans.ChallengeName[numChallengesadded] )						
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_II_amount"], "GAME_NUM_" .. GUI.CompletedChallengeFans.numFans[numChallengesadded] )	
				
				local length = UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_II_text"], true)
				length = length + 1
				length = length + UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_II_amount"])
				
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_II_text"], (-length * 0.5), nil, nil )	
				
						
			end
			if numCompleteChallenges > 2 then
				numChallengesadded = numChallengesadded +1
				UIButtons.SetActive( SCUI.name_to_id["challenge_bonus_III"], true )
		
				local px, py, pz = UIButtons.GetPosition( SCUI.name_to_id["challenge_bonus_III"] )
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_III"], px, (py * numChallengesadded), pz )	
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_III_text"], GUI.CompletedChallengeFans.ChallengeName[numChallengesadded] )				
				UIButtons.ChangeText( SCUI.name_to_id["challenge_bonus_III_amount"], "GAME_NUM_" .. GUI.CompletedChallengeFans.numFans[numChallengesadded] )
				
				local length = UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_III_text"], true)
				length = length + 1
				length = length + UIButtons.GetStaticTextLength(SCUI.name_to_id["challenge_bonus_III_amount"])
				
				UIButtons.ChangePosition( SCUI.name_to_id["challenge_bonus_III_text"], (-length * 0.5), nil, nil )					
							
			end
		
		
		end
	
	end
	
	if GUI.race_summary.rank_up == true or numCompleteChallenges > 0 then
		GUI.RankUpData = Multiplayer.POBProgressionForRank(GUI.race_summary.rank + 1)
			
		for i = 1, 3 do
			GUI.unlock_nodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowRaceSummary.lua", "unlock_node" )
			UIButtons.AddListItem( SCUI.name_to_id["unlock_list"], GUI.unlock_nodes[i] )
		end		
		
		UIButtons.SetActive( SCUI.name_to_id["new_unlocks"], false )	
	end
	
	if numCompleteChallenges > 0 then
		if GUI.RankUpData == nil then
			GUI.RankUpData = {}			
		end
		
		if GUI.RankUpData.unlocks == nil then		
			GUI.RankUpData.unlocks = {}
		end
		
		GUI.RankUpData.unlocks.challenges =  GUI.CompletedChallengeFans.ChallengeReward
	end
	
	if GUI.RankUpData ~= nil then
		if GUI.RankUpData.unlocks ~= nil then
			RaceSummary_Progression_RefreshUnlocks( SCUI.name_to_id["unlock_list"], GUI.unlock_nodes, GUI.RankUpData.unlocks)
		end
	end
	
	--upcoming unlock
	GUI.UpcomingRankUpData = Multiplayer.POBProgressionForRank(GUI.race_summary.rank + 2)
		
	for i = 1, 3 do
		GUI.upcoming_unlock_nodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowRaceSummary.lua", "unlock_node" )
		UIButtons.AddListItem( SCUI.name_to_id["upcoming_unlock_list"], GUI.upcoming_unlock_nodes[i] )
	end
	
	if GUI.UpcomingRankUpData ~= nil then
		if GUI.UpcomingRankUpData.unlocks ~= nil then
			RaceSummary_Progression_RefreshUnlocks( SCUI.name_to_id["upcoming_unlock_list"], GUI.upcoming_unlock_nodes, GUI.UpcomingRankUpData.unlocks )
		end
	end
	
	FillRaceSummaryResults()
	
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	
	if message == UIEnums.Message.MenuNext and UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if GUI.current_screen_id == 1 then
			if RaceSummary_CanViewGamerCard(  ) == true then			
				NetServices.ViewRaceSummaryGamerCard( Profile.GetPrimaryPad(), UIButtons.GetSelection( SCUI.name_to_id[ "node_list" ] ) )			
			end
		end
	elseif message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		PopScreen()
	elseif message == UIEnums.Message.ButtonRight then
		if GUI.current_screen_id == 1 and NetRace.IsPrivateGame() == false then
			UIButtons.TimeLineActive( "MoveLeft_1", true, 0)
			GUI.current_screen_id = (GUI.current_screen_id + 1)
			RaceSummaryTransitionRightFinished()
			UIButtons.SetSelected( SCUI.name_to_id[ "node_list" ], false )
			UISystem.PlaySound( UIEnums.SoundEffect.StickersPage )
		end	
	elseif message == UIEnums.Message.ButtonLeft then
		if GUI.current_screen_id == 2 then
			UIButtons.TimeLineActive( "MoveLeft_1", false, 1)
			GUI.current_screen_id = (GUI.current_screen_id - 1)
			RaceSummaryTransitionLeftFinished()
			UIButtons.SetSelected( SCUI.name_to_id[ "node_list" ], true )			
			UISystem.PlaySound( UIEnums.SoundEffect.StickersPage )
		end
	end

end

function FrameUpdate( time )
end

function EnterEnd()	
	RestoreInfoLine()
	UIButtons.TimeLineActive( "Hide_Lobby", false )
	Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
	PlaySfxGraphicNext()
end

function End()
	--
end

function RaceSummary_CanViewGamerCard( )
	return ( NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true )
end

function RaceSummaryTransitionRightFinished()
	UIButtons.SetActive( SCUI.name_to_id[ "Right_Arrow" ], false )
	UIButtons.SetActive( SCUI.name_to_id[ "Left_Arrow" ], true )
	UIButtons.TimeLineActive( "show_left_arrow", true )
	UIButtons.TimeLineActive( "show_right_arrow", false )
	RaceSummary_SetupInfoLines()	
end

function RaceSummaryTransitionLeftFinished()
	UIButtons.SetActive( SCUI.name_to_id[ "Right_Arrow" ], true )
	UIButtons.SetActive( SCUI.name_to_id[ "Left_Arrow" ], false )
	UIButtons.TimeLineActive( "show_right_arrow", true )	
	UIButtons.TimeLineActive( "show_left_arrow", false )
	RaceSummary_SetupInfoLines()
end

function RaceSummary_SetupInfoLines()

	local options = {  }
	
	if GUI.current_screen_id == 1 then
		if RaceSummary_CanViewGamerCard() == true then
			options[ #options + 1 ] = UIText.INFO_VIEW_GAMER_CARD
		end
	end
	
	options[ #options + 1 ] = UIText.INFO_B_BACK
	
	SetupInfoLine( unpack( options ) )
end

function SetupRaceSummaryResults()

	local numPlayers = Amax.GetNumberOfPeopleInRaceSummaryResults()

	local node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i = 1, GUI.num_rows, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowRaceSummary.lua", "_node_race_results" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local text_gamertag_id = UIButtons.FindChildByName( node_id, "_text_gamertag" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "_rank_icon" )
		local text_rank_id = UIButtons.FindChildByName( node_id, "_text_rank" )
		local text_race_time_id = UIButtons.FindChildByName( node_id, "_text_race_time" )
		local background_id = UIButtons.FindChildByName( node_id, "_background_box" )
	
		UIButtons.SetActive( background_id, ( i % 2 == 1 ) )
		UIButtons.SetActive( rank_icon_id, false )

		UIButtons.AddListItem( node_list_id, node_id, i )
		
		if i <= numPlayers then
			UIButtons.SetNodeItemLocked( node_list_id, i-1 , false )
		else
			UIButtons.SetNodeItemLocked( node_list_id, i-1 , true )
		end
		
		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, rank_icon = rank_icon_id, rank = text_rank_id, gamertag = text_gamertag_id, race_time = text_race_time_id, }
	end

	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	local node_title_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpShowRaceSummary.lua", "_node_race_results_title" )
	
	if Multiplayer.GetLobbyRaceResultsGameModeIsDestruction() == true then
		local race_time_title_id = UIButtons.FindChildByName( node_title_id, "_text_title_race_time" )
		UIButtons.ChangeText( race_time_title_id, UIText.MP_SCOREBOARD_POINTS  )
	end

	UIButtons.AddListItem( node_list_title_id, node_title_id, 1 )
end

function SetupRivalAndPrey()

	if Multiplayer.IsNemesisValid() == true then
		UIButtons.ChangeText( SCUI.name_to_id[ "text_nemesis_gamertag" ], "GAME_LOBBY_RACESUMMARY_NEMESIS"  )
		UIButtons.ChangeText( SCUI.name_to_id[ "text_hitcount_nemesis" ], "GAME_LOBBY_RACESUMMARY_HITCOUNT_NEMESIS"  )
	else
		UIButtons.ChangeText( SCUI.name_to_id[ "text_nemesis_hit_you_the_most" ], UIText.MP_RACE_SUMMARY_NO_NEMESIS  )
	end
	
	if Multiplayer.IsPreyValid() == true then
		UIButtons.ChangeText( SCUI.name_to_id[ "text_prey_gamertag" ], "GAME_LOBBY_RACESUMMARY_PREY"  )
		UIButtons.ChangeText( SCUI.name_to_id[ "text_hitcount_prey" ], "GAME_LOBBY_RACESUMMARY_HITCOUNT_PREY"  )
	else
		UIButtons.ChangeText( SCUI.name_to_id[ "text_prey_you_hit_the_most" ],  UIText.MP_RACE_SUMMARY_NO_PREY )	
	end
	

end

function FillRaceSummaryResults()
	
	local isTeamGame = NetRace.IsTeamGame()
	
	local colour_me = "Support_4"
	local team_a_colour = "team_a"
	local team_b_colour = "team_b"
	

	for i = 1, GUI.num_rows, 1 do
		
		--TODO Who am I?
		local is_me = Multiplayer.GetLobbyRaceResultsIsMe( i )
		
		UIButtons.ChangeText( GUI.rows_id[ i ].position, "GAME_LOBBY_RACESUMMARY_ROW_POSITION" .. i )
		UIButtons.ChangeText( GUI.rows_id[ i ].rank, "GAME_LOBBY_RACESUMMARY_ROW_PLAYER_RANK" .. i )
		UIButtons.ChangeText( GUI.rows_id[ i ].gamertag, "GAME_LOBBY_RACESUMMARY_ROW_GAMERTAG" .. i )
		UIButtons.ChangeText( GUI.rows_id[ i ].race_time, "GAME_LOBBY_RACESUMMARY_ROW_STAT" .. i )
		
		--We only need to colour team games
		if is_me == true then
			MpRaceSummary_ColourRow( i, colour_me )

		elseif isTeamGame == true then	
		
			local team = Multiplayer.GetLobbyRaceResultsTeam( i ) 
		
			if team == 0 then
				print("FillRaceSummaryResults() > team 0 consists of row ", i)
				MpRaceSummary_ColourRow( i, team_a_colour )
			else
				print("FillRaceSummaryResults() > team 1 consists of row ", i)
				MpRaceSummary_ColourRow( i, team_b_colour )
			end
		end
		
		if Multiplayer.GetLobbyRaceResultsRank( i ) ~= -1 then
			UIButtons.SetActive(  GUI.rows_id[ i ].rank_icon, true )	
			Mp_RankIcon( GUI.rows_id[ i ].rank_icon, Multiplayer.GetLobbyRaceResultsRank( i ), Multiplayer.GetLobbyRaceResultsLegendLevel( i ) )
		end
			
	end
end

function MpRaceSummary_ColourRow( rowIndex, colour )
	UIButtons.ChangeColour( GUI.rows_id[ rowIndex ].position, colour)
	UIButtons.ChangeColour( GUI.rows_id[ rowIndex ].rank, colour)
	UIButtons.ChangeColour( GUI.rows_id[ rowIndex ].gamertag, colour)
	UIButtons.ChangeColour( GUI.rows_id[ rowIndex ].race_time, colour )

end

function RaceSummary_Progression_RefreshUnlocks( listId, nodes, unlocks )

	-- turn all nodes off
	for i, v in pairs( nodes ) do
		UIButtons.SetActive( v, false )
	end

	local index = 1
	
	if unlocks.features ~= nil then
		for i, v in ipairs( unlocks.features ) do
			if index <= 3 then
				RaceSummary_Progression_SetupUnlock( nodes[index], v, "feature" )
				index = index + 1
			end
		end
	end
	
	if unlocks.packs ~= nil then	
		for i, v in ipairs( unlocks.packs ) do
			if index <= 3 then
				RaceSummary_Progression_SetupUnlock( nodes[index], v, "pack" )
				index = index + 1
			end
		end
	end
	
	if unlocks.challenges ~= nil then	
		for i, v in ipairs( unlocks.challenges ) do
			if index <= 3 and v.Reward ~= 0 then
				RaceSummary_Progression_SetupUnlock( nodes[index], v, "upgrade" )
				index = index + 1
			end
		end
	end
	
	
	if unlocks.abilities ~= nil then	
		for i, v in ipairs( unlocks.abilities ) do
			if index <= 3 then
				RaceSummary_Progression_SetupUnlock( nodes[index], v, "mod" )
				index = index + 1
			end
		end
	end
	
	if unlocks.cars ~= nil then		
		for i, v in ipairs( unlocks.cars ) do
			if index <= 3 then
				RaceSummary_Progression_SetupUnlock( nodes[index], v, "car" )
				index = index + 1
			end
		end
	end
	
	local pos_x = GUI.unlock_start_x - ( GUI.unlock_node_size_x * ( index - 2 ) )
	
	local px, py, pz = UIButtons.GetPosition( listId )
	
	UIButtons.ChangePosition( listId, px + pos_x, py + GUI.unlock_start_y, 0 )

	-- update total time
	GUI.UnlockTime = ( GUI.UnlockItemTime * index )
end

function RaceSummary_Progression_SetupUnlock( node, unlock, title )

	local icon_id = UIButtons.FindChildByName( node, "node_icon" )
	local title_id = UIButtons.FindChildByName( node, "title" )
	local name_id = UIButtons.FindChildByName( node, "name" )
	
	if title == "upgrade" then
	
		UIButtons.ChangeText( name_id, GameData.GetVehicle(unlock.VehicleId).name_full )
		UIButtons.ChangeText( title_id, "MPL_CAR_UPGRADE_TYPE" .. unlock.Type .. "_" .. (unlock.Tier) )
		
		local RewardIcons =
		{
			[UIEnums.MpChallengeRewards.HealthSlot]		= "health",
			[UIEnums.MpChallengeRewards.BonusFans]		= "extra_slot", -- Need to change this to new icon name when we have one
			[UIEnums.MpChallengeRewards.SpeedBoost]		= "horsepower",

			[UIEnums.MpChallengeRewards.Pearlescent]	= "pearl",
			[UIEnums.MpChallengeRewards.Metallic]		= "metallic",
			[UIEnums.MpChallengeRewards.Candy]			= "candy",
		}

		UIShape.ChangeSceneName( icon_id, "fe_icons" )
		UIShape.ChangeObjectName( icon_id, RewardIcons[unlock.Reward] )

		UIButtons.SetActive( node, true, true )
		UIButtons.TimeLineActive( "show_unlocks", true, 0 )
		UIButtons.TimeLineActive( "hide_unlocks", false, 0 )
		
	else
		if title == "car" then
			UIButtons.ChangeText( name_id, unlock.name_full )
		else
			UIButtons.ChangeText( name_id, unlock.name )
		end
		
		UIButtons.ChangeText( title_id, GUI.unlock_titles[title] )

		UIShape.ChangeSceneName( icon_id, unlock.sheet )
		UIShape.ChangeObjectName( icon_id, unlock.icon )

		UIButtons.SetActive( node, true, true )
		UIButtons.TimeLineActive( "show_unlocks", true, 0 )
		UIButtons.TimeLineActive( "hide_unlocks", false, 0 )
		
	end

	if title == "car" then
		UIButtons.ChangeSize( icon_id, 14, 14 )
	end
end
