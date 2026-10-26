GUI =
{
	finished = false,

	StartGame = function()
		GoScreen( "Intro\\BizarreSplash.lua" )
	end,
}

function Init()
		print("PS3 : Install attract movie")
		UIGlobals.Ps3InstallState = UIEnums.Ps3InstallState.COPY_ATTRACT
		Amax.Ps3PrimeInstaller( false )
	
	Profile.AllowProfileChanges( true )
	Profile.ActOnProfileChanges( false )
	Profile.AllowAllPadInput( true )
	
	AddSCUI_Elements( )
	
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUpDown" )

	SetupMovie( 0, "Ui\\Movies\\atvi.bik", nil, nil, false, true )
	UISystem.EnableMovieLooping( 0, false )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2 )

	if GUI.finished == true then
		return
	end
	
	if ( message == UIEnums.Message.MovieFinished and arg1 == 0 ) then
		GUI.StartGame()
	elseif ( message == UIEnums.Message.ButtonStart ) or ( message == UIEnums.Message.MenuNext ) then
		if UIGlobals.Ps3InstallState == UIEnums.Ps3InstallState.NONE then
			PlaySfxNext()
			GUI.StartGame()
		end
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
	UISystem.DestroyMovie( 0 )
end
