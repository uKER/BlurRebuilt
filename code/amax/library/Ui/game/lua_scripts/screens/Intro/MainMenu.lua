GUI =
{
	finished = false,
	
	Mode = 
	{
		SinglePlayer	= 1,
		Multiplayer		= 2,
	},
	CurrentMode = 0,
	
	EffectName =
	{
		"SelectSP",
		"SelectMP",
	},
}

function Init()
	AddSCUI_Elements( )

	-- yet to choose a game mode.
	Amax.SetGameMode( UIEnums.GameMode.Nothing )

	-- No input	
	UIButtons.SetSelected( SCUI.name_to_id[ "CaraouselSP" ], false )
	UIButtons.SetActive( SCUI.name_to_id[ "CaraouselSP" ], false )
	
	UIButtons.SetSelected( SCUI.name_to_id[ "CaraouselMP" ], false )
	UIButtons.SetActive( SCUI.name_to_id[ "CaraouselMP" ], false )
	
	SetupMenuOptions()
	
	-- tempory hack.
	RaceBook.LookingAtGroup( 529879209 )
	
	-- cleanup just incase we got here by error
	net_FlushEverything( )
	
	-- enable global update network logic, game invites etc..
	net_EnableGlobalUpdate( true )
	
end

function StartLoop( time )
	StoreScreen( UIEnums.ScreenStorage.FE_HUB ) -- Store current hub
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then	
		GoScreen( "Intro\\StartScreen.lua" )
		
	elseif message == UIEnums.Message.MenuNext and arg2 == true then	
		-- Singleplayer
		if GUI.CurrentMode == GUI.Mode.SinglePlayer then
			Amax.SetGameMode( UIEnums.GameMode.SinglePlayer )
			GoScreen( "RaceWorld\\SpMain.lua" )
			
		-- Multiplayer
		elseif GUI.CurrentMode == GUI.Mode.Multiplayer then
			GoScreen( GUINetwork.multiplayer_root )
		end
	
	elseif message == UIEnums.Message.ButtonLeft and arg2 == true then
		-- Singleplayer
		if GUI.CurrentMode ~= GUI.Mode.SinglePlayer then
			GUI.CurrentMode = GUI.Mode.SinglePlayer
			
			-- play animation
			UIButtons.TimeLineActive( GUI.EffectName[ GUI.Mode.SinglePlayer ], true )
			UIButtons.TimeLineActive( GUI.EffectName[ GUI.Mode.Multiplayer ], false )
		end
			
	elseif message == UIEnums.Message.ButtonRight and arg2 == true then
		-- Multiplayer
		if GUI.CurrentMode ~= GUI.Mode.Multiplayer then
			GUI.CurrentMode = GUI.Mode.Multiplayer
			
			-- play animation
			UIButtons.TimeLineActive( GUI.EffectName[ GUI.Mode.Multiplayer ], true )
			UIButtons.TimeLineActive( GUI.EffectName[ GUI.Mode.SinglePlayer ], false )
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
end
