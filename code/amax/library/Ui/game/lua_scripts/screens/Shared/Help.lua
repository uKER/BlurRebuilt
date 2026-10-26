GUI =
{
	finished = false,	
	carousel_branch = "Help",

	CanExit = function( _GUI )
		UISystem.PlaySound( UIEnums.SoundEffect.GraphicBackward )
		return true
	end,
	
	node_ids = { }
}

function Init()
	if UIGlobals.IsIngame == false then
		Amax.ChangeUiCamera( "Op_1", 0.6, 0 )
		net_SetRichPresence( UIEnums.RichPresence.Help )
	end
	
	AddSCUI_Elements()
	CarouselApp_SetScreenTimers()
	
	GUI.MenuId = SCUI.name_to_id[ "menu" ]
	
	GUI.node_ids[1] = Mp_SetupMenuItemHelpText( GUI.MenuId, 0,	UIText.CMN_OPT_POWERUPS_HELP, UIText.CMN_HLP_OPT_POWERUPS, "perk_question", "common_icons" )
	
	if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
		GUI.node_ids[2] = Mp_SetupMenuItemHelpText( GUI.MenuId, 1,	UIText.CMN_OPT_SHARING_HELP, UIText.CMN_HLP_OPT_SHARING, "message", "common_icons" )
		GUI.node_ids[3] = Mp_SetupMenuItemHelpText( GUI.MenuId, 2,	UIText.CMN_OPT_SP_PROGRESSION_HELP, UIText.CMN_HLP_OPT_SP_PROGRESSION, "bio", "common_icons" )
		GUI.node_ids[4] = Mp_SetupMenuItemHelpText( GUI.MenuId, 3,	UIText.CMN_OPT_SP_GAME_MODES_HELP, UIText.CMN_HLP_OPT_SP_GAME_MODES, "style_racing", "common_icons" )
		if UIGlobals.IsIngame == false then
			GUI.node_ids[5] = Mp_SetupMenuItemHelpText( GUI.MenuId, 4,	UIText.CMN_OPT_VIDEO_HELP, UIText.CMN_HLP_OPT_VIDEOS, "video", "common_icons" )
		end
	else
		GUI.node_ids[2] = Mp_SetupMenuItemHelpText( GUI.MenuId, 1,	UIText.CMN_OPT_MP_PROGRESSION_HELP, UIText.CMN_HLP_OPT_MP_PROGRESSION, "groups", "common_icons" )
		GUI.node_ids[3] = Mp_SetupMenuItemHelpText( GUI.MenuId, 2,	UIText.CMN_OPT_MP_GAME_MODES_HELP, UIText.CMN_HLP_OPT_MP_GAME_MODES, "Powered_up_Racing", "common_icons" )
	end
	
	StoreInfoLine()
	SetupMenuOptions()	
	
	SetupScreenTitle( UIText.CMN_HELP_TITLE, SCUI.name_to_id["_CarouselDummy"], "help" )
	SetupBottomHelpBar( UIText.CMN_HLP_SCREEN_HELP )
end

function PostInit()
	local s = GetScreenOtions().selection
	
	-- Set to previous selection if it exists
	UIButtons.SetSelection( GUI.MenuId, 0 )
	if IsNumber( s ) == true then
		UIButtons.SetSelection( GUI.MenuId, s )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if IsControllerLocked() == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext()
		local selected = UIButtons.GetSelection( GUI.MenuId )
		if selected == 0 then
			GoScreen("Shared\\Help_Powerups.lua")
		elseif selected == 1 then
			if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
				GoScreen("Shared\\Help_Sharing.lua")			
			else
				GoScreen("Shared\\Help_MP_Progression.lua")
			end
		elseif selected == 2 then
			if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
				GoScreen("Shared\\Help_SP_Progression.lua")
			else
				GoScreen("Shared\\Help_MP_GameModes.lua")
			end
		elseif selected == 3 then
			GoScreen("Shared\\Help_SP_GameModes.lua")
		elseif selected == 4 then
			GoScreen("Shared\\Help_Video.lua")
		else
			print("whaatisit?",selected)
		end
	end
end

function FrameUpdate( time )
	--
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	GetScreenOtions().selection = UIButtons.GetSelection( GUI.MenuId )
end
