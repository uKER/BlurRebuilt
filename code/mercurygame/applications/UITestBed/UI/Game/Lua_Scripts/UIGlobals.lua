UIGlobals =
{
	ScreenStore = {  },
	
	-- have we kicked them back for changing profile?
	ShowUserChangedPopup = false,
	
	-- the person that pressed start
	ProfilePressedStart = 0,
	-- primary pad
	ProfilePrimaryPad = 0,
	-- number of active profiles detected
	ProfilesFound = 0,
	-- profile states
	ProfileState =
	{
		[ 0 ] = UIEnums.Profile.None,
		[ 1 ] = UIEnums.Profile.None,
		[ 2 ] = UIEnums.Profile.None,
		[ 3 ] = UIEnums.Profile.None,
	},
	-- for Profile\\AsyncLoad.lua - only load in certain profiles
	LoadProfile =
	{
		[ 0 ] = false,
		[ 1 ] = false,
		[ 2 ] = false,
		[ 3 ] = false,
	},
	-- which profile we are looking to load. -1 to only load the person who presses start - flagging the rest as PreLoad
	ProfileLookingFor = -1,
	-- used if some pulls an MU chooses not to pick another storage device on first prompt - or they have no free space
	DoNotSave =						
	{
		[ 0 ] = false,
		[ 1 ] = false,
		[ 2 ] = false,
		[ 3 ] = false,
	},
	-- who we are attempting to save out
	ProfileSaveIndex = 0,
	-- who we are attempting to enumerate
	ProfileEnumerateIndex = 0,
	-- size of the profile
	ProfileSize = 200 * 1024,	-- TODO - push in from the game
	-- check overwrites
	ProfileCheckOverwrites = false,
	-- save out to list - PS3 only
	ProfileSaveToList = false,
}
