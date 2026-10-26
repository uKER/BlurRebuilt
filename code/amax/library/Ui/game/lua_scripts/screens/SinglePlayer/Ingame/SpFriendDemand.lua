GUI =
{
	finished = false,
	loading_ui = false,
	
	light_ids = 
	{
		result = 
		{
			primary = {  },
		},
	},
	
	timer = 0,
	
	allow_next_and_share = false,
	
	count_failed = 0,
	count = 0,
}

function Init()
	UIScreen.SetScreenTimers( 1, UIGlobals.screen_time.default_end ) -- [PR] #35572
	
	AddSCUI_Elements()
	
	SetupInfoLine()

	if UIGlobals.FriendDemandSent == true then
		UIButtons.SetActive( SCUI.name_to_id[ "background" ], true )
	end

	if IsWideScreen() == false then	-- [Blur2PC] retail: IsWideScreen
		UIButtons.ChangeScale( SCUI.name_to_id[ "centre" ], 1.35, 1.35, 1.35 )
	end
end

function PostInit()
	GUI.is_boss_race = Amax.SP_IsBossBattleFD()
	
	local result = Amax.SP_GetLevelResult()
	
	local fd_index = UIGlobals.FriendDemandAttemptingIndex
	local fd_info = FriendDemand.GetInfo( fd_index, false, true )
	--show_table( fd_info )
	local event_info = SinglePlayer.EventInfo( fd_info.eventid_sp )
	--show_table( event_info )
	
	-- primary
	local primary_dummy = UIButtons.CloneXtGadgetByName( "SinglePlayer\\Ingame\\SpRaceAwards.lua", "_obj_dummy" )
	UIButtons.SetParent( primary_dummy, SCUI.name_to_id[ "primary_rb" ], UIEnums.Justify.MiddleCentre )
	
	SpFriendDemand_CreatePrimaryLights( UIButtons.FindChildByName( primary_dummy, "_obj_light_dummy" ), result.state )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( primary_dummy, "_obj_title" ), UIText.SP_EVENT_PRIMARY )
	UIButtons.ChangeText( UIButtons.FindChildByName( primary_dummy, "_obj_info" ), "HUD_SP_RACE_RESULT" )
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( primary_dummy, "_obj_icon" ), Sp_KindToShape( event_info.kind ) )
	
	-- criteria
	local criteria_dummy = UIButtons.CloneXtGadgetByName( "SinglePlayer\\Ingame\\SpRaceAwards.lua", "_obj_dummy" )
	UIButtons.SetParent( criteria_dummy, SCUI.name_to_id[ "criteria_rb" ], UIEnums.Justify.MiddleCentre )
	
	local criteria_icon_id = UIButtons.FindChildByName( criteria_dummy, "_obj_icon" )
	local criteria_graphic_id = UIButtons.FindChildByName( criteria_dummy, "_obj_graphic" )
	
	UIButtons.SetActive( criteria_icon_id, false )
	UIButtons.SetActive( criteria_graphic_id, true )
	
	-- criteria icon
	local criteria_info = Fd_GetCriteriaIconInfo( event_info.kind )
	UIButtons.ChangeTexture( criteria_info.tex, 0, criteria_graphic_id )
	UIButtons.ChangeEffectIndex( criteria_graphic_id, criteria_info.effect )	
	
	UIButtons.ChangeText( UIButtons.FindChildByName( criteria_dummy, "_obj_title" ), "HUD_SP_FD_CRITERIA_RESULT_TITLE" )
	UIButtons.ChangeText( UIButtons.FindChildByName( criteria_dummy, "_obj_info_lights" ), "HUD_SP_FD_CRITERIA_RESULT_THEM" )
	UIButtons.ChangeText( UIButtons.FindChildByName( criteria_dummy, "_obj_info" ), "HUD_SP_FD_CRITERIA_RESULT_YOU" )
	
	if fd_info.extra_modifier == UIEnums.FriendDemandModifer.NotUsed then
		-- shift the two panels
		UIButtons.ChangePosition( SCUI.name_to_id[ "primary_rb" ], 100, nil, nil, true )
		UIButtons.ChangePosition( SCUI.name_to_id[ "criteria_rb" ], 100, nil, nil, true )
		GUI.has_extra_modifier = false
	else
		-- param
		local param_dummy = UIButtons.CloneXtGadgetByName( "SinglePlayer\\Ingame\\SpRaceAwards.lua", "_obj_dummy" )
		UIButtons.SetParent( param_dummy, SCUI.name_to_id[ "param_rb" ], UIEnums.Justify.MiddleCentre )
		
		local param_icon_id = UIButtons.FindChildByName( param_dummy, "_obj_icon" )
		local param_graphic_id = UIButtons.FindChildByName( param_dummy, "_obj_graphic" )
		local param_graphic_lights_id = UIButtons.FindChildByName( param_dummy, "_obj_graphic_lights" )
		
		UIButtons.SetActive( param_icon_id, false )
		UIButtons.SetActive( param_graphic_id, true )
		UIButtons.SetActive( param_graphic_lights_id, true )
		
		-- modifier icon
		local modifier_info = Fd_GetModifierIconInfo( fd_info.extra_modifier )
		UIButtons.ChangeTexture( modifier_info.tex, 0, param_graphic_lights_id )
		UIButtons.ChangeEffectIndex( param_graphic_lights_id, modifier_info.effect )
	
		-- param icon
		local param_info = Fd_GetParamIconInfo( fd_info.extra_param )
		UIButtons.ChangeTexture( param_info.tex, 0, param_graphic_id )
		UIButtons.ChangeEffectIndex( param_graphic_id, param_info.effect )
		UIButtons.ChangeColour( param_graphic_id, param_info.colour )
		
		UIButtons.ChangeText( UIButtons.FindChildByName( param_dummy, "_obj_title" ), "HUD_SP_FD_PARAM_RESULT_TITLE" )
		UIButtons.ChangeText( UIButtons.FindChildByName( param_dummy, "_obj_info_lights" ), "HUD_SP_FD_PARAM_RESULT_THEM" )
		UIButtons.ChangeText( UIButtons.FindChildByName( param_dummy, "_obj_info" ), "HUD_SP_FD_PARAM_RESULT_YOU" )
	end
	
	-- fd info
	local blurb_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "blurb_node" )
	UIButtons.SetParent( blurb_id, SCUI.name_to_id[ "blurb_node_rb" ], UIEnums.Justify.MiddleCentre )
	UIButtons.ChangePanel( blurb_id, UIEnums.Panel._3DAA_LIGHT, true )
	UIButtons.ChangeOrientation( blurb_id, 0, 0, 0 )
	
	-- need to force send the gadget_selected timelines
	UIButtons.SetSelected( blurb_id, false, true )
	UIButtons.SetSelected( blurb_id, true, true )
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( blurb_id, "blurb_icon" ), Sp_KindToShape( event_info.kind ) )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_id, "blurb_event_text" ), Sp_KindToText( event_info.kind ) )
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_id, "blurb_name" ), "GAME_BLURB_NORMAL_MSG" .. fd_index .. "_NAME" )
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_id, "blurb_info_time_sent" ), "GAME_BLURB_NORMAL_MSG" .. fd_index.. "_TIME_SENT" )
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_id, "blurb_info_days_left" ), "GAME_BLURB_NORMAL_MSG" .. fd_index .. "_ATTEMPTS" )
	
	local type_icon_id = UIButtons.FindChildByName( blurb_id, "blurb_type_icon" )
	UIButtons.SetActive( type_icon_id, false )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( blurb_id, "blurb_sender" ), "GAME_BLURB_NORMAL_MSG" .. fd_index .. "_SENDER" )
	UIButtons.SetActive( UIButtons.FindChildByName( blurb_id, "blurb_icon" ), false )
	UIButtons.SetActive( UIButtons.FindChildByName( blurb_id, "blurb_event_text" ), false )
	
	UIButtons.ChangeText( SCUI.name_to_id[ "selection_box_text_no_highlight" ], "GAME_BLURB_NORMAL_MSG" .. fd_index .."_DESC" )
	
	-- pass fail
	local fd_pass_fail_info = FriendDemand.GetResult()
	fd_pass_fail_info.passed = ( fd_pass_fail_info.passed_state == true and fd_pass_fail_info.passed_criteria == true )
	
	if fd_info.extra_modifier ~= UIEnums.FriendDemandModifer.NotUsed then
		fd_pass_fail_info.passed = ( fd_pass_fail_info.passed == true and fd_pass_fail_info.passed_param == true )
	end
	
	UIButtons.ChangeText( SCUI.name_to_id[ "pass_fail_info" ], Select( fd_pass_fail_info.passed, UIText.FDE_POST_RACE_PASS, UIText.FDE_POST_RACE_FAIL ) )
	UIButtons.ChangeColour( SCUI.name_to_id[ "pass_fail_info" ], Select( fd_pass_fail_info.passed, "Support_0", "Support_3" ) )
	UIButtons.ChangeColour( SCUI.name_to_id[ "pass_fail_icon" ], Select( fd_pass_fail_info.passed, "Support_0", "Support_3" ) )
	GUI.fd_passed = fd_pass_fail_info.passed
	
	-- figure out how many effects we need
	if fd_pass_fail_info.passed_state == false then
		GUI.count_failed = GUI.count_failed + 1
		UIButtons.ReplaceTimeLineLabel( SCUI.name_to_id[ "primary_rb" ], "darken", "darken_" .. GUI.count_failed )
	end
	if fd_pass_fail_info.passed_criteria == false then
		GUI.count_failed = GUI.count_failed + 1
		UIButtons.ReplaceTimeLineLabel( SCUI.name_to_id[ "criteria_rb" ], "darken", "darken_" .. GUI.count_failed )
	end
	if fd_info.extra_modifier ~= UIEnums.FriendDemandModifer.NotUsed and fd_pass_fail_info.passed_param == false then
		GUI.count_failed = GUI.count_failed + 1
		UIButtons.ReplaceTimeLineLabel( SCUI.name_to_id[ "param_rb" ], "darken", "darken_" .. GUI.count_failed )
	end

end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.GameFlowMessage.UILoaded and UIGlobals.LoadFromDebug == true then
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_RETURN ) )
	elseif message == UIEnums.GameFlowMessage.StartGameRendering then
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_RETURN ) )
	end
	
	if SubScreenActive() == true or GUIBank.loading == true or GUI.loading_ui == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		if GUI.allow_next_and_share == false and GUI.has_taken_photo == true then
			SpFriendDemand_Skip()
		else
			if GUI.allow_next_and_share == true then
				SetupCustomPopup( UIEnums.CustomPopups.SpPostRaceOptions )
			end
		end
		
	elseif message == UIEnums.Message.ButtonLeftShoulder then
		if GUI.allow_next_and_share == true and FriendDemand.IsComplete() > 0 and Amax.CanUseShare() == true then
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK )
			UIGlobals.ShareFromWhatPopup = -1
			SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then
		local type = UIEnums.SocialNetworkingItemType.FriendDemandFailed
		
		if FriendDemand.IsComplete() > 0 then
			type = UIEnums.SocialNetworkingItemType.FriendDemandPassed
		end

		if arg3 == UIEnums.ShareOptions.Facebook then
			Amax.CreateBlurb( type, 1, -1 )
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Ingame\\SpFriendDemand.lua" )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then
			Amax.CreateBlurb( type, 0, -1 )
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Ingame\\SpFriendDemand.lua" )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then
			Amax.CreateBlurb( type, 2, -1 )
		end
		
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SpPostRaceOptions then
		if arg3 == 0 then
			SpFriendDemand_Retry()
			
		elseif arg3 == 1 then
			SpFriendDemand_BackToUiStart()
			
		elseif arg3 == 2 then
			if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
				
				if net_CanReconnectToDemonware() == false then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
				else
					net_StartServiceConnection(true, nil, false)
				end
				
			elseif LSP.IsConnected() == false then
				SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
			elseif FriendDemand.HasPrivilege() == false then
				SetupCustomPopup( UIEnums.CustomPopups.FriendChallengeNoPrivilege )
			else
				StoreScreen( UIEnums.ScreenStorage.FE_FRIEND_DEMAND )
				FriendDemand.PopulateRechallengeText()
				GoScreen( "Ingame\\ReChallengeFriendDemand.lua" )
			end
		end
	end
end

function FrameUpdate( time )
	--
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	if GUI.loading_ui == true then
		SpFriendDemand_BackToUiEnd()
	end
end

function SpFriendDemand_CreatePrimaryLights( parent, state )
	local num_stars = SP_MaxStars( GUI.is_boss_race )
	local num_stars_complete = Sp_EventStateToStars( state, GUI.is_boss_race )
	
	local star_sx = 0
	
	for i = 1, num_stars, 1 do
		local star_id = Sp_CreateStar( ( i <= num_stars_complete ), i, state, nil, nil, 12, true, false )
		
		for j, w in ipairs( star_id ) do
			UIButtons.SetParent( w, parent, UIEnums.Justify.TopRight )
			UIButtons.ChangePanel( w, UIEnums.Panel._3DAA_0, true )
			UIButtons.ChangeJustification( w, UIEnums.Justify.MiddleCentre )
			UIButtons.ChangeScale( w, 12, 12 )
			
			if i == 1 and j == 1 then
				star_sx, _, _ = UIButtons.GetSize( w )
			end
		end
		
		GUI.light_ids.result.primary[ #GUI.light_ids.result.primary + 1 ] = star_id
	end
	
	local offset = ( num_stars - 1 ) * star_sx * 13 * 0.5
	
	local px, py, pz = UIButtons.GetPosition( parent )
	UIButtons.ChangePosition( parent, px - offset, py, pz )
end

-- called externally from a timeline
function SpFriendDemand_ParamOn()
	if GUI.has_extra_modifier ~= false then
		UISystem.PlaySound( UIEnums.SoundEffect.PostRaceGoalSpawn )
	end
end

-- called externally from a timeline
function SpFriendDemand_PassFailOn()
	UISystem.PlaySound( Select( ( GUI.fd_passed == true ), UIEnums.SoundEffect.PostRacePassed, UIEnums.SoundEffect.PostRaceFailed ) )
end

function SpFriendDemand_TakePhoto()

	if UIGlobals.FriendDemandSent == true then
		SetupCustomPopup( UIEnums.CustomPopups.SpPostRaceOptions )
	else
		Amax.TakePhoto( 1, false, false )
	end

	UIButtons.SetActive( SCUI.name_to_id[ "background" ], true )
	Amax.SendMessage( UIEnums.GameFlowMessage.Pause )
	UISystem.PlaySound( UIEnums.SoundEffect.EndOfRacePause )	
	GUI.has_taken_photo = true
end

function SpFriendDemand_BackToUiStart()
	GUI.loading_ui = true
	
	Amax.SendMessage( UIEnums.GameFlowMessage.UnPause )
	UISystem.PlaySound( UIEnums.SoundEffect.EndOfRaceUnPause )
	Amax.SendMessage( UIEnums.GameFlowMessage.QuitRace )
	
	--[[
	StopIngameMusic()
	Amax.SetLoadStateBackToUI()
	
	UIGlobals.Ingame = {  }
	GameProfile.ClearRaceDifficulty()

	if UIGlobals.LoadFromDebug == false then
		Profile.AllowProfileChanges( true )
		Profile.ActOnProfileChanges( false )
	end
	
	SpRaceAwards_TransmitterOn()
	--]]
	
	UIScreen.SetScreenTimers( 0, 0.3 )
	
	UIButtons.TimeLineActive( "end_alpha", true )
	UIButtons.TimeLineActive( "start_load", true )
	
	SetupInfoLine()
	
	--StartUiLoad()
	StartAsyncSave()
	GoScreen( "Loading\\LoadingUi.lua" )
end

function SpFriendDemand_BackToUiEnd()
	--[[
	UIGlobals.IsIngame = false
	
	Amax.CreateUiResources()
	
	if UIGlobals.LoadFromDebug == false then
		Profile.ForceProfileUpdate()
		Profile.ActOnProfileChanges( true )
	end
	--]]
end

--[[
function SpFriendDemand_TransmitterOn()
	UIButtons.SetActive( ContextTable[ UIEnums.Context.Main ].SCUI.name_to_id[ "Txt_Loading" ], true, true )
	UIButtons.TimeLineActive( "CSB_off_full", true )
	UIButtons.TimeLineActive( "start_load", true )
end
--]]

function SpFriendDemand_Retry()
	FriendDemand.ReAttempt()

	UIButtons.SetActive( ContextTable[ UIEnums.Context.Main ].SCUI.name_to_id[ "Txt_Loading" ], true, true )
	UIButtons.TimeLineActive( "CSB_off_full", true )
	UIButtons.TimeLineActive( "start_load", true )

	GameProfile.RetryingEvent()
	StopIngameMusic()
	
	Amax.SendMessage( UIEnums.GameFlowMessage.StopGameRendering )
	Amax.SendMessage( UIEnums.GameFlowMessage.StopGameUpdate )

	UIGlobals.IsQuickRestart = true
	
	UIGlobals.Ingame = {  }
	
	--SpFriendDemand_TransmitterOn()

	GoLoadingScreen( "Loading\\LoadingSpGame.lua" )
end

function SpFriendDemand_Skip()
	if GUI.has_skipped == true then
		return
	end
	
	if GUI.has_taken_photo ~= true then
		SpFriendDemand_TakePhoto()
	end
	
	local max_time = 100	-- super safe
	
	UIButtons.TimeLineActive( "CSB_off", true, max_time, true )
	UIButtons.TimeLineActive( "commands", true, max_time, true )
	UIButtons.TimeLineActive( "primary_on", true, max_time, true )
	UIButtons.TimeLineActive( "fan_par_on", true, max_time, true )
	UIButtons.TimeLineActive( "fan_run_on", true, max_time, true )
	UIButtons.TimeLineActive( "blurb_info_on", true, max_time, true )
	UIButtons.TimeLineActive( "darken_1", true, max_time, true )
	UIButtons.TimeLineActive( "darken_2", true, max_time, true )
	UIButtons.TimeLineActive( "darken_3", true, max_time, true )
	UIButtons.TimeLineActive( "pass_fail_info_on", true, max_time, true )
	
	SpFriendDemand_EffectsFinished()
	
	GUI.has_skipped = true
end

-- called externally from a timeline
function SpFriendDemand_DarkenFinished( initial_check )
	if initial_check ~= true then
		GUI.count = GUI.count + 1
	end
	
	if GUI.count == GUI.count_failed then
		UIButtons.TimeLineActive( "pass_fail_info_on", true, 0 )
	end
end

-- called externally from a timeline
function SpFriendDemand_EffectsFinished()
	GUI.allow_next_and_share = true
	
	if FriendDemand.IsComplete() > 0 then
		SetupInfoLine( UIText.INFO_A_NEXT, "GAME_SHARE_BUTTON" )
	else
		SetupInfoLine( UIText.INFO_A_NEXT )
	end
end
