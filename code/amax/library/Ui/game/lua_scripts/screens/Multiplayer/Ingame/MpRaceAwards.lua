GUI =
{
	finished = false,
	can_leave = false,

	countdown	= 5,
	
	scoreboard_writetime = 0,
}

function Init()
	AddSCUI_Elements( )
	UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpRaceResults.lua", "MpBgStuff" )

	UIScreen.SetScreenTimers( 0, 0.3 ) -- Time for fade
	
	GUI.timeout_id = SCUI.name_to_id[ "countdown" ]

	GUI.GamerPictures = Profile.GetRemoteGamerPictureMap()
	GUI.race_awards = Multiplayer.GetRaceAwards()
	Multiplayer.PostFlaggedDataEvents()

	local size_x = 35
	local x = -( size_x * ( ( #GUI.race_awards-1 ) / 2 ) ) y = -4

	for i, v in ipairs( GUI.race_awards ) do
		local node_id = Awards_SetupAward( v, i )

		UIButtons.ChangePosition( node_id, x, y, 0.0 )
		x = x + size_x

		-- offset timeline
		local end_time = UIButtons.GetXtVar( node_id, "time_lines.0.time.end" )
		local start_time = UIButtons.GetXtVar( node_id, "time_lines.0.time.start" )
		local time = end_time - start_time - 0.15

		UIButtons.SetXtVar( node_id, "time_lines.0.time.end", end_time + ( time * (i-1) ) )
		UIButtons.SetXtVar( node_id, "time_lines.0.time.start", start_time + ( time * (i-1) ) )
	end

	-- only for online ranked games
	if Amax.GetGameMode() ~= UIEnums.GameMode.SplitScreen then
		-- no game invites until sesison has ended and stats written.
		net_EnableGlobalUpdate( false )
		
		-- start the session end process, if game mode doesn't use scoreboards this wont change anything.
		Amax.StartScoreboardWrite( )
		print("Writing to scoreboard")
	end
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["award_node"], false )

	if IsSplitScreen() == true then
		UIButtons.SetActive( SCUI.name_to_id["countdown"], false )
	else
		SetupInfoLine( UIText.INFO_QUIT_RACE )
	end
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

	if message == UIEnums.Message.MenuBack and IsSplitScreen() == false then
		SetupCustomPopup( UIEnums.CustomPopups.ExitRace )
	end	
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	if IsSplitScreen() == true then
		GUI.countdown = GUI.countdown - time

		if GUI.countdown <= 0 then
			GUI.countdown = 0
			StopIngameMusic() -- stop audio
			GoScreen( "Loading\\LoadingUI.lua" )
		end
	end

	-- only for online ranked games
	if Amax.GetGameMode() ~= UIEnums.GameMode.SplitScreen then
		if GUI.can_leave == false then
			GUI.scoreboard_writetime = GUI.scoreboard_writetime + time
			
			if Amax.ScoreboardWriteComplete() == true or GUI.scoreboard_writetime > 15 then
				print("Scoreboard write finished. Time : ", GUI.scoreboard_writetime )
				EnableLeave()
			end

		-- once everybody has ranked up and the scoreboard write has finished
		elseif NetRace.CanStartResultsCountdown() == true then
		
			local old_time = math.ceil(GUI.countdown);
			GUI.countdown = GUI.countdown - time
			local new_time = math.ceil(GUI.countdown);
		
			if new_time ~= old_time then
				-- dont play the sound on zero
				if new_time > 0 then
					if new_time <= 3 then
						UISystem.PlaySound( UIEnums.SoundEffect.CountDown03 )
					elseif new_time <= 10 then
						UISystem.PlaySound( UIEnums.SoundEffect.CountDown )
					end
				end	
			end

			-- check we havent started disconnecting
			if GUI.countdown <= 0 then
				GUI.countdown = 0

				if net_Disconnecting() == false then
					StopIngameMusic() -- stop audio 
					GoScreen("Loading\\LoadingUI.lua")
					StartAsyncSave()
				end
			end

			-- update timeout text or waiting text
			UIButtons.ChangeText( GUI.timeout_id, "MPL_RETURN_TO_LOBBY" .. math.ceil( GUI.countdown ) )
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
	UIScreen.CancelPopup()
end

function Awards_SetupAward( award, index )
	local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpRaceAwards.lua", "award_node" )

	local name_id = UIButtons.FindChildByName( node_id, "name" )
	local desc_id = UIButtons.FindChildByName( node_id, "desc" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )
	local amount_id = UIButtons.FindChildByName( node_id, "amount" )
	local owner_id = UIButtons.FindChildByName( node_id, "gamertag" )
	local frame_id = UIButtons.FindChildByName( node_id, "frame" )
	local gamerpic_id = UIButtons.FindChildByName( node_id, "gamerpic" )
	local rank_id = UIButtons.FindChildByName( node_id, "rank" )
	local rank_icon_id = UIButtons.FindChildByName( node_id, "rank_icon" )
	local tied_id = UIButtons.FindChildByName( node_id, "tied_notification" )

	UIButtons.ChangeText( name_id, award.name )
	UIButtons.ChangeText( desc_id, award.description )
	UIButtons.ChangeText( amount_id, "MPL_AWARDS_AMOUNT" .. index )
	UIButtons.ChangeText( owner_id, "MPL_AWARDS_OWNER" .. index )
	
	if award.tied == true then
		UIButtons.SetActive(tied_id, true)
	end

	UIShape.ChangeObjectName( icon_id, award.icon_name )

	if IsSplitScreen() == true then
		LocalGamerPicture( gamerpic_id, award.pad_index )
		UIButtons.ChangeColour( owner_id, UIGlobals.Splitscreen.colours[ award.pad_index ] )
		UIButtons.ChangeColour( frame_id, UIGlobals.Splitscreen.colours[ award.pad_index ] )

		UIButtons.ChangeColour( name_id, UIGlobals.Splitscreen.colours[ award.pad_index ] )
		UIButtons.ChangeColour( icon_id, UIGlobals.Splitscreen.colours[ award.pad_index ] )

		UIButtons.SetActive( rank_id, false, true )
		UIButtons.SetActive( tied_id, false )
	else
		if award.local_player == true then
			LocalGamerPicture( gamerpic_id, Profile.GetPrimaryPad() )
			UIButtons.ChangeColour( owner_id, "Support_4" )
			UIButtons.ChangeColour( frame_id, "Support_4" )

			UIButtons.ChangeColour( name_id, "Support_4" )
			UIButtons.ChangeColour( icon_id, "Support_4" )
			UIButtons.ChangeColour( tied_id, "Support_4" )
		else
			RemoteGamerPicture( gamerpic_id, GUI.GamerPictures[award.join_ref] )
		end

		-- setup rank info
		UIButtons.ChangeText( rank_id, "MPL_AWARDS_RANK" .. index )
		Mp_RankIcon( rank_icon_id, award.rank, award.legend )
	end

	if index % 2 == 1 then
		UIButtons.SetAlphaIntensity( UIButtons.FindChildByName( node_id, "backing" ), nil, 0.45 )
	end

	return node_id
end

function EnableLeave( )
	if GUI.can_leave == false then
		NetRace.EnterResultsIdle()
		net_EnableGlobalUpdate( true )

		GUI.can_leave = true
	end
end
