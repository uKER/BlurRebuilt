GUI =
{
	finished = false,
	do_start_load = true,
	ui_audio_loaded = false,
	ui_scene_loaded = false,
	
	max_wait_time	= 15,
	disconnecting	= false,
	
	network_load_flag = false,
}

function Init()
	net_LockoutFriendsOverlay( true )
	
	print( "LoadingUI Init()" )
	
	Camera_UseFrontend()
	StopIngameMusic()

	UIScreen.SetScreenTimers( 0, 0.3 )
	NetRace.EndPlay( true ) --- safe to call for non network races also. 

	if UIGlobals.Multiplayer.LaunchScreen ~= UIEnums.MpLaunchScreen.MultiplayerLobby then
		Amax.SetLoadStateBackToUI()
	else
		Amax.SetLoadStateBackToLobby()		
	end


	if UIGlobals.LoadFromDebug == false then
		Profile.AllowProfileChanges( true )
		Profile.ActOnProfileChanges( false )
		UIGlobals.ActUponFriendChallenges = false
	end
			
--	UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_CityBackground" )

	if UIGlobals.Multiplayer.LaunchScreen ~= UIEnums.MpLaunchScreen.MultiplayerLobby then
		-- Clear ingame tempories
		UIGlobals.Ingame = { }
		ClearMultiplayerGlobals()
	end

	GameProfile.ClearRaceDifficulty()

	AddSCUI_Elements( )
	
	-- PR - sort out splitscreen pad control after a race
	if IsSplitScreen() == true then
		Profile.SetPrimaryPad( UIGlobals.Splitscreen.primary_pad )
		Profile.LockToPad( UIGlobals.Splitscreen.primary_pad )
	end
	
	-- make sure we're unpaused
	Amax.SendMessage( UIEnums.GameFlowMessage.UnPause )
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Cover the viewport when returning from a race to the menus.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "Gfx_LoadingBG" ], SCREEN_WIDTH )
		-- [Blur2PC] The completed-race variant uses these separate covers.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "background" ], SCREEN_WIDTH )
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "LoadingTex" ], SCREEN_WIDTH )
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "LoadingFade" ], SCREEN_WIDTH )
	end

	GUI.loadingSegs = AddLoadingSegs()
	
	if ShouldShowRaceImage() == false then
		-- use boring loading screen texture since we may not have a "race image" to come back to
		UIButtons.SetActive( SCUI.name_to_id["background"], false )
		UIButtons.SetActive( SCUI.name_to_id["StencilWriteOn"], false )
		UIButtons.SetActive( SCUI.name_to_id["CSB_M"], false )
		UIButtons.SetActive( SCUI.name_to_id["StencilWriteOff"], false )
		UIButtons.SetActive( SCUI.name_to_id["StencilReadOn"], false )
		UIButtons.SetActive( SCUI.name_to_id["LoadingTex"], false )
		UIButtons.SetActive( SCUI.name_to_id["LoadingFade"], false )
		UIButtons.SetActive( SCUI.name_to_id["Gfx_LoadingBG"], true )
	end
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
	
	if message == UIEnums.GameFlowMessage.UILoaded and UIGlobals.LoadFromDebug == true then
		UIButtons.SetActive( GUI.loadingSegs, false )
		GUI.finished_load = true
		
		Amax.ExitNetworkLoad()
	end
	
	if message == UIEnums.GameMessage.UIAudioPreLoaded then
		GUI.ui_audio_loaded = true
	elseif message == UIEnums.GameMessage.UISceneReady then
		GUI.ui_scene_loaded = true		
	end
	
	--[[if message == UIEnums.GameFlowMessage.StartGameRendering then
		UIButtons.SetActive( GUI.loadingSegs, false )
		GUI.finished_load = true
		
		-- [PR] #37265 - moved from End() - need to call this at the earlist oppurtunity so the return screen doesn't get fully initialised and spawn any subscreens as this causes the kick back to fail
		if UIGlobals.LoadFromDebug == false then
			Profile.ForceProfileUpdate()
			Profile.ActOnProfileChanges( true )
			UIGlobals.ActUponFriendChallenges = true
		end
	end]]--
end

function FrameUpdate( time )
	if SubScreenActive() == true then
		return
	end
	
	if GUI.ui_scene_loaded == true and UIGlobals.ShowingUnlocks == false and UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false then
		Amax.HandleUISceneReady()		
		UIButtons.SetActive( GUI.loadingSegs, false )
		
		-- [PR] #37265 - moved from End() - need to call this at the earlist oppurtunity so the return screen doesn't get fully initialised and spawn any subscreens as this causes the kick back to fail
		if UIGlobals.LoadFromDebug == false then
			Profile.ForceProfileUpdate()
			Profile.ActOnProfileChanges( true )
			UIGlobals.ActUponFriendChallenges = true
		end
	end
	
	if GUI.ui_audio_loaded == true and GUI.ui_scene_loaded == true then
		GUI.finished_load = true
	
		-- dont want to be doing this every frame
		if GUI.network_load_flag == false then
			GUI.network_load_flag = true
			Amax.ExitNetworkLoad()
		end
	end
		
	if GUI.do_start_load == true then
		GUI.do_start_load = false
		
		Amax.EnterNetworkLoad() -- no network calls/structures should be referenced between the and ExitNetworkLoad()
		StartUiLoad()		
		
		-- no unlock screen if we're in multiplayer, if we've just done a friend challenge or if we quit (prevFans == nil) or get kicked back for whatever reason
		if ShouldShowRaceImage() == true then
			PushScreen( "Shared\\Unlocks.lua" )
		end
	end
	
	if GUI.finished_load == true and UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false then		
		-- only if we are not on our way back to the start screen
		if IsNetworkMultiplayerMode() == true and UserKickBackActive() == false and UIGlobals.FriendDemandAttemptFromMessage == false then
			GoScreen( "Multiplayer\\MpMain.lua" )
		else
			GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_RETURN ) )
		end
	end
	
	-- [WillB] #38096 - Check for players interupting the save and sittig on a message prompt for too long. This was causing the lobbys to wait indefinatly
	if IsNetworkMultiplayerMode() == true and UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == true then
		GUI.max_wait_time = GUI.max_wait_time - time
	
		if GUI.disconnecting == false and GUI.max_wait_time <= 0 then
			print( "LoadingUI: Save has been interupted for too long. Disconnecting from network game" )
		
			GUI.disconnecting = true
			GUI.max_wait_time = 0
			
			-- disconnect from the network game so that we arnt holding the session in the lobby
			net_FlushSessionEnumerator()
			net_CloseAllSessions()
			
			UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.MultiplayerRoot
			UIGlobals.IdlePlayerKicked = true
		end
	end
end

function ShouldShowRaceImage()
	return ( UserKickBackActive() ~= true and UIGlobals.Sp_PreviousFans ~= nil and IsNetworkMultiplayerMode() == false 
									and IsSplitScreen() == false and Amax.SP_IsFriendDemand() == false and UIGlobals.LoadFromDebug == false )
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	UIGlobals.FadeUpLoading = 0--Select( ShouldShowRaceImage(), 1, 0 )
	
	Amax.CreateUiResources()
	-- These are handled in MessageUpdate_Wrapper
--	UIGlobals.IsLoading = false

	-- still need this setting for the black quad
	UIGlobals.IsIngame = false

	if UIGlobals.Multiplayer.LaunchScreen ~= UIEnums.MpLaunchScreen.MultiplayerLobby then
		-- reset it just incase its still true
		UIGlobals.Multiplayer.InitialLobby = true
	end
	
	Amax.SetFrontEndLoaded()
	
	ClearGlobals()
end
