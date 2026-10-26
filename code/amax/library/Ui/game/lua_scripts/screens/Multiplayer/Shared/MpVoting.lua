GUI =
{
	finished = false,

	State =
	{
		Voting		= 1,
		Voted		= 2,
		Results		= 3,
		Finished	= 4,
	},
	CurrentState = 1,

	NextPlayTime = 0,

	Timer		= 0.0,
	ResultTime	= 1.5,
	EndTime		= 3,

	RaceIndex		= -1,
	RaceOverviewID	= -1,

	VotePassed = false,

	LocalVoteTable =
	{
		[0] = -1,
		[1] = -1,
		[2] = -1,
		[3] = -1,
	}
}

function Init()
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	AddSCUI_Elements( )

	GUI.VoteInfo = SCUI.name_to_id[ "vote_info" ]
	GUI.TimerID = SCUI.name_to_id[ "timer" ]
	GUI.VotesID	= SCUI.name_to_id[ "votes" ]

	GUI.Timer = UIGlobals.Multiplayer.LobbyTimer
	if IsSplitScreen() == true then
		GUI.Timer = 10
		Profile.LockToPad( -1 )
	end
	GUI.NextPlayTime = GUI.Timer - 1

	UIGlobals.Multiplayer.VotingFinished = false
	UIButtons.ChangeText( GUI.TimerID, "MPL_TIMER_VOTE_TIME" .. GUI.Timer )

	StoreInfoLine()

	if IsSplitScreen() == true then
		SetupInfoLine( UIText.INFO_CAST_VOTE )
	else
		SetupInfoLine( UIText.INFO_EXIT_LOBBY, UIText.INFO_CAST_VOTE )
	end
end

function PostInit()
--	Mp_RouteThumbnail( SCUI.name_to_id["location"], race.thumbnail, 1 )

	local left_race_index, right_race_index = Multiplayer.GetVoteRaceIndicies()

	-- create left race
	local left_race = Multiplayer.GetPlaylistRace( left_race_index )
	local left_node_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "race_overview_node" )
	UIButtons.SetParent( left_node_id, SCUI.name_to_id[ "left_backing" ], UIEnums.Justify.MiddleCentre )

	Mp_RefreshRaceOverview( left_node_id, left_race, left_race_index )

	-- create right race
	local right_race = Multiplayer.GetPlaylistRace( right_race_index )
	local right_node_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "race_overview_node" )
	UIButtons.SetParent( right_node_id, SCUI.name_to_id[ "right_backing" ], UIEnums.Justify.MiddleCentre )

	Mp_RefreshRaceOverview( right_node_id, right_race, right_race_index )
end

function StartLoop( time ) 
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true and IsSplitScreen() == false then
		SetupCustomPopup( UIEnums.CustomPopups.ExitRaceLobby )

	elseif message == UIEnums.Message.ButtonLeftTrigger and GUI.CurrentState == GUI.State.Voting and arg2 == true then
		PlaySfxNext()

		if IsSplitScreen() == true then
			-- only allowed 1 vote per player
			if GUI.LocalVoteTable[arg1] == -1 then
				Multiplayer.CastLocalVote( 0 )
				GUI.LocalVoteTable[arg1] = 0

				UIButtons.TimeLineActive( "vote_cast_local_a", true, 0 )
			end
		else
			UIButtons.TimeLineActive( "vote_cast", true )
			UIButtons.SetActive( SCUI.name_to_id["right_trigger"], false )
			
			NetRace.VoteOnNextRace( 0 )

			GUI.CurrentState = GUI.State.Voted
			SetupInfoLine( UIText.INFO_EXIT_LOBBY )
		end


	elseif message == UIEnums.Message.ButtonRightTrigger and GUI.CurrentState == GUI.State.Voting and arg2 == true then
		PlaySfxNext()

		if IsSplitScreen() == true then
			-- only allowed 1 vote per player
			if GUI.LocalVoteTable[arg1] == -1 then
				Multiplayer.CastLocalVote( 1 )
				GUI.LocalVoteTable[arg1] = 1

				UIButtons.TimeLineActive( "vote_cast_local_b", true, 0 )
			end
		else
			UIButtons.TimeLineActive( "vote_cast", true )
			UIButtons.SetActive( SCUI.name_to_id["left_trigger"], false )

			NetRace.VoteOnNextRace( 1 )

			GUI.CurrentState = GUI.State.Voted
			SetupInfoLine( UIText.INFO_EXIT_LOBBY )
		end
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	GUI.Timer = GUI.Timer - time
	if GUI.Timer < 0 then
		GUI.Timer = 0
	end
	
	if GUI.CurrentState == GUI.State.Voting or GUI.CurrentState == GUI.State.Voted then
		-- check we arnt out of sync too much
		if IsSplitScreen() == false then
			if NetRace.IsHost() == false and math.abs( UIGlobals.Multiplayer.LobbyTimer - GUI.Timer ) >= 1 then
				GUI.Timer = UIGlobals.Multiplayer.LobbyTimer
			end
		end

		local all_voted, left_count, right_count = Multiplayer.GetVotingValues()
		
		-- all votes cast so finish early
		if all_voted == true then
			GUI.Timer = 0
		end

		-- play sound
		if GUI.Timer < GUI.NextPlayTime then
			if GUI.NextPlayTime <= 3 then
				UISystem.PlaySound( UIEnums.SoundEffect.LobbyCountDown03 )
			elseif GUI.NextPlayTime <= 10 then
				UISystem.PlaySound( UIEnums.SoundEffect.LobbyCountDown10 )
			end
			
			GUI.NextPlayTime = GUI.NextPlayTime - 1

			if GUI.NextPlayTime < 0 then
				GUI.NextPlayTime = 0
			end
		end

		if GUI.Timer <= 0 then
			
			local can_continue = false
			if IsSplitScreen() == true then
				Multiplayer.CalculateLocalVoteResults()
				can_continue = true
			else
				-- only the host and He-man have the power
				if NetRace.IsHost() and NetRace.ReceivedVotingResults() == false then
					NetRace.SendVoteResults()
				end
				can_continue = NetRace.ReceivedVotingResults()
			end

			-- all the rest can wait there turn
			if can_continue == true then
				GUI.Timer = GUI.ResultTime
				GUI.CurrentState = GUI.State.Results

				-- turn unneeded gadgets off
				UIButtons.SetActive( SCUI.name_to_id["left_trigger"], false )
				UIButtons.SetActive( SCUI.name_to_id["right_trigger"], false )
				UIButtons.SetActive( SCUI.name_to_id["vote_ends_in"], false, true )

				Voting_ShowWinner()

				SetupInfoLine( UIText.INFO_EXIT_LOBBY )
			end
		end

		UIButtons.ChangeText( GUI.TimerID, "MPL_TIMER_VOTE_TIME" .. math.ceil( GUI.Timer ) )

	elseif GUI.CurrentState == GUI.State.Results then
		if GUI.Timer <= 0 and net_Disconnecting() == false then
			GUI.CurrentState = GUI.State.Finished
			GUI.Timer = 0
		end

	elseif GUI.CurrentState == GUI.State.Finished then
		if GUI.Timer <= 0 and net_Disconnecting() == false then
			UIGlobals.Multiplayer.VotingFinished = true

			-- close the voting screen
			if IsSplitScreen() == true then
				SplitscreenLobby_EndVoting()
			end
		end
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function EnterEnd()
	RestoreInfoLine()

	UIButtons.TimeLineActive( "end_voting", true, 0.0 )
	UIButtons.TimeLineActive( "darken", false )
	UIButtons.SetActive( SCUI.name_to_id["vote_info"], false, true )
end

function End()
end

function Voting_ShowWinner()

	local value, tied = Multiplayer.GetVotingResults()
	local names = 
	{
		[0] = "left",
		[1] = "right",
	}

	UIButtons.ChangeText( SCUI.name_to_id[names[value] .. "_votes"], UIText.MP_VOTE_RESULT )
	UIButtons.ChangeColour( SCUI.name_to_id[names[value] .. "_votes"], "Support_0" )

	UIButtons.TimeLineActive( names[value] .. "_winner", true )
	
	-- something special
	if tied == true then
		UIButtons.ChangeText( SCUI.name_to_id[names[value] .. "_votes"], UIText.MP_VOTE_TIE )
		UIButtons.ChangeColour( SCUI.name_to_id[names[value] .. "_votes"], "Support_4" )
	end
	
	-- [Bone] #34745
	local left_race_index, right_race_index = Multiplayer.GetVoteRaceIndicies()
	local race = Multiplayer.GetPlaylistRace( Select( value == 0, left_race_index, right_race_index ) )
	if IsString( race.thumbnail ) == true then
		UISystem.PlaySound( UIEnums.SoundEffect[ race.thumbnail ] )
	end
end
