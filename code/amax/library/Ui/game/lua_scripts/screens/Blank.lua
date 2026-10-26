GUI =
{
	finished = false,
}

function Init()
	AddSCUI_Elements( )
	
	print("Blank.lua")
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
end

function FrameUpdate( time )
	--
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end
