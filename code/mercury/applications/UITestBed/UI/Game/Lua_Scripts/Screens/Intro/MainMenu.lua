GUI =
{
	finished = false,
}

function Init()
	--
end

function StartLoop( time )
	return true
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.ButtonB then
		GUI.finished = true
		UIScreen.SetNextScreen( "Intro\\Welcome.lua" )
	end
	
	-- TODO - a button to test out AsyncSave.lua
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
