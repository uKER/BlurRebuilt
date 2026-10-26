GUI =
{
	finished = false,
	prep_time = 0,
	waiting_for_ctx = 0,
}

function Init()
	UIScreen.SetScreenTimers( 0, 0 )
	
	net_LockoutFriendsOverlay( true )
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Keep the intermediate loading cover full-width after the fade.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "LoadingBG" ], SCREEN_WIDTH )
	end
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
	if GUI.finished == false then
		-- We're all finished
		if UserKickBackActive() == false and UIScreen.IsContextActive( UIEnums.Context.CarouselApp ) == false and SubScreenActive() == false and UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false then
			GUI.all_ok = true
			
			-- Onwards
			GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_GAME_LOADING ) )
			
			-- Clear
			StoreScreen( UIEnums.ScreenStorage.FE_GAME_LOADING, nil )
			
			return
		end
	end	
	--[[
	-- Debug
	GUI.prep_time = GUI.prep_time + time
	print("Prep : ", GUI.prep_time )
	]]--
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	if GUI.all_ok ~= true then
		return
	end
	
	Camera_UseInGame()

	if Amax.IsGameModeSplitScreen() == true then
		Multiplayer.SetupLevelState( UIGlobals.Splitscreen )
		
	elseif Amax.IsGameModeMultiplayer() == true then
		Multiplayer.SetupLevelState()
	
	elseif UIGlobals.DebugEvent ~= nil then
		Amax.SetupDebugRace(UIGlobals.DebugEvent)
	else
		Amax.SetupRace()
	end
	
	print("[Sync boot assets]")
	Amax.SendMessage( UIEnums.GameFlowMessage.SyncBootAssets )
	
	-- New we're done
	if UIGlobals.IsQuickRestart == false then
		Amax.DestroyUiResources()
	else
		Amax.SetLoadingScreen(true)
	end
	
	ClearGlobals()
end
