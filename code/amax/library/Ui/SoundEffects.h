#ifndef UISOUNDEFFECTS_H
#define UISOUNDEFFECTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

enum E_UiSoundEffect
{
	E_UiSoundEffect_None = -1,		// Flag as not used
	// --
	E_UiSoundEffect_Start,			// Message to say the player has entered the UI (not to be used for pause screens as this starts the music)
	E_UiSoundEffect_Stop,			// Message to say the player is leaving the UI to enter an event (stops the music and pauses the updating of the object)
	E_UiSoundEffect_Move,			// Any basic menu navigation movement
	E_UiSoundEffect_Forwards,		// Any input to move forwards to a new screen
	E_UiSoundEffect_Back,			// Any input to move backwards to a previous screen
	E_UiSoundEffect_Toggle,			// Any input where you are selecting between options or values
	E_UiSoundEffect_TabsOpen,
	E_UiSoundEffect_Error,  //8

	E_UiSoundEffect_CurrentHealthLevel,

	E_UiSoundEffect_TargetLocked,
	E_UiSoundEffect_TargetOnYou,  

	E_UiSoundEffect_AcceptActionIn,
	E_UiSoundEffect_MenuOpenActionIn,
	E_UiSoundEffect_AppOpenActionIn,
	E_UiSoundEffect_CancelActionIn,
	E_UiSoundEffect_MenuCloseActionIn, //16
	E_UiSoundEffect_AppCloseActionIn,
	E_UiSoundEffect_NoticeActionIn,
	E_UiSoundEffect_ErrorActionIn,
	E_UiSoundEffect_PressStartActionIn,
	E_UiSoundEffect_TabsOpenActionIn,
	E_UiSoundEffect_ToggleActionIn,
	E_UiSoundEffect_MoveActionIn,
	E_UiSoundEffect_WaitingStartActionIn,	//24
	E_UiSoundEffect_WaitingStopActionIn,
	E_UiSoundEffect_FansCountingActionIn,
	E_UiSoundEffect_FansSmallCheerActionIn,
	E_UiSoundEffect_FansMidCheerActionIn,
	E_UiSoundEffect_FansLargeCheerActionIn,
	E_UiSoundEffect_NewMessageActionIn,
	E_UiSoundEffect_NewMessageMinorActionIn,
	E_UiSoundEffect_NewFriendActionIn,	//32
	E_UiSoundEffect_NewAppActionIn,
	E_UiSoundEffect_TargetLockedActionIn,
	E_UiSoundEffect_TargetOnYouActionIn,
	E_UiSoundEffect_OSDMessageActionIn,
	E_UiSoundEffect_OSDMessageComboActionIn,
	E_UiSoundEffect_OSDMessageBonusActionIn,
	E_UiSoundEffect_OSDMessageRepairActionIn,
	E_UiSoundEffect_OSDMessageNegativeActionIn, //40
	E_UiSoundEffect_RaceCountdownActionIn,
	E_UiSoundEffect_RaceGoActionIn,
	E_UiSoundEffect_PerksCountdownActionIn,
	E_UiSoundEffect_PerksGoActionIn,
	E_UiSoundEffect_MPCountdownActionIn,
	E_UiSoundEffect_RaceTargetAchievingActionIn,
	E_UiSoundEffect_RaceTargetFailingActionIn,

	E_UiSoundEffect_MoneyCountingOutActionIn,		//48
	E_UiSoundEffect_MoneyCountingInActionIn,
	E_UiSoundEffect_PhotoTakenActionIn,

	E_UiSoundEffect_PerkBargeSelectedActionIn,
	E_UiSoundEffect_PerkBombSelectedActionIn,
	E_UiSoundEffect_PerkNitrousSelectedActionIn,
	E_UiSoundEffect_PerkShockSelectedActionIn,
	E_UiSoundEffect_PerkShuntSelectedActionIn,

	E_UiSoundEffect_SpeedCameraFlashActionIn,	//56
	E_UiSoundEffect_FanDemandPickupActionIn,
	E_UiSoundEffect_FanDemandCountdownActionIn,
	E_UiSoundEffect_FanDemandSuccessActionIn,
	E_UiSoundEffect_FanDemandFailActionIn,

	E_UiSoundEffect_BuyActionIn,
	E_UiSoundEffect_ResprayActionIn,		//62

	E_UiSoundEffect_RankUpActionIn,
	E_UiSoundEffect_ViewCarActionIn,
	E_UiSoundEffect_RankCountUpActionIn,

	E_UiSoundEffect_ModShopAttachActionIn,
	E_UiSoundEffect_ModShopRemoveActionIn,
	E_UiSoundEffect_GarageSwitchCarActionIn,
	E_UiSoundEffect_GarageSelectActionIn,
	E_UiSoundEffect_GarageEnterActionIn,		//70
	E_UiSoundEffect_GarageExitActionIn,
	E_UiSoundEffect_GarageReSprayActionIn,
	E_UiSoundEffect_GarageColourPickerMoveActionIn,
	E_UiSoundEffect_GarageFinishStyleSelect01ActionIn,
	E_UiSoundEffect_GarageFinishStyleSelect02ActionIn,
	E_UiSoundEffect_GarageFinishStyleSelect03ActionIn,
	E_UiSoundEffect_GarageFinishStyleSelect04ActionIn,
	E_UiSoundEffect_GarageSelectVehicleActionIn,
	E_UiSoundEffect_LobbyJoinActionIn,
	E_UiSoundEffect_LobbyLeaveActionIn,		//80
	E_UiSoundEffect_LobbyCountDown10ActionIn,
	E_UiSoundEffect_LobbyCountDown03ActionIn,
	E_UiSoundEffect_ChallengeNewItemActionIn,
	E_UiSoundEffect_RaceStartActionIn,
	E_UiSoundEffect_LockHardActionIn,
	E_UiSoundEffect_LockSoftActionIn,
	E_UiSoundEffect_LockHardAIActionIn,
	E_UiSoundEffect_LockSoftAIActionIn,
	E_UiSoundEffect_HealthLowActionIn,
	E_UiSoundEffect_ModUse01ActionIn,	//90
	E_UiSoundEffect_ModUse02ActionIn,
	E_UiSoundEffect_ModUse03ActionIn,
	E_UiSoundEffect_ModDropActionIn,
	E_UiSoundEffect_BankChainActionIn,
	E_UiSoundEffect_ChallengeCompleteActionIn,
	E_UiSoundEffect_FansActionIn,
	E_UiSoundEffect_ChainMultiplierActionIn,
	E_UiSoundEffect_CountDownActionIn,
	E_UiSoundEffect_BankFanActionIn,
	E_UiSoundEffect_MessageOtherActionIn,	//100
	E_UiSoundEffect_PerkSwitchActionIn,
	E_UiSoundEffect_EliminationCountDown10ActionIn,
	E_UiSoundEffect_EliminationCountDown03ActionIn,
	E_UiSoundEffect_FansCountUpActionIn,
	E_UiSoundEffect_FansTotalSmallActionIn,
	E_UiSoundEffect_FansTotalMediumActionIn,
	E_UiSoundEffect_FansTotalLargeActionIn,
	E_UiSoundEffect_RankNextActionIn,
	E_UiSoundEffect_PlayerFirstActionIn,
	E_UiSoundEffect_PlayerSecondActionIn,	//110
	E_UiSoundEffect_PlayerThirdActionIn,
	E_UiSoundEffect_LeftActionIn,
	E_UiSoundEffect_RightActionIn,
	E_UiSoundEffect_LightTrailsActionIn,

	E_UiSoundEffect_RaceTargetFailedActionIn,
	E_UiSoundEffect_RaceTargetPassActionIn,

	E_UiSoundEffect_LobbyVoteCountActionIn,
	E_UiSoundEffect_EnrouteToNextEventActionIn,
	E_UiSoundEffect_MotorMashCountActionIn,
	E_UiSoundEffect_MotorMashLightImpactActionIn,
	E_UiSoundEffect_MotorMashHeavyImpactActionIn,
	E_UiSoundEffect_MotorMashWreckedActionIn,

	E_UiSoundEffect_GraphicMenuTopActionIn,				//In MP, there is a blue bar that enters and fills the top of the screen, just before the ‘TabsOpen’ sound plays. 
	E_UiSoundEffect_GraphicForwardActionIn,
	E_UiSoundEffect_GraphicBackwardActionIn,			// There is a graphics motion in and out, often tied to the A (FE_Forward) or B (FE_Backward). When selecting some screens, like OPTIONS, the in and out movement sound is appropriate, but in others like QUIT, hitting A or B doesn’t trigger a zoom.  So, if we can separate the 
	E_UiSoundEffect_GraphicPowerButtonActionIn,			 // This needs a hook when it and the title "blur" appear and pulses
	E_UiSoundEffect_GraphicStartFlyForwardActionIn,		// just to confirm, this is when you hit the start button?  We’d also like to tie a sound to the graphics as they fly forward.  FE_GraphicStartFlyForward ?  Likewise, if you hit the back button, the graphics do a flyback, so FE_GraphicStartFlyBackward
	E_UiSoundEffect_GraphicStartFlyBackwardActionIn,
	E_UiSoundEffect_GraphicCityActionIn,				 // When entering Messages screen, the city the player is in stamps in below Fans. 
	E_UiSoundEffect_GraphicTextBoxActionIn,				 // There’s a blue or orange box with text that pops up.
	E_UiSoundEffect_GraphicPictureBoxActionIn,			 // Often paired with the text box, a picture or movie box appears on the left side.

	E_UiSoundEffect_RaceStartCountdownActionIn,

	E_UiSoundEffect_WreckedActionIn,

	//special case to let audio to access profile
	E_UiSoundEffect_AudioOptionsSaved,

	E_UiSoundEffect_StartInGameMusic,

	E_UiSoundEffect_UnlockContentActionIn, //– when a new item is unlocked after a race
	E_UiSoundEffect_GiftHealthActionIn,//when a Mod gives you health
	E_UiSoundEffect_GiftPowerUpActionIn, //when a Mod gives you a perk

	E_UiSoundEffect_MPPlayerFirstActionIn,
	E_UiSoundEffect_MPFinalScoreActionIn,	
	E_UiSoundEffect_MPRankAwayActionIn,
	E_UiSoundEffect_MPRankInActionIn,

	E_UiSoundEffect_MPFocusOnWinnerActionIn,
	E_UiSoundEffect_MPPositionInActionIn,
	E_UiSoundEffect_MPNumberGlowActionIn,
	E_UiSoundEffect_MPFinishInActionIn,

	E_UiSoundEffect_TutorialSplashScreen01ActionIn,
	E_UiSoundEffect_TutorialSplashScreen02ActionIn,
	E_UiSoundEffect_TutorialSplashScreen03ActionIn,

	E_UiSoundEffect_IronFistActivateActionIn,

	E_UiSoundEffect_CountDown03ActionIn,

	E_UiSoundEffect_DiscardPickupActionIn,
	E_UiSoundEffect_DiscardDecoyPickupActionIn,
	E_UiSoundEffect_PreviouslyOnBlurActionIn,

	E_UiSoundEffect_DestructionPassActionIn,
	E_UiSoundEffect_DestructionFailActionIn,
	E_UiSoundEffect_DestructionWreckedActionIn,
	E_UiSoundEffect_DestructionWreckLongshotActionIn,

	E_UiSoundEffect_GarageSwitchCarLockedActionIn,

	E_UiSoundEffect_PassedCheckpointActionIn,
	E_UiSoundEffect_PassedStartLineActionIn,
	E_UiSoundEffect_PressStartButtonScreenActionIn,
	E_UiSoundEffect_PickupsFireErrorActionIn,
	E_UiSoundEffect_PassedFanRunCheckpointActionIn,
	E_UiSoundEffect_LastGaspInitiatedActionIn,

	E_UiSoundEffect_ScramblerActivatedActionIn,
	E_UiSoundEffect_ScramblerDeactivatedActionIn,

	E_UiSoundEffect_Route66ActionIn,
	E_UiSoundEffect_TumbleweedAlleyActionIn,
	E_UiSoundEffect_PasseigDeGraciaActionIn,
	E_UiSoundEffect_ElCarmelHeightsActionIn,
	E_UiSoundEffect_CatalanClimbActionIn,
	E_UiSoundEffect_CityBreachActionIn,
	E_UiSoundEffect_SpeedwayActionIn,
	E_UiSoundEffect_PromenadeLoopActionIn,
	E_UiSoundEffect_SeafrontStripActionIn,
	E_UiSoundEffect_CoastalCruiseActionIn,
	E_UiSoundEffect_PacificReachActionIn,
	E_UiSoundEffect_CargoRunActionIn,
	E_UiSoundEffect_TheMoneyRunActionIn,
	E_UiSoundEffect_HighRiseRingActionIn,
	E_UiSoundEffect_HarborFreewayActionIn,
	E_UiSoundEffect_ShoreditchSweepSoundActionIn,
	E_UiSoundEffect_UrbanBeltActionIn,
	E_UiSoundEffect_CentralSprintActionIn,
	E_UiSoundEffect_HollywoodRiftActionIn,
	E_UiSoundEffect_DowntownVistaActionIn,
	E_UiSoundEffect_ConcreteBasinActionIn,
	E_UiSoundEffect_StormdrainSurgeActionIn,
	E_UiSoundEffect_AscentActionIn,
	E_UiSoundEffect_ManhattanViewActionIn,
	E_UiSoundEffect_BrooklynTourActionIn,
	E_UiSoundEffect_RussianSteppesActionIn,
	E_UiSoundEffect_GoldenGateRushActionIn,
	E_UiSoundEffect_BayAreaTourActionIn,
	E_UiSoundEffect_BayShoreRouteActionIn,
	E_UiSoundEffect_WangansenActionIn,
	E_UiSoundEffect_CarverActionIn,		
	E_UiSoundEffect_DraykeActionIn,
	E_UiSoundEffect_EvilShannonActionIn,
	E_UiSoundEffect_KhanActionIn,
	E_UiSoundEffect_NatalyaActionIn,
	E_UiSoundEffect_RhymerActionIn,
	E_UiSoundEffect_ShannonActionIn,
	E_UiSoundEffect_TalonActionIn,
	E_UiSoundEffect_TwinsActionIn,

	E_UiSoundEffect_GamePausedQuitActionIn,

	E_UiSoundEffect_StopwatchCollectedActionIn,
	E_UiSoundEffect_MedalIncreaseActionIn,
	E_UiSoundEffect_MedalDecreaseActionIn,
	E_UiSoundEffect_MedalInThreatActionIn,
	E_UiSoundEffect_LapIncrementedActionIn,
	E_UiSoundEffect_LastLapActionIn,

	E_UiSoundEffect_PreviouslyRewardActionIn,
	E_UiSoundEffect_PreviouslyOutActionIn,
	E_UiSoundEffect_PreviouslySparksActionIn,

	E_UiSoundEffect_UpActionIn,
	E_UiSoundEffect_DownActionIn,
	E_UiSoundEffect_CarouselLeftActionIn,
	E_UiSoundEffect_CarouselRightActionIn,
	E_UiSoundEffect_StickersPageActionIn,

	E_UiSoundEffect_EndOfRacePauseActionIn,
	E_UiSoundEffect_EndOfRaceUnPauseActionIn,

	E_UiSoundEffect_TabsCloseActionIn,
	E_UiSoundEffect_FilterActionIn,
	E_UiSoundEffect_FriendVs1ActionIn,
	E_UiSoundEffect_FriendVs2ActionIn,
	E_UiSoundEffect_FriendVs3ActionIn,
	E_UiSoundEffect_FriendTaleOfTheTapeActionIn,
	E_UiSoundEffect_BossDemandIncrementActionIn,
	E_UiSoundEffect_BossDemandCompleteActionIn,
	E_UiSoundEffect_BossUnlockedActionIn,
	E_UiSoundEffect_ChooseActionIn,
	E_UiSoundEffect_CascadeActionIn,
	E_UiSoundEffect_BossRaceWinActionIn,
	E_UiSoundEffect_BossRaceLoseActionIn,
	E_UiSoundEffect_LoadingScreenNavActionIn,

	E_UiSoundEffect_ApplyBossModActionIn,
	E_UiSoundEffect_HudProgressSlideActionIn,
	E_UiSoundEffect_HudFanComboEndActionIn,
	E_UiSoundEffect_HudPositionIncrementActionIn,
	E_UiSoundEffect_HudHealthLossActionIn,
	E_UiSoundEffect_HudFanComboActiveActionIn,
	E_UiSoundEffect_PostRacePassedLightTrailActionIn,
	E_UiSoundEffect_PostRacePassedActionIn,
	E_UiSoundEffect_PostRaceFailedActionIn,
	E_UiSoundEffect_PostRaceBestActionIn,
	E_UiSoundEffect_PostRaceNotBestActionIn,
	E_UiSoundEffect_PostRaceGoalSpawnActionIn,
	E_UiSoundEffect_PostRaceLeaderboardSpawnActionIn,
	E_UiSoundEffect_PostRaceNewUnlocksActionIn,
	E_UiSoundEffect_PreviouslyWhooshActionIn,
	E_UiSoundEffect_PreviouslyCounterActionIn,

	E_UiSoundEffect_SkipVistaActionIn,	

	// --
	E_UiSoundEffect_Count,
	E_UiSoundEffect_Unknown
};

};	// namespace UI

#endif /* UISOUNDEFFECTS_H */
