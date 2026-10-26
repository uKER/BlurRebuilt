GUI =
{
	finished = false,

	PlayerGadgets = {},
	countdown		= 5,

	SelectedSlot = -1,

	PlayerColour	= "Support_4",
	FailedColour	= "Support_3",
	TimeoutColour	= "Support_2",
	OtherColour		= "Support_1",
	WinnerColour	= "Support_1",

	SetupResults = false,
}

function Init()
	AddSCUI_Elements()

	-- Create the player lists
	GUI.PlayersMenu		= SCUI.name_to_id[ "players_list" ]

	-- setup result data
	GUI.Results = Multiplayer.GetRaceResults()
	GUI.num_finished = 0
	GUI.num_racers = Multiplayer.VehiclesFinished().num_players

	GUI.GamerPictures = Profile.GetRemoteGamerPictureMap()

	local templates =
	{
		[1] = "player_left",
		[2] = "player_right",
	}

	local flip = 1
	for i = 1, GUI.num_racers do
		GUI.PlayerGadgets[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpRaceResults.lua", templates[flip] )
		UIButtons.AddListItem( GUI.PlayersMenu, GUI.PlayerGadgets[i], i )

		-- urgh
		if flip == 1 then
			flip = 2
		else
			flip = 1
		end

		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.PlayerGadgets[i], "position" ), "GAME_POS_" .. i )
	end

	if ( NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true or NetServices.CanSubmitPlayerReview( Profile.GetPrimaryPad() ) == true ) then
		SetupInfoLine( UIText.INFO_VIEW_PLAYER, UIText.INFO_QUIT_RACE )
	else
		SetupInfoLine( UIText.INFO_QUIT_RACE )
	end

	UISystem.PlaySound( UIEnums.SoundEffect.Toggle )
	UIGlobals.Multiplayer.ViewingResults = true -- so we know which list to look in for viewing gamerpics
end

function StartLoop( time ) 
	--
end

function PostInit()

	-- turn off templates
	UIButtons.SetActive( SCUI.name_to_id["player_left"], false, true )
	UIButtons.SetActive( SCUI.name_to_id["player_right"], false, true )

	RefreshList()
	
	-- [WillB] #37271 - Check to make sure we arnt trying to view the gamercard of an AI player.
	MpRaceResults_RefreshInfoLines()
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end

	if message == UIEnums.Message.MenuNext then
		-- [WillB] #37271 - Check to make sure we arnt trying to view the gamercard of an AI player.
		local selection = UIButtons.GetSelectionIndex( GUI.PlayersMenu ) + 1
		
		if GUI.Results.standings[ selection ].ai_driver == false then
			UIGlobals.Multiplayer.SelectPlayerJoinRef = GUI.Results.standings[ UIButtons.GetSelectionIndex( GUI.PlayersMenu ) + 1 ].join_ref
			SetupCustomPopup( UIEnums.CustomPopups.ViewPlayer )
		end
		
	elseif message == UIEnums.Message.MenuBack then
		SetupCustomPopup( UIEnums.CustomPopups.ExitRace )
	end	
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	GUI.countdown = GUI.countdown - time

	-- cant move on if we are disconnecting or it will break big time
	if GUI.countdown <= 0 and net_Disconnecting() == false then
		GUI.countdown = 0
		
		if Amax.IsGameModeRanked() == true then
			GoScreen( "Multiplayer\\Ingame\\MpProgression.lua" )
		else
			GoScreen( "Multiplayer\\Ingame\\MpRaceAwards.lua" )
		end
	end
	
	-- [WillB] #37271 - Need to only show "View Player" if its a human.
	MpRaceResults_RefreshInfoLines()
end

function MpRaceResults_RefreshInfoLines()
	local selection = UIButtons.GetSelectionIndex( GUI.PlayersMenu ) + 1
	local ai_driver = GUI.Results.standings[ selection ].ai_driver
	
	-- change info lines?
	if GUI.ai_driver ~= ai_driver then
		if ai_driver == true then
			SetupInfoLine( UIText.INFO_QUIT_RACE )
		else
			SetupInfoLine( UIText.INFO_VIEW_PLAYER, UIText.INFO_QUIT_RACE )
		end
	end
end

function RefreshList()

	GUI.Results = Multiplayer.GetRaceResults()

	for i, v in ipairs( GUI.Results.standings ) do

		if v.racing == false then
			local gamertag		= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "gamertag" )
			local gamerpic		= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "gamerpic" )
			local gamerpic_image= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "gamerpic_image" )
			local frame			= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "frame" )
			local rank			= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "rank" )
			local rank_icon		= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "rank_icon" )
			local result		= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "result" )
			local position		= UIButtons.FindChildByName( GUI.PlayerGadgets[i], "position" )

			-- setup correct names
			UIButtons.ChangeText( gamertag, "MPL_RESULT_NAME" .. i - 1 )
			UIButtons.ChangeText( rank,		"MPL_RESULT_RANK" .. i - 1 )
			UIButtons.ChangeText( result,	"MPL_RESULT_RESULT" .. i - 1 )

			-- set rank icon
			Mp_RankIcon( rank_icon, v.rank, v.legend )

			UIButtons.PrivateTimeLineActive( gamerpic, "fly_in", true )

			-- default colours
			UIButtons.ChangeColour( gamertag, GUI.OtherColour )
			UIButtons.ChangeColour( frame, GUI.OtherColour )
			UIButtons.ChangeColour( position, "Support_2" )
			UIButtons.ChangeColour( result, "Main_White" )

			-- either timed out, wrecked, eliminated etc
			if v.timeout == true then
				UIButtons.ChangeColour( result, GUI.TimeoutColour )

			elseif v.finished == false then
				UIButtons.ChangeColour( result, GUI.FailedColour )
			end

			-- if player
			if v.ai_driver == true then
				AIGamerPicture( gamerpic_image, v.ai_avatar_id )

			elseif i == GUI.Results.player_index then
				UIButtons.ChangeColour( gamertag, GUI.PlayerColour )
				UIButtons.ChangeColour( frame, GUI.PlayerColour )
				UIButtons.ChangeColour( position, GUI.PlayerColour )

				LocalGamerPicture( gamerpic_image, Profile.GetPrimaryPad() )
			else
				RemoteGamerPicture( gamerpic_image, GUI.GamerPictures[v.join_ref] )
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

function End()
	UIScreen.SetScreenTimers( 0, 0 )
	UIScreen.CancelPopup()

	UIGlobals.Multiplayer.ViewingResults = false
end
