GUI =
{
	finished = false,
}

function Init()
	Amax.SendMessage(UIEnums.GameFlowMessage.CutsceneVideoStarted)
	RemapMovieTracks()
	SetupMovie( 0, "Ui\\Movies\\AmaxAttract.bik", nil, nil, false, true )
	
	UISystem.EnableMovieLooping( 0, false )
	
	--Amax.BGMusicCheck(0)

	UIScreen.SetScreenTimers( 0.3, 0.3 )
	UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUpDown" )
	
	SetupInfoLine()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2 )

	if GUI.finished == true then
		return
	end
		
	if ( message == UIEnums.Message.MovieFinished and arg1 == 0 ) then
		GoScreen( "SinglePlayer\\SpMain.lua" )
	elseif ( message == UIEnums.Message.ButtonStart or message == UIEnums.Message.MenuNext ) and arg2 == true then
		PlaySfxNext()
		GoScreen( "SinglePlayer\\SpMain.lua" )
	end
end

function FrameUpdate( time )
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	--
	Amax.SendMessage( UIEnums.GameFlowMessage.CutsceneVideoFinished )
	UISystem.DestroyMovie( 0 )
	
	Amax.SetUICarToMultiplayer(false)
	--Amax.SendMessage(UIEnums.GameFlowMessage.LoadUIScene)
	print("IntroMove finished, calling LoadSPProfileCar")
	Amax.SendMessage(UIEnums.GameFlowMessage.LoadSPProfileCar)
end
