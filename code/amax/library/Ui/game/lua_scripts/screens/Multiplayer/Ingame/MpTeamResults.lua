GUI =
{
	finished	= false,
	countdown	= 10,

	team_names =
	{
		["team_a"] = UIText.MP_TEAM_A_NAME,
		["team_b"] = UIText.MP_TEAM_B_NAME,
	},

	team_results =
	{
		["team_a"] = "MPL_TEAM_A_RESULT",
		["team_b"] = "MPL_TEAM_B_RESULT",
	},
}

function Init()
	AddSCUI_Elements()
	UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpRaceResults.lua", "MpBgStuff" )

	SetupInfoLine( UIText.INFO_QUIT_RACE )

	GUI.results = Multiplayer.GetTeamResults()

	-- top team
	local backing_top = UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpTeamResults.lua", "table_dummy" )
	UIButtons.SetParent( backing_top, SCUI.name_to_id["top_dummy"], UIEnums.Justify.MiddleCentre )
	MpTeamResults_SetupTeamResults( backing_top, GUI.results.winner, GUI.results[GUI.results.winner] )

	-- bottom team
	local backing_bottom =  UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpTeamResults.lua", "table_dummy" )
	UIButtons.SetParent( backing_bottom, SCUI.name_to_id["bottom_dummy"], UIEnums.Justify.MiddleCentre )
	MpTeamResults_SetupTeamResults( backing_bottom, GUI.results.loser, GUI.results[GUI.results.loser] )
	
	UISystem.PlaySound( UIEnums.SoundEffect.Toggle )
end

function StartLoop( time ) 
	--
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["table_dummy"], false, true )
	UIButtons.SetActive( SCUI.name_to_id["player_node"], false, true )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end

	if message == UIEnums.Message.MenuBack then
		SetupCustomPopup( UIEnums.CustomPopups.ExitRace )
	end	
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	GUI.countdown = GUI.countdown - time
	if GUI.countdown <= 0 then
		GUI.countdown = 0

		if net_Disconnecting() == false then
			if Amax.IsGameModeRanked() == true then
				GoScreen( "Multiplayer\\Ingame\\MpProgression.lua" )
			else
				GoScreen( "Multiplayer\\Ingame\\MpRaceAwards.lua" )
			end
		end
	end
end

function MpTeamResults_SetupTeamResults( root_id, team, results )
	local list_id		= UIButtons.FindChildByName( root_id, "players" )
	local title_id		= UIButtons.FindChildByName( root_id, "title" )
	local result_id		= UIButtons.FindChildByName( root_id, "result" )
	local icon_id		= UIButtons.FindChildByName( root_id, "icon" )

	UIButtons.ChangeText( title_id, GUI.team_names[team] )
	UIButtons.ChangeText( result_id, GUI.team_results[team] )

	UIButtons.ChangeTextureUV( icon_id, 0, UIGlobals.TeamIcons[team].u, UIGlobals.TeamIcons[team].v )
	UIButtons.ChangeColour( icon_id, UIGlobals.TeamColours[team] )
	UIButtons.ChangeColour( title_id, UIGlobals.TeamColours[team] )

	local nodes = {}
	for i = 1, 10 do
		nodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpTeamResults.lua", "player_node" )

		UIButtons.SetActive( UIButtons.FindChildByName( nodes[i], "backing" ), ( i % 2 == 0 ) )
		--UIButtons.ChangeColour( UIButtons.FindChildByName( nodes[i], "backing" ), UIGlobals.TeamColours[team] )

		UIButtons.AddListItem( list_id, nodes[i] )
	end

	-- create players
	for i, v in ipairs( results ) do
		UIButtons.ChangeText( UIButtons.FindChildByName( nodes[i], "position" ),	"MPL_RESULT_POSITION" .. v.index )
		UIButtons.ChangeText( UIButtons.FindChildByName( nodes[i], "name" ),		"MPL_RESULT_NAME" .. v.index )
		UIButtons.ChangeText( UIButtons.FindChildByName( nodes[i], "rank" ),		"MPL_RESULT_RANK" .. v.index )
		UIButtons.ChangeText( UIButtons.FindChildByName( nodes[i], "result" ),		"MPL_RESULT_SCORE" .. v.index )
		
		UIButtons.ChangeColour( UIButtons.FindChildByName( nodes[i], "position" ),	UIGlobals.TeamColours[team] )
		UIButtons.ChangeColour( UIButtons.FindChildByName( nodes[i], "name" ), UIGlobals.TeamColours[team] )
		UIButtons.ChangeColour( UIButtons.FindChildByName( nodes[i], "result" ), UIGlobals.TeamColours[team] )

		UIButtons.SetActive( UIButtons.FindChildByName( nodes[i], "rank" ), true, true )
		Mp_RankIcon( UIButtons.FindChildByName( nodes[i], "rank_icon" ), v.rank, v.legend )
		

		if v.player then
			UIButtons.ChangeColour( UIButtons.FindChildByName( nodes[i], "position" ),	"Support_4" )
			UIButtons.ChangeColour( UIButtons.FindChildByName( nodes[i], "name" ),		"Support_4" )
			UIButtons.ChangeColour( UIButtons.FindChildByName( nodes[i], "result" ),	"Support_4" )
		end
	end
end