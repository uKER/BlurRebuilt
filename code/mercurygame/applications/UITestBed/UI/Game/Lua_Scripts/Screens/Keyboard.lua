KB_GUI =
{
	--
}

function KB_Init()
	AddSCUI_Elements( SCUI_Keyboard )
end

function KB_StartLoop( time )
	return true
end

function KB_MessageUpdate( message, arg1, arg2 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end
end

function KB_FrameUpdate( time )
	--
end

function KB_Render()
	--
end

function KB_EndLoop( time )
	--
end

function KB_End()
	--
end
