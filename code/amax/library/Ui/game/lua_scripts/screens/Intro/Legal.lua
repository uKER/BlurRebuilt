GUI =
{
	finished = false,
	ui_audio_loaded = false,
}

function Init()
	UIScreen.SetScreenTimers( 0, 0 )
		
	PS3_CheckForInstallingMessage()
	
	Profile.AllowProfileChanges( true )
	Profile.ActOnProfileChanges( false )
	Profile.AllowAllPadInput( true )	
	if UIGlobals.Ps3Installing ~= true then
		Amax.SetLoadingScreen(true)
	end
end

function PostInit()
	if Amax.IsBootPreCachingComplete() == false or UIGlobals.Ps3Installing == true or Amax.BootUiSceneReady() == false then
		GUI.loadingSegs = AddLoadingSegs()
	end
end

function MessageUpdate( message, arg1, arg2 )
	if message == UIEnums.GameMessage.UIAudioPreLoaded then
		GUI.ui_audio_loaded = true
	end
end

function FrameUpdate( time )
	PS3_CheckForInstallingMessage()
	
	-- [Bone] #3743 - Wait here until this has done
	if Amax.IsBootPreCachingComplete() == false then
		return
	end
	
	if UIGlobals.Ps3Installing == true then
		if Amax.IsInstallerScreenEnabled() == true then
			GoScreen("Intro\\Ps3Installer.lua")
		end
	else
		-- Showroom scene is here, let's go
		if Amax.BootUiSceneReady() == true then --and GUI.ui_audio_loaded == true then
			Amax.SetLoadingScreen(false)
			Amax.BootFinished()
			Amax.SendMessage(UIEnums.GameFlowMessage.StartGameRendering)
			--Amax.HandleUISceneReady(true)
			--Amax.CacheGlobalAnimationResources()
			Amax.InstallTrophies()
			GoScreen("Intro\\StartScreen.lua")
		end
	end
end

function End()
	print( "[Storing Globals Table]" )
	_Globals = { }
    for i,v in pairs( _G ) do 
		_Globals[ i ] = v 
	end
end
