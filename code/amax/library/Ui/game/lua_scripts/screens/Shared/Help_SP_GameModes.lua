GUI =
{
	finished = false,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupBack()
	
	SetupScreenTitle( UIText.CMN_HELP_SP_GAME_MODES_TITLE, SCUI.name_to_id["_CarouselDummy"], "style_racing" )
	SetupBottomHelpBar( UIText.CMN_HLP_OPT_SP_GAME_MODES )
	
	UISystem.PlaySound( UIEnums.SoundEffect.Cascade )
end

function PostInit()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
		
	if message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		GoScreen("Shared\\Help.lua")
	end
end

function FrameUpdate( time )
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	--
end
