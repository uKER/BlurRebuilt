GUI =
{
	finished = false,
	
	current_time = 0,
	wait_time	 = 1,
	end_time	 = 6,
	ready		= false,
}

function Init()
	net_LockoutFriendsOverlay( true )
	
	AddSCUI_Elements( )	

	-- update the session to say that the race is almost finished so new players can join
	NetRace.FlagInRace( false )
	
	-- nuke the wrecked screen if it's up
	local wrecked_context = UIEnums.Context.SpWrecked
	
	if UIScreen.IsContextActive( wrecked_context ) == true then
		UIScreen.SetScreenTimers( 0, 0, wrecked_context )
		EndScreen( wrecked_context )
	end
end

function PostInit()
	UIButtons.TimeLineActive( "show_finished", true, 0 )
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
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	GUI.current_time = GUI.current_time + time

	-- intial delay to allow results to get to peers
	if GUI.current_time > GUI.wait_time and GUI.ready == false then
		GUI.ready = true
		
		-- dont display in team motormash
		if Amax.IsRaceMpDestruction() == true and NetRace.IsTeamGame() == true then
			GUI.end_time = GUI.end_time - 2 -- Just so we dont have to wait too long
		else
			SetupPlayerResult()
		end

	elseif GUI.current_time > GUI.end_time then

		-- inform other players that we have are ready to move on
		NetRace.ChangeLocalPlayerState( Profile.GetPrimaryPad(), UIEnums.Network.PlayerStates.eNetPlayerStateFinished )

		-- the race has finished and all other players have checked in
		if Multiplayer.RaceFinished() == true and NetRace.CanShowWinner() == true then 
			GoScreen( "Multiplayer\\Ingame\\MpWinner.lua" )
		else
			Mp_ResetSpectateVars()
			GoScreen( "Multiplayer\\Ingame\\MpSpectate.lua" )
		end
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
end

function SetupPlayerResult()
	local results = Multiplayer.GetRaceResults()
	
	-- Swap colours out
	UIButtons.SwapColour( SCUI.name_to_id[ "PosDummy" ], "Main_1", GetResultColour( results.player_index ) )
	
	-- Turn on swooooshie
	if results.player_index <= 3 then
		UIButtons.SetActive( SCUI.name_to_id[ "Dummy_" .. results.player_index ], true )
	end
	
	if IsTable( results.standings[ results.player_index ] ) then
		UIButtons.ChangeText( SCUI.name_to_id[ "Pos_Place" ], "MPL_RESULT_POS_WHOLE" .. results.player_index-1 )
	end

	UIButtons.TimeLineActive( "show_result", true )
end
