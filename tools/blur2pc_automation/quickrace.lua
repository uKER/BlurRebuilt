-- [Blur2PC] Automation helper: start a career event straight from the start screen.
--
-- Run from an automation script (see system/debug/Blur2PCAutomation.h) once the profile has
-- loaded and the start screen shows the mode select:
--     0 lua Blur2PC.Run([[<this file>]])
--     0 wait 120 Blur2PC_PressStartReady()
--     0 lua Blur2PC_PressStart()
--     0 wait 60 Blur2PC_QuickRaceReady()
--     0 lua Blur2PC_QuickRace(1, 1)
--
-- It makes the calls the career menus make on the way to a race, in their order:
--   StartScreen.lua (MenuNext, single player)   SetGameMode, SetPlayingSpGameLatest, EnterSpRaceBook
--   SpTierSelect.lua / SpEventSelect.lua         UIGlobals.Sp, SetCurrentSpTier, SetCurrentEvent
--   SpPreRace.lua (MenuNext)                     SpSelectMode_Garage, SetSPEvent
--   Garage.lua Garage_End / Garage_SpSetCar      SetCurrentCar, EventSetLastVehicle, GameSetLastVehicle,
--                                                MarkNotNew, SpGoLoadingScreen, SpReturnMode
-- The race itself is the stock career event: no debug-event upgrades (compare Amax.SetupDebugRace).

-- true when the start screen shows "Press START" and has finished its transition
function Blur2PC_PressStartReady()
	return UIScreen.GetCurrentScreen() == [[intro\startscreen.lua]]
		and UIScreen.GetScreenActive( UIEnums.Context.Main ) == true
		and GUI ~= nil and GUI.current_stage == GUI.STAGE_PRESS_START
		and UIScreen.IsPopupActive() == false
end

-- presses Confirm on pad 0 (retail's start screen continues on MenuNext only, T146)
function Blur2PC_PressStart()
	ContextTable[ UIEnums.Context.Main ].MessageUpdate( UIEnums.Message.MenuNext, 0, true )
end

-- true when the start screen has finished loading the profile and waits for the mode select
function Blur2PC_QuickRaceReady()
	return UIScreen.GetCurrentScreen() == [[intro\startscreen.lua]]
		and UIGlobals.ProfileBootFinished == true
		and GUI ~= nil and GUI.current_stage == GUI.STAGE_MODE_SELECT
		and UIScreen.IsPopupActive() == false
end

-- tier, event: career indices as the tier and event menus count them (1-based)
-- car: vehicle id; nil takes the event's last vehicle, which the garage preselects
function Blur2PC_QuickRace( tier, event, car )
	-- StartScreen.lua, single player selected
	Amax.SetGameMode( UIEnums.GameMode.SinglePlayer )
	Amax.SetPlayingSpGameLatest( true )
	EnterSpRaceBook( false )

	-- SpTierSelect.lua / SpEventSelect.lua
	UIGlobals.Sp = { TierInfo = SinglePlayer.TierInfo(), CurrentTier = tier, CurrentEvent = event }
	Amax.SetCurrentSpTier( tier )
	local sp_event = UIGlobals.Sp.TierInfo[ tier ].events[ event ]
	local info = SinglePlayer.EventInfo( sp_event )
	Amax.SetCurrentEvent( info.eventId )

	-- SpPreRace.lua
	UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_Garage
	Amax.SetSPEvent( sp_event )

	-- Garage.lua
	car = car or info.lastVehicle
	Amax.SetCurrentCar( car )
	SinglePlayer.EventSetLastVehicle( sp_event, car )
	SinglePlayer.GameSetLastVehicle( car )
	SinglePlayer.MarkNotNew( sp_event )
	SpGoLoadingScreen()
	UIGlobals.SpReturnMode = UIGlobals.SpSelectMode_Event

	Blur2PC.Log( "quick race: tier " .. tier .. " event " .. event .. " (sp event " .. tostring( sp_event ) .. ", event id " .. tostring( info.eventId ) .. ") car " .. tostring( car ) )
end

return true
