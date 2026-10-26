GUI =
{
	finished = false,
	wait_for_copy = false,
}

function Init()
	AddSCUI_Elements( )
	
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUpDown" )

	SetupMovie( 0, "Ui\\Movies\\BizarreSplash.bik", nil, nil, false, true )
	UISystem.EnableMovieLooping( 0, false )

	-- Where to go after attract splash	
	StoreScreen( UIEnums.ScreenStorage.LOAD_NEXT, "Intro\\Legal.lua" )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2 )

	if GUI.finished == true then
		return
	end
	
	if ( message == UIEnums.Message.MovieFinished and arg1 == 0 ) then
		print("State",UIGlobals.Ps3InstallState)
		if UIGlobals.Ps3InstallState == UIEnums.Ps3InstallState.NONE then
			GoScreen( "Intro\\Legal.lua" )
		elseif UIGlobals.Ps3InstallState == UIEnums.Ps3InstallState.COPY_DATA then
			GoScreen("Intro\\AttractSplash.lua")
		elseif UIGlobals.Ps3InstallState == UIEnums.Ps3InstallState.COPY_ATTRACT then
			-- Eek still copying attract sequence movie to disc - wait here
			AddTransmitter( Screen.safe.right - 32, 424, 32, 32, UIEnums.Panel._2D )
			GUI.wait_for_copy = true
		end
	elseif ( message == UIEnums.Message.ButtonStart ) or ( message == UIEnums.Message.MenuNext ) then
		if UIGlobals.Ps3InstallState == UIEnums.Ps3InstallState.NONE then
			GoScreen( "Intro\\Legal.lua" )
			PlaySfxNext()
		end
	end

end

function FrameUpdate( time )
	if GUI.wait_for_copy == true then
		if UIGlobals.Ps3InstallState == UIEnums.Ps3InstallState.COPY_DATA then
			GUI.wait_for_copy = false
			GoScreen("Intro\\AttractSplash.lua")
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
	UISystem.DestroyMovie( 0 )

	UIScreen.SetScreenTimers( 0, 0 )
end
