GUI =
{
	finished = false,
	
	currentScreen = 0,
	numScreensChosen = 0,
	numMPScreensChosen = 0,
	numCurrentScreens = 0,
	maxScreens = 5,
	distanceBetweenScreens = -75,
	distanceBetweenIcons = 12.5,

	sp_game_info = { },
	sp_upcoming_unlocks = { },
	sp_boss_info = { },
	sp_vehicle_info = { },
	sp_mod_info = { },
	sp_dlc_info = { },
	sp_friend_demand_info = { },
	mp_game_info = { },
	
	
	count_up_values = {},
	fan_bar_values = nil,
	rank_info = {},

	rows_id = { },

	screen_weighting_large = 4,
	screen_weighting_small = 1,

	POBSPDLCScreens =  { false, },
	POBSPFriendDemandScreens =  { false, false, false, },
	POBSPUpcomingUnlockScreens =  { false, false, false, },
	POBSPUnlockScreens =  { false, false, false, false, },
	POBSPMilestoneScreens = { false, },
	POBSPAvailableScreens = { false, false, false, false, false, },
	POBMPUpcomingUnlockScreens = { false, false, },
	POBMPUnlockScreens = { false, false, },
	POBMPAvailableScreens = { false, false, false, },
	POBMPStatsScreens = { false, },
	POBMPCommunityScreens = { false, },
	POBMPMilestoneScreens = { false, },
	POBMPLeaderboardScreens = { false, },
	POBSharedUpcomingUnlockScreens = { false, },
	POBSharedUnlockScreens = { false, },
	POBSharedSocialScreens = { false, false, },

	Mode = 
	{
		POB_Start		= 1,
		POB_Page1		= 2,
		POB_Page2		= 3,
		POB_Page3		= 4,
		POB_Page4		= 5,
		POB_Page5		= 6,
		SinglePlayer	= 7,
		Multiplayer		= 8,
	},
	
	CurrentMode = 1,
	EndMode = 7,

	UiCameraName = 
	{		
		"POB_Start",
		"POB_Page1",
		"POB_Page2",
		"POB_Page3",
		"POB_Page4",
		"POB_Page5",
		"Sp_5",
		"Mp_5",
	},
	
	Lock = 
	{
		filename = "FE_icons",
		icon	 = "Lock",
	},
	
	Unlock = 
	{
		filename = "FE_icons",
		icon	 = "Unlock",
	},
	
	ScreenTimer = 3,
	max_screen_time = 4,
	
	deactivate_previous_screen = false,
	deactivate_previous_screen_timer = 0,
	
	audio_timer = 1,
	have_played_audio = false,
	
	sticker_colours = 
	{
		"Support_4", -- Normal stickers
		"Main_1", -- Multiplayer stickers
		"Support_0", -- Powerup stickers
		"Support_2", -- Boss stickers
	},
	
	node_ids = {},
	
	aborting = false,
	doneEnding = false,

	CanExit = function( _GUI )
		return false
	end,
}


function Init()

	AddSCUI_Elements()

	StoreInfoLine()
	SetupInfoLine()

	UIScreen.BlockInputToContext( true )

end

function PostInit()
	--
	UIScreen.BlockInputToContext( true )
	--UIButtons.TimeLineActive( "PrevOnBlur", true, 0)
	
	GUI.sp_game_info = SinglePlayer.GameInfo()
	CheckUpcomingUnlocks()
	GUI.sp_dlc_info = Amax.GetPOBDLCInfo()
	GUI.sp_friend_demand_info = Amax.GetPOBFriendDemandInfo()
	GUI.sp_boss_info = SinglePlayer.BossInfo()
	GUI.mp_game_info = Multiplayer.POBGameInfo()
	GUI.sp_tier_info = SinglePlayer.TierInfo()
	GUI.sp_vehicle_info = SinglePlayer.VehicleInfo()
	GUI.sp_mod_info = SinglePlayer.ModInfo()
	
	GUI.rank_info = SinglePlayer.RankInfo()
	
	GUI.maxScreens = 5
	
	if Amax.HasBeenPlayingSpGameLatest() == true then
		GUI.EndMode = GUI.Mode.SinglePlayer
		CheckCanUseScreens(GUI.screen_weighting_large)
		CheckCanUseMPScreens(GUI.screen_weighting_small)
	else
		GUI.EndMode = GUI.Mode.Multiplayer
		CheckCanUseScreens(GUI.screen_weighting_small)
		CheckCanUseMPScreens(GUI.screen_weighting_large)
	end	
	
	
	if GUI.numScreensChosen <= 0 and GUI.numMPScreensChosen <= 0 then
		GUI.aborting = true	
		
		UIButtons.SetActive(SCUI.name_to_id["summary_text"], false)
		UIButtons.SetActive(SCUI.name_to_id["Shp_BlurLogo"], false)
			
	else
		--Go to initial screen
		Amax.ChangeUiCamera( GUI.UiCameraName[ GUI.CurrentMode ], 1, 0 )	
	end
	
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.aborting == true	then
		return
	end
	
	if message == UIEnums.Message.MenuNext  then
		PreviouslyOnBlurEnd()
	elseif message == UIEnums.Message.MenuBack then
		PreviouslyOnBlurEnd()
	end
end

function FrameUpdate( time )
	if GUI.aborting == true	then
		if GUI.doneEnding == false then
			PreviouslyOnBlurEnd()
			GUI.doneEnding = true
		end
		return
	end
	
	HandleScreenTimings( time )
	
	UpdateCountUpValues( time )
	
	UpdateFanBars( time )
	
		
end

function AddCountUpValue( delay, start_value, end_value, node_id, datafinder_entry )

 GUI.count_up_values[#GUI.count_up_values +1 ] = 
 {
	mDelay = delay,
	mCurrentValue = start_value,
	mPreviousNumber = 0,
	mEndValue = end_value,
	mNodeId = node_id,
	mDatafinderEntry = datafinder_entry,	
 }
 
end

function UpdateCountUpValues( time )
	for i = 1, #GUI.count_up_values, 1 do
	
		if GUI.count_up_values[i].mDelay > 0 then
			GUI.count_up_values[i].mDelay = GUI.count_up_values[i].mDelay - time
			UIButtons.ChangeText( GUI.count_up_values[i].mNodeId , GUI.count_up_values[i].mDatafinderEntry .. GUI.count_up_values[i].mCurrentValue  )
		else
			if GUI.count_up_values[i].mCurrentValue < GUI.count_up_values[i].mEndValue then 
				GUI.count_up_values[i].mCurrentValue = GUI.count_up_values[i].mCurrentValue + (100/60 ) --to get to 100 in 2 seconds
				
				if math.floor(GUI.count_up_values[i].mCurrentValue) >= (GUI.count_up_values[i].mPreviousNumber + 1 )then
					GUI.count_up_values[i].mPreviousNumber = math.floor(GUI.count_up_values[i].mCurrentValue)
					--UISystem.PlaySound( UIEnums.SoundEffect.PreviouslyCounter )
				end
				
				if GUI.count_up_values[i].mCurrentValue > GUI.count_up_values[i].mEndValue then
					GUI.count_up_values[i].mCurrentValue = GUI.count_up_values[i].mEndValue
				end
				
				
				
				UIButtons.ChangeText( GUI.count_up_values[i].mNodeId , GUI.count_up_values[i].mDatafinderEntry .. GUI.count_up_values[i].mCurrentValue  )				
			end
		end
		
	end	
end

function StartPOBProcessFanGain( delay, progressBarId, bar_size_x, txt_fans_last_race_id, txt_total_fans_id, txt_target_fans_id, txt_Fan_Status_id, total_fans, previous_total_fans, fans_to_rank, current_rank_fan_bound, next_rank_fan_bound)
	
	GUI.fan_bar_values = 
	{
		mDelay = delay,
		mProgrssBarId = progressBarId,
		mBar_size_x = bar_size_x,
		mTxt_fans_last_race_id = txt_fans_last_race_id,		--UIButtons.FindChildByName( GUI.dummy_id, "Amount" )
		mTxt_total_fans_id = txt_total_fans_id,				-- UIButtons.FindChildByName( GUI.dummy_id, "Amount_Total_Level" )
		mTxt_target_fans_id = txt_target_fans_id,			--UIButtons.FindChildByName( GUI.dummy_id, "Amount_Target" )
		mTotal_fans = total_fans,
		mPrevious_total_fans = previous_total_fans,
		mFans_to_rank = fans_to_rank,
		mCurrent_rank_fan_bound = current_rank_fan_bound,
		mNext_rank_fan_bound = next_rank_fan_bound,		
		mLerp_target_fans = 0,
		mLerped_fans = 0,
		mLerp_timer = 0,
		mFinished_all_lerp = false,
	}
	
	UIButtons.ChangeText( txt_fans_last_race_id, "GAME_NUM_" .. (total_fans - previous_total_fans) )
	UIButtons.ChangeText( txt_total_fans_id, "GAME_UNLOCKS_FANS_" .. previous_total_fans )
	UIButtons.ChangeText( txt_Fan_Status_id, "GAME_UNLOCKS_CURRENT_RANK_" .. GUI.rank_info[FindPOBNextRankFromFans( total_fans )].name )
	UIButtons.ChangeText( txt_target_fans_id, "GAME_UNLOCKS_NEXT_RANK_" .. fans_to_rank .. "_TO_" .. GUI.rank_info[FindPOBNextRankFromFans( previous_total_fans )].name )
	
	UpdatePOBProgressBar( previous_total_fans - current_rank_fan_bound, next_rank_fan_bound - current_rank_fan_bound )
	
	-- figure out our target to lerp the progress bar to
	if total_fans >= next_rank_fan_bound then
		GUI.fan_bar_values.mLerp_target_fans = GUI.fan_bar_values.mNext_rank_fan_bound
	else
		GUI.fan_bar_values.mLerp_target_fans = GUI.fan_bar_values.mTotal_fans
	end
	
	GUI.fan_bar_values.mLerped_fans = 0
	GUI.fan_bar_values.mLerp_timer = 0
	
end

function UpdatePOBProcessFanGain( time, current )
	-- lerp to our target over some lovely hard coded number of seconds
	local lerp_time = 1
	local finished = false
	
	GUI.fan_bar_values.mLerp_timer = GUI.fan_bar_values.mLerp_timer + time
	
	if GUI.fan_bar_values.mLerp_timer > lerp_time then
		GUI.fan_bar_values.mLerp_timer = lerp_time
		finished = true
	end
	
	GUI.fan_bar_values.mLerped_fans = ( GUI.fan_bar_values.mLerp_target_fans - GUI.fan_bar_values.mPrevious_total_fans ) * ( GUI.fan_bar_values.mLerp_timer / lerp_time )
	
	UIButtons.ChangeText( GUI.fan_bar_values.mTxt_fans_last_race_id, "GAME_NUM_" .. (GUI.fan_bar_values.mTotal_fans - GUI.fan_bar_values.mPrevious_total_fans) - GUI.fan_bar_values.mLerped_fans )
	UIButtons.ChangeText( GUI.fan_bar_values.mTxt_total_fans_id, "GAME_UNLOCKS_FANS_" .. GUI.fan_bar_values.mPrevious_total_fans + GUI.fan_bar_values.mLerped_fans )
	UIButtons.ChangeText( GUI.fan_bar_values.mTxt_target_fans_id, "GAME_UNLOCKS_NEXT_RANK_" .. math.floor(GUI.fan_bar_values.mFans_to_rank - GUI.fan_bar_values.mLerped_fans) .. "_TO_" .. GUI.rank_info[FindPOBNextRankFromFans( GUI.fan_bar_values.mPrevious_total_fans + GUI.fan_bar_values.mLerped_fans )].name )
	
	UpdatePOBProgressBar( ( GUI.fan_bar_values.mPrevious_total_fans - GUI.fan_bar_values.mCurrent_rank_fan_bound ) + GUI.fan_bar_values.mLerped_fans, GUI.fan_bar_values.mNext_rank_fan_bound - GUI.fan_bar_values.mCurrent_rank_fan_bound )
	
	return finished
end

function UpdatePOBProgressBar( current, max )
	UIButtons.ChangeSizeX( GUI.fan_bar_values.mProgrssBarId, ( GUI.fan_bar_values.mBar_size_x / max ) * current )

end

function NewPOBRankUpdate()
	-- recalculate all our bounds and values for the new rank
	local next_rank = FindPOBNextRankFromFans( GUI.fan_bar_values.mNext_rank_fan_bound + 1 )
	GUI.fan_bar_values.mCurrent_rank_fan_bound = GUI.fan_bar_values.mNext_rank_fan_bound
	GUI.fan_bar_values.mNext_rank_fan_bound = GUI.rank_info[next_rank].fans
	GUI.fan_bar_values.mPrevious_total_fans = GUI.fan_bar_values.mCurrent_rank_fan_bound
	GUI.fan_bar_values.mFans_to_rank = GUI.fan_bar_values.mNext_rank_fan_bound - GUI.fan_bar_values.mCurrent_rank_fan_bound
	
	-- begin the lerp process again
	UpdatePOBProcessFanGain()
end

function UpdateFanBars( time )
	-- fan bar update logic here

	if GUI.fan_bar_values ~= nil then

		if GUI.fan_bar_values.mDelay > 0 then
			GUI.fan_bar_values.mDelay = GUI.fan_bar_values.mDelay - time
		else		
			if GUI.fan_bar_values.mFinished_all_lerp == false then

				-- have we just finished a single lerp?
				if math.floor(GUI.fan_bar_values.mFans_to_rank - GUI.fan_bar_values.mLerped_fans) == 0 then
					NewPOBRankUpdate()
				end
				
				if UpdatePOBProcessFanGain( time ) == true then
					-- we've finished a lerp, did we rank up or are we properly done?
					if GUI.fan_bar_values.mFans_to_rank == GUI.fan_bar_values.mLerped_fans then
						--finished
					else
						GUI.fan_bar_values.mFinished_all_lerp = true
					end
				end
			end
		end
	end
end

-- given a number of fans, finds the rank that will next be unlocked
function FindPOBNextRankFromFans( num_fans )
	
	-- logic below breaks otherwise
	if num_fans == 0 then
		return 2
	end
	
	local rankInfo = GUI.rank_info
	
	for i,v in ipairs(rankInfo) do
		if v.fans >= num_fans then
			return ( v.index )
		end
	end
end

function HandleScreenTimings( time )

	GUI.audio_timer = GUI.audio_timer - time
	if	GUI.audio_timer <= 0 then
		if GUI.have_played_audio == false then	
			UISystem.PlaySound( UIEnums.SoundEffect.PreviouslyOnBlur )
			GUI.have_played_audio = true
		end
	end
	
	if deactivate_previous_screen == true then
		deactivate_previous_screen_timer = deactivate_previous_screen_timer - time
		if deactivate_previous_screen_timer <= 0 then
			UIButtons.SetActive( SCUI.name_to_id[ GUI.UiCameraName[ (GUI.CurrentMode-1) ].."_RB" ], false, true )
			deactivate_previous_screen = false
		end
	end

	GUI.ScreenTimer = GUI.ScreenTimer - time
	if GUI.ScreenTimer <= 0 then
		GUI.ScreenTimer = GUI.max_screen_time
		GUI.CurrentMode = GUI.CurrentMode + 1
		
		if GUI.CurrentMode >  (GUI.maxScreens + 1)  then 
				PreviouslyOnBlurEnd()
		else
			GUI.currentScreen = (GUI.currentScreen + 1)
			-- //Create a new screen

			if GetNextPOBScreen( SCUI.name_to_id[ GUI.UiCameraName[ GUI.CurrentMode ].."_RB" ] ) == true then
				-- //Fill our new screen with data
				FillPOBScreen( GUI.numCurrentScreens )
				
				deactivate_previous_screen = true
				deactivate_previous_screen_timer = 0.5
				
				UIButtons.SetActive(SCUI.name_to_id[ ("PoBLP_" .. (GUI.currentScreen -1)) ], true)
				UISystem.PlaySound( UIEnums.SoundEffect.PreviouslyWhoosh )
				
				Amax.ChangeUiCamera( GUI.UiCameraName[ GUI.CurrentMode ], 0.5, 0 )
				
				UISystem.PlaySound( UIEnums.SoundEffect.PreviouslyReward )
				
			else 
				PreviouslyOnBlurEnd()
				
			end
		end
	end

end

function EnterEnd()

	if GUI.aborting == false then
		SinglePlayer.ClearLastSessionInfo()
		Multiplayer.ClearPOBGameInfo()
		UISystem.PlaySound( UIEnums.SoundEffect.PreviouslyOut )
	end

	--zoom back to the carousel
	Amax.ChangeUiCamera( GUI.UiCameraName[ GUI.EndMode ], 1, 0 )

	RestoreInfoLine()
end

function End()
	--
	PreviouslyOnBlurEnd = nil
	CheckCanUseScreens = nil
	CheckCanUseMPScreens = nil
	AddCountUpValue = nil
	UpdateCountUpValues = nil
	StartPOBProcessFanGain = nil
	UpdatePOBProcessFanGain = nil
	UpdatePOBProgressBar = nil
	NewPOBRankUpdate = nil
	UpdateFanBars = nil
	FindPOBNextRankFromFans = nil
	HandleScreenTimings = nil
	PreviouslyOnBlurEnd = nil
	CheckCanUseScreens = nil
	CheckCanUseMPScreens = nil
	GetNextPOBScreen = nil
	FillPOBScreen = nil
	CheckNewDLC = nil
	CheckFriendDemandBeatenAndReturned = nil
	CheckNewFriendDemand = nil
	CheckFriendDemandBeaten = nil
	CheckUpcomingUnlocks = nil
	CheckUpcomingBossUnlock = nil
	CheckUpcomingTierUnlock = nil
	CheckUpcomingUnlocksFanStatus = nil
	CheckNewTierUnlocked = nil
	GetNewTierUnlocked = nil
	CheckNewBossUnlocked = nil
	GetNewBossUnlocked = nil
	CheckNewBossDefeated = nil
	GetNewBossDefeated = nil
	CheckNewFanStatus = nil
	CheckSinglePlayerProgression = nil
	GetSinglePlayerProgression = nil
	CheckAvailableBosses = nil
	CheckAvailableCars = nil
	CheckAvailableMods = nil
	CheckAvailableEvents = nil
	CheckAvailableStars = nil
	CheckMPUpcomingUnlocksChallenge = nil
	CheckMPUpcomingUnlocksFanStatus = nil
	CheckMPUnlocksFanStatus = nil
	CheckMPUnlocksChallenges = nil
	CheckMPAvailableCars = nil
	CheckMPAvailableMods = nil
	CheckMPAvailablePlaylists = nil
	CheckMPStats = nil
	CheckMPCommunity = nil
	CheckMPProgression = nil
	CheckMPLeaderboard = nil
	CheckSharedUpcomingUnlocksAchievements = nil
	CheckSharedUnlocksAchievements = nil
	CheckSharedSocialNetworksTwitter = nil
	CheckSharedSocialNetworksFacebook = nil
	SetupPOBScreenTitle = nil
	SetupDefaultBossScreen = nil
	SetupTestScreen = nil
	SetupDLCNewInfoScreen = nil
	FillDLCNewInfoScreen = nil
	SetupFriendDemandBeatenAndReturned = nil
	POBCreateBlurb = nil
	SetupFriendDemandNew = nil
	FillFriendDemandBeatenAndReturned = nil
	FillFriendDemandNew = nil
	SetupFriendDemandBeaten = nil
	FillFriendDemandBeaten = nil
	SetupUpcomingUnlocksBossBattleScreen = nil
	FillUpcomingUnlocksBossBattleScreen = nil
	SetupUpcomingUnlocksNewTierScreen = nil
	FillUpcomingUnlocksNewTierScreen = nil
	SetupUpcomingUnlocksNewFanStatus = nil
	SetupUnlocksNewTierScreen = nil
	FillUpcomingUnlocksNewFanStatus = nil
	FillUnlocksNewTierScreen = nil
	SetupUnlocksNewFanStatusScreen = nil
	FillUnlocksNewFanStatusScreen = nil
	SetupUnlocksNewBossBattle = nil
	FillUnlocksNewBossBattle = nil
	SetupUnlocksNewBossDefeated = nil
	FillUnlocksNewBossDefeated = nil
	SetupMilestonesSPGameProgressScreen = nil
	FillMilestonesSPGameProgressScreen = nil
	SetupAvailableBossBattle = nil
	FillAvailableBossBattle = nil
	SetupAvailableCars = nil
	FillAvailableCars = nil
	SetupAvailableMods = nil
	FillAvailableMods = nil
	SetupAvailableEvents = nil
	FillAvailableEvents = nil
	SetupAvailableStars = nil
	FillAvailableStars = nil
	SetupMPUpcomingUnlocksChallenges = nil
	FillMPUpcomingUnlocksChallenges = nil
	RefreshCarChallengeRewards = nil
	SetupMPUpcomingUnlocksFanStatus = nil
	FillMPUpcomingUnlocksFanStatus = nil
	SetupMPUnlocksChallenges = nil
	FillMPUnlocksChallenges = nil
	SetupMPUnlocksFanStatus = nil
	FillMPUnlocksFanStatus = nil
	SetupMPAvailableCars = nil
	FillMPAvailableCars = nil
	SetupMPAvailableMods = nil
	FillMPAvailableMods = nil
	SetupMPAvailablePlaylists = nil
	FillMPAvailablePlaylists = nil
	SetupMPStats = nil
	FillMPAvailableStats = nil
	SetupMPCommunity = nil
	FillMPAvailableCommunity = nil
	SetupMPMilestones = nil
	FillMPAvailableMilestones = nil
	SetupMPLeaderboard = nil
	FillMPAvailableLeaderboard = nil
	SetupUpcomingUnlocksAchievements = nil
	GetQuadUV = nil
	FillUpcomingUnlocksAchievements = nil
	SetupUnlocksAchievements = nil
	FillUnlocksAchievements = nil
	SetupSocialTweets = nil
	FillSocialTweets = nil
	SetupSocialFacebooked = nil
	FillSocialFacebooked = nil
end

function PreviouslyOnBlurEnd()
	
	if(GUI.currentScreen -1) > 0 then
		local EndMode = nil
		if GUI.EndMode == GUI.Mode.SinglePlayer then
			EndMode = "_SP"
		else
			EndMode = "_MP"
		end
		print("END MODE", GUI.currentScreen-1,  EndMode)
		UIButtons.SetActive(SCUI.name_to_id[ ("LP_Ending_" .. (GUI.currentScreen-1)) .. EndMode ], true)
	end
		
	if GUI.aborting == false then
		StartAsyncSave()
	end	
	
	PopScreen()
end

function CheckCanUseScreens( max_sp_screens )
	
	-- //replace hard coded falses with functions that check if the screen is ok to be displayed

	--GUI.POBSPDLCScreens[UIEnums.POBSPDLCScreens.NewDLC]												= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckNewDLC(), false ) 
	GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.NewFriendDemandBeatenAndReturned] = Select( ( GUI.numScreensChosen < max_sp_screens ), CheckFriendDemandBeatenAndReturned(), false )  
	GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.BossBattle]					= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckUpcomingBossUnlock() , false ) 
	GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.Tier]											= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckNewTierUnlocked() , false )  
	GUI.POBSPMilestoneScreens[UIEnums.POBSPMilestoneScreens.GameProgression	]						= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckSinglePlayerProgression() , false )
	--GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Bosses]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckAvailableBosses() , false )
	GUI.POBSharedUpcomingUnlockScreens[UIEnums.POBSharedUpcomingUnlockScreens.Achievements]			= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckSharedUpcomingUnlocksAchievements(), false ) 
	GUI.numScreensChosen = Select( ( GUI.POBSharedUpcomingUnlockScreens[UIEnums.POBSharedUpcomingUnlockScreens.Achievements] == true ), (GUI.numScreensChosen + 1), GUI.numScreensChosen ) 

	GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.NewFriendDemandRecieved]			= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckNewFriendDemand() , false ) 	
	GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.Tier]							= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckUpcomingTierUnlock() , false ) 	
	GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.BossBattle]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckNewBossUnlocked() , false ) 
	--GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Cars]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckAvailableCars() , false )
	--GUI.POBSharedUnlockScreens[UIEnums.POBSharedUnlockScreens.Achievements]							= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckSharedUnlocksAchievements() , false )
	GUI.numScreensChosen = Select( ( GUI.POBSharedUnlockScreens[UIEnums.POBSharedUnlockScreens.Achievements] == true ), (GUI.numScreensChosen + 1), GUI.numScreensChosen ) 

	--GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.SentFriendDemandBeaten]			= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckFriendDemandBeaten()	 , false )
	GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.FanStatus]					= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckUpcomingUnlocksFanStatus() , false )		
	--GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.BossDefeated]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckNewBossDefeated() , false )
	--GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Mods]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckAvailableMods() , false )	
	--GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Twitter]								= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckSharedSocialNetworksTwitter() , false )
	GUI.numScreensChosen = Select( ( GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Twitter] == true ), (GUI.numScreensChosen + 1), GUI.numScreensChosen ) 
	
	GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.FanStatus]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckNewFanStatus() , false )	
	--GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Events]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckAvailableEvents() , false )
	--GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Facebook]								= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckSharedSocialNetworksFacebook() , false )
	GUI.numScreensChosen = Select( ( GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Facebook] == true ), (GUI.numScreensChosen + 1), GUI.numScreensChosen ) 
	
	--GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Stars]									= Select( ( GUI.numScreensChosen < max_sp_screens ), CheckAvailableStars() , false )

end

function CheckCanUseMPScreens( max_mp_screens )

	GUI.POBMPUpcomingUnlockScreens[UIEnums.POBMPUpcomingUnlockScreens.Challenge]			= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPUpcomingUnlocksChallenge(), false )
	GUI.POBMPUnlockScreens[UIEnums.POBMPUnlockScreens.FanStatus]							= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPUnlocksFanStatus(), false )
	--GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Cars]							= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPAvailableCars(), false )
	GUI.POBMPStatsScreens[UIEnums.POBMPStatsScreens.LastSession]							= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPStats(), false )
	--GUI.POBMPCommunityScreens[UIEnums.POBMPCommunityScreens.NemesisPrey]					= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPCommunity(), false )
	--GUI.POBMPMilestoneScreens[UIEnums.POBMPMilestoneScreens.ChallengeProgression]			= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPProgression(), false )
	--GUI.POBMPLeaderboardScreens[UIEnums.POBMPLeaderboardScreens.Leaderboard]				= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPLeaderboard(), false )
	GUI.POBSharedUpcomingUnlockScreens[UIEnums.POBSharedUpcomingUnlockScreens.Achievements]	= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckSharedUpcomingUnlocksAchievements(), false ) 
	GUI.numMPScreensChosen = Select( ( GUI.POBSharedUpcomingUnlockScreens[UIEnums.POBSharedUpcomingUnlockScreens.Achievements] == true ), (GUI.numMPScreensChosen + 1), GUI.numMPScreensChosen ) 

	GUI.POBMPUpcomingUnlockScreens[UIEnums.POBMPUpcomingUnlockScreens.FanStatus]	= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPUpcomingUnlocksFanStatus(), false )
	GUI.POBMPUnlockScreens[UIEnums.POBMPUnlockScreens.Challenge]					= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPUnlocksChallenges(), false )
	--GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Mods]					= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPAvailableMods(), false )	
	--GUI.POBSharedUnlockScreens[UIEnums.POBSharedUnlockScreens.Achievements]			= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckSharedUnlocksAchievements() , false )
	--GUI.numMPScreensChosen = Select( ( GUI.POBSharedUnlockScreens[UIEnums.POBSharedUnlockScreens.Achievements] == true ), (GUI.numMPScreensChosen + 1), GUI.numMPScreensChosen ) 

	--GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Playlists]				= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckMPAvailablePlaylists(), false )
	--GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Twitter]				= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckSharedSocialNetworksTwitter() , false )
	--GUI.numMPScreensChosen = Select( ( GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Twitter] == true ), (GUI.numMPScreensChosen + 1), GUI.numMPScreensChosen ) 
	
	--GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Facebook]				= Select( ( GUI.numMPScreensChosen < max_mp_screens ), CheckSharedSocialNetworksFacebook() , false )
	--GUI.numMPScreensChosen = Select( ( GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Facebook] == true ), (GUI.numMPScreensChosen + 1), GUI.numMPScreensChosen ) 

end

--function GetNextPOBScreen()
-- // THIS FUNCTION NEEDS RE WRITING TO GET APPROPRIATE SCREENS 
--	return GetNextSPPOBScreen()
--end

function GetNextPOBScreen( parent_id )

	if GUI.POBSPDLCScreens[UIEnums.POBSPDLCScreens.NewDLC] ==  true then

		GUI.POBSPDLCScreens[UIEnums.POBSPDLCScreens.NewDLC] = false
		SetupDLCNewInfoScreen( parent_id )
		return true

	elseif GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.NewFriendDemandBeatenAndReturned] == true then

		GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.NewFriendDemandBeatenAndReturned] = false
		SetupFriendDemandBeatenAndReturned( parent_id )
		return true

	elseif GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.NewFriendDemandRecieved] == true then

		GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.NewFriendDemandRecieved] = false
		SetupFriendDemandNew( parent_id )
		return true

	elseif GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.SentFriendDemandBeaten] == true then	

		GUI.POBSPFriendDemandScreens[UIEnums.POBSPFriendDemandScreens.SentFriendDemandBeaten] = false
		SetupFriendDemandBeaten( parent_id )
		return true

	elseif GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.BossBattle] == true then 
		
		GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.BossBattle] = false
		SetupUpcomingUnlocksBossBattleScreen( parent_id )
		return true

	elseif GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.Tier] == true then

		GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.Tier] = false
		SetupUpcomingUnlocksNewTierScreen( parent_id )
		return true

	elseif GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.FanStatus] == true then	

		GUI.POBSPUpcomingUnlockScreens[UIEnums.POBSPUpcomingUnlockScreens.FanStatus] = false
		SetupUpcomingUnlocksNewFanStatus( parent_id )
		return true

	elseif GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.Tier] == true then
		
		GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.Tier] = false
		SetupUnlocksNewTierScreen( parent_id )
		return true

	elseif GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.BossBattle] == true then
		
		GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.BossBattle] = false
		SetupUnlocksNewBossBattle( parent_id )
		return true

	elseif GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.BossDefeated] == true then

		GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.BossDefeated] = false
		SetupUnlocksNewBossDefeated( parent_id )
		return true

	elseif GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.FanStatus] == true then

		GUI.POBSPUnlockScreens[UIEnums.POBSPUnlockScreens.FanStatus] = false
		SetupUnlocksNewFanStatusScreen( parent_id )
		return true

	elseif GUI.POBSPMilestoneScreens[UIEnums.POBSPMilestoneScreens.GameProgression] == true then

		GUI.POBSPMilestoneScreens[UIEnums.POBSPMilestoneScreens.GameProgression] = false
		SetupMilestonesSPGameProgressScreen( parent_id )
		return true

	elseif GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Bosses] == true then
		
		GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Bosses] = false
		SetupAvailableBossBattle( parent_id )
		return true

	elseif GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Cars] == true then

		GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Cars] = false
		SetupAvailableCars( parent_id )
		return true

	elseif GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Mods] == true then
		
		GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Mods] = false
		SetupAvailableMods( parent_id )
		return true

	elseif GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Events] == true then

		GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Events] = false
		SetupAvailableEvents( parent_id )
		return true

	elseif GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Stars] == true then

		GUI.POBSPAvailableScreens[UIEnums.POBSPAvailableScreens.Stars] = false
		SetupAvailableStars( parent_id )
		return true

	elseif GUI.POBMPUpcomingUnlockScreens[UIEnums.POBMPUpcomingUnlockScreens.Challenge] == true then

		GUI.POBMPUpcomingUnlockScreens[UIEnums.POBMPUpcomingUnlockScreens.Challenge] = false
		SetupMPUpcomingUnlocksChallenges( parent_id )
		return true

	elseif GUI.POBMPUpcomingUnlockScreens[UIEnums.POBMPUpcomingUnlockScreens.FanStatus] == true then

		GUI.POBMPUpcomingUnlockScreens[UIEnums.POBMPUpcomingUnlockScreens.FanStatus] = false
		SetupMPUpcomingUnlocksFanStatus( parent_id )
		return true

	elseif GUI.POBMPUnlockScreens[UIEnums.POBMPUnlockScreens.FanStatus] == true then

		GUI.POBMPUnlockScreens[UIEnums.POBMPUnlockScreens.FanStatus] = false
		SetupMPUnlocksFanStatus( parent_id )
		return true

	elseif GUI.POBMPUnlockScreens[UIEnums.POBMPUnlockScreens.Challenge] == true then

		 GUI.POBMPUnlockScreens[UIEnums.POBMPUnlockScreens.Challenge] = false
		 SetupMPUnlocksChallenges( parent_id )
		 return true

	elseif GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Cars] == true then

		GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Cars] = false
		SetupMPAvailableCars( parent_id )
		return true

	elseif GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Mods] == true then

		GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Mods] = false
		SetupMPAvailableMods( parent_id )
		return true

	elseif GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Playlists] == true then
		
		GUI.POBMPAvailableScreens[UIEnums.POBMPAvailableScreens.Playlists] = false
		SetupMPAvailablePlaylists( parent_id )
		return true

	elseif GUI.POBMPStatsScreens[UIEnums.POBMPStatsScreens.LastSession] == true then

		GUI.POBMPStatsScreens[UIEnums.POBMPStatsScreens.LastSession] = false
		SetupMPStats( parent_id )
		return true

	elseif GUI.POBMPCommunityScreens[UIEnums.POBMPCommunityScreens.NemesisPrey] == true then

		GUI.POBMPCommunityScreens[UIEnums.POBMPCommunityScreens.NemesisPrey] = false
		SetupMPCommunity( parent_id )
		return true

	elseif GUI.POBMPMilestoneScreens[UIEnums.POBMPMilestoneScreens.ChallengeProgression] == true then
		
		GUI.POBMPMilestoneScreens[UIEnums.POBMPMilestoneScreens.ChallengeProgression] = false
		SetupMPMilestones( parent_id )
		return true

	elseif GUI.POBMPLeaderboardScreens[UIEnums.POBMPLeaderboardScreens.Leaderboard] == true then

		GUI.POBMPLeaderboardScreens[UIEnums.POBMPLeaderboardScreens.Leaderboard] = false
		SetupMPLeaderboard( parent_id )
		return true

	elseif GUI.POBSharedUpcomingUnlockScreens[UIEnums.POBSharedUpcomingUnlockScreens.Achievements] == true then

		GUI.POBSharedUpcomingUnlockScreens[UIEnums.POBSharedUpcomingUnlockScreens.Achievements] = false
		SetupUpcomingUnlocksAchievements( parent_id )
		return true

	elseif GUI.POBSharedUnlockScreens[UIEnums.POBSharedUnlockScreens.Achievements] == true then

		GUI.POBSharedUnlockScreens[UIEnums.POBSharedUnlockScreens.Achievements] = false
		SetupUnlocksAchievements( parent_id )
		return true

	elseif GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Twitter] == true then

		GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Twitter] = false
		SetupSocialTweets( parent_id )
		return true

	elseif GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Facebook] == true then

		GUI.POBSharedSocialScreens[UIEnums.POBSharedSocialScreens.Facebook] = false
		SetupSocialFacebooked( parent_id )
		return true

	end

	return false

end

function FillPOBScreen( screen_id )
	-- //Write the correct data to the right screens
	if GUI.rows_id[ screen_id ].screen_type == "UpcomingUnlocksBossBattleScreen" then
		FillUpcomingUnlocksBossBattleScreen( screen_id )
	elseif GUI.rows_id[ screen_id ].screen_type == "UpcomingUnlocksNewTierScreen" then
		FillUpcomingUnlocksNewTierScreen( screen_id )
	elseif GUI.rows_id[ screen_id ].screen_type == "UnlocksNewTierScreen" then
		FillUnlocksNewTierScreen( screen_id )
	elseif GUI.rows_id[ screen_id ].screen_type == "UnlocksNewFanStatusScreen" then
		FillUnlocksNewFanStatusScreen( screen_id )
	elseif GUI.rows_id[ screen_id ].screen_type == "MilestonesSPGameProgressScreen" then
		FillMilestonesSPGameProgressScreen( screen_id )
	else
		--print("POB trying to fill a screen that doesnt exist?")
	end
end

-- //CHECKING AND GETTING THE DATA NEEDED FOR EACH SCREEN  ------------------------------------------------------------------------------------------------------------------------

function CheckNewDLC()

	if Amax.CheckIsCommunityNewsNew() == true then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	end
		
	if Amax.CheckIsDLCNew() == true then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	end
		
	if Amax.CheckIsNewsNew() == true then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	end
	
	return false;
	
end

function CheckFriendDemandBeatenAndReturned()

	if FriendDemand.ReadFinished() == true and FriendDemand.NewFriendDemandsReChallengeRecieved() == true then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	end

	return false
end

function CheckNewFriendDemand()
	
	if FriendDemand.ReadFinished() == true and FriendDemand.NewFriendDemandsRecieved() == true then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	end

	return false
end

function CheckFriendDemandBeaten()

	if GUI.sp_friend_demand_info.ChallengesComplete > 0 then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	end

	return false

end

function CheckUpcomingUnlocks()
	GUI.sp_upcoming_unlocks = SinglePlayer.UpcomingUnlocks()
end

function CheckUpcomingBossUnlock()

	for i = 1, #GUI.sp_upcoming_unlocks, 1 do
		if GUI.sp_upcoming_unlocks[i].kind == "boss" then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			return true
		end
	end

	return false
end

function CheckUpcomingTierUnlock()

	for i = 1, #GUI.sp_upcoming_unlocks, 1 do
		if GUI.sp_upcoming_unlocks[i].kind == "tier" then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			return true
		end
	end

	return false
end

function CheckUpcomingUnlocksFanStatus()

	for i = 1, #GUI.sp_upcoming_unlocks, 1 do
		if GUI.sp_upcoming_unlocks[i].kind == "rank" then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			return true
		end
	end

	return false
end

function CheckNewTierUnlocked()
	--dont want to show that we have unlocked shannons tier because thats a bit silly
	if GUI.sp_game_info.lastSessionTier ~= 0 and GUI.sp_game_info.lastSessionTier ~= 1 then 
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	else
		return false
	end
end

function GetNewTierUnlocked()
	return GUI.sp_game_info.lastSessionTier 

end

function CheckNewBossUnlocked()
	if GUI.sp_game_info.lastSessionBossUnlocked ~= 0 then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	else
		return false
	end	
end

function GetNewBossUnlocked()
	return GUI.sp_game_info.lastSessionBossUnlocked 
end

function CheckNewBossDefeated()
	if GUI.sp_game_info.lastSessionBossDefeated ~= 0 then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	else
		return false
	end
end

function GetNewBossDefeated()
	return GUI.sp_game_info.lastSessionBossDefeated
end

function CheckNewFanStatus()

	if GUI.sp_game_info.rank ~= 1 then		
		if GUI.sp_game_info.lastSessionRankedUp == true then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			return true
		else
			return false
		end
	end

	return false
end

function CheckSinglePlayerProgression()
	if GUI.sp_game_info.totalStars ~= 0 then
		GUI.numScreensChosen = GUI.numScreensChosen + 1
		return true
	else
		return false
	end
end

function GetSinglePlayerProgression()
	local percentageComplete = (GUI.sp_game_info.totalStars / GUI.sp_game_info.maxStars)
	return percentageComplete
end

function CheckAvailableBosses()

	for i = 1, #GUI.sp_boss_info , 1 do		
		if GUI.sp_boss_info[i].unlocked == true and GUI.sp_boss_info[i].defeated == false then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			return true
		end
	end

	return false
end

function CheckAvailableCars()
	
 for i, v in pairs( GUI.sp_vehicle_info ) do
		if v.unlocked == true and v.new == true then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			return true
		end
 end
	return false
end

function CheckAvailableMods()
	-- for i, v in pairs( GUI.sp_mod_info ) do
	--	if v.unlocked == true and v.new == true then
	--		GUI.numScreensChosen = GUI.numScreensChosen + 1
	--		return true
	--	end
	--end
	return false
end

function CheckAvailableEvents()

	for i = 1, #GUI.sp_tier_info , 1 do		
		if GUI.sp_tier_info[i].unlocked == true and GUI.sp_tier_info[i].incompleteEvents > 0 then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			print("Events Available")
			return true
		end
	end
	
	return false
end

function CheckAvailableStars()

	for i = 1, #GUI.sp_tier_info , 1 do		
		if GUI.sp_tier_info[i].unlocked == true and GUI.sp_tier_info[i].availableStars > 0 then
			GUI.numScreensChosen = GUI.numScreensChosen + 1
			print("Stars Available")
			return true
		end
	end
	
	return false
end

function CheckMPUpcomingUnlocksChallenge()

	local Packs = Multiplayer.GetChallengePacks()
	
	local carChallenges = {}
		
	for class = 1, 4, 1 do
		carChallenges[(#carChallenges + 1)] =  Multiplayer.GetCarChallenges( class, false )
	end

	for i, v in ipairs( Packs ) do		
			if v.locked == false then

				local Challenges = Multiplayer.GetChallenges( v.category )

				for j, w in ipairs( Challenges ) do		
					
					if v.category == UIEnums.MpChallengePack.Car then
					
						for classes = 1, #carChallenges, 1 do
							for cars =1,  #carChallenges[classes], 1 do		
								if carChallenges[classes][cars].progress > 0.75 and carChallenges[classes][cars].complete == false then

									GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
									return true
								
								end
							end
						end
						
					else
						if w.progress > 0.75 and w.complete == false then

							GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
							return true
					
						end
					end
				end
			end
		end

	return false
end

function CheckMPUpcomingUnlocksFanStatus()

	if Multiplayer.POBUpcomingUnlocksInfo().rankUp == true then		
		--need to make sure we actually have somthing to show
		local next_rank = Multiplayer.POBGetCurrentRank() + 1
		local rank_info = Multiplayer.POBProgressionForRank(next_rank)	
		local playlist_info = Multiplayer.POBGetPlaylistUnlocks(next_rank)	
		
		local canShowCar = false
		local canShowAbility = false
		local canShowPlaylist = false
		
		if rank_info.unlocks.cars ~= nil then
			if #rank_info.unlocks.cars > 0 then
				canShowCar = true
			end
		end
		
		if rank_info.unlocks.abilities ~= nil then
			if #rank_info.unlocks.abilities > 0 then
				canShowAbility = true
			end
		end
		
		if playlist_info.playlists ~= nil then	
			if #playlist_info.playlists > 0 then
				canShowPlaylist = true
			end
		end	
		
		if canShowCar == true or  canShowAbility == true or canShowPlaylist == true then		
			GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
			return true
		end
	end

	return false
end

function CheckMPUnlocksFanStatus()

	if GUI.mp_game_info.RanksGained > 0 then
		--need to make sure we actually have somthing to show
		local current_rank = Multiplayer.POBGetCurrentRank()
		local rank_info = Multiplayer.POBProgressionForRank(current_rank)	
		local playlist_info = Multiplayer.POBGetPlaylistUnlocks(current_rank)		
			
		local canShowCar = false
		local canShowAbility = false
		local canShowPlaylist = false
		
		if rank_info.unlocks.cars ~= nil then
			if #rank_info.unlocks.cars > 0 then
				canShowCar = true
			end
		end
		
		if rank_info.unlocks.abilities ~= nil then
			if #rank_info.unlocks.abilities > 0 then
				canShowAbility = true
			end
		end
		
		if playlist_info.playlists ~= nil then	
			if #playlist_info.playlists > 0 then
				canShowPlaylist = true
			end
		end	
		
		if canShowCar == true or  canShowAbility == true or canShowPlaylist == true then	
			GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
			return true
		end
	end

	return false
end

function CheckMPUnlocksChallenges()

	if GUI.mp_game_info.LastChallengeCompleted ~= -1 then
		
		GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
		return true
	end

	return false
end

function CheckMPAvailableCars()

	GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1

	return true
end

function CheckMPAvailableMods()

	GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1

	return true
end

function CheckMPAvailablePlaylists()

	GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1

	return true
end

function CheckMPStats()

	if GUI.mp_game_info.Races > 0 then
		
		GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
		return true
	end

	return false
end

function CheckMPCommunity()
--CANNED
	--GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1

	return false
end

function CheckMPProgression()

	local Packs = Multiplayer.GetChallengePacks()

	local totalProgress = 0
	local numChallenges = 0 -- this is the number of challenge packs

	for i, v in ipairs( Packs ) do		
		totalProgress = totalProgress + v.progress 
		numChallenges = numChallenges + 1
	end

	if numChallenges > 0 then
		
		GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
		return true
	end
	
	return false

end

function CheckMPLeaderboard()

	if net_CanReconnectToDemonware() == true then
		
		GUI.numMPScreensChosen = GUI.numMPScreensChosen + 1
		return true
	end
end

function CheckSharedUpcomingUnlocksAchievements()
	local achievements = GameData.POBGetUpcomingAchievements()
		
	if achievements.stickers ~= nil then
		if #achievements.stickers > 0 then
			return true
		end
	end
		
	return false
end

function CheckSharedUnlocksAchievements()

	return true
end

function CheckSharedSocialNetworksTwitter()

	return true
end

function CheckSharedSocialNetworksFacebook()

	return true
end

-- SCREEN TITLE -------------------------------------------------------

function SetupPOBScreenTitle( parent_id, title_text, subtitle_text, icon_name, filename, new_colour, isTexture )

	if isTexture == nil then
		isTexture = false
	end
	
	local node_id = nil 
	
	node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_screen_title_top" )
		
	if node_id ~= nil then
	
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local title_text_id = UIButtons.FindChildByName( node_id, "_title_text" )
		local subtitle_text_id = UIButtons.FindChildByName( node_id, "_subtitle_text" )
		local icon_id	= UIButtons.FindChildByName( node_id, "_icon" )
		local underline_id = UIButtons.FindChildByName( node_id, "_underline")
		local texture_id = UIButtons.FindChildByName( node_id, "_icon_texture_light")
		
		--change the title text
		if title_text ~= nil then
			UIButtons.ChangeText(title_text_id,  title_text )
		else
			UIButtons.SetActive( title_text_id, false )
		end
		
		--change the subtitle text
		if title_text ~= nil then
			UIButtons.ChangeText(subtitle_text_id,  subtitle_text )
		else
			UIButtons.SetActive( subtitle_text_id, false )
		end
		
		--change the title icon
		if isTexture == false then
		
			if icon_name == nil then
				UIButtons.SetActive( icon_id, false )
			else	
				if filename ~= nil then
					UIShape.ChangeSceneName( icon_id, filename )
				end
				
				-- print(new_colour)
				if new_colour ~= nil then
					-- print("Changing the colour")
					UIButtons.ChangeColour(icon_id, new_colour)
				end
				
				UIShape.ChangeObjectName( icon_id, icon_name )
			end
		
		else
			UIButtons.SetActive( icon_id, false )
			UIButtons.SetActive( texture_id, true )
			-- print("Texture is active")
		end

		--set the parent of the title
		UIButtons.SetParent( node_id, parent_id, UIEnums.Justify.MiddleCentre )
		
		return UIButtons.FindChildByName( node_id, "_screen_position" )
		
	else
		print("SetupPOBScreenTitle - ERROR")	
	end
	
	

end

function SetupDefaultBossScreen( parent_id, video_id, unlocked )

		local Sp = { }
		Sp.BossInfo = SinglePlayer.BossInfo()

		local tier = nil
		local boss_name = nil
		local boss_id = nil
		local score = nil

		if unlocked == true then	
			boss_id = GetNewBossUnlocked()
			if boss_id ~= nil then
				for i = 1, #GUI.sp_tier_info , 1 do		
					if GUI.sp_tier_info[i].unlocked == true and GUI.sp_tier_info[i].bossId == boss_id then
						tier = i
					end
				end
			end
			
		else
		
			for i = 1, #GUI.sp_upcoming_unlocks, 1 do			
				if GUI.sp_upcoming_unlocks[i].kind == "boss" then
					tier = GUI.sp_upcoming_unlocks[i].tier
					boss_id = GUI.sp_upcoming_unlocks[i].bossId
					score = GUI.sp_upcoming_unlocks[i].score
				end
			end
			
		end

		if  boss_id ~= nil then
			boss_name = Amax.GetTierFromBossId( boss_id )
			
			--setup the boss logo
			local bossLogonodename = "_b" .. tier .. "_l1"
			local boss_logo_id = UIButtons.FindChildByName(parent_id, bossLogonodename)
			UIButtons.SetActive( boss_logo_id, true ) 
			
			local boss_car_filename = GameData.GetVehicle( Sp.BossInfo[ tier ].vehicle_id ).tag 
			
			--show boss car image
			UIButtons.SetActive( video_id, true ) 			
			UIButtons.ChangeTexture( { filename = boss_car_filename }, 0,  video_id)
			
			if  UIGlobals.CurrentLanguage ~= UIEnums.Language.English then
				for i, v in ipairs( GUI.sp_tier_info ) do		
					if i == tier then
						--add subtitle for non english languages					
						local subtitleId =  UIButtons.FindChildByName( parent_id, "TS_Subtitle" )						
						UIButtons.SetActive( subtitleId, true )
						UIButtons.ChangeText( subtitleId, v.name )
						UIButtons.SetParent( subtitleId, boss_logo_id, UIEnums.Justify.MiddleCentre  )

					end
				end			
			end
			
		else
			print("SetupDefaultBossScreen - ERROR")
		end
end

-- //SCREEN SETUP AND FUNCTIONS TO FILL THE SCREENS IN  ----------------------------------------------------------------------------------------------------------------------------------------

function SetupTestScreen()
	
	for i = 1, GUI.maxScreens, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_test_screen" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, parent_id, UIEnums.Justify.MiddleCentre )

		GUI.rows_id[ i ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, }
	end

end

-- //DLC ----------------------------------------------------------------------------------------------------------------------------------------

function SetupDLCNewInfoScreen( parent_id )
	 print("Setup DLC New Info Screen")
	 
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)				

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_dlc_new_dlc_info" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )	
		
		local screensToShow = {}
		
		if Amax.CheckIsCommunityNewsNew() == true then
			screensToShow[(#screensToShow + 1)] = "CommunityNews"
		end
		
		if Amax.CheckIsDLCNew() == true then
			screensToShow[(#screensToShow + 1)] = "DLC"
		end
		
		if Amax.CheckIsNewsNew() == true then
			screensToShow[(#screensToShow + 1)] = "News"
		end
		
		if #screensToShow > 0 then
			local screen_to_show = math.random( #screensToShow )			
			
			if  screensToShow[screen_to_show] == "News" then
				local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_DLC_TITLE, UIText.POB_DLC_SUBTITLE, "alert", "common_icons", "Support_0")
				UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
				UIButtons.ChangeText(UIButtons.FindChildByName( node_id, "_tms_title" ),UIText.POB_DLC_TITLE_NEWS)			
				UIButtons.ChangeText(UIButtons.FindChildByName( node_id, "_tms_text" ),UIText.POB_NEWS_TMS_TEXT)
				UIButtons.SetActive( UIButtons.FindChildByName( node_id, "_BlurLogo" ), true )
				Amax.SetNewNewsId()
			end
			
			if screensToShow[screen_to_show] == "DLC" then
				local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_DLC_TITLE, UIText.POB_DLC_SUBTITLE, "alert", "common_icons", "Support_0")
				UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
				UIButtons.ChangeText(UIButtons.FindChildByName( node_id, "_tms_title" ),UIText.POB_DLC_TITLE_DLC)	
				UIButtons.ChangeText(UIButtons.FindChildByName( node_id, "_tms_text" ),UIText.POB_DLC_TMS_TEXT)
				UIButtons.SetActive( UIButtons.FindChildByName( node_id, "_icon" ), true )
				UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_icon" ), "dlc" )
				Amax.SetNewDlcId()
			end
			
			if screensToShow[screen_to_show] == "CommunityNews" then
				local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_DLC_TITLE, UIText.POB_DLC_SUBTITLE, "alert", "common_icons", "Support_0" )
				UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
				UIButtons.ChangeText(UIButtons.FindChildByName( node_id, "_tms_title" ),UIText.POB_DLC_TITLE_COMMUNITY)	
				UIButtons.ChangeText(UIButtons.FindChildByName( node_id, "_tms_text" ),UIText.POB_COMMUNITY_TMS_TEXT)
				UIButtons.SetActive( UIButtons.FindChildByName( node_id, "_icon" ), true )
				UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_icon" ), "community_events" )
				Amax.SetNewCommunityNewsId()
			end		
		end

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "DLCNewInfoScreen" }
	end

end

function FillDLCNewInfoScreen( screen_id )
	print("DLCNewInfoScreen")
end

-- //Friend Demand Updates ----------------------------------------------------------------------------------------------------------------------------------------

function SetupFriendDemandBeatenAndReturned( parent_id )
	 print("Setup Friend Demand Beaten And Returned")
	 
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_FRIEND_DEMANDS_TITLE_SP, UIText.POB_FRIEND_DEMAND_RETURNED_SUBTITLE, "message", "common_icons" )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_friend_demand_beaten_and_returned" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		
		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "FriendDemandBeatenAndReturned" }
		
		
		if FriendDemand.ReadFinished() == true then
	
			if FriendDemand.NewFriendDemandsReChallengeRecieved() == true then
				local blurbs = FriendDemand.GetNewFriendDemandsReChallengeRecieved()
				local blurb_to_show = math.random( #blurbs )
				
				POBCreateBlurb( UIButtons.FindChildByName( node_id, "_node_list" ), UIButtons.FindChildByName( node_id, "_panel_dummy" ), blurbs[blurb_to_show] )		
			end
	
		end		

	end

end

function POBCreateBlurb( node_list_id, panel_node_id, blurb )
	
	local is_server = ( blurb.type == UIEnums.FriendDemandMessageType.Server )
	local msg_prefix = Select( is_server, "GAME_BLURB_SERVER_MSG", "GAME_BLURB_NORMAL_MSG" )
	
	local blurb_node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "pob_blurb_node" )
	UIButtons.ChangePanel( blurb_node_id, UIEnums.Panel._3DAA_WORLD, true )

	local fd_info = FriendDemand.GetInfo( blurb.index, is_server )

	local event_info = SinglePlayer.EventInfo( fd_info.eventid_sp )

	UIShape.ChangeObjectName( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_icon" ), Sp_KindToShape( event_info.kind ) )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_event_text" ), msg_prefix .. blurb.index .. "_SENDER" )
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_name" ), msg_prefix .. blurb.index .. "_NAME" )
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_info_time_sent" ), msg_prefix .. blurb.index .. "_TIME_SENT" )
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_info_days_left" ), msg_prefix .. blurb.index .. Select( is_server, "_DAYS_LEFT", "_ATTEMPTS" ) )
	
	local type_icon_id = UIButtons.FindChildByName( blurb_node_id, "pob_blurb_type_icon" )
	UIShape.ChangeObjectName( type_icon_id, Select( is_server, "challenge_received", "challenge_open" ) )
	UIButtons.ChangeColour( type_icon_id, Select( fd_info.rechallenge, "Support_4", "Support_0" ) )
	
	if UIGlobals.FriendDemandFilter == 3 then
		UIButtons.ChangeText( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_sender" ), msg_prefix .. blurb.index .. "_SENDER" )
		UIButtons.SetActive( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_icon" ), false )
		UIButtons.SetActive( UIButtons.FindChildByName( blurb_node_id, "pob_blurb_event_text" ), false )
	end
	
	UIButtons.AddListItem( node_list_id, blurb_node_id, 1 )
	GUI.node_ids[1] = blurb_node_id

	local panel_id = panel_node_id
	POB_Fd_CreatePanel( panel_id, true )
	UIButtons.ChangePanel( panel_id, UIEnums.Panel._3DAA_WORLD, true)
	POB_Fd_UpdatePanel( blurb.index, true )	
	
end

function FillFriendDemandBeatenAndReturned( screen_id )
	print("Fill Friend Demand Beaten And Returned")
end


function SetupFriendDemandNew( parent_id )
	 print("Setup Friend Demand New")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_FRIEND_DEMANDS_TITLE_SP, UIText.POB_NEW_FRIEND_DEMAND_SUBTITLE, "message", "Common_icons" )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_friend_demand_new" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "FriendDemandNew" }
		
		if FriendDemand.ReadFinished() == true then
			
			if FriendDemand.NewFriendDemandsRecieved() == true then
				local blurbs = FriendDemand.GetNewFriendDemandsRecieved()
				local blurb_to_show = math.random( #blurbs )
				POBCreateBlurb( UIButtons.FindChildByName( node_id, "_node_list" ), UIButtons.FindChildByName( node_id, "_panel_dummy" ), blurbs[blurb_to_show] )					
			end
			
		end	
		
	end

end

function FillFriendDemandNew( screen_id )
	print("Fill Friend Demand New")
end

function SetupFriendDemandBeaten( parent_id )
	 print("Setup Friend Demand Beaten")	 
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_FRIEND_DEMANDS_TITLE, UIText.POB_FRIEND_DEMAND_BEATEN_SUBTITLE, GUI.Lock.icon, GUI.Lock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_friend_demand_beaten" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "FriendDemandBeaten" }
	end

end

function FillFriendDemandBeaten( screen_id )
	print("Fill Friend Demand Beaten")
end

-- //UPCOMING UNLOCKS  ----------------------------------------------------------------------------------------------------------------------------------------

function SetupUpcomingUnlocksBossBattleScreen( parent_id )
	 print("Setup Upcoming Unlocks Boss Battle Screen")

	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)

		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UPCOMING_UNLOCKS_TITLE_SP, UIText.POB_UPCOMING_UNLOCKS_BOSS_BATTLE_SUBTITLE, GUI.Lock.icon, GUI.Lock.filename )		
		print("screen_position_id", screen_position_id)
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_upcoming_unlocks_boss_battle" )
		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		
		
		local boss_template_node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_boss_template" )	
			
		local text_position_id = UIButtons.FindChildByName( boss_template_node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( boss_template_node_id, "_debug_icon" )
		local progression_text_id	= UIButtons.FindChildByName( boss_template_node_id, "_progress_amount" )
		local boss_video_id = UIButtons.FindChildByName( boss_template_node_id, "_boss_video")		
		local boss_image_icon_id = UIButtons.FindChildByName( boss_template_node_id,"_boss_image_icon")
		
		UIButtons.SetParent( boss_template_node_id, node_id, UIEnums.Justify.MiddleCentre )
		
		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UpcomingUnlocksBossBattleScreen", }

		SetupDefaultBossScreen( node_id, boss_video_id, false)
			
	end
end

function FillUpcomingUnlocksBossBattleScreen( screen_id )
	print("FillUpcomingUnlocksBossBattleScreen")
end

function SetupUpcomingUnlocksNewTierScreen( parent_id )
	 print("Setup Upcoming Unlocks New Tier Screen")

	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UPCOMING_UNLOCKS_TITLE_SP, UIText.POB_UPCOMING_UNLOCKS_TIER_UNLOCK_SUBTITLE, GUI.Lock.icon, GUI.Lock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_upcoming_unlocks_new_tier" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local boss_cutout_id = UIButtons.FindChildByName( node_id, "_boss_image_icon" )
		local number_of_lights_needed_id = UIButtons.FindChildByName( node_id, "_lights_number" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UnlocksNewTierScreen", }

		local tier = nil
		local score = nil
		local numberOfLightsNeeded = nil

		for i = 1, #GUI.sp_upcoming_unlocks, 1 do
			if GUI.sp_upcoming_unlocks[i].kind == "tier" then
				tier = GUI.sp_upcoming_unlocks[i].tier
				score = GUI.sp_upcoming_unlocks[i].score
				numberOfLightsNeeded = GUI.sp_upcoming_unlocks[i].lightsNeeded
			end
		end
		
		if tier ~= nil then
			--setup the boss logo 1
			local bossLogo1nodename = "_b" .. tier .. "_l1"
			local boss_logo1_id = UIButtons.FindChildByName(node_id, bossLogo1nodename)
			UIButtons.SetActive( boss_logo1_id, true ) 
			
			--setup the boss logo 2
			local bossLogo2nodename = "_b" .. tier .. "_l2"
			local boss_logo2_id = UIButtons.FindChildByName(node_id, bossLogo2nodename)
			UIButtons.SetActive( boss_logo2_id, true ) 
						
			--change boss image
			UIButtons.ChangeTexture( { filename = "boss" .. tier .. "_cutout" }, 0,  boss_cutout_id)
			if tier == #GUI.sp_tier_info then
				UIButtons.TimeLineActive( "HideBoss9", true, 0)
			end
			
			--change the text stating the number of lights needed
			UIButtons.ChangeText(number_of_lights_needed_id,  "SPL_POB_LIGHTS_NEEDED" .. numberOfLightsNeeded )
			
			if  UIGlobals.CurrentLanguage ~= UIEnums.Language.English then
				for i, v in ipairs( GUI.sp_tier_info ) do		
					if i == tier then
						--add subtitle for non english languages					
						local subtitleId =  UIButtons.FindChildByName( parent_id, "TS_Subtitle" )						
						UIButtons.SetActive( subtitleId, true )
						UIButtons.ChangeText( subtitleId, v.name )
						UIButtons.SetParent( subtitleId, boss_logo1_id, UIEnums.Justify.MiddleCentre  )
					end
				end			
			end
			
		end
			
	end
end

function FillUpcomingUnlocksNewTierScreen( screen_id )
	print("FillUpcomingUnlocksNewTierScreen")
end

function SetupUpcomingUnlocksNewFanStatus( parent_id )
	 print("Setup Upcoming Unlocks New Fan Status")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UPCOMING_UNLOCKS_TITLE_SP, UIText.POB_UPCOMING_UNLOCKS_FAN_STATUS_UNLOCK_SUBTITLE, GUI.Lock.icon, GUI.Lock.filename )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_upcoming_unlocks_new_fan_status" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )

		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		
		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UpcomingUnlocksNewFanStatus", }

		local aboutToRankUp = false
					
			local rank = nil		
			local score = nil	
			local current_fans = nil
			local next_rank_fans = nil
			local previous_rank_fans = nil
		
		for i = 1, #GUI.sp_upcoming_unlocks, 1 do
			if GUI.sp_upcoming_unlocks[i].kind == "rank" then
				aboutToRankUp = true				
				rank = GUI.sp_upcoming_unlocks[i].rank	
				score = GUI.sp_upcoming_unlocks[i].score	
				current_fans = GUI.sp_upcoming_unlocks[i].current_fans
				next_rank_fans = GUI.sp_upcoming_unlocks[i].next_rank_fans
				previous_rank_fans = GUI.sp_upcoming_unlocks[i].previous_rank_fans				
			end
		end
		
		if aboutToRankUp == true then --and rank < 25 then
			local rank_info = SinglePlayer.RankInfo()

			-- //go through all of the vehicles and add a pic to the screen
			if rank_info[GUI.sp_game_info.rank +1].vehicles ~= nil then

				local possibleVehiclesToShow = {}				

				for i, v in pairs(rank_info[GUI.sp_game_info.rank + 1].vehicles) do
					
					possibleVehiclesToShow[#possibleVehiclesToShow + 1] = 
					{
						-- this references the texture aswell as the icon
						icon = GameData.GetVehicle( i ).icon,
						name = GameData.GetVehicle( i ).name_full,
						class = GameData.GetVehicle( i ).class,
						style = GameData.GetVehicle( i ).style,
					}
					
				end
				
				local classes = 
				{
					"class_a",
					"class_b",
					"class_c",
					"class_d",
				}
				
				local style_names =
				{
					[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
					[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
					[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
					[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
					[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
					[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
				}
				
				--choose vehicle at random
				vehicle_to_show = math.random( #possibleVehiclesToShow )
				
				--change the class icon
				local class_icon_id = UIButtons.FindChildByName( node_id, "_class_icon" )
				UIShape.ChangeObjectName( class_icon_id, classes[ possibleVehiclesToShow[vehicle_to_show].class + 1 ] )
				
				--change style text
				local style_text_id = UIButtons.FindChildByName( node_id, "_stat_style_text" )
				UIButtons.ChangeText( style_text_id , style_names[ possibleVehiclesToShow[vehicle_to_show].style ] )
				
				--change the rank text
				local rank_text_id = UIButtons.FindChildByName( node_id, "_rank_text" )
				UIButtons.ChangeText( rank_text_id , "SPL_POB_NEXT_RANKUP_TEXT" .. (GUI.sp_game_info.rank + 1) )
				
				--change the name of the vehicle
				local vehicle_name_text_id = UIButtons.FindChildByName( node_id, "_vehcile_name_text" )
				UIButtons.ChangeText( vehicle_name_text_id , possibleVehiclesToShow[vehicle_to_show].name )
				
				--change the vehicle texture
				--local vehicle_texture_id = UIButtons.FindChildByName( node_id, "_vehicle_image" )
				--UIButtons.ChangeTexture( { filename = possibleVehiclesToShow[vehicle_to_show].icon }, 0,  vehicle_texture_id)
				
				--fan bar
				local UnlockBar_Fill_Progress_id = UIButtons.FindChildByName( node_id, "UnlockBar_Fill_Progress" )
				local sx = UIButtons.GetSize(UnlockBar_Fill_Progress_id)
				
				StartPOBProcessFanGain( 2.5, UnlockBar_Fill_Progress_id, sx, UIButtons.FindChildByName( node_id, "Amount"), UIButtons.FindChildByName( node_id, "Amount_Total_Level"), UIButtons.FindChildByName( node_id, "Amount_Target"), UIButtons.FindChildByName( node_id, "txt_Fan_Status"), current_fans, previous_rank_fans, (next_rank_fans - previous_rank_fans), previous_rank_fans, next_rank_fans)
				
			end			
		end
	end

end

function FillUpcomingUnlocksNewFanStatus( screen_id )
	print("Fill Upcoming Unlocks New Fan Status" )
end


--  //UNLOCKS ----------------------------------------------------------------------------------------------------------------------------------------



function SetupUnlocksNewTierScreen( parent_id )
	 print("Setup Unlocks New Tier Screen")

	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UNLOCKS_TITLE_SP, UIText.POB_UNLOCKS_TIER_UNLOCK_SUBTITLE, "alert", "common_icons", "Support_0" )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_unlocks_new_tier" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local boss_cutout_id = UIButtons.FindChildByName( node_id, "_boss_image_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UnlocksNewTierScreen", }

		local tier = nil
		
		tier = GUI.sp_game_info.lastSessionTier
		if tier ~= nil then
			--setup the boss logo 1
			local bossLogo1nodename = "_b" .. tier .. "_l1"
			local boss_logo1_id = UIButtons.FindChildByName(node_id, bossLogo1nodename)
			UIButtons.SetActive( boss_logo1_id, true ) 
			
			--setup the boss logo 2
			local bossLogo2nodename = "_b" .. tier .. "_l2"
			local boss_logo2_id = UIButtons.FindChildByName(node_id, bossLogo2nodename)
			UIButtons.SetActive( boss_logo2_id, true ) 
						
			--change boss image
			UIButtons.ChangeTexture( { filename = "boss" .. tier .. "_cutout" }, 0,  boss_cutout_id)
			if tier == #GUI.sp_tier_info then
				UIButtons.TimeLineActive( "HideBoss9", true, 0)
			end
			
			if  UIGlobals.CurrentLanguage ~= UIEnums.Language.English then
				for i, v in ipairs( GUI.sp_tier_info ) do		
					if i == tier then
						--add subtitle for non english languages					
						local subtitleId =  UIButtons.FindChildByName( parent_id, "TS_Subtitle" )						
						UIButtons.SetActive( subtitleId, true )
						UIButtons.ChangeText( subtitleId, v.name )
						UIButtons.SetParent( subtitleId, boss_logo1_id, UIEnums.Justify.MiddleCentre )

					end
				end			
			end
			
		end

	end
end

function FillUnlocksNewTierScreen( screen_id )
	print("FillUnlocksNewTierScreen")
end

function SetupUnlocksNewFanStatusScreen( parent_id )
	 print("Setup Unlocks New Fan Status Screen")

	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)

		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UNLOCKS_TITLE_SP, UIText.POB_UNLOCKS_FAN_STATUS_UNLOCK_SUBTITLE, "alert", "common_icons", "Support_0" )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_unlocks_new_fan_status" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )

		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		
		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UnlocksNewFanStatusScreen", }
		
		if GUI.sp_game_info.lastSessionRankedUp == true then
			local rank_info = SinglePlayer.RankInfo()
			
			-- //go through all of the vehicles and add a pic to the screen
			if rank_info[GUI.sp_game_info.rank].vehicles ~= nil then

				local possibleVehiclesToShow = {}
				

				for i, v in pairs(rank_info[GUI.sp_game_info.rank].vehicles) do
					
					possibleVehiclesToShow[#possibleVehiclesToShow + 1] = 
					{
						-- this references the texture aswell as the icon
						icon = GameData.GetVehicle( i ).icon,
						name = GameData.GetVehicle( i ).name_full,
						class = GameData.GetVehicle( i ).class,
						style = GameData.GetVehicle( i ).style,
					}
					
				end
				
				local classes = 
				{
					"class_a",
					"class_b",
					"class_c",
					"class_d",
				}
				
				local style_names =
				{
					[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
					[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
					[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
					[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
					[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
					[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
				}
				
				--choose vehicle at random
				vehicle_to_show = math.random( #possibleVehiclesToShow )
				
				--change the class icon
				local class_icon_id = UIButtons.FindChildByName( node_id, "_class_icon" )
				UIShape.ChangeObjectName( class_icon_id, classes[ possibleVehiclesToShow[vehicle_to_show].class + 1 ] )
				
				--change style text
				local style_text_id = UIButtons.FindChildByName( node_id, "_stat_style_text" )
				UIButtons.ChangeText( style_text_id , style_names[ possibleVehiclesToShow[vehicle_to_show].style ] )
				
				--change the rank text
				local rank_text_id = UIButtons.FindChildByName( node_id, "_rank_text" )
				UIButtons.ChangeText( rank_text_id , "SPL_POB_RANKUP_TEXT" .. GUI.sp_game_info.rank )
				
				--change the name of the vehicle
				local vehicle_name_text_id = UIButtons.FindChildByName( node_id, "_vehcile_name_text" )
				UIButtons.ChangeText( vehicle_name_text_id , possibleVehiclesToShow[vehicle_to_show].name )
				
				--change the vehicle texture
				local vehicle_texture_id = UIButtons.FindChildByName( node_id, "_vehicle_image" )
				UIButtons.ChangeTexture( { filename = possibleVehiclesToShow[vehicle_to_show].icon }, 0,  vehicle_texture_id)
				
			end

		end
	end
	
end

function FillUnlocksNewFanStatusScreen( screen_id )
	print("FillUnlocksNewFanStatusScreen")
end

function SetupUnlocksNewBossBattle( parent_id )
	 print("Setup Unlocks New Boss Battle")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UNLOCKS_TITLE_SP, UIText.POB_UNLOCKS_BOSS_BATTLE_UNLOCK_SUBTITLE, "alert", "common_icons", "Support_0"  )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_unlocks_new_boss_battle" )
		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		
		local boss_template_node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_boss_template" )	
			
		local text_position_id = UIButtons.FindChildByName( boss_template_node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( boss_template_node_id, "_debug_icon" )
		local progression_text_id	= UIButtons.FindChildByName( boss_template_node_id, "_progress_amount" )
		local boss_video_id = UIButtons.FindChildByName( boss_template_node_id, "_boss_video")		
		local boss_image_icon_id = UIButtons.FindChildByName( boss_template_node_id,"_boss_image_icon")
		local event_text_id = UIButtons.FindChildByName( boss_template_node_id, "_Event_text")
		
		UIButtons.ChangeText( event_text_id, UIText.POB_UNLOCKS_BOSS_BATTLE_TEXT_3 )
		
		UIButtons.SetParent( boss_template_node_id, node_id, UIEnums.Justify.MiddleCentre )

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UnlocksNewBossBattle" }
	
		SetupDefaultBossScreen( node_id, boss_video_id, true )

	end

end

function FillUnlocksNewBossBattle( screen_id )
	print("Fill Unlocks New Boss Battle" )
end

function SetupUnlocksNewBossDefeated( parent_id )
	 print("Setup Unlocks New Boss Defeated")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)

		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UNLOCKS_TITLE_SP, UIText.POB_UNLOCKS_BOSS_DEFEATED_UNLOCK_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_unlocks_new_boss_defeated" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UnlocksNewBossDefeated" }
		
	--Change The picture of the boss
		local tier = nil
		local boss_name = nil
		local boss_id = GetNewBossDefeated()

		if boss_id ~= nil then

			for i = 1, #GUI.sp_tier_info , 1 do		
				if GUI.sp_tier_info[i].unlocked == true and GUI.sp_tier_info[i].bossId == boss_id then
					tier = i
					boss_name = Amax.GetTierFromBossId( boss_id )
					UIButtons.ChangeTexture( { filename = "boss_" .. tier }, 0, UIButtons.FindChildByName( node_id, "_boss_image_icon" ) )
				end
			end

		end
	end

end

function FillUnlocksNewBossDefeated( screen_id )
	print("Fill Unlocks New Boss Defeated" )
end



-- //MILESTONES  ----------------------------------------------------------------------------------------------------------------------------------------


function SetupMilestonesSPGameProgressScreen( parent_id )
	 print("Setup Milestones SP Game Progress Screen")

	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_MILESTONES_TITLE_SP, UIText.POB_MILESTONES_SP_SUBTITLE, nil, nil, nil,  true )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_milestones_sp_game_progress" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )

		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )	

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MilestonesSPGameProgressScreen", }
		
	
		local sp_progression = math.floor((GUI.sp_game_info.totalStars / GUI.sp_game_info.maxStars)* 100)
		
		UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_light_progress_text") , "SPL_POB_TOTAL_LIGHTS" .. GUI.sp_game_info.totalStars .. "_" .. GUI.sp_game_info.maxStars )		
	
		AddCountUpValue( 1.5, 0, sp_progression, UIButtons.FindChildByName( node_id, "_progress_text"), "GAME_PERCENTAGE_" )
		
	end
end

function FillMilestonesSPGameProgressScreen( screen_id )
	print("Fill Milestones SP Game Progress Screen")
end

-- //AVAILABLE OR INCOMPLETE ----------------------------------------------------------------------------------------------------------------------------------------


function SetupAvailableBossBattle( parent_id )
	 print("Setup Available Boss Battle")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_AVAILABLE_INCOMPLETE_TITLE, UIText.POB_AVAILABLE_BOSSES_TITLE_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_available_boss_battle" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "AvailableBossBattle" }
	

		boss_node_list_id = UIButtons.FindChildByName( node_id, "_boss_list" )


		local currentBossCount = 0

		for i = 1, #GUI.sp_boss_info , 1 do		
			if (GUI.sp_boss_info[i].unlocked == true) and (GUI.sp_boss_info[i].defeated == false) then

				currentBossCount = currentBossCount + 1
				local boss_node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_boss_node" )			
				local boss_image_id = UIButtons.FindChildByName( boss_node_id, "_boss_image_icon" )

				UIButtons.SetParent( boss_node_id, boss_node_list_id, UIEnums.Justify.MiddleCentre )

				local px, py, pz = UIButtons.GetPosition( boss_node_id )
				UIButtons.ChangePosition( boss_node_id,(currentBossCount * GUI.distanceBetweenIcons)*0.5 , py, pz )

				--Change The picture of the boss
				local tier = nil
				local boss_name = nil
				local boss_id = GUI.sp_boss_info[i].id

				if boss_id ~= nil then
					for k = 1, #GUI.sp_tier_info , 1 do		
						if GUI.sp_tier_info[k].unlocked == true and GUI.sp_tier_info[k].bossId == boss_id then
							tier = k
							boss_name = Amax.GetTierFromBossId( boss_id )
							UIButtons.ChangeTexture( { filename = "boss_" .. tier }, 0, UIButtons.FindChildByName( boss_node_id, "_boss_image_icon" ) )
						end
					end
				end

			end
		end
	
	end

end

function FillAvailableBossBattle( screen_id )
	print("Fill Available Boss Battle" )
end

function SetupAvailableCars( parent_id )
	 print("Setup Available Cars")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_AVAILABLE_INCOMPLETE_TITLE, UIText.POB_AVAILABLE_CARS_TITLE_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_available_unused_cars" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "AvailableCars" }
	

		car_node_list_id = UIButtons.FindChildByName( node_id, "_vehicle_list" )
		local currentCarCount = 0

		for i, v in pairs( GUI.sp_vehicle_info ) do
			if v.unlocked == true and v.new == true then

				local car_id = v.id
				local car_name = v.name

				currentCarCount = currentCarCount + 1
				local vehicle_node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_unlocks_new_fan_status_vehicle_icon" )			
				local vehicle_image_id = UIButtons.FindChildByName( vehicle_node_id, "_vehicle_icon" )
			
				UIButtons.SetParent( vehicle_node_id, car_node_list_id, UIEnums.Justify.MiddleCentre )

				local px, py, pz = UIButtons.GetPosition( vehicle_node_id )
				UIButtons.ChangePosition( vehicle_node_id,(currentCarCount * GUI.distanceBetweenIcons) , py, pz )

				UIShape.ChangeObjectName( vehicle_image_id, GameData.GetVehicle( car_id ).icon )
			end
		end	
	end

end

function FillAvailableCars( screen_id )
	print("Fill Available Cars" )
end

function SetupAvailableMods( parent_id )
	 --THIS IS CUT
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_available_unused_mods" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, parent_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "AvailableMods" }
	end

end

function FillAvailableMods( screen_id )
	print("Fill Available mods" )
end

function SetupAvailableEvents( parent_id )
	 print("Setup Available Events")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_AVAILABLE_INCOMPLETE_TITLE, UIText.POB_AVAILABLE_EVENTS_TITLE_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_available_open_events" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "AvailableEvents" }
	
	
		local tier_node_list_id = UIButtons.FindChildByName( node_id, "_tier_list" )

		local currentEventCount = 0

		for i = 1, #GUI.sp_tier_info , 1 do		
			if GUI.sp_tier_info[i].unlocked == true and GUI.sp_tier_info[i].incompleteEvents > 0 then
				
				currentEventCount = currentEventCount + 1
				local tier_node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_tier_openevents_node" )			
				local tier_image_id = UIButtons.FindChildByName( tier_node_id, "_boss_image_icon" )
				local tier_amount_id = UIButtons.FindChildByName( tier_node_id, "_amount_text" )

				UIButtons.SetParent( tier_node_id, tier_node_list_id, UIEnums.Justify.MiddleCentre )

				local px, py, pz = UIButtons.GetPosition( tier_node_id )
				UIButtons.ChangePosition( tier_node_id,(currentEventCount * GUI.distanceBetweenIcons)*0.5 , py, pz )

				--Change The picture of the boss
				local tier = i	

				UIButtons.ChangeTexture( { filename = "boss_" .. tier }, 0, UIButtons.FindChildByName( tier_node_id, "_boss_image_icon" ) )
				UIButtons.ChangeText( tier_amount_id, "SPL_POB_STRING_"..GUI.sp_tier_info[i].incompleteEvents )			

			end
		end	
	end

end

function FillAvailableEvents( screen_id )
	print("Fill Available Events" )
end

function SetupAvailableStars( parent_id )
	 print("Setup Available Stars")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_AVAILABLE_INCOMPLETE_TITLE, UIText.POB_AVAILABLE_STARS_TITLE_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_available_stars" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "AvailableStars" }
	
	
		local tier_node_list_id = UIButtons.FindChildByName( node_id, "_tier_list" )

		local currentEventCount = 0

		for i = 1, #GUI.sp_tier_info , 1 do		
			if GUI.sp_tier_info[i].unlocked == true and GUI.sp_tier_info[i].availableStars > 0 then
				
				currentEventCount = currentEventCount + 1
				local tier_node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_tier_openevents_node" )			
				local tier_image_id = UIButtons.FindChildByName( tier_node_id, "_boss_image_icon" )
				local tier_amount_id = UIButtons.FindChildByName( tier_node_id, "_amount_text" )

				UIButtons.SetParent( tier_node_id, tier_node_list_id, UIEnums.Justify.MiddleCentre )

				local px, py, pz = UIButtons.GetPosition( tier_node_id )
				UIButtons.ChangePosition( tier_node_id,(currentEventCount * GUI.distanceBetweenIcons)*0.5 , py, pz )

				--Change The picture of the boss
				local tier = i	

				UIButtons.ChangeTexture( { filename = "boss_" .. tier }, 0, UIButtons.FindChildByName( tier_node_id, "_boss_image_icon" ) )
				UIButtons.ChangeText( tier_amount_id, "SPL_POB_STRING_"..GUI.sp_tier_info[i].availableStars )			

			end
		end
	
	end

end

function FillAvailableStars( screen_id )
	print("Fill Available Stars" )
end


-- //MULTIPLAYER -- MULTIPLAYER -- MULTIPLAYER -- MULTIPLAYER -- MULTIPLAYER -- MULTIPLAYER -- MULTIPLAYER -- MULTIPLAYER -- MULTIPLAYER

-- //Upcoming Unlocks ----------------------------------------------------------------------------------------------------------------------------------------

function SetupMPUpcomingUnlocksChallenges( parent_id )
	 print("Setup MP Upcoming Unlocks Challenges")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
				
		local screen_position_id= SetupPOBScreenTitle( parent_id, UIText.POB_UPCOMING_UNLOCKS_TITLE_MP, UIText.POB_MPUPCOMING_UNLOCKS_CHALLENGES_SUBTITLE, GUI.Lock.icon, GUI.Lock.filename )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_upcoming_unlocks_challenges" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )
		local progression_text_id = UIButtons.FindChildByName( node_id, "_progress_text" )
		local name_text_id = UIButtons.FindChildByName( node_id, "_progress_name" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPUpcomingUnlocksChallenges" }

		local progress = 0
		local name = nil

		local Packs = Multiplayer.GetChallengePacks()
		local carChallenges = {}
		
		for class = 1, 4, 1 do
			carChallenges[(#carChallenges + 1)] =  Multiplayer.GetCarChallenges( class, false )
		end
				
		local possibleChallengesToShow = {}
		local possibleCarChallengesToShow = {}
		
		for i, v in ipairs( Packs ) do		
			if v.locked == false then

				local Challenges = Multiplayer.GetChallenges( v.category )

				for j, w in ipairs( Challenges ) do		
					
					if v.category == UIEnums.MpChallengePack.Car then
					
						for classes = 1, #carChallenges, 1 do
							for cars =1,  #carChallenges[classes], 1 do		
								if carChallenges[classes][cars].progress > 0.75 and carChallenges[classes][cars].complete == false then
								
								possibleCarChallengesToShow[ (#possibleCarChallengesToShow + 1) ] = 
								{ 
									id = carChallenges[classes][cars].id,
									progress = carChallenges[classes][cars].progress,
									name = w.type,
									tier = w.tier,
									tiers = w.tiers,
									complete = carChallenges[classes][cars].complete,
									icon = GameData.GetVehicle(carChallenges[classes][cars].id).icon,
									sheet = GameData.GetVehicle(carChallenges[classes][cars].id).sheet,
									name_text = GameData.GetVehicle(carChallenges[classes][cars].id).name_full,									
								}
								end
							end
						end
						
					else
						if w.progress > 0.75 and w.complete == false then
						possibleChallengesToShow[ (#possibleChallengesToShow + 1) ] = 
						{						
							progress = w.progress,
							name = w.type,
							tier = w.tier,
							tiers = w.tiers,
							complete = w.complete,
						}
					
						end
					end
				end
			end
		end
		
		local possibleScreensToShow = {}
		
		if #possibleCarChallengesToShow > 0 then
			possibleScreensToShow[#possibleScreensToShow +1] = "Car"
		end
		
		if #possibleChallengesToShow > 0 then
			possibleScreensToShow[#possibleScreensToShow +1] = "Other"
		end
		
		local currently_showing = nil
		if #possibleScreensToShow > 0 then
			currently_showing = possibleScreensToShow[ math.random( #possibleScreensToShow ) ]
		end
		
		if currently_showing == "Car" then
			local challenge_to_show = math.random( #possibleCarChallengesToShow )	
			
			--move everything into the middle of the screen to take account of the car
			local px, py, pz = UIButtons.GetPosition( dummy_id )
			UIButtons.ChangePosition( dummy_id, px + 10, py, pz )
			
			--turn the pic of the car and name of the car on
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "_car_icon" ), true)
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "_car_name" ), true)
			
			--change the picture of the car			
			UIShape.ChangeSceneName( UIButtons.FindChildByName( node_id, "_car_icon" ), possibleCarChallengesToShow[challenge_to_show].sheet )			
			UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_car_icon" ), possibleCarChallengesToShow[challenge_to_show].icon )
			
			--change the name of the car
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_car_name" ), possibleCarChallengesToShow[challenge_to_show].name_text ) 
			
			--set the name of the challenge
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_name" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" ..possibleCarChallengesToShow[challenge_to_show].name .. "_NAME" )
			
			-- change the challenge description
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_desc" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" .. possibleCarChallengesToShow[challenge_to_show].name .. "_DESC")
			
			-- change the challenge progress
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_amount" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" .. possibleCarChallengesToShow[challenge_to_show].name .. "_PROGRESS")
			
			-- change the challenge help
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_help" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" .. possibleCarChallengesToShow[challenge_to_show].name .. "_REWARD")
			
			
			--make the bar grow
			local backing_id = UIButtons.FindChildByName( node_id, "backing" )
			local progress_id = UIButtons.FindChildByName( node_id, "progress" )

			local size_x = UIButtons.GetSize( backing_id )
			local x, size_y = UIButtons.GetSize( progress_id )

			UIButtons.ChangeSize( progress_id, ( size_x * possibleCarChallengesToShow[challenge_to_show].progress ), size_y, 0.0 )
			
			--setup the lights for the tier
			for i = 1, possibleCarChallengesToShow[challenge_to_show].tiers do
				-- turn on each pip
				if i < possibleCarChallengesToShow[challenge_to_show].tier or possibleCarChallengesToShow[challenge_to_show].complete == true then -- completed
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Support_0" )
				else
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Main_Black" )
				end
			end

			local RepeatRewardNodes = {}
			local WinnerRewardNodes = {}
			
			local CarRewards = Multiplayer.GetCarChallengeRewards()
			
			local RewardIcons =
			{
				[UIEnums.MpChallengeRewards.HealthSlot]		= "health",
				[UIEnums.MpChallengeRewards.BonusFans]		= "extra_slot", -- Need to change this to new icon name when we have one
				[UIEnums.MpChallengeRewards.SpeedBoost]		= "horsepower",

				[UIEnums.MpChallengeRewards.Pearlescent]	= "pearl",
				[UIEnums.MpChallengeRewards.Metallic]		= "metallic",
				[UIEnums.MpChallengeRewards.Candy]			= "candy",
			}
			
			-- create car winner rewards		
			local pos_x = 3.75			
			for i = 1, 3 do
				
				if  Multiplayer.ChallengeTypeIsRace(possibleCarChallengesToShow[challenge_to_show].name) == true then
					RepeatRewardNodes[i] = UIButtons.CloneXtGadgetByName( "POB_Items", "reward_node" )

					UIButtons.ChangePosition( RepeatRewardNodes[i], pos_x, -6, 0 )
					UIButtons.SetParent( RepeatRewardNodes[i], UIButtons.FindChildByName( node_id, "challenge_frame"), UIEnums.Justify.BottomLeft )				

					for j = 1, i do
						UIButtons.SetActive( UIButtons.FindChildByName( RepeatRewardNodes[i], "tier_" .. j ), true )
						UIShape.ChangeObjectName( UIButtons.FindChildByName( RepeatRewardNodes[i], "icon" ), RewardIcons[CarRewards["repeat_offender"][j]] )
					end
				end

				-- winner
				if	Multiplayer.ChallengeTypeIsWinner(possibleCarChallengesToShow[challenge_to_show].name) == true then
					WinnerRewardNodes[i] = UIButtons.CloneXtGadgetByName( "POB_Items", "reward_node" )

					UIButtons.ChangePosition( WinnerRewardNodes[i], pos_x, -6, 0 )
					UIButtons.SetParent( WinnerRewardNodes[i], UIButtons.FindChildByName( node_id, "challenge_frame"), UIEnums.Justify.BottomLeft )
					
					for j = 1, i do
						UIButtons.SetActive( UIButtons.FindChildByName( WinnerRewardNodes[i], "tier_" .. j ), true )
						UIShape.ChangeObjectName( UIButtons.FindChildByName( WinnerRewardNodes[i], "icon" ), RewardIcons[CarRewards["winner"][j]] )
					end
				end
					pos_x = pos_x + 8.9
				
			end
			
			local nodes =
			{
				[1] = RepeatRewardNodes,
				[2] = WinnerRewardNodes,
			}
			
			for j = 1, 3 do
				if Multiplayer.ChallengeTypeIsRace(possibleCarChallengesToShow[challenge_to_show].name) == true then
					RefreshCarChallengeRewards( nodes[1][j], possibleCarChallengesToShow[challenge_to_show], j )
				else
					RefreshCarChallengeRewards( nodes[2][j], possibleCarChallengesToShow[challenge_to_show], j )
				end
			end
			
		
		end--end of car challenges
				
		if currently_showing == "Other" then
		
			local challenge_to_show = math.random( #possibleChallengesToShow )	
					
			--set the name of the challenge
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_name" ), "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_NAME" )
			
			-- change the challenge description
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_desc" ), "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_DESC")
			
			-- change the challenge progress
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_amount" ), "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_PROGRESS")
			
			--make the bar grow
			local backing_id = UIButtons.FindChildByName( node_id, "backing" )
			local progress_id = UIButtons.FindChildByName( node_id, "progress" )

			local size_x = UIButtons.GetSize( backing_id )
			local x, size_y = UIButtons.GetSize( progress_id )

			UIButtons.ChangeSize( progress_id, ( size_x * possibleChallengesToShow[challenge_to_show].progress ), size_y, 0.0 )
			
			--setup the lights for the tier
			for i = 1, possibleChallengesToShow[challenge_to_show].tiers do
				-- turn on each pip
				if i < possibleChallengesToShow[challenge_to_show].tier or possibleChallengesToShow[challenge_to_show].complete == true then -- completed
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Support_0" )
				else
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Main_Black" )
				end
			end
			
			--setup the reward bit at the bottom
			local RewardNodes = {}
			local pos_x = 4.5
			for i = 1, 3 do
				RewardNodes[i] = UIButtons.CloneXtGadgetByName( "POB_Items", "fan_reward_node" )

				UIButtons.ChangePosition( RewardNodes[i], pos_x, -12.5, 0 )
				UIButtons.SetParent( RewardNodes[i], UIButtons.FindChildByName( node_id, "challenge_frame"), UIEnums.Justify.BottomLeft )
				pos_x = pos_x + 9.75

				for j = 1, i do
					UIButtons.SetActive( UIButtons.FindChildByName( RewardNodes[i], "tier_" .. j ), true )
				end
			end
			
			for i = 1, 3 do
				local frame_id = UIButtons.FindChildByName( RewardNodes[i], "reward_frame" )
				local fill_id = UIButtons.FindChildByName( RewardNodes[i], "reward_fill" )
				local fans_id = UIButtons.FindChildByName( RewardNodes[i], "fan_icon" )
				local reward_id = UIButtons.FindChildByName( RewardNodes[i], "fans" )
				local tick_id = UIButtons.FindChildByName( RewardNodes[i], "tick" )

				-- change fan reward text
				UIButtons.ChangeText( reward_id, "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_" .. i - 1 .. "_FANS" )
			
				local main_colour = "Main_White"

				-- doesnt have this tier
				if i > possibleChallengesToShow[challenge_to_show].tiers then
					UIButtons.SetActive( RewardNodes[i], false )
				else
					UIButtons.SetActive( RewardNodes[i], true )

					if i < possibleChallengesToShow[challenge_to_show].tier or possibleChallengesToShow[challenge_to_show].complete == true then -- completed
						main_colour = "Support_0"
						UIButtons.SetActive( tick_id, true )

						UIButtons.ChangeColour( frame_id, main_colour )
						UIButtons.ChangeColour( fans_id, main_colour )
						UIButtons.ChangeColour( reward_id, main_colour )
						UIButtons.ChangeColour( fill_id, "Main_Black" )

					elseif i == possibleChallengesToShow[challenge_to_show].tier then -- active
						main_colour = "Main_White"
						UIButtons.SetActive( tick_id, false )

						UIButtons.ChangeColour( fans_id, main_colour )
						UIButtons.ChangeColour( reward_id, main_colour )
						UIButtons.ChangeColour( frame_id, "Main_Black" )
						UIButtons.ChangeColour( fill_id, "Support_2" )

					else -- not active yet
						main_colour = "Main_Black"
						UIButtons.SetActive( tick_id, false )
						
						UIButtons.ChangeColour( frame_id, main_colour )
						UIButtons.ChangeColour( fill_id, main_colour )
						UIButtons.ChangeColour( fans_id, main_colour )
						UIButtons.ChangeColour( reward_id, main_colour )
					end

					for j = 1, 3 do
						UIButtons.ChangeColour( UIButtons.FindChildByName( RewardNodes[i], "tier_" .. j ), main_colour )
					end
				end
			end
			
		end -- end of other challenges
	end

end

function RefreshCarChallengeRewards( reward_node, challenge, index )

	local frame_id = UIButtons.FindChildByName( reward_node, "reward_frame" )
	local fill_id = UIButtons.FindChildByName( reward_node, "reward_fill" )
	local icon_id = UIButtons.FindChildByName( reward_node, "icon" )
	local tick_id = UIButtons.FindChildByName( reward_node, "tick" )

	-- change fan reward text
	local main_colour = "Main_White"

	-- doesnt have this tier
	if index > challenge.tiers then
		UIButtons.SetActive( reward_node, false )
	else
		UIButtons.SetActive( reward_node, true )

		if index < challenge.tier or challenge.complete == true then -- completed
			main_colour = "Support_0"
			UIButtons.SetActive( tick_id, true )

			UIButtons.ChangeColour( frame_id, main_colour )
			UIButtons.ChangeColour( icon_id, main_colour )
			UIButtons.ChangeColour( fill_id, "Main_Black" )

		elseif index == challenge.tier then -- active
			main_colour = "Main_White"
			UIButtons.SetActive( tick_id, false )

			UIButtons.ChangeColour( icon_id, main_colour )
			UIButtons.ChangeColour( frame_id, "Main_Black" )
			UIButtons.ChangeColour( fill_id, "Support_2" )

		else -- not active yet
			main_colour = "Main_Black"
			UIButtons.SetActive( tick_id, false )
			
			UIButtons.ChangeColour( frame_id, main_colour )
			UIButtons.ChangeColour( fill_id, main_colour )
			UIButtons.ChangeColour( icon_id, main_colour )
		end

		for i = 1, 3 do
			UIButtons.ChangeColour( UIButtons.FindChildByName( reward_node, "tier_" .. i ), main_colour )
		end
	end
end


function FillMPUpcomingUnlocksChallenges( screen_id )
	print("Fill MP Upcoming Unlocks Challenges" )
end

function SetupMPUpcomingUnlocksFanStatus( parent_id )
	 print("Setup MP Upcoming Unlocks FanStatus")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UPCOMING_UNLOCKS_TITLE_MP, UIText.POB_MPUPCOMING_UNLOCKS_FAN_STATUS_SUBTITLE, GUI.Lock.icon, GUI.Lock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_upcoming_unlocks_fan_status" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPUpcomingUnlocksFanStatus" }
		
		if Multiplayer.POBUpcomingUnlocksInfo().rankUp == true then
			local next_rank = Multiplayer.POBGetCurrentRank() + 1
			local rank_info = Multiplayer.POBProgressionForRank(next_rank)	
			local playlist_info = Multiplayer.POBGetPlaylistUnlocks(next_rank)			
			
			-- //go through all of the vehicles and add a pic to the screen

			local possibleScreensToShow = {}
			
			local possibleVehiclesToShow = {}
			local possibleModsToShow = {}
			local possibleFeaturesToShow = {}
			local icon  = nil
			local name = nil
			local class = nil
			local style = nil		
					
			if rank_info.unlocks.cars ~= nil then
				for i, v in pairs(rank_info.unlocks.cars) do		
					possibleVehiclesToShow[#possibleVehiclesToShow + 1] = 
					{
						-- this references the texture aswell as the icon						
						icon	= GameData.GetVehicle( v.id ).icon,
						name	= GameData.GetVehicle( v.id ).name_full,
						class	= GameData.GetVehicle( v.id).class,
						style	= GameData.GetVehicle( v.id ).style,
					}
					
				end
				if #possibleVehiclesToShow > 0 then
					possibleScreensToShow[#possibleScreensToShow + 1] = "Vehicles"
				end
			end
			
			if rank_info.unlocks.abilities ~= nil then
				for i, v in pairs(rank_info.unlocks.abilities) do
									
					possibleModsToShow[#possibleModsToShow + 1] = 
					{					
						icon		= v.icon,
						name		= v.name,
						description = v.description,
						sheet		= v.sheet,
						category	= v.category,
					}
					
				end
				
				if #possibleModsToShow > 0 then
					possibleScreensToShow[#possibleScreensToShow + 1] = "Mods"
				end
			end
			
			if playlist_info.playlists ~= nil then
				for i, v in pairs(playlist_info.playlists) do
									
					possibleFeaturesToShow[#possibleFeaturesToShow + 1] = 
					{					
						icon			= v.icon,
						name			= v.name,
						sheet			= v.sheet,
						description		= v.description,
						filename		= v.filename,
					}
						print("description", v.description)
				end
					
				if #possibleFeaturesToShow > 0 then
					possibleScreensToShow[#possibleScreensToShow + 1] = "Features"
				end
			end
			
			if #possibleScreensToShow > 0 then
				
				local currently_showing = possibleScreensToShow[ math.random( #possibleScreensToShow ) ]

				if currently_showing == "Vehicles" then
				
					--set the right render branch to be on
					UIButtons.SetActive( UIButtons.FindChildByName( node_id,"_new_car"), true )
					
					local classes = 
					{
						"class_a",
						"class_b",
						"class_c",
						"class_d",
					}
						
					local style_names =
					{
						[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
						[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
						[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
						[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
						[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
						[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
					}
					
					--choose vehicle at random
					local vehicle_to_show = math.random( #possibleVehiclesToShow )
					
					--change the class icon
					local class_icon_id = UIButtons.FindChildByName( node_id, "_class_icon" )
					UIShape.ChangeObjectName( class_icon_id, classes[ possibleVehiclesToShow[vehicle_to_show].class + 1 ] )
					
					--change style text
					local style_text_id = UIButtons.FindChildByName( node_id, "_stat_style_text" )
					UIButtons.ChangeText( style_text_id , style_names[ possibleVehiclesToShow[vehicle_to_show].style ] )
					
					--change the rank text
					local rank_text_id = UIButtons.FindChildByName( node_id, "_rank_text" )
					UIButtons.ChangeText( rank_text_id , "MPL_POB_UPCOMING_RANKUP_TEXT" .. next_rank )
					
					--change the name of the vehicle
					local vehicle_name_text_id = UIButtons.FindChildByName( node_id, "_vehcile_name_text" )
					UIButtons.ChangeText( vehicle_name_text_id , possibleVehiclesToShow[vehicle_to_show].name )
					
				end -- vehicles
				
				if currently_showing == "Mods" then
				
					--set the right render branch to be on
					UIButtons.SetActive( UIButtons.FindChildByName( node_id,"_new_mod"), true )
				
					local mod_to_show = math.random( #possibleModsToShow )	
					
					--change the mod icon to the right icon
					UIShape.ChangeSceneName( UIButtons.FindChildByName( node_id, "_mod_icon" ), possibleModsToShow[ mod_to_show ].sheet )				
					UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_mod_icon" ), possibleModsToShow[ mod_to_show ].icon )
					
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "_mod_icon" ), UIGlobals.CategoryColour[ (possibleModsToShow[ mod_to_show ].category) ] )

					--change the name of the mod to the right mod name
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_mod_name" ) , possibleModsToShow[ mod_to_show ].name )
					
					--change the mod description to the right description
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_mod_description" ) , possibleModsToShow[ mod_to_show ].description )					
					
					
				end--mods
				
				if currently_showing == "Features" then
					
					--set the right render branch to be on
					UIButtons.SetActive( UIButtons.FindChildByName( node_id,"_new_playlist"), true )				
				
					local playlist_to_show = math.random( #possibleFeaturesToShow )	
					
					--change the playlist_to_show icon to the right icon
					UIShape.ChangeSceneName( UIButtons.FindChildByName( node_id, "_playlist_icon" ), possibleFeaturesToShow[ playlist_to_show ].sheet )				
					UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_playlist_icon" ), possibleFeaturesToShow[ playlist_to_show ].icon )
					
					--change the name of the playlist_to_show to the right playlist_to_show name
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_playlist_name" ) , possibleFeaturesToShow[ playlist_to_show ].name )
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_playlist_description" ) , possibleFeaturesToShow[ playlist_to_show ].description )
					
					-- change the playlist image
					UIButtons.ChangeTexture( { filename = possibleFeaturesToShow[ playlist_to_show ].filename }, 0, UIButtons.FindChildByName( node_id, "_playlist_image" ) )
	
					
				end -- playlists
				
			end -- if we have any screens to show
		end
	end

end

function FillMPUpcomingUnlocksFanStatus( screen_id )
	print("Fill MP Upcoming Unlocks FanStatus" )
end


-- // Unlocks ----------------------------------------------------------------------------------------------------------------------------------------

function SetupMPUnlocksChallenges( parent_id )
	 print("Setup MP Unlocks Challenges")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UNLOCKS_TITLE_MP, UIText.POB_MPUNLOCKS_CHALLENGES_SUBTITLE, "alert", "common_icons", "Support_0" )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_unlocks_challenges" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		
		local progress = 0
		local name = nil

		local Packs = Multiplayer.GetChallengePacks()
		local carChallenges = {}
		
		for class = 1, 4, 1 do
			carChallenges[(#carChallenges + 1)] =  Multiplayer.GetCarChallenges( class, false )
		end	
		
		local possibleChallengesToShow = {}
		local possibleCarChallengesToShow = {}
		
		for i, v in ipairs( Packs ) do		
			if v.locked == false then

				local Challenges = Multiplayer.GetChallenges( v.category )

				for j, w in ipairs( Challenges ) do		
					
					if v.category == UIEnums.MpChallengePack.Car then
						if GUI.mp_game_info.LastChallengeCarId ~= -1 then
							if w.type == GUI.mp_game_info.LastChallengeCompleted then
					
								for classes = 1, #carChallenges, 1 do
									for cars =1,  #carChallenges[classes], 1 do		
										if carChallenges[classes][cars].id == GUI.mp_game_info.LastChallengeCarId then
										
											if w.tier == ( GUI.mp_game_info.LastChallengeCompletedLevel + 1 ) then
										
												local temp_tier = (w.tier + 1)
												if temp_tier > w.tiers then
													temp_tier = w.tiers
												end
												possibleCarChallengesToShow[ (#possibleCarChallengesToShow + 1) ] = 
												{ 
													id = carChallenges[classes][cars].id,
													progress = 1.0,
													name = w.type,
													tier = temp_tier,
													tiers = w.tiers,
													complete = carChallenges[classes][cars].complete,
													icon = GameData.GetVehicle(carChallenges[classes][cars].id).icon,
													sheet = GameData.GetVehicle(carChallenges[classes][cars].id).sheet,
													name_text = GameData.GetVehicle(carChallenges[classes][cars].id).name_full,									
												}
											end
										end
									end
								end
								
							end
						end
						
					else
						if w.type == GUI.mp_game_info.LastChallengeCompleted then						
							local temp_tier = (w.tier + 1)
							if temp_tier > w.tiers then
								temp_tier = w.tiers
							end
							possibleChallengesToShow[ (#possibleChallengesToShow + 1) ] = 
							{						
								progress = 1.0,
								name = w.type,
								tier = w.tier,
								tiers = w.tiers,
								complete = w.complete,
							}
							
						end
					end
				end
			end
		end
		
		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPUnlocksChallenges" }
		
		local possibleScreensToShow = {}
		
		if #possibleCarChallengesToShow > 0 then
			possibleScreensToShow[#possibleScreensToShow +1] = "Car"
		end
		
		if #possibleChallengesToShow > 0 then
			possibleScreensToShow[#possibleScreensToShow +1] = "Other"
		end
		
		local currently_showing = nil
		if #possibleScreensToShow > 0 then
			currently_showing = possibleScreensToShow[ math.random( #possibleScreensToShow ) ]
		end
		
		if currently_showing == "Car" then
			local challenge_to_show = math.random( #possibleCarChallengesToShow )	
			
			--move everything into the middle of the screen to take account of the car
			local px, py, pz = UIButtons.GetPosition( dummy_id )
			UIButtons.ChangePosition( dummy_id, px + 10, py, pz )

			
			--turn the pic of the car and name of the car on
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "_car_icon" ), true )
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "_car_name" ), true )
			
			--change the picture of the car			
			UIShape.ChangeSceneName( UIButtons.FindChildByName( node_id, "_car_icon" ), possibleCarChallengesToShow[challenge_to_show].sheet )			
			UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_car_icon" ), possibleCarChallengesToShow[challenge_to_show].icon )
			
			--change the name of the car
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_car_name" ), possibleCarChallengesToShow[challenge_to_show].name_text ) 
			
			--set the name of the challenge
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_name" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" ..possibleCarChallengesToShow[challenge_to_show].name .. "_PREV_NAME" )
			
			-- change the challenge description
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_desc" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" .. possibleCarChallengesToShow[challenge_to_show].name .. "_PREV_DESC")
			
			-- change the challenge progress
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_amount" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" .. possibleCarChallengesToShow[challenge_to_show].name .. "_PREV_PROGRESS")
			
			-- change the challenge help
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_help" ), "MPL_CAR_CHALLENGE" .. possibleCarChallengesToShow[challenge_to_show].id .. "_" .. possibleCarChallengesToShow[challenge_to_show].name .. "_PREV_REWARD")
			
			
			--make the bar grow
			local backing_id = UIButtons.FindChildByName( node_id, "backing" )
			local progress_id = UIButtons.FindChildByName( node_id, "progress" )

			local size_x = UIButtons.GetSize( backing_id )
			local x, size_y = UIButtons.GetSize( progress_id )

			UIButtons.ChangeSize( progress_id, ( size_x * possibleCarChallengesToShow[challenge_to_show].progress ), size_y, 0.0 )
			
			--setup the lights for the tier
			for i = 1, possibleCarChallengesToShow[challenge_to_show].tiers do
				-- turn on each pip
				if i < possibleCarChallengesToShow[challenge_to_show].tier then -- completed
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Support_0" )
				else
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Main_Black" )
				end
			end

			local RepeatRewardNodes = {}
			local WinnerRewardNodes = {}
			
			local CarRewards = Multiplayer.GetCarChallengeRewards()
			
			local RewardIcons =
			{
				[UIEnums.MpChallengeRewards.HealthSlot]		= "health",
				[UIEnums.MpChallengeRewards.BonusFans]		= "extra_slot", -- Need to change this to new icon name when we have one
				[UIEnums.MpChallengeRewards.SpeedBoost]		= "horsepower",

				[UIEnums.MpChallengeRewards.Pearlescent]	= "pearl",
				[UIEnums.MpChallengeRewards.Metallic]		= "metallic",
				[UIEnums.MpChallengeRewards.Candy]			= "candy",
			}
			
			-- create car winner rewards		
			local pos_x = 3.75			
			for i = 1, 3 do
				if Multiplayer.ChallengeTypeIsRace(possibleCarChallengesToShow[challenge_to_show].name) == true then
					RepeatRewardNodes[i] = UIButtons.CloneXtGadgetByName( "POB_Items", "reward_node" )

					UIButtons.ChangePosition( RepeatRewardNodes[i], pos_x, -6, 0 )
					UIButtons.SetParent( RepeatRewardNodes[i], UIButtons.FindChildByName( node_id, "challenge_frame"), UIEnums.Justify.BottomLeft )				

					for j = 1, i do
						UIButtons.SetActive( UIButtons.FindChildByName( RepeatRewardNodes[i], "tier_" .. j ), true )
						UIShape.ChangeObjectName( UIButtons.FindChildByName( RepeatRewardNodes[i], "icon" ), RewardIcons[CarRewards["repeat_offender"][j]] )
					end
				end

				-- winner
				if Multiplayer.ChallengeTypeIsWinner(possibleCarChallengesToShow[challenge_to_show].name) == true then
					WinnerRewardNodes[i] = UIButtons.CloneXtGadgetByName( "POB_Items", "reward_node" )

					UIButtons.ChangePosition( WinnerRewardNodes[i], pos_x, -6, 0 )
					UIButtons.SetParent( WinnerRewardNodes[i], UIButtons.FindChildByName( node_id, "challenge_frame"), UIEnums.Justify.BottomLeft )
					
					for j = 1, i do
						UIButtons.SetActive( UIButtons.FindChildByName( WinnerRewardNodes[i], "tier_" .. j ), true )
						UIShape.ChangeObjectName( UIButtons.FindChildByName( WinnerRewardNodes[i], "icon" ), RewardIcons[CarRewards["winner"][j]] )
					end
				end
					pos_x = pos_x + 8.9
				
			end
			
			local nodes =
			{
				[1] = RepeatRewardNodes,
				[2] = WinnerRewardNodes,
			}
			
			for j = 1, 3 do
				if Multiplayer.ChallengeTypeIsRace(possibleCarChallengesToShow[challenge_to_show].name) == true then
					RefreshCarChallengeRewards( nodes[1][j], possibleCarChallengesToShow[challenge_to_show], j )
				else
					RefreshCarChallengeRewards( nodes[2][j], possibleCarChallengesToShow[challenge_to_show], j )
				end
			end
			
		
		end--end of car challenges
				
		if currently_showing == "Other" then
		
			local challenge_to_show = math.random( #possibleChallengesToShow )	
					
			--set the name of the challenge
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_name" ), "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_PREV_NAME" )
			
			-- change the challenge description
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_desc" ), "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_PREV_DESC")
			
			-- change the challenge progress
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "challenge_amount" ), "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_PREV_PROGRESS")
			
			--make the bar grow
			local backing_id = UIButtons.FindChildByName( node_id, "backing" )
			local progress_id = UIButtons.FindChildByName( node_id, "progress" )

			local size_x = UIButtons.GetSize( backing_id )
			local x, size_y = UIButtons.GetSize( progress_id )

			UIButtons.ChangeSize( progress_id, ( size_x * possibleChallengesToShow[challenge_to_show].progress ), size_y, 0.0 )
			
			--setup the lights for the tier
			for i = 1, possibleChallengesToShow[challenge_to_show].tiers do
				-- turn on each pip
				if i < possibleChallengesToShow[challenge_to_show].tier then -- completed
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Support_0" )
				else
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "part_" .. i ), "Main_Black" )
				end
			end
			
			--setup the reward bit at the bottom
			local RewardNodes = {}
			local pos_x = 4.5
			for i = 1, 3 do
				RewardNodes[i] = UIButtons.CloneXtGadgetByName( "POB_Items", "fan_reward_node" )

				UIButtons.ChangePosition( RewardNodes[i], pos_x, -12.5, 0 )
				UIButtons.SetParent( RewardNodes[i], UIButtons.FindChildByName( node_id, "challenge_frame"), UIEnums.Justify.BottomLeft )
				pos_x = pos_x + 9.75

				for j = 1, i do
					UIButtons.SetActive( UIButtons.FindChildByName( RewardNodes[i], "tier_" .. j ), true )
				end
			end
			
			for i = 1, 3 do
				local frame_id = UIButtons.FindChildByName( RewardNodes[i], "reward_frame" )
				local fill_id = UIButtons.FindChildByName( RewardNodes[i], "reward_fill" )
				local fans_id = UIButtons.FindChildByName( RewardNodes[i], "fan_icon" )
				local reward_id = UIButtons.FindChildByName( RewardNodes[i], "fans" )
				local tick_id = UIButtons.FindChildByName( RewardNodes[i], "tick" )

				-- change fan reward text
				UIButtons.ChangeText( reward_id, "MPL_CHALLENGE" .. possibleChallengesToShow[challenge_to_show].name .. "_" .. i - 1 .. "_FANS" )
			
				local main_colour = "Main_White"

				-- doesnt have this tier
				if i > possibleChallengesToShow[challenge_to_show].tiers then
					UIButtons.SetActive( RewardNodes[i], false )
				else
					UIButtons.SetActive( RewardNodes[i], true )

					if i < possibleChallengesToShow[challenge_to_show].tier or possibleChallengesToShow[challenge_to_show].complete == true then -- completed
						main_colour = "Support_0"
						UIButtons.SetActive( tick_id, true )

						UIButtons.ChangeColour( frame_id, main_colour )
						UIButtons.ChangeColour( fans_id, main_colour )
						UIButtons.ChangeColour( reward_id, main_colour )
						UIButtons.ChangeColour( fill_id, "Main_Black" )

					elseif i == possibleChallengesToShow[challenge_to_show].tier then -- active
						main_colour = "Main_White"
						UIButtons.SetActive( tick_id, false )

						UIButtons.ChangeColour( fans_id, main_colour )
						UIButtons.ChangeColour( reward_id, main_colour )
						UIButtons.ChangeColour( frame_id, "Main_Black" )
						UIButtons.ChangeColour( fill_id, "Support_2" )

					else -- not active yet
						main_colour = "Main_Black"
						UIButtons.SetActive( tick_id, false )
						
						UIButtons.ChangeColour( frame_id, main_colour )
						UIButtons.ChangeColour( fill_id, main_colour )
						UIButtons.ChangeColour( fans_id, main_colour )
						UIButtons.ChangeColour( reward_id, main_colour )
					end

					for j = 1, 3 do
						UIButtons.ChangeColour( UIButtons.FindChildByName( RewardNodes[i], "tier_" .. j ), main_colour )
					end
				end
			end
			
		end -- end of other challenges
	end

end

function FillMPUnlocksChallenges( screen_id )
	print("Fill MP Unlocks Challenges" )
end

function SetupMPUnlocksFanStatus( parent_id )
	 print("Setup MP Unlocks FanStatus")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UNLOCKS_TITLE_MP, UIText.POB_MPUNLOCKS_FAN_STATUS_SUBTITLE, "alert", "common_icons", "Support_0" )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_unlocks_fan_status" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPUnlocksFanStatus" }
			
		if (GUI.mp_game_info.RanksGained > 0)  then
			local current_rank = Multiplayer.POBGetCurrentRank()
			local rank_info = Multiplayer.POBProgressionForRank(current_rank)
			local playlist_info = Multiplayer.POBGetPlaylistUnlocks(current_rank)			
			
			-- //go through all of the vehicles and add a pic to the screen

			local possibleScreensToShow = {}
			
			local possibleVehiclesToShow = {}
			local possibleModsToShow = {}
			local possibleFeaturesToShow = {}
			local icon  = nil
			local name = nil
			local class = nil
			local style = nil		
					
			if rank_info.unlocks.cars ~= nil then
				for i, v in pairs(rank_info.unlocks.cars) do		
					possibleVehiclesToShow[#possibleVehiclesToShow + 1] = 
					{
						-- this references the texture aswell as the icon						
						icon	= GameData.GetVehicle( v.id ).icon,
						name	= GameData.GetVehicle( v.id ).name_full,
						class	= GameData.GetVehicle( v.id).class,
						style	= GameData.GetVehicle( v.id ).style,
					}
					
				end
				if #possibleVehiclesToShow > 0 then
					possibleScreensToShow[#possibleScreensToShow + 1] = "Vehicles"
				end
			end
			
			if rank_info.unlocks.abilities ~= nil then
				for i, v in pairs(rank_info.unlocks.abilities) do
									
					possibleModsToShow[#possibleModsToShow + 1] = 
					{					
						icon		= v.icon,
						name		= v.name,
						description = v.description,
						sheet		= v.sheet,
						category	= v.category,
					}
					
				end
				
				if #possibleModsToShow > 0 then
					possibleScreensToShow[#possibleScreensToShow + 1] = "Mods"
				end
			end
			
			if playlist_info.playlists ~= nil then
				for i, v in pairs(playlist_info.playlists) do
				
												
					possibleFeaturesToShow[#possibleFeaturesToShow + 1] = 
					{					
						icon			= v.icon,
						name			= v.name,
						sheet			= v.sheet,
						description		= v.description,
						filename		= v.filename,
					}
						print("description", v.description)
				end
					
				if #possibleFeaturesToShow > 0 then
					possibleScreensToShow[#possibleScreensToShow + 1] = "Features"
				end
			end
			
			if #possibleScreensToShow > 0 then
				
				local currently_showing = possibleScreensToShow[ math.random( #possibleScreensToShow ) ]

				if currently_showing == "Vehicles" then
				
					--set the right render branch to be on
					UIButtons.SetActive( UIButtons.FindChildByName( node_id,"_new_car"), true )
					
					local classes = 
					{
						"class_a",
						"class_b",
						"class_c",
						"class_d",
					}
						
					local style_names =
					{
						[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
						[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
						[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
						[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
						[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
						[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
					}
					
					--choose vehicle at random
					local vehicle_to_show = math.random( #possibleVehiclesToShow )
					
					--change the class icon
					local class_icon_id = UIButtons.FindChildByName( node_id, "_class_icon" )
					UIShape.ChangeObjectName( class_icon_id, classes[ possibleVehiclesToShow[vehicle_to_show].class + 1 ] )
					
					--change style text
					local style_text_id = UIButtons.FindChildByName( node_id, "_stat_style_text" )
					UIButtons.ChangeText( style_text_id , style_names[ possibleVehiclesToShow[vehicle_to_show].style ] )
					
					--change the rank text
					local rank_text_id = UIButtons.FindChildByName( node_id, "_rank_text" )
					UIButtons.ChangeText( rank_text_id , "MPL_POB_RANKUP_TEXT" .. current_rank )
					
					--change the name of the vehicle
					local vehicle_name_text_id = UIButtons.FindChildByName( node_id, "_vehcile_name_text" )
					UIButtons.ChangeText( vehicle_name_text_id , possibleVehiclesToShow[vehicle_to_show].name )
					
					--change the vehicle texture
					local vehicle_texture_id = UIButtons.FindChildByName( node_id, "_vehicle_image" )
					UIButtons.ChangeTexture( { filename = possibleVehiclesToShow[vehicle_to_show].icon }, 0,  vehicle_texture_id)
				end -- vehicles
				
				if currently_showing == "Mods" then
				
					--set the right render branch to be on
					UIButtons.SetActive( UIButtons.FindChildByName( node_id,"_new_mod"), true )
				
					local mod_to_show = math.random( #possibleModsToShow )	
					
					--change the mod icon to the right icon
					UIShape.ChangeSceneName( UIButtons.FindChildByName( node_id, "_mod_icon" ), possibleModsToShow[ mod_to_show ].sheet )				
					UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_mod_icon" ), possibleModsToShow[ mod_to_show ].icon )
					
					UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "_mod_icon" ), UIGlobals.CategoryColour[ (possibleModsToShow[ mod_to_show ].category) ] )

					--change the name of the mod to the right mod name
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_mod_name" ) , possibleModsToShow[ mod_to_show ].name )
					
					--change the mod description to the right description
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_mod_description" ) , possibleModsToShow[ mod_to_show ].description )					
					
					
				end--mods
				
				if currently_showing == "Features" then
					
					--set the right render branch to be on
					UIButtons.SetActive( UIButtons.FindChildByName( node_id,"_new_playlist"), true )				
				
					local playlist_to_show = math.random( #possibleFeaturesToShow )	
					
					--change the playlist_to_show icon to the right icon
					UIShape.ChangeSceneName( UIButtons.FindChildByName( node_id, "_playlist_icon" ), possibleFeaturesToShow[ playlist_to_show ].sheet )				
					UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "_playlist_icon" ), possibleFeaturesToShow[ playlist_to_show ].icon )
					
					--change the name of the playlist_to_show to the right playlist_to_show name
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_playlist_name" ) , possibleFeaturesToShow[ playlist_to_show ].name )
					UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_playlist_description" ) , possibleFeaturesToShow[ playlist_to_show ].description )
					
					-- change the playlist image
					UIButtons.ChangeTexture( { filename = possibleFeaturesToShow[ playlist_to_show ].filename }, 0, UIButtons.FindChildByName( node_id, "_playlist_image" ) )
	
					
				end -- playlists
				
			end -- if we have any screens to show
				
		end
	end

end

function FillMPUnlocksFanStatus( screen_id )
	print("Fill MP Unlocks FanStatus" )
end


-- // Available / Incomplete  ----------------------------------------------------------------------------------------------------------------------------------------


function SetupMPAvailableCars( parent_id )
	 print("Setup MP Available Cars")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_AVAILABLE_INCOMPLETE_TITLE, UIText.POB_AVAILABLE_CARS_TITLE_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		
		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_available_cars" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPAvailableCars" }
	end

end

function FillMPAvailableCars( screen_id )
	print("Fill MP Available Cars" )
end

function SetupMPAvailableMods( parent_id )
	 print("Setup MP Available Mods")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_AVAILABLE_INCOMPLETE_TITLE, UIText.POB_AVAILABLE_MODS_TITLE_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_available_mods" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPAvailableMods" }
	end

end

function FillMPAvailableMods( screen_id )
	print("Fill MP Available Mods" )
end

function SetupMPAvailablePlaylists( parent_id )
	 print("Setup MP Available Playlists")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_AVAILABLE_INCOMPLETE_TITLE, UIText.POB_AVAILABLE_PLAYLISTS_TITLE_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_available_playlists" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPAvailablePlaylists" }
	end

end

function FillMPAvailablePlaylists( screen_id )
	print("Fill MP Available Playlists" )
end

-- // STATS  ----------------------------------------------------------------------------------------------------------------------------------------

function SetupMPStats( parent_id )
	 print("Setup MP Stats")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
	
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_STATS_TITLE, UIText.POB_STATS_SUBTITLE, "groups", "common_icons" )		
		
		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_stats_last_session" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		local total_races_id	= UIButtons.FindChildByName( node_id, "_total_races_text" )
		local total_wins_id		= UIButtons.FindChildByName( node_id, "_total_wins_text" )
		local total_points_id	= UIButtons.FindChildByName( node_id, "_total_points_text" )
		local fans_earned_id	= UIButtons.FindChildByName( node_id, "_total_fans_earned_text" )
		local ranks_earned_id	= UIButtons.FindChildByName( node_id, "_total_ranks_earned_text" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPStats" }
	
	
		UIButtons.ChangeText(total_races_id,  "SPL_POB_NUMBER_"..GUI.mp_game_info.Races )
		UIButtons.ChangeText(total_wins_id,	  "SPL_POB_NUMBER_"..GUI.mp_game_info.Wins )
		UIButtons.ChangeText(total_points_id, "SPL_POB_NUMBER_"..GUI.mp_game_info.NumRacePoints )
		UIButtons.ChangeText(fans_earned_id,  "SPL_POB_NUMBER_"..GUI.mp_game_info.FansEarned )
		
		if GUI.mp_game_info.RanksGained >= 0 then
			UIButtons.ChangeText(ranks_earned_id, "SPL_POB_NUMBER_"..GUI.mp_game_info.RanksGained )
		else
			GUI.mp_game_info.RanksGained = (Multiplayer.GetMaxRank() + (GUI.mp_game_info.RanksGained)) -1
			UIButtons.ChangeText(ranks_earned_id, "SPL_POB_NUMBER_"..GUI.mp_game_info.RanksGained )			
		end

	
	end

end

function FillMPAvailableStats( screen_id )
	print("Fill MP Available Stats" )
end

-- // Community ----------------------------------------------------------------------------------------------------------------------------------------

function SetupMPCommunity( parent_id )
	 print("Setup MP Community")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_COMMUNITY_TITLE, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_community_nemesis_prey" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPCommunity" }
	end

end

function FillMPAvailableCommunity( screen_id )
	print("Fill MP Available Community" )
end

-- // Milestones ----------------------------------------------------------------------------------------------------------------------------------------


function SetupMPMilestones( parent_id )
	 print("Setup MP Milestones")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_MILESTONES_TITLE_MP, UIText.POB_MILESTONES_MP_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_milestones_challenge_progression" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPMilestones" }

		local Packs = Multiplayer.GetChallengePacks()

		local totalProgress = 0
		local numChallenges = 0 -- this is the number of challenge packs

		for i, v in ipairs( Packs ) do		
			totalProgress = totalProgress + v.progress 
			numChallenges = numChallenges + 1
		end

		if numChallenges > 0 then
			local progress = (totalProgress / numChallenges)
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "_progress_text" ), "GAME_PERCENTAGE_" .. progress  )

		end

	end

end

function FillMPAvailableMilestones( screen_id )
	print("Fill MP Available Milestones" )
end

-- // Leaderboard ----------------------------------------------------------------------------------------------------------------------------------------


function SetupMPLeaderboard( parent_id )
	 print("Setup MP Milestones")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_LEADERBOARD_TITLE, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_mp_leaderboard" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "MPLeaderboard" }
	end

end

function FillMPAvailableLeaderboard( screen_id )
	print("Fill MP Available Leaderboard" )
end

-- // UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL -- UNIVERSAL

-- // Upcoming Unlocks ----------------------------------------------------------------------------------------------------------------------------------------

function SetupUpcomingUnlocksAchievements( parent_id )
	 print("Setup Upcoming Unlocks Achievements")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UPCOMING_UNLOCKS_TITLE, UIText.POB_UPCOMING_UNLOCKS_STICKERS_SUBTITLE, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_upcoming_unlocks_achievements" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		
		local achievements = GameData.POBGetUpcomingAchievements()
		local possibleAchievementsToShow = {}
		
		for i = 1, #achievements.stickers, 1 do
		
			possibleAchievementsToShow[(#possibleAchievementsToShow + 1)] = 
			{				
				description = achievements.stickers[i].description,
				progress = achievements.stickers[i].progress,
				goal = achievements.stickers[i].goal,
				achievement_location = achievements.stickers[i].achievement_location,
				sticker_progress = achievements.stickers[i].sticker_progres,
				sticker_index = achievements.stickers[i].sticker_index,
			}
				
		end
		
		local sticker_to_show = math.random( #possibleAchievementsToShow )		
		
		--change the description of the sticker
		Amax.PopulateAchievementName( possibleAchievementsToShow[sticker_to_show].achievement_location, UIButtons.FindChildByName( node_id, "txt_AchievementName" ) )
		
		local iconIndex, iconCategory = Amax.GetAchievementTextureInfo( possibleAchievementsToShow[sticker_to_show].achievement_location )
		local score = Amax.GetAchievementScore( possibleAchievementsToShow[sticker_to_show].achievement_location )
		
	
		local progress_id = UIButtons.FindChildByName( node_id, "Fill_Progress_1" )

		UIButtons.ChangeScale( progress_id, (possibleAchievementsToShow[sticker_to_show].progress/possibleAchievementsToShow[sticker_to_show].goal), 1 )
		UIButtons.SetActive( UIButtons.FindChildByName( node_id, "Fill_1" ), true, true )
		
		
		UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "Amount_1" ), "GAME_PROGRESS_" .. math.floor( possibleAchievementsToShow[sticker_to_show].progress ) .. "_" .. math.floor( possibleAchievementsToShow[sticker_to_show].goal ) )
		
		local progress, goal, oneShot = Amax.GetStickerInformation( possibleAchievementsToShow[sticker_to_show].achievement_location, possibleAchievementsToShow[sticker_to_show].sticker_index, UIButtons.FindChildByName( node_id, "Desc_1" ) )
		
		for i = 1, 4 do

			local iconQuadId = UIButtons.FindChildByName( node_id, "hide_" .. i )
			local u,v = GetQuadUV( i )
			if Amax.IsStickerInQuadrantComplete( possibleAchievementsToShow[sticker_to_show].achievement_location, i ) == true then
				UIButtons.ChangeTextureUV( iconQuadId, 0, (iconIndex * (1/64)) + u, v )
				UIButtons.ChangeColour( iconQuadId, GUI.sticker_colours[iconCategory + 1] )
			else
				complete = false
				UIButtons.ChangeTextureUV( iconQuadId, 0, (61/64) + u, v )
				UIButtons.ChangeColour( iconQuadId, "Dark_White" )
			end
		
		end
		
		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UpcomingUnlocksAchievements" }
	end

end

function GetQuadUV( quad )
	local u = 0
	local v = 0
	if quad == 3 or quad == 4 then
		v = 0.5
	end
	if quad == 2 or quad == 3 then
		u = 1/128
	end
	
	return u,v
end

function FillUpcomingUnlocksAchievements( screen_id )
	print("Fill Upcoming Unlocks Achievements" )
end

-- // Unlocks ----------------------------------------------------------------------------------------------------------------------------------------


function SetupUnlocksAchievements( parent_id )
	 print("Setup Unlocks Achievements")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_UNLOCKS_ACHIEVEMENTS_SUBTITLE, nil, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_unlocks_achievements" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "UnlocksAchievements" }
	end

end

function FillUnlocksAchievements( screen_id )
	print("Fill Unlocks Achievements" )
end

-- // Social ----------------------------------------------------------------------------------------------------------------------------------------

function SetupSocialTweets( parent_id )
	 print("Setup Social Tweets")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_SOCIAL_NETWORK_TITLE, nil, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_social_tweets" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "SocialTweets" }
	end

end

function FillSocialTweets( screen_id )
	print("Fill Social Tweets" )
end

function SetupSocialFacebooked( parent_id )
	 print("Setup Social Facebooked")
	
	if GUI.numCurrentScreens < GUI.maxScreens then
		GUI.numCurrentScreens = (GUI.numCurrentScreens + 1)
		
		local screen_position_id = SetupPOBScreenTitle( parent_id, UIText.POB_SOCIAL_NETWORK_TITLE, nil, GUI.Unlock.icon, GUI.Unlock.filename )		

		local node_id = UIButtons.CloneXtGadgetByName( "POB_Items", "_node_social_facebooked" )
		local dummy_id = UIButtons.FindChildByName( node_id, "_dummy" )
		local text_position_id = UIButtons.FindChildByName( node_id, "_text_position" )
		local debug_icon_id	= UIButtons.FindChildByName( node_id, "_debug_icon" )

		UIButtons.SetParent( node_id, screen_position_id, UIEnums.Justify.MiddleCentre )
		

		GUI.rows_id[ GUI.numCurrentScreens ] = { node = node_id, dummy = dummy_id, position = text_position_id, debug_icon = debug_icon_id, screen_type = "SocialFacebooked" }
	end

end

function FillSocialFacebooked( screen_id )
	print("Fill Social Facebooked" )
end


