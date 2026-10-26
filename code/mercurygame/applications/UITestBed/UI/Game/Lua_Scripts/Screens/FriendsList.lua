FL_GUI =
{
	--
}

function FL_Init()
	AddSCUI_Elements( SCUI_FriendsList )
end

function FL_StartLoop( time )
	return true
end

function FL_MessageUpdate( message, arg1, arg2 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end
end

function FL_FrameUpdate( time )
	--
end

function FL_Render()
	--
end

function FL_EndLoop( time )
	--
end

function FL_End()
	--
end
