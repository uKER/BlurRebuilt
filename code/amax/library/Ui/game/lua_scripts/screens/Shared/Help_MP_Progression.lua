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
	
	SetupScreenTitle( UIText.CMN_HELP_MP_PROGRESSION_TITLE, SCUI.name_to_id["_CarouselDummy"], "groups" )
	SetupBottomHelpBar( UIText.CMN_HELP_MP_PROGRESSION_BOTTOM )
	
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
