GUI =
{
	finished = false,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
	
	net_SetRichPresence( UIEnums.RichPresence.Leaderboards )

	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupMenuOptions()
	
	GUI.MenuId = SCUI.name_to_id[ "menu" ]
	
	local options = 
	{
		{ value = UIEnums.MpScoreboard.TotalWins,	text = UIText.MP_SCOREBOARD_NEW_TOTAL_POINTS,		help_text = UIText.MP_SCOREBOARD_NEW_TOTAL_POINTS_HELP,			icon_sheet = "common_icons",	icon_name = "points_target", },
		{ value = UIEnums.MpScoreboard.TotalFans,	text = UIText.MP_SCOREBOARD_NEW_FANS_TOTAL,			help_text = UIText.MP_SCOREBOARD_NEW_FANS_TOTAL_HELP,			icon_sheet = "Common_icons",	icon_name = "Fan", },
		{ value = UIEnums.MpScoreboard.PerkRatio,	text = UIText.MP_SCOREBOARD_TOTAL_POWERUPS,			help_text = UIText.MP_SCOREBOARD_TOTAL_POWERUPS_HELP,			icon_sheet = "fe_icons",		icon_name = "perk_hit", },
		{ value = UIEnums.MpScoreboard.Legends,		text = UIText.MP_SCOREBOARD_NEW_LEGENDS,			help_text = UIText.MP_SCOREBOARD_NEW_LEGENDS_HELP,				icon_sheet = "fe_icons",		icon_name = "Legend", },
	}
	
	for i, v in ipairs( options ) do
		Mp_SetupMenuItemHelpText( GUI.MenuId, v.value, v.text, v.help_text, v.icon_name, v.icon_sheet, UIEnums.Panel._3DAA_WORLD )
	end
end

function PostInit()
	RestoreScreenSelection( GUI, "MenuId" )

	SetupScreenTitle( UIText.MP_CHOOSE_SCOREBOARD_TO_VIEW, SCUI.name_to_id["TitleDummy"], "leaderboards", "fe_icons" )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext( )		

		local selection = UIButtons.GetSelection( GUI.MenuId )
		
		Amax.ScoreboardLookAt( selection )
		UIGlobals.ScoreboardLookingAt = selection
		
		GoScreen("Multiplayer\\MpShowLeaderboard.lua" )

	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		net_StopScoreboardRequests( )
		PlaySfxBack()
		PlaySfxGraphicBack()
		
		GoScreen("Multiplayer\\MpOnline.lua" )
	end
end

function FrameUpdate( time )
end

function EnterEnd()
	RestoreInfoLine()
end

function End()
	StoreScreenSelection( GUI, "MenuId" )
end
