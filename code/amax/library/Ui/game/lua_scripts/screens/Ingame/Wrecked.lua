GUI =
{
	finished = false,

	timeout = 0,
}

function Init()
	Splitscreen_AddSplits()
	
	AddSCUI_Elements( )
	--SetupNext()
	

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
	
	if GUI.finished == true then
		return
	end

	if message == UIEnums.GameFlowMessage.RaceFinished then
		UIGlobals.Ingame.RaceFinishedMsg = message
	end
end

function FrameUpdate( time )
	GUI.timeout = GUI.timeout + time

	if GUI.timeout > 3 then
		if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
			print("##### SinglePlayer. Nowhere to go ####")
			print("##### SinglePlayer. Please fix-up ####")
			
		elseif Amax.GetGameMode() == UIEnums.GameMode.Online then
			GoScreen( "Multiplayer\\Ingame\\MpWinner.lua" )
			Amax.EndWreckedEffect( 0 )
		elseif IsSplitScreen() == true then
			Amax.SetupResults()
			GoScreen( "Multiplayer\\Ingame\\MpSplitscreenResults.lua" )	
		else
			GoScreen("Loading\\LoadingUI.lua")
		end
	end
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
