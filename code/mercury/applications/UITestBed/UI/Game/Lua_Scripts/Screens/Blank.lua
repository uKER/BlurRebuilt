GUI =
{
	finished = false,
}

function Init()
	AddSCUI_Elements( )
end


function StartLoop(time)
	return true
end


function MessageUpdate(message,arg1,arg2,arg3,arg4)
	SCUI_MessageUpdate( message, arg1, arg2 )
	
	if GUI.finished == true then
		return
	end

	return false
end

function FrameUpdate(time)

end


function Render()
	
end

function End()

end

function EndLoop(time)
	return true
end

