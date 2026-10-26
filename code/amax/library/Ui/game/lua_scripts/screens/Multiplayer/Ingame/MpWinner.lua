GUI =
{
	finished = false,

	disable_loading		= false, -- turn me to true so you can work on this screen
	resources_loaded	= false,
	can_move_on			= false,
	
	num_views			= 1,

	team_result_titles =
	{
		["team_a"] = UIText.MP_TEAM_A_WINS,
		["team_b"] = UIText.MP_TEAM_B_WINS,
	},

	team_draw_texture =
	{
		filename = "team_icons",
		pos = { u = 0, v = 0 },
		size = { u = 1, v = 1 },
	},
}

function Init()
	-- [Bone] #37699 - Remove 'quit' popup on this screen
	if net_Disconnecting() == false then
		UIScreen.CancelPopup()
	end
	
	AddSCUI_Elements( )

	Amax.SetupResults()

	if IsSplitScreen() == true then
		Amax.SetNumViewports(1)
	else
		-- change play state to results idle
		NetRace.EnterResults()	
	end

	if NetRace.IsTeamGame() == true then
		GUI.Results = Multiplayer.GetTeamResults()
		GUI.WinnerIndex = GUI.Results[GUI.Results.winner][1].vehicle_index -- set to the highest player in the winning team
	else
		GUI.Results = Multiplayer.GetRaceResults()
		GUI.WinnerIndex = GUI.Results.standings[1].vehicle_index -- set to winning player
	end
	GUI.GamerPictures = Profile.GetRemoteGamerPictureMap()	
end

function PostInit()

	if NetRace.IsTeamGame() == true then
		Winner_SetupTeam( GUI.Results.winner, GUI.Results.draw )
	else
		-- create 1st, 2nd and 3rd
		for i, v in ipairs( GUI.Results.standings ) do
			if i <= 3 then
				Winner_SetupPlayer( v, i )
			end
		end
	end

	Amax.RepairVehicle(GUI.WinnerIndex)

	-- view winning car
	for i = 1, Amax.GetNumViewports() do
		if Amax.IsRaceMpDestruction() == true then
			Amax.AutoSelectAnimatedCamera( GUI.WinnerIndex, i-1 )
			Amax.DisableCarControls()
		else
			Amax.StartAnimatedCamera( UIEnums.AnimatedCameraSequenceType.MpWinner, false, GUI.WinnerIndex, i-1 )
		end
	end

	UISystem.PlaySound( UIEnums.SoundEffect.MPFocusOnWinner )
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

	if message == UIEnums.GameFlowMessage.UILoaded then
		GUI.resources_loaded = true
		Debug.IncrementMemoryTag("ui")
		
		Amax.ExitNetworkLoad()
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	-- frontend resource and winner sequence has finished
	if GUI.resources_loaded == true and GUI.can_move_on == true then
		if IsSplitScreen() == true then
			GoScreen( "Multiplayer\\Ingame\\MpSplitscreenResults.lua" )
		else
			if NetRace.IsTeamGame() == true then
				GoScreen( "Multiplayer\\Ingame\\MpTeamResults.lua" )
			else
				GoScreen( "Multiplayer\\Ingame\\MpRaceResults.lua" )
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
	-- [WillB] #27200 - enable disconnection messages and game invites since we have loaded
	Profile.ForceProfileUpdate()
	Profile.ActOnProfileChanges( true )	
	
	net_EnableGlobalUpdate( true )
end

function Winner_SetupPlayer( player, index )
	local node			= SCUI.name_to_id["player_node_" .. index]
	local gamerpic		= UIButtons.FindChildByName( node, "gamerpic" )
	local gamertag		= UIButtons.FindChildByName( node, "gamertag" )
	local rank			= UIButtons.FindChildByName( node, "rank" )
	local rank_icon		= UIButtons.FindChildByName( node, "rank_icon" )

	-- setup correct names
	UIButtons.ChangeText( gamertag, "MPL_RESULT_NAME" .. index - 1 )
	UIButtons.ChangeText( rank,		"MPL_RESULT_RANK" .. index - 1 )

	if index == 1 then
		UIButtons.ChangeText( UIButtons.FindChildByName( node, "streak" ),	"MPL_RESULT_STREAK" .. index - 1 )
	end

	-- Bug #36055 - Splitscreen gamerpic only worked for primary profile player
	if IsSplitScreen() == true and GUI.Results.standings[index].vehicle_index < #UIGlobals.Splitscreen.players then
		LocalGamerPicture( gamerpic, UIGlobals.Splitscreen.players[GUI.Results.standings[index].vehicle_index+1].pad )
	elseif index == GUI.Results.player_index then
		LocalGamerPicture( gamerpic, Profile.GetPrimaryPad() )
	elseif player.ai_driver == true then
		AIGamerPicture( gamerpic, player.ai_avatar_id )
	else
		RemoteGamerPicture( gamerpic, GUI.GamerPictures[player.join_ref] )
	end

	if IsSplitScreen() == true then
		UIButtons.SetActive( rank, false, true )

		if index == 1 then
			UIButtons.ChangePosition( gamertag, 0, -6, 0 )
		else
			UIButtons.ChangePosition( gamertag, 0, -2.5, 0 )
		end
	else
		-- set rank icon
		Mp_RankIcon( rank_icon, player.rank, player.legend )
	end

	UIButtons.SetActive( node, true )
end

function Winner_SetupTeam( team_tag, draw )
	local node_id = SCUI.name_to_id["team_node"]
	local name_id = UIButtons.FindChildByName( node_id, "team" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )

	if draw == true then
		UIButtons.ChangeText( name_id, UIText.MP_TEAM_DRAW )
		UIButtons.ChangeSize( icon_id, 26, 13, 0 )

		UIButtons.ChangeTexture( GUI.team_draw_texture, 0, icon_id )
	else
		UIButtons.ChangeText( name_id, GUI.team_result_titles[team_tag] )
		UIButtons.ChangeColour( name_id, UIGlobals.TeamColours[team_tag] )
		UIButtons.ChangeColour( icon_id, UIGlobals.TeamColours[team_tag] )

		UIButtons.ChangeTextureUV( icon_id, 0, UIGlobals.TeamIcons[team_tag].u, UIGlobals.TeamIcons[team_tag].v )
	end

	UIButtons.SetActive( SCUI.name_to_id["team_node"], true )
end

function Winner_StartLoad()
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI

	-- [WillB] #27200 - Stop the game from progressing if we are disconnecting. This will occur when accepting a game invite
	if _GUI.disable_loading == true or net_Disconnecting() == true then
		return
	end
	
	if _GUI.resources_loaded == false then
		-- [WillB] #27200 - disable disconnection messages and game invites while we load otherwise it will explode
		Profile.AllowProfileChanges( true )
		Profile.ActOnProfileChanges( false )
		
		net_EnableGlobalUpdate( false )
		
		Amax.EnterNetworkLoad() -- no network calls/structures should be referenced between the and ExitNetworkLoad()
	
		Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
		Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)
		Amax.SendMessage(UIEnums.GameFlowMessage.StartUILoad)
	end
end

function Winner_NextScreen( )
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI

	_GUI.can_move_on = true
end

function Winner_Snapshot()
	Amax.TakePhoto( 1, false, false )
	UIButtons.SetActive( SCUI.name_to_id["background"], true )
	UIButtons.TimeLineActive( "fade_bg", true )
end
