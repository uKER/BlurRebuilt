UIEnums =
{
	-- profile states
	Profile = 
	{
		None					= 0,	-- no profile set
		PreLoad					= 1,	-- profile exists but not save data has NOT been loaded
		GamerProfile			= 2,	-- profile exists and save data HAS been loaded
		Blagged					= 3,	-- profile was blagged
	},
	
	ScreenStorage =
	{
		FE_BACK					= 0,
		FE_NEXT					= 1,
		SAVE_NEXT				= 2,	-- where to go after saving
	},
}

if UIHardware.WideScreen() then
	SCREEN_WIDTH = (640 * (4 / 3)) + 0.5 -- 853.333
	SCREEN_LEFT = ((640 - SCREEN_WIDTH) / 2) - 0.25
	SCREEN_RIGHT = SCREEN_LEFT + SCREEN_WIDTH
else
	SCREEN_LEFT = 0
	SCREEN_WIDTH = 640
	SCREEN_RIGHT = SCREEN_LEFT + SCREEN_WIDTH
end
