GUI =
{
	finished = false,
	
	RollCredits = false,
}

function Init()
	AddSCUI_Elements()
		
	UIGlobals.OnMovieScreenInEventSelect = true

	if UIGlobals.ActiveSPMovie[1].Filename == -1 then
		Print("ERROR: ACTIVE SP MOVIE HAS NO FILENAME, LEAVING SP MOVIE SCREEN")
		finished = true
		return
	end

	StoreInfoLine()
	SetupInfoLine()

	--StopFrontendMusic()
	UISystem.DestroyMovie( 0 )	

	if UIGlobals.ActiveSPMovie[1].FullScreen == true then
		UIScreen.SetScreenTimers( 0.3, 0.3 )
		UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUpDown" )
	end

	Amax.SendMessage(UIEnums.GameFlowMessage.CutsceneVideoStarted)

	Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)
	
	RemapMovieTracks()
	print(UIGlobals.ActiveSPMovie[1].Filename)
	if UIGlobals.ActiveSPMovie[1].FullScreen == true then
		SetupMovie( 0, UIGlobals.ActiveSPMovie[1].Filename, nil, nil, false, true )
	else
		UIButtons.SetActive( SCUI.name_to_id[ "vid" ], true) 
		UISystem.InitMovie( 0, UIGlobals.ActiveSPMovie[1].Filename, false, nil, false, true )
	end
	UISystem.EnableMovieLooping( 0, false )

	UIScreen.BlockInputToContext( true )
	
	if UIGlobals.SPRollCredits == true then
		GUI.RollCredits = true
	end 
end

function PostInit()
	UIButtons.TimeLineActive( "movie_start", true )
	
	print("Should Save ",UIGlobals.SpMovieScreenShouldSave)
	if UIGlobals.SpMovieScreenShouldSave == true then
		--save so we know we have played the video if the player turns off console after video
		print("Saving")
		StartAsyncSave()
		SpMovieScreenShouldSave = false
	end
	
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2 )

	if GUI.finished == true then
		PopScreen( )
		return
	end
		
	if ( message == UIEnums.Message.MovieFinished and arg1 == 0 ) then
		if CheckQueuedMovies() == true then

		else
			if GUI.RollCredits == true then
				UIGlobals.SPRollCredits = false
				UIGlobals.EndOfGame = true
				GoScreen("Shared\\Options_Credits.lua")				
			else
				PopScreen( )
			end
		end
		
	elseif ( message == UIEnums.Message.ButtonStart or message == UIEnums.Message.MenuNext ) and arg2 == true then
			if UIGlobals.ActiveSPMovie[1].Skipable == true or UIGlobals.DevMode == true then
				if CheckQueuedMovies() == true then
					PlaySfxNext()
				else
					PlaySfxNext()									
					if GUI.RollCredits == true then
						UIGlobals.SPRollCredits = false
						GoScreen("Shared\\Options_Credits.lua")				
					else
						PopScreen()
					end
				end
			end
	end
end

function FrameUpdate( time )
	--
	if GUI.finished == true then
		return
	end
end

function Render()
	--
end

function CheckQueuedMovies()

	UIGlobals.ActiveSPMovie[1].Filename = -1
	UIGlobals.ActiveSPMovie[1].Skipable = -1
	UIGlobals.ActiveSPMovie[1].FullScreen = -1

	for i=1, #UIGlobals.ActiveSPMovie do
		if IsTable( UIGlobals.ActiveSPMovie[i] ) == true then
			if IsTable( UIGlobals.ActiveSPMovie[i+1] ) == true then
				UIGlobals.ActiveSPMovie[i].Filename = UIGlobals.ActiveSPMovie[i+1].Filename
				UIGlobals.ActiveSPMovie[i].Skipable = UIGlobals.ActiveSPMovie[i+1].Skipable
				UIGlobals.ActiveSPMovie[i].FullScreen = UIGlobals.ActiveSPMovie[i+1].FullScreen
			else
				UIGlobals.ActiveSPMovie[i].Filename = -1
				UIGlobals.ActiveSPMovie[i].Skipable = -1
				UIGlobals.ActiveSPMovie[i].FullScreen = -1
			end
		end
	end

	if UIGlobals.ActiveSPMovie[1].Filename ~= -1 then -- play the next video

		UISystem.DestroyMovie( 0 )	

		if UIGlobals.ActiveSPMovie[1].FullScreen == true then
			UIScreen.SetScreenTimers( 0.3, 0.3 )
		end

		Amax.SendMessage(UIEnums.GameFlowMessage.CutsceneVideoStarted)

		RemapMovieTracks()
		print(UIGlobals.ActiveSPMovie[1].Filename)
		if UIGlobals.ActiveSPMovie[1].FullScreen == true then
			SetupMovie( 0, UIGlobals.ActiveSPMovie[1].Filename, nil, nil, false, true )
		else
			UIButtons.SetActive( SCUI.name_to_id[ "vid" ], true) 
			UISystem.InitMovie( 0, UIGlobals.ActiveSPMovie[1].Filename, false, nil, false, true )
		end
		UISystem.EnableMovieLooping( 0, false )

		return true
	else	
		return false
	end
end

function EndLoop( time )
	--
end

function EnterEnd()
end

function End()
	SinglePlayer.ClearUnlocks()
	--StartFrontendMusic()
	
	if UserKickBackActive() == false and Sp_ReturnFromGame() == true and GUI.RollCredits == false and UIGlobals.EndOfGame ~= true then
		OpenApp( UIEnums.RwBranch.Races, nil, "SinglePlayer\\SpTierSelect.lua" )
	end

	RestoreInfoLine()

	Amax.SendMessage(UIEnums.GameFlowMessage.StartGameRendering)

	Amax.SendMessage( UIEnums.GameFlowMessage.CutsceneVideoFinished )
	UISystem.DestroyMovie( 0 )

	if UIGlobals.ActiveSPMovie.FullScreen == true then
		UIGlobals.DoFadeUp = true
	end

	-- Clear The current Movie from UIGlobals
	UIGlobals.ActiveSPMovie[1].Filename = -1
	UIGlobals.ActiveSPMovie[1].Skipable = -1
	UIGlobals.ActiveSPMovie[1].FullScreen = -1
	

	UIGlobals.OnMovieScreenInEventSelect = false
end
