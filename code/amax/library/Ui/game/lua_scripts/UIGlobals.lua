UIGlobals =
{
	ProfileBootContext = nil,			-- context that the profile boot screen is running on
	ProfileBootFinished = false,		-- finished loading profile, checking for DLC and connected to servers
	StorageDownloadCallback = nil,		-- callback function for downloading user profiles
	CallbackDLC = nil,					-- callback for DLC, success or failure
	NetConnectionSilent = false,		-- run connection popup silent
		
	automation_timer = 0.0,				-- Debug
	automation_length = 30.0,			-- Debug
	automation_event_id = 0,			-- Debug
		
	LoadFromDebug = false,
	DebugEvent = nil,

	ReturnMode = UIEnums.ReturnMode.None,

	-- These flags control kicking the user back to the initial start screen
	UserKickBackMode	= UIEnums.UserKickBackMode.None,	-- The reason for why we are kicking the poor user back to the start screen
	UserKickBackActive	= false,							-- Means that a kick back is active
	UserKickBackScreen	= "intro\\startscreen.lua",			-- Sreen we should return the user too
	
	-- Commands can lock the controlle input when running.
	ControllerLocked	= false,
	
	-- Command variables for selecting a group event
	JumpGroupId			= -1,
	JumpEventId			= -1,
	
	--
	Ps3Installing		= true,
	
	-- Flag next screen to do fade from black
	DoFadeUp			= false,

	-- Last message char viewed
	SelectedCharacterMessageIndex	= 0,
	-- Profile friend selected
	SelectedProfileFriendID			= -1,
	
	-- Group Creator mode
	GroupCreatorMode = UIEnums.GcMode.Editing,
	GroupCreatorSlot = 0,
	-- To calculate U coordinates along the texture
	NumMajorAvatarIcons = 32,
	NumMinorAvatarIcons = 64,
	-- Multiplayer mode
	-- Mutiplayer connection type 
	ConnectionType	= UIEnums.ConnectionType.None,
	-- global checking for mod shop keyboard finished so we can save if we get kicked into a game
	MpModShopKeyboardCheck = -1,

	MpLobbyMenuSelection = nil, -- current lobby menu item
	
	StorageFailType = UIEnums.StorageFailType.Generic,
	
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
	-- [Blur2PC] Retail PC remembers the device that joined each split-screen profile.
	ProfileDevice = { [ 0 ] = -1, [ 1 ] = -1, [ 2 ] = -1, [ 3 ] = -1 },
	LastInputDevice = -1,
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
	
	-- Screen timers
	screen_time =
	{
		default_start	= 0.15,
		default_end		= 0.15,
		
		popup_start		= 0.15,
		popup_end		= 0.15,
	},
	-- Screen storage area
	ScreenStore =
	{
		-- Filled in by GetStoredScreen()
	},
	-- Screen options
	ScreenOptions = 
	{
		-- Filled by each screen that needs to save a users positions (options)
	},	
	-- Ingame tempories cleared when we go back into the UI
	Ingame = 
	{
		-- Filled by RaceResults, RaceRules etc.
	},	
	-- Is the game loading, don't want to be forced back to the UI if we are currently loading.
	IsLoading = false,
	
	-- are we in game play
	IsIngame = false,
	
	-- goes to true during a quick restart
	IsQuickRestart = false,

	-- Was the player just kicked from a MP game for idling?
	IdlePlayerKicked = false, -- Used to detect when player has been kicked
		
	-- used to connect to platform servers.
	server_connection = 
	{
		online_mode		= false,
		callback		= nil,
		active			= false,
	},

	ModShopFlag				= 0,
	
	-- SinglePlayer sub screen modes
	SpSelectMode_Tier			= 0,
	SpSelectMode_Event			= 1,
	SpSelectMode_PreRace		= 2,
	SpSelectMode_Garage			= 3,
	SpSelectMode_FriendDemand	= 4,

	-- All multiplayer globals should go in here
	Multiplayer =
	{
		LaunchScreen				= UIEnums.MpLaunchScreen.None,
		EventFilter					= UIEnums.MpEventFilter.None,
		PublicGame					= true,
		SelectPlayerJoinRef			= -1,	 -- join reference of the player we wish to view
		ChallengePackViewing		= -1,
		RateEvent					= false,
		IngameModShop				= false,
		
		ModShopActive				= false, -- flags so lobby knows when they end
		GarageActive				= false, -- flags so lobby knows when they end

		NotEnoughPlayersKick		= false,
		InitialLobby				= false,
		LobbyState					= UIEnums.MpLobbyState.FindingPlayers,
		LobbyTimer					= 31,
		LobbyUpdateLoadout			= false,
		LobbyUpdateCar				= false,
		CountdownStarted			= false,

		InLobby						= false, -- used for disconnection message
		InPartyLobby				= false,
		RaceFinished				= false,
		PrimaryVehicleFinished		= false,
		RaceTimeout					= false,
		ViewingResults				= false,
		CanEquipMods				= false,
		
		LobbyConnectionLost			= false,
		Disconnecting				= false,
		DisconnectRaceAsPartyHost	= false, -- used in the disconnecting from race popup. are we the party host.
	},
	
	SplitscreenMessages =
	{
		-- see Splitscreen_ClearMessages()
	},
	
	splitscreen_primary_pad_original = -1,	-- for saving if primary profile has changed on sign-in screen
	
	Splitscreen =
	{
		primary_pad	= -1,	-- for returning the correct player control after a race
		can_vote = false,
		
		players =
		{
			-- this for each player
			{
				pad = nil,
				car_id = nil,
				colour_id = nil,
			},
		},
		pad_to_player =
		{
			-- deref by pad into players table above
		},
		
		-- ui colours
		colours = 
		{
			[ 0 ] = "Main_1",
			[ 1 ] = "Support_4",
			[ 2 ] = "Support_3",
			[ 3 ] = "Support_0",
		},
		inv_colours = 
		{
			[ 0 ] = "Main_Black",
			[ 1 ] = "Main_Black",
			[ 2 ] = "Main_Black",
			[ 3 ] = "Main_Black",
		},
	},
	
	Network = 
	{
		JoinErrorCode						= 0,
		JoinVersionErrorCode				= 0,
		LanEnumeratedGameIndexToJoin		= 0,
		MatchingEnumeratedGameIndexToJoin	= 0,
		MatchingNumSesionsEnumerated		= 0,
		GlobalUpdateEnabled					= false,
		CurrentRichPresence					= 0,		-- the last RP value we set, need to set this every frame 
		RichPresenceUpdateEnabled			= false,
		UnrecoverableSessionError			= false,
		RouterMonitoringEnabled				= false,
		UnrecoverableRouterError			= false,
		LockoutFriendsOverlay				= false,
		PartyLobbyMatchingDelay				= 0,		
		GlobalUpdatePartyCommands			= true,

		UserGroupsEnumerated				= 0,
		UserGroupEnumeratedIndex			= 0,
		
		AgeConnectionError					= false,
		VoiceChatRestricted					= false,

		GameInviteState						= UIEnums.MpGameInviteState.Polling,
		ReturnPartyLobbyState				= UIEnums.MpReturnPartyLobbyState.Polling,
		RaceInviteState						= UIEnums.MpRaceInviteState.Polling,
		GameInviteType						= UIEnums.MpGameInviteType.Unknown,
	},

	TeamColours =
	{
		["team_a"] = "Team_A",
		["team_b"] = "Team_B",
	},
	TeamIcons =
	{
		["team_a"] = { u = 0.0, v = 0.0 },
		["team_b"] = { u = 0.5, v = 0.0 },
	},
	
	FontSize = 
	{
		-- setting and lerp from Sorcery using fixed index
		[ 1 ] = 12,
		[ 2 ] = 16,
		[ 3 ] = 24,
		[ 4 ] = 32,
		[ 5 ] = 64,
		[ 6 ] = 86,
		[ 7 ] =120,
	
		-- setting individually
		VerySmall	= 12,
		Small		= 16,
		Medium		= 24,
		Large		= 32,
		VeryLarge	= 64,
		QuiteSilly	= 86,
		JustSilly	= 120,
	},
	
	-- Default interpolation time
	CameraLerpTime	= 0.3,
	-- Actor node names
	CameraNames = 
	{
		-- Singleplayer
		SpCarousel	= "Sp_5",
		-- Multiplayer
		MpCarousel	= "Mp_5",
		MpAppBase	= "Mp_4",
		MpLobby		= "Sp_1",
		MpOnline	= "Mp_4",
	},
	
	-- Racebook flags
	ToolBarActive = false,

	--Debug Event Loading Flag
	AlreadySetupRace = false,
	
	MessagesAlert = 
	{
		-- filled in when we have an alert
		title_id = nil, 
		message_id = nil,
	},
	CircuitTypeNames = 
	{
		[UIEnums.UiCircuitType.Sweeping] =	UIText.CMN_CIRCUIT_TYPE_SWEEPING,
		[UIEnums.UiCircuitType.Tight] =		UIText.CMN_CIRCUIT_TYPE_TIGHT,
		[UIEnums.UiCircuitType.OffRoad] =	UIText.CMN_CIRCUIT_TYPE_OFF_ROAD,
		[UIEnums.UiCircuitType.Hilly] =		UIText.CMN_CIRCUIT_TYPE_HILLY,
		[UIEnums.UiCircuitType.Fast] =		UIText.CMN_CIRCUIT_TYPE_FAST,
		[UIEnums.UiCircuitType.Technical] =	UIText.CMN_CIRCUIT_TYPE_TECHNICAL,
		[UIEnums.UiCircuitType.Urban] =		UIText.CMN_CIRCUIT_TYPE_URBAN,
		[UIEnums.UiCircuitType.Drifty] =	UIText.CMN_CIRCUIT_TYPE_DRIFTY,
		[UIEnums.UiCircuitType.Varied] =	UIText.CMN_CIRCUIT_TYPE_VARIED,
		[UIEnums.UiCircuitType.Oval] =		UIText.CMN_CIRCUIT_TYPE_OVAL,
		[UIEnums.UiCircuitType.Straight] =	UIText.CMN_CIRCUIT_TYPE_STRAIGHT,
	},
	
	GarageCheatReload = false,
	GarageFromMessageLink = false,	
	GarageMiniClassFilter = {  },	-- filter for class in mini-garage
	GarageMiniFromPause = false,

	TwitterClose = false,	-- force close twitter from an error
	TwitterBackToTweetScreen=false,
	
	FacebookBackToStoryScreen=false,

	FacebookClose = false,	-- force close facebook from an error
	
	CategoryColour =
	{
		[UIEnums.MpAbilitySlot.Slot1] = "Slot1",
		[UIEnums.MpAbilitySlot.Slot2] = "Slot2",
		[UIEnums.MpAbilitySlot.Slot3] = "Slot3",
	},

	ClassNames =
	{
		[UIEnums.MpVehicleClass.ClassA] = UIText.MP_CLASS_A,
		[UIEnums.MpVehicleClass.ClassB] = UIText.MP_CLASS_B,
		[UIEnums.MpVehicleClass.ClassC] = UIText.MP_CLASS_C,
		[UIEnums.MpVehicleClass.ClassD] = UIText.MP_CLASS_D,
		[UIEnums.MpVehicleClass.Any] = UIText.MP_CLASS_ANY,
	},

	ClassIcons =
	{
		[UIEnums.MpVehicleClass.ClassA] = "class_a",
		[UIEnums.MpVehicleClass.ClassB] = "class_b",
		[UIEnums.MpVehicleClass.ClassC] = "class_c",
		[UIEnums.MpVehicleClass.ClassD] = "class_d",
		[UIEnums.MpVehicleClass.Any]	= "all_classes",
	},
	
	CarStyles =
	{
		[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
		[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
		[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
		[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
		[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
		[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
	},
	
	CarUpgradeIcons = 
	{
		[ UIEnums.MpChallengeRewards.HealthSlot]	= "health",
		[ UIEnums.MpChallengeRewards.BonusFans]		= "extra_slot",
		[ UIEnums.MpChallengeRewards.SpeedBoost]	= "horsepower",
		[ UIEnums.MpChallengeRewards.Pearlescent]	= "pearl",
		[ UIEnums.MpChallengeRewards.Metallic]		= "metallic",
		[ UIEnums.MpChallengeRewards.Candy]			= "candy",
	},
	
	ScoreboardLookingAt = -1,
	
	PopupSpawned = false,	-- set when called PopupSpawn()

	AuthenticateFromPhoto=false,
	
	MaxTweets=100,
	MaxStories=100,
	MaxBlurbs=100,

	PhotoDoMyLIst=true,

	FinishedVoting=false,

	SharingOptionsChosen = -1,

	RivalsCountdownActive = false,
	CurrentFriendDemandMessage = -1,
	CurrentFriendDemandEvent = -1,
	CurrentFriendFreeSlot = -1,
	CurrentFriendDemandMessageIndex = -1,
	
	CurrentFacebookStory = -1,
	SendFacebookStory = false,

	RaceHistoryClose=false,

	ActiveSPMovie =
	{
		{
			Filename = -1,
			Skipable = -1,
			FullScreen = -1,
		},
		{
			Filename = -1,
			Skipable = -1,
			FullScreen = -1,
		},
		{
			Filename = -1,
			Skipable = -1,
			FullScreen = -1,
		},
		{
			Filename = -1,
			Skipable = -1,
			FullScreen = -1,
		},
		{
			Filename = -1,
			Skipable = -1,
			FullScreen = -1,
		},
	},

	CanShowPreviouslyOnBlur = true,
	RefreshFriendDemandScreen = false,

	CustomRaceSettings = {},
	
	FriendDemandSwitchChosen = -1,

	FriendDemandFriends = {},
	
	FriendDemandFilter = 0,
	FriendDemandFilterFriend = 0,
	FriendDemandFilterEvent = 0,
	
	FriendDemandCurrentType = 0,
	FriendDemandAttemptingIndex = 0,

	SpLeaderboardInCarousel = false,
	SpLeaderboardCarouselFocusOnMenu = true,
	
	FriendDemandSent = false,
	
	FriendDemandAttemptFromMessage = false,
	
	FriendDemandAttemptFromServer = false,
	
	ShareFromWhatPopup = -1,
	
	SPMovieNextStartScreen = false,
	SpMovieScreenShouldSave = false,
	
	FacebookPhotoUploadStage = 0,
	FacebookUploadingPhoto = false,
	
	FacebookLocked = false,
	TwitterLocked = false,
	
	ShareShowTier = false,
	OnMovieScreenInEventSelect = false,
	
	UpdateEventOwners = false,
	UpdateEventOwnersPreRace = false,
	
	SPPostRaceLeaderboardDone = false,
	
	DoResults = true,

	boss_avatar_ids = 
	{
		16,		-- Shannon
		6,		-- Khan
		2,		-- Rhymer
		3,		-- Drayke
		11,		-- Natalya
		12,		-- Talon
		14,		-- Ayumi and Harumi
		1,		-- Carver
		22,		-- Shannon Evil
	},
	
	ActUponFriendChallenges = true,
	
	PendingFriendChallengeInfo =
	{
	},
	
	Mp_SpectateCurrentVehicle = 0,
	Mp_SpectateCurrentCamera = 1,
	
	SPRA_UpdateRival = false,
	
	ShowingUnlocks = false,

	LeaderboardReTryPassed = nil,
}



