GUI =
{
	stringNo = 0,
	finished = false,
}

function Init()
	AddSCUI_Elements( )
	
	Amax.ParseHDDForReplays()

	local replay_table = Amax.GetReplayFilesFromHDD()

	UIButtons.ClearItems(SCUI.name_to_id[ "ReplayMenu" ])
	for i,v in ipairs(replay_table) do

		print(v.name)

		UIButtons.StoreString(GUI.stringNo, v.name)
		UIButtons.AddItem(SCUI.name_to_id[ "ReplayMenu" ], v.id, GUI.stringNo, true)
		
		GUI.stringNo = GUI.stringNo + 1
	end
end

function StartLoop( time )
	--
end

function DebugLoadReplay()
	if Amax.DebugLoadReplay() == true then
 
		-- Keep hud on for now. See WillB otherwise
		-- Need to huck in a debug variable to use the ingame hud or replay screen. 
		--UIGlobals.IsDebugReplayMode = true
		StoreScreen( UIEnums.ScreenStorage.FE_RETURN )
		GoLoadingScreen("Loading\\LoadingGame.lua")
	end
end



function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.ButtonRightTrigger and arg2 == true then
		GoScreen("Debug\\DebugEventScreen.lua")
	elseif message == UIEnums.Message.MenuNext and arg2 == true then
		if Amax.SetLoadReplayIndex(UIButtons.GetSelection(SCUI.name_to_id[ "ReplayMenu" ])) == true then
			
			-- Keep hud on for now. See WillB otherwise
			-- Need to huck in a debug variable to use the ingame hud or replay screen. 
			--UIGlobals.IsDebugReplayMode = true
			StoreScreen( UIEnums.ScreenStorage.FE_RETURN )
			GoLoadingScreen("Loading\\LoadingGame.lua")
		end
	elseif message == UIEnums.Message.ButtonX and arg2 == true then
	--[[
		if Amax.SetEditCutsceneIndex(UIButtons.GetSelection(SCUI.name_to_id[ "ReplayMenu" ])) == true then
		
			-- Keep hud on for now. See WillB otherwise
			-- Need to huck in a debug variable to use the ingame hud or replay screen. 
			--UIGlobals.IsDebugReplayMode = true
			StoreScreen( UIEnums.ScreenStorage.FE_RETURN )
			GoLoadingScreen("Loading\\LoadingGame.lua")
		end
	]]--
	
		Amax.SetConvertReplayIndex(UIButtons.GetSelection(SCUI.name_to_id[ "ReplayMenu" ]))
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
