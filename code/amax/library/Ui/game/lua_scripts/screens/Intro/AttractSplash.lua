GUI =
{
	finished = false,
}

function Init()
	Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)

	UIScreen.SetScreenTimers( 0.3, 0.3 )
	UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUpDown" )

	Amax.SendMessage(UIEnums.GameFlowMessage.CutsceneVideoStarted)

	RemapMovieTracks()
	SetupMovie( 0, "Ui\\Movies\\AmaxAttract.bik", nil, nil, false, true )
	UISystem.EnableMovieLooping( 0, false )
	
	--Amax.BGMusicCheck(0)
end

function StartLoop( time )
	--
end

function AttractSplash_GoStartScreen()
	Amax.SetLoadingScreen(false)
	Amax.SendMessage(UIEnums.GameFlowMessage.StartGameRendering)
	GoScreen( "Intro\\StartScreen.lua" )
end

function MessageUpdate( message, arg1, arg2 )

	if GUI.finished == true then
		return
	end
		
	if ( message == UIEnums.Message.MovieFinished and arg1 == 0 ) then
		AttractSplash_GoStartScreen()
	elseif ( message == UIEnums.Message.ButtonStart or message == UIEnums.Message.MenuNext ) and arg2 == true then
		PlaySfxNext()
		AttractSplash_GoStartScreen()
	end
end

function FrameUpdate( time )
	--
	if GUI.finished == true then
		return
	end
	
	if NetServices.GameInvitePending() == true then
		AttractSplash_GoStartScreen()
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
	AttractSplash_GoStartScreen = nil
	Amax.SendMessage( UIEnums.GameFlowMessage.CutsceneVideoFinished )
	UISystem.DestroyMovie( 0 )
	UIGlobals.DoFadeUp = true
end
