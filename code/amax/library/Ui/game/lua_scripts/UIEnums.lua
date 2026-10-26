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
		SAVE_BACK				= 3,	-- where to go after cancelling saving
		LOAD_NEXT				= 4,	-- where to go after loading
		FE_RETURN				= 5,	-- Normal return from game to FE
		FE_HUB					= 6,	-- hub can define different portions of the game, time attack, career etc.
		FE_GAME_LOADING			= 7,	-- See PrepLoading.lua
		FE_SOCIAL_NETWORK		= 8,	-- after we Tweet/Facebook - where do we go back to?
		FE_FRIEND_DEMAND		= 9,	-- when we create a friend demand post race
	},
	
	ExtendedCameraType =
	{
		IntoGarage				= 0,	-- Interpolate to the garage camera
	},
	-- Connection type
	ConnectionType = 
	{
		None					=-1,
		CustomSearch			= 0,
		HostGame				= 1,
		QuickMatch				= 2,
	},
	MpOnlineGameCreation = 
	{
		BandwidthScan		= 0,
		InitialDelay		= 1,
		Creating			= 2,
		SyncPartyStart		= 3,
		SyncPartyEnd		= 4,
		SyncPartyJoinNext	= 5,
		SyncPartyCreate		= 6,
		Complete			= 7,
		Failed				= 8,
	},
	MpOnlineGameJoining =
	{
		BandwidthScan		= 0,
		JoinStart			= 1,
		JoinContinue		= 2,
		SyncPartyStart		= 3,
		SyncPartyJoinNext	= 4,
		SyncPartyCreate		= 5,
		SyncPartyEnd		= 6,
		Complete			= 7,
		Failed				= 8,
	},

	MpOnlineGameEnumeration = 
	{
		BandwidthScan			= 0,
		Enumerating				= 1,
		Complete				= 2,
	},
	MpOnlineQuickMatching = 
	{
		TitleStorage			= 0,
		BandwidthScan			= 1,
		Enumerating				= 2,
		JoinStart				= 3,
		JoinContinue			= 4,
		Complete				= 5,
		Failed					= 6,
	},
	MpOnlineMatching = 
	{
		BandwidthScan			= 0,
		JoinStart				= 1,
		JoinContinue			= 2,
		WaitingForParty			= 3,
		PartyJoinFailure		= 4,
		Complete				= 5,
	},
	MpGameInviteState = 
	{
		Polling			=	0,
		Initialising	=	1,
		PreProcessing	=	2,
		Processing		=	3,
	},
	MpReturnPartyLobbyState = 
	{
		Polling			=	0,
		Initialising	=	1,
		PreProcessing	=	2,
		Processing		=	3,
	},
	MpRaceInviteState = 
	{
		Polling			=	0,
		Initialising	=	1,
		PreProcessing	=	2,
		Processing		=	3,
	},
	MpGameInviteType = 
	{
		Unknown			=	0,
		Race			=	1,
		Party			=	2,
	},

	ModShopFlags =
	{
		None	= 0,
		Confirm = 1,
		Keep	= 2,
	},

	-- Saved to profile (max 32 bits)
	RaceBookHelpPopups = 
	{
		Overview			= 0,
	},
	
	LspState =
	{
		Ready		= 1,
		Searching	= 2,
		MetaData	= 3,
		Uploading	= 4,
		Downloading = 5,
		Finished	= 6,
	},

	PopupState =
	{
		Ready			= 1,
		Busy			= 2,
		TitleStorage	= 3,
		Failed			= 4,
		Finished		= 5,
		SilentExit		= 6,
		ServerTime		= 8,
	},

	TitleStorageState =
	{
		Busy		= 1,
		Failed		= 2,
		Finished	= 3,
	},
		
	NetTaskType	= 
	{
		TitleStorage	= 1,
		UserStorage		= 2,
	},
	
	NetTaskState =
	{
		Ready		= 1,
		Busy		= 2,
		Failed		= 3,
		Complete	= 4,
	},

	CustomPopups = 
	{
		Default									= 0,
		ExitApplication							= 1,
		ExitMultiplayer							= 2,
		NoLanGames								= 3,
		ExitRaceLobby							= 4,
		ExitPartyLobby							= 5,
		MultiplayerDisconnectFromRace			= 6,
		MultiplayerDisconnectFromParty			= 7,
		MultiplayerJoinLanGame					= 8,
		MultiplayerVersionError					= 9,
		MultiplayerJoinLanGameFailed			= 10,
		MultiplayerTooManyProfiles				= 11,
		MultiplayerInvalidPrivileges			= 12,
		MultiplayerOnlineAccountRequired		= 13,
		MultiplayerCreateFailed					= 14,
		MultiplayerCreateFromPartyFailed		= 15,
		MultiplayerCreateOnlineGame				= 16,
		MultiplayerSearchOnlineGame				= 17,
		MultiplayerJoinOnlineGame				= 18,
		MultiplayerMatching						= 19,
		MultiplayerQuickMatching				= 20,
		MultiplayerNoMatchingGames				= 21,
		MultiplayerJoinMatchingGame				= 22,
		MultiplayerJoinMatchingGameFailed		= 23,
		MultiplayerAcceptGameInvitation			= 24,
		MultiplayerAcceptGameInvitationFailed	= 25,
		MultiplayerUnrecoverableError			= 26,
		MultiplayerRouterError					= 27,
		MultiplayerCreateOnlineParty			= 28,
		MultiplayerPartyTooBig					= 29,
		SaveEventChanges						= 30,
		SaveNewEvent							= 31,
		DeleteGroup								= 32,
		MaximumEventsReached					= 33,
		NoUserGroups							= 34,
		BuyUpgrade								= 35,
		MultiplayerNoEvents						= 36,
		MultiplayerLanSearching					= 37,
		MultiplayerE3Searching					= 38,
		ExitRace								= 39,
		ProfilePreLoad							= 40,
		ProfileKickedYou						= 41,
		MessagesAlert							= 42,
		OptionsChangeStorageDevice				= 43,
		GroupsCarDoNotHaveOne					= 44,
		ContentServerData						= 45,
		SplitscreenResetScores					= 46,
		ServiceConnectionLost					= 47,
		GarageBuyVehicle						= 48,
		UploadUserGroup							= 49,
		UploadUserEvent							= 50,
		UserEventSearch							= 51,
		DownloadUserEvents						= 52,
		ContentServerError						= 53,
		ContentServerCantUpload					= 54,
		GarageMiniBuyVehicle					= 55,
		ContentServerNoEventsFound				= 56,
		MultiplayerRateItem						= 57,
		NoUserEvents							= 58,
		DeleteEvent								= 59,
		NoFavouriteEvents						= 60,
		NoRecentEvents							= 61,
		VerifyEventName							= 62,
		VerifyEventDesc							= 63,
		InvalidEventName						= 64,
		InvalidEventDesc						= 65,
		SubmittingEventRating					= 66,
		EventRatingComplete						= 67,
		TwitterUserUnknown						= 68,
		SaveEventChanges						= 69,
		ConfirmRemoveRace						= 70,
		ContentServerGeneralError				= 71,
		ContentServerUploadBandwidthError		= 72,
		ContentServerDownloadBandwidthError		= 73,
		ThankyouForVoting						= 74,
		ContentServerNoResultsFromSearch		= 75,
		ContentServerNoUserPhotos				= 76,
		ContentServerLeaderboardError			= 77,
		AlreadyVoted							= 78,
		OverwritePhotoServerSlot				= 79,
		RetryRace								= 80,
		TwitterPostError						= 81,
		TwitterConnectionError					= 82,
		MaxEventRaces							= 83,
		TwitterPosted							= 84,
		EventOptions							= 85,
		NetConnectionTask						= 86,
		NetConnectionError						= 87,
		NetConnectionErrorNoOnline				= 88,
		LeaderboardNoScore						= 89,
		LeaderboardNoFriends					= 90,
		ConfirmLegendMode						= 91,
		MaxFavourties							= 92,
		TwitterNotAvailable						= 93,
		PS3NoVoiceAllowed						= 94,
		EventNameOverwrite						= 95,
		EventNoRaces							= 96,
		NoLocalPhotos							= 97,
		FacebookUserUnknown						= 98,
		FacebookPostError						= 99,
		FacebookConnectionError					= 100,
		FacebookPosted							= 101,
		FacebookNotAvailable					= 102,
		EventNotUploaded						= 103,
		EventNoCarClasses						= 104,
		EventSaveWarning						= 105,
		IdlePlayerKicked						= 106,
		UploadPhotoToFacebook					= 107,
		FacebookPhotoUploadComplete				= 108,
		FacebookPhotoAlbumFull					= 109,
		ConfirmModConfig						= 110,
		DiscardEventSettings					= 111,
		DiscardEventPlaylist					= 112,
		DiscardEventPlaylistRace				= 113,
		FacebookLegal							= 114,
		TwitterLegal							= 115,
		TwitterPost								= 116,
		DeleteTwitterPost						= 117,
		FacebookPost							= 118,
		DeleteFacebookPost						= 119,
		TwitterPosting							= 120,
		FacebookPosting							= 121,
		NetConnectionErrorNoLSP					= 122,
		CheckForDLC								= 123,
		ErrorWithDLC							= 124,
		SaveNewRace								= 125,
		DiscardEventChanges						= 126,
		ModShopOptions							= 127,
		MatchMakingOptions						= 128,
		LobbyOptions							= 129,
		ViewPlayer								= 130,
		MpBetaExpired							= 131,
		AcceptPartyRaceInvite					= 132,
		SharingOptions							= 133,
		MPOnlineSharingOptions					= 134,
		MPLobbyLocalSharingOptions				= 135,
		TwitterAuthenticated					= 136,
		SendingFriendDemand						= 137,
		FriendDemandSent						= 138,
		FriendDemandSendingError				= 139,
		FriendDemandRetrieving					= 140,
		FriendDemandMoreOnServer				= 141,
		FriendDemandRetrievingError				= 142,
		FriendDemandDelete						= 143,
		FriendDemandDeleting					= 144,
		SpChangeDifficulty						= 145,
		MultiplayerOnlineConnectionLost			= 146,
		DiscardModshopLoadoutChanges			= 147,
		AutoSaveWarning							= 148,
		ActiveProfileChanged					= 149,
		SpPostRaceSharingOptions				= 150,
		AcceptingFriendDemand					= 151,
		AcceptedFriendDemand					= 152,
		SpTierSelectSharingOptions				= 153,
		SpMainSelectSharingOptions				= 154,
		SpEventSelectSharingOptions				= 155,
		NoHistory								= 156,
		OnlinePhotoSharingOptions				= 157,
		FacebookAuthenticated					= 158,
		SpPostRaceOptions						= 159,
		HostChangedSettings						= 160,
		SsRetry									= 161,
		TaleOfTheTapeSharingOptions				= 162,
		FriendDemandOptions						= 163,
		FriendCompare							= 164,
		NetworkCableRemoved						= 165,
		SSLobbySharingOptions					= 166,
		FriendDemandCustomAttemptError			= 167,
		FriendDemandCustomAttemptPadError		= 168,
		ContentServerSearching					= 169,
		DiscardOptionsChanges					= 170,
		ResetToDefaults							= 171,
		LeaderboardReadError					= 172,
		FriendCompareChosen						= 173,
		FriendChallengeNoFriends				= 174,
		FriendDemandInboxOptions				= 175,
		FriendDemandNoFreeSlots					= 176,
		FriendDemandNoBlurbs					= 177,
		FriendChallengeNoPrivilege				= 178,
		FriendDemandNotOnServer					= 179,
		InvalidNetworkConnection				= 180,
		FriendDemandRemoveOldChallenges			= 181,
		ProfileUploadFailed						= 182,
		ProfileDownloadFailed					= 183,
		ReturnToLobby							= 184,
		SPLeaderboardPostRaceError				= 185,
		StorageDownload							= 186,
		StorageDownloadFailed					= 187,
		FriendDemandNoFriendsChosen				= 188,
		LostLobbyConnection						= 189,
		DisconnectFromFacebook					= 190,
		DisconnectingFromFacebook				= 191,
		FacebookDisconnected					= 192,
		TrophiesNotInstalled					= 193,
		GarageFirstModUnlocked					= 194,
		ReconnectToLeaderboards					= 195,
		FacebookAuthenticating					= 196,
		TwitterAuthenticating					= 197,
		FailedAgeCheck							= 198,
		FailedWriteToLeaderboards				= 199,
		RetryWriteToLeaderboards				= 200,
		FriendDemandRemovedFriend				= 201,
		FriendDemandNotFriend					= 202,
		ConfirmVideoSettings					= 203,	-- [Blur2PC] retail Blur PC graphics options (retail value 225)
		KillApplication							= 204,	-- [Blur2PC] retail Blur PC: quit to desktop from the start screen (T146)
	},
	
	-- Racebook message system types
	RacebookMessage =
	{
		ExitRacebook		= 0,	-- quit racebook
		ChangeApplication	= 1,	-- change context
		ChangeScreen		= 2,	-- change screen on current context
		LoadRace			= 3,	-- load the race.
		DisableRaceBook		= 4,	-- turn the exit and toolbar off.
		EnableRaceBook		= 5,	-- turn the exit and toolbar on.
	},
	
	GcMode = 
	{
		Editing		= 0,
		Creation	= 1,
	},

	MpLaunchScreen =
	{
		None				= 0,
		MultiplayerRoot		= 1,
		MultiplayerLobby	= 2,
		SplitscreenLobby	= 3,
		PartyLobby			= 4,
		LanBrowser			= 5,
	},

	MpGroupMode =
	{
		None			= 0,
		Joining			= 1,
		Hosting			= 2,
	},
	
	PhotoMode =
	{
		Camera			= 0,
		Effects			= 1,
		EffectsColour	= 2,
		Viewing			= 3,
		CamControls		= 4,
	},
	
	ReturnMode =
	{
		QuitSpGame					= 1,
		QuitMpGame					= 2,
	},

	-- These are all the reasons you could be kicked back to the start scree. The game will close down as part of this process.
	UserKickBackMode =
	{
		None						= 0,
		UserChanged					= 1,
		ServiceConnectionLost		= 2,
		MpBetaExpired				= 3,
		InactiveGameInvite			= 4,
		ProfileUploadFailed 		= 5,
		XLSPConnectionLost			= 6,
		NetworkCableRemoved			= 7,
		EndOfGame					= 8, -- not too sure about this
	},

	ModShopOptions = 
	{
		Use						= 0,
		Edit					= 1,
		Rename					= 2,
	},

	LobbyOptions =
	{
		CarSelect				= 0,
		ModShop					= 1,
		RaceRules				= 2,
		RaceSummary				= 3,
		Challenges				= 4,
		Awards					= 5,
		Scoreboard				= 6,
		SessionScoreboard		= 7,
		RaceSummery				= 8,
		Settings				= 9,
		RacePreview				= 10,
	},

	PlayerOptions =
	{
		GamerCard			= 0,
		Review				= 1,
		Voice				= 2,
	},

	ShareOptions =
	{
		Facebook			= -1,
		Twitter				= -2,
		Blurb				= -3,
		FriendDemand		= -4,
		FacebookAlbum		= -5,
	},

	MPOnlineSharingOptions =
	{
		Rank				= 0,
		EventRating			= 1,
		TotalWins			= 2,
		PowerUpRatio		= 3,
		TotalRaces			= 4,
		TotalPlaytime		= 5,
		TotalFans			= 6,
		LegendRank			= 7,
	},

	PopupOptions = 
	{
		Yes				= 0,
		No				= 1,
		QuitWithParty	= 2,
		ExitToLobby		= 3,
	},
	
	MPLobbyLocalSharingOptions =
	{
		Position			= 0,
		Rank				= 1,
	},

	SpTierSelectSharingOptions =
	{
		FanStatusRank			= 0,
		CompletionPercentage	= 1,
		NumberOfFans			= 2,
		NumberOfStars			= 3,
		NumberOfCars			= 4,
		BossesBeat				= 5,
		TierComplete			= 6,
	},
	
	SSLobbySharingOptions = 
	{
		Standings				= 0,
		Wins					= 1,
		Points					= 2,
		TotalGames				= 3,
		TotalPlaytime			= 4,	
	},

	TaleOfTheTapeSharingOptions =
	{
		Accepted			= 0,
		Won					= 1,
		KOs					= 2,
		MostAttempts		= 3,
		AvgAttempts			= 4,
	},

	POBSPDLCScreens = 
	{
		NewDLC		= 1,
	},

	POBSPFriendDemandScreens = 
	{
		NewFriendDemandBeatenAndReturned		= 1,
		NewFriendDemandRecieved					= 2,
		SentFriendDemandBeaten					= 3,
	},

	POBSPUpcomingUnlockScreens = 
	{
		BossBattle								= 1,
		Tier									= 2,
		FanStatus								= 3,
	},

	POBSPUnlockScreens = 
	{		
		Tier								= 1,
		BossBattle							= 2,
		BossDefeated						= 3,
		FanStatus							= 4,
	},

	POBSPMilestoneScreens = 
	{
		GameProgression		= 1,
	},

	POBSPAvailableScreens = 
	{		
		Bosses								= 1,
		Cars								= 2,
		Mods								= 3,
		Events								= 4,
		Stars								= 5,
	},

	POBMPUpcomingUnlockScreens = 
	{
		Challenge								= 1,
		FanStatus								= 2,
	},

	POBMPUnlockScreens = 
	{
		FanStatus								= 1,
		Challenge								= 2,
	},

	POBMPAvailableScreens = 
	{
		Cars								= 1,
		Mods								= 2,
		Playlists							= 3,
	},

	POBMPStatsScreens = 
	{
		LastSession								= 1,
	},

	POBMPCommunityScreens = 
	{
		NemesisPrey								= 1,
	},

	POBMPMilestoneScreens = 
	{
		ChallengeProgression					= 1,
	},

	POBMPLeaderboardScreens = 
	{
		Leaderboard					= 1,
	},

	POBSharedUpcomingUnlockScreens = 
	{
		Achievements					= 1,
	},

	POBSharedUnlockScreens = 
	{
		Achievements					= 1,
	},

	POBSharedSocialScreens = 
	{
		Twitter					= 1,
		Facebook				= 2,
	},
	
	FriendDemandOptions = 
	{
		Attempt	= 0,
		Store	= 1,
		Delete	= 2,
	},
	
	EventOwnerResult = 
	{
		Losing		= 0,
		Winning		= 1,
		NoScoreDraw = 2,
		ScoreDraw	= 3,
	},

	StorageFailType =
	{
		UserFailed		= 0,
		UserCorrupted	= 1,
		TitleFailed		= 2,
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
