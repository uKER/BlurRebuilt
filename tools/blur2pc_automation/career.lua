-- [Blur2PC] Automation helper: reach Career (the tier select screen) through the real menus.
--
-- The front end does not always take the same path: "Previously on Blur" (UI context 6) sometimes
-- opens after the profile loads and waits for a key, sometimes flashes and closes on its own, and the
-- profile save (context 2) can hold input for a moment. So the menus are driven by what is on screen:
--     0 lua Blur2PC.Run([[<this file>]])
--     ...                                   (quickrace.lua's start-screen waits)
--     0 wait 60 Blur2PC_GoCareer()
-- Blur2PC_GoCareer() is polled once per UI frame by the `wait`; it presses RETURN (Blur2PC.Tap) at most
-- once per 60 polls, only when the screen on top would take it, and returns true on the tier select screen.

local polls = 60

-- one tap per 60 polls (about a second of UI frames), so a screen has time to react before the next
local function tapOnce()
	if polls >= 60 and Blur2PC.Tap( "RETURN" ) then
		polls = 0
	end
end

function Blur2PC_GoCareer()
	polls = polls + 1
	local main = UIScreen.GetCurrentScreen( 0 )
	local career = UIScreen.GetCurrentScreen( 5 )
	local overlay = UIScreen.GetCurrentScreen( 6 )
	local saving = UIScreen.GetCurrentScreen( 2 ) == [[profile\asyncsave.lua]]
	if career == [[singleplayer\sptierselect.lua]] then
		return true
	end
	if saving or UIScreen.IsPopupActive() then
		return false
	end
	if overlay == [[intro\previouslyonblur.lua]] then
		tapOnce()				-- dismiss it
	elseif main == [[intro\startscreen.lua]] and GUI ~= nil and GUI.current_stage == GUI.STAGE_MODE_SELECT then
		tapOnce()				-- single player (the default selection)
	elseif main == [[singleplayer\spmain.lua]] then
		tapOnce()				-- career (the default carousel item)
	end
	return false
end

return true
