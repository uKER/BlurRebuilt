GUI =
{
	finished = false,	
	carousel_branch = "Options",

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	GUI.MessageListId = SCUI.name_to_id[ "OptionsList" ]
	
	StoreInfoLine()
	SetupMenuOptions()	
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if (message == UIEnums.Message.MenuBack or message == UIEnums.Message.MenuNext) and arg2 == true then
		PlaySfxNext()
		GoScreen("Shared\\Options.lua")
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
end
