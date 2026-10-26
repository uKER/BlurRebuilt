GUI =
{
	finished = false,	
	carousel_branch = "Options",
	
	initial_timer = 0.25,
	timer = 0.25,
	can_play_activision_credits = false,
	
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	GUI.MessageListId = SCUI.name_to_id[ "OptionsList" ]
	
	StoreInfoLine()

	PlaySPMovieFullScreen(UIEnums.SPVideoConfig.VIDEO_END_OF_GAME, true)	
end

function PostInit()
	GUI.can_play_activision_credits = true
	
	if UIGlobals.EndOfGame == true then
		Amax.ChangeUiCamera( "Op_1", 0.6, 0 )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then
		if UIGlobals.EndOfGame == true then
			UIGlobals.EndOfGame = nil
			ReturnToStartScreen( UIEnums.UserKickBackMode.EndOfGame )
		else
			PlaySfxBack()
			GoScreen("Shared\\Options.lua")
		end
	end
	
	if message == UIEnums.Message.CreditsFinished then
		if UIGlobals.EndOfGame == true then
			UIGlobals.EndOfGame = nil	
			ReturnToStartScreen( UIEnums.UserKickBackMode.EndOfGame )
		else
			GoScreen("Shared\\Options.lua")
		end	
	end
	
end

function FrameUpdate( time )
	if GUI.initial_timer > 0 then
		GUI.initial_timer = GUI.initial_timer - time
	elseif UIGlobals.OnMovieScreenInEventSelect == false then
		if GUI.can_play_activision_credits == true then
			
			UIButtons.SetActive(SCUI.name_to_id[ "Txt_Credits" ], true)
			UIButtons.SetActive(SCUI.name_to_id[ "Shp_Credits" ], true)
			GUI.timer = GUI.timer - time
			
			if GUI.timer <= 0 then
				SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_DPAD_SCROLL )
				UIButtons.SetActive( SCUI.name_to_id[ "credits" ], true )
				GUI.can_play_activision_credits = false
			end
		end
	end
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
end
