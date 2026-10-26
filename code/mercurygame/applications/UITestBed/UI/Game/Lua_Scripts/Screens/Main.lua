GUI =
{
	finished = false,
}

function Init()
	AddSCUI_Elements( )
	UIScreen.SetScreenTimers(0.0,0.0)
end

function PostInit()
end

function StartLoop(time)
	return true
end

function MessageUpdate(message,arg1,arg2,arg3,arg4)
	return false
end

function FrameUpdate(time)
	UIButtons.DummyLerpMatrixCopy( SCUI.name_to_id[ "fish" ], "pie" )
end


function Render()
end

function End()
--	UISystem.DestroyMovie( 0 )
end

function EndLoop(time)
	return true
end

