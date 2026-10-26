GUI =
{
	finished = false,

	DisableLoading = false,
	WaitForFinishLoad = false,

	move_cool_down	= 0,
	Tabs = 
	{
		Min			= 0,
		Map			= 1,
		Goal		= 2,
		FanPar		= 3,
		FanRun		= 4,
		Max			= 5,
	},

	TabsActive = 
	{
		true,
		true,
		true,
		true,
	},

	CurrentTab = 0,

	RaceType = 0,

	tip_timer = 0,
	tip_change_time = 10,

	transmitter_id = 0,
	
	reading_leaderboard = false,
	server_timeout = 0,
	
	local_tips_table = {},
	CurrentlySelectingFromLocalTips = true,
}

function HandleLoad()

	--print("HandleLoad()")
	Amax.EnterNetworkLoad() -- no network calls/structures should be referenced between the and ExitNetworkLoad()
	
	if UIGlobals.IsQuickRestart == false then
		--print("HandleLoad() : UIGlobals.IsQuickRestart == false")
		Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)
		Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
		if Amax.AmaxWorldLayerExists() == true then
			--print("HandleLoad() : Amax.AmaxWorldLayerExists() == true")
			Amax.SendMessage(UIEnums.GameFlowMessage.DumpLevel)			
		end	
	else
		--print("HandleLoad() : UIGlobals.IsQuickRestart == true, call StartGameLoad()")
		StartGameLoad()
	end
	
	UIGlobals.Ingame = { }
	
	--retry the old fashioned way
	UIGlobals.IsLoading = true

	
end

function Init()
	net_LockoutFriendsOverlay( true )
	
	if	UIGlobals.SuccessfullyCreatedNetworkSession == true and UIGlobals.NetworkSessionStarted == false then
		Amax.StartPlayStatsOnlyMatchingSession()
		UIGlobals.NetworkSessionStarted = true
		print("Network Session Started: ", UIGlobals.NetworkSessionStarted )
	end
	

	-- make sure boss demands are up to date
	if IsTable( UIGlobals.Sp ) == true and IsTable( UIGlobals.Sp.BossInfo ) == true and IsTable( UIGlobals.SpBossDemandStore ) == true then
		CloneTable( UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ], UIGlobals.SpBossDemandStore[ UIGlobals.Sp.CurrentTier ] )
	end
	
	AddSCUI_Elements( )
	UIScreen.SetScreenTimers( 0, 1 )

	GUI.event_id = nil
	GUI.is_fd = SP_IsFriendDemand()
	
	if GUI.is_fd == true then
		GUI.fd_index = UIGlobals.FriendDemandAttemptingIndex
		GUI.fd_info = FriendDemand.GetInfo( GUI.fd_index )
		GUI.event_id = GUI.fd_info.eventid_sp
	else
		GUI.event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
	end
	
	GUI.event_info = SinglePlayer.EventInfo( GUI.event_id )
	GUI.amax_event_info = GameData.GetEvent( GUI.event_info.eventId )
	
	local t = Amax.GetAnimatedCameraParams( UIEnums.AnimatedCameraSequenceType.RouteVista, GUI.amax_event_info.route_id )
	if IsTable( t ) == true then
		UIButtons.ChangePosition( SCUI.name_to_id[ "VistaDestination" ], XtToScreenSpaceX( t.px, true ), XtToScreenSpaceY( t.py, true ) )
	end
	
	UIButtons.DummyLerpMatrixCopy( SCUI.name_to_id[ "VistaDestination" ], SCUI.name_to_id[ "VistaDummy" ] )

	if GUI.DisableLoading == true then
		return
	end
	
	Profile.AllowProfileChanges( true )
	Profile.ActOnProfileChanges( false )
	UIGlobals.ActUponFriendChallenges = false
	
	UIGlobals.Ingame = { }
	Amax.SetLoadStateIntoRace()
	UIGlobals.Sp_PreviousFans = nil
	
	if Amax.IsFrontEndLoading() == true then
		GUI.WaitForFinishLoad = true
	else
		HandleLoad()
	end
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Cover the viewport throughout the loading-to-vista zoom.
		-- Keep the preview stencil and its animated transform at authored size.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "background" ], SCREEN_WIDTH )
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "FadeDown_Loading" ], SCREEN_WIDTH )
	end

	SetupInfoLine( UIText.INFO_NEXT_TIP_RT )
	AddLoadingSegs()
	
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true or (UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and LSP.IsConnected() == true) then
		GUI.reading_leaderboard = true
		print("Reading the scoreboard")
		Amax.StartFriendScoreboardRead( 0 )
	end
	
	local panel_node_id = SCUI.name_to_id[ "VistaDestination" ]
	
	UIButtons.TimeLineActive("Goal_Up", true, 1 )
	UIButtons.TimeLineActive("FanPar_Up", true, 1 )
	UIButtons.TimeLineActive("FanRun_Up", true, 1 )
	UIButtons.TimeLineActive("Map_Up", true, 1 )
	
	Sp_UpdateEventPreview( SCUI.name_to_id[ "VistaDestination" ], GUI.event_info, GUI.amax_event_info, GUI.event_id )
	if SP_ChangeRestrictionText( SCUI.name_to_id[ "Restriction_Title" ], SCUI.name_to_id[ "Restriction_Text" ], GUI.event_info ) ~= true then
		local frame_id = SCUI.name_to_id[ "RacesFrame" ]
		local inner_id = SCUI.name_to_id[ "BoxInner" ]
		local sx, sy = UIButtons.GetSize( frame_id )
		sy = sy - 7
		UIButtons.ChangeSize( frame_id, sx, sy )
		UIButtons.ChangeSize( inner_id, sx, sy )
	end

	UpdateRouteShapes( "Shp_Route", 4, GUI.amax_event_info.city_id, GUI.amax_event_info.route_id, "Shp_StartLine" )

	GUI.CurrentTab = GUI.Tabs.Goal
	
	UIShape.ChangeObjectName( SCUI.name_to_id[ "big_KindIcon" ], Sp_KindToShape( GUI.event_info.kind ) )
	
	if GUI.is_fd == true then
		UIButtons.ChangeText( SCUI.name_to_id[ "goal_title" ], UIText.FDE_MEDAL_ACHIEVED )
		
		UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_gold" ], "SPL_EVENT_" .. GUI.event_id .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( GUI.fd_info.state ) .. "_" .. GUI.fd_info.difficulty )
		UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_num_lights_gold" ], "SPL_EVENT_STAR_COUNT_" .. Sp_EventStateToStars( GUI.fd_info.state, ( GUI.event_info.kind == "boss" ) ) )
		
		SpSetStarTexture( SCUI.name_to_id[ "BlurLight_On_gold" ], true, GUI.fd_info.state )
		
		UIButtons.SetActive( SCUI.name_to_id[ "goal_silver" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "goal_bronze" ], false )
		
		UIButtons.SetActive( SCUI.name_to_id[ "fan_par_light_dummy" ], false, true )
		UIButtons.SetActive( SCUI.name_to_id[ "fan_run_light_info" ], false, true )
		
		UIShape.ChangeObjectName( SCUI.name_to_id[ "fan_par_icon" ], Fd_GetCriteriaShape( GUI.event_info.kind ) )
		
		UIButtons.ChangeText( SCUI.name_to_id[ "fan_par_title" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_TITLE_" .. GUI.fd_index .. "_0" )
		UIButtons.ChangeText( SCUI.name_to_id[ "fan_par_fd_info" ], "GAME_FRIEND_DEMAND_STORE_CRITERIA_VAR_" .. GUI.fd_index .. "_0" )
		
		if GUI.fd_info.extra_modifier == UIEnums.FriendDemandModifer.NotUsed then
			GUI.Tabs.Max = GUI.Tabs.FanPar + 1
		else
			UIButtons.SetActive( SCUI.name_to_id[ "fan_run_icon" ], false )
			
			UIButtons.ChangeText( SCUI.name_to_id[ "fan_run_title" ], "GAME_FRIEND_DEMAND_STORE_PARAM_TITLE_" .. GUI.fd_index .. "_0" )
			UIButtons.ChangeText( SCUI.name_to_id[ "fan_run_fd_info" ], "GAME_FRIEND_DEMAND_STORE_PARAM_VAR_" .. GUI.fd_index .. "_0" )
			
			local param_info = Fd_GetParamIconInfo( GUI.fd_info.extra_param )
			local param_icon_id = SCUI.name_to_id[ "fan_run_graphic" ]
			
			UIButtons.SetActive( param_icon_id, true )
			UIButtons.ChangeTexture( param_info.tex, 0, param_icon_id )
			UIButtons.ChangeEffectIndex( param_icon_id, param_info.effect )
			UIButtons.ChangeColour( param_icon_id, param_info.colour )
			
			local modifier_info = Fd_GetModifierIconInfo( GUI.fd_info.extra_modifier )
			local modifier_icon_id = SCUI.name_to_id[ "fan_run_modifier_graphic" ]
			
			UIButtons.SetActive( modifier_icon_id, true )
			UIButtons.ChangeTexture( modifier_info.tex, 0, modifier_icon_id )
			UIButtons.ChangeEffectIndex( modifier_icon_id, modifier_info.effect )
		end
		
	else
		-- Just the map and goal tab for the boss
		if GUI.event_info.kind == "boss" then
			GUI.Tabs.Max = GUI.Tabs.Goal + 1
		end
	
		UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_gold" ], "SPL_EVENT_" .. GUI.event_id .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( "gold" ) )
		
		if GUI.event_info.kind ~= "boss" then
			UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_num_lights_gold" ], "SPL_EVENT_STAR_COUNT_" .. Sp_EventStateToStars( "gold" ) )

			UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_silver" ],	"SPL_EVENT_" .. GUI.event_id .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( "silver" ) )
			UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_num_lights_silver" ], "SPL_EVENT_STAR_COUNT_" .. Sp_EventStateToStars( "silver" ) )

			UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_bronze" ],	"SPL_EVENT_" .. GUI.event_id .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( "bronze" ) )
			UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_num_lights_bronze" ], "SPL_EVENT_STAR_COUNT_" .. Sp_EventStateToStars( "bronze" ) )

		else
			UIButtons.ChangeText( SCUI.name_to_id[ "primary_info_num_lights_gold" ], "SPL_EVENT_STAR_COUNT_" .. Sp_EventStateToStars( "gold", true ) )

			LoadingSPGameSetTabEnabled(GUI.Tabs.FanPar)
			LoadingSPGameSetTabEnabled(GUI.Tabs.FanRun)

			UIButtons.SetActive( SCUI.name_to_id[ "goal_silver" ], false)
			UIButtons.SetActive( SCUI.name_to_id[ "goal_bronze" ], false)
		end
		
		UIButtons.ChangeText( SCUI.name_to_id[ "secondary_info" ],	"SPL_EVENT_" .. GUI.event_id .. "_SECONDARY_GOAL" )
		UIButtons.ChangeText( SCUI.name_to_id[ "fan_demand_info" ],	UIText.SP_EVENT_GFD_INFO )
	end

	LoadingSPGameSetActiveTab( GUI.CurrentTab )

	GUI.RaceType = GUI.event_info.kind

	LoadingSp_HelpTextInit()
	LoadingSp_HelpTextInitLocalTips()
	LoadingSp_HelpTextNext()

end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if GUI.finished == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.GameFlowMessage.LevelDumped and UIGlobals.IsQuickRestart == false then
		StartGameLoad()
	end
	
	if message == UIEnums.Message.ButtonRightTrigger then
		LoadingSp_HelpTextNext()
		
	elseif message == UIEnums.Message.ButtonRight then
		if LoadingSPGameSetActiveTab( GUI.CurrentTab + 1 ) == true then
			UISystem.PlaySound( UIEnums.SoundEffect.LoadingScreenNav )
			UIButtons.TimeLineActive("Bounce_RightArrow", true, 0, true )
		end
		
	elseif message == UIEnums.Message.ButtonLeft then
		if LoadingSPGameSetActiveTab( GUI.CurrentTab - 1 ) == true then
			UISystem.PlaySound( UIEnums.SoundEffect.LoadingScreenNav )
			UIButtons.TimeLineActive("Bounce_LeftArrow", true, 0, true )
		end
	end

end

function FrameUpdate( time )
	if GUI.move_cool_down > 0 then
		GUI.move_cool_down = GUI.move_cool_down - time
	end
	
	LoadingSp_HelpTextUpdate( time )
	
	if GUI.reading_leaderboard == true then
		local complete, rows, row_me, rank_me = Amax.ScoreboardReadComplete()

		GUI.server_timeout = GUI.server_timeout + time

		if GUI.server_timeout > 15 then
			print("LEADERBOARD READ FAILED!!!!!!--------------------------------------------")
			complete = true
			rows = 0
			row_me = -1
			rank_me = -1
		end
		
		if complete == true then
			GUI.reading_leaderboard = false
			print("Finished Reading Leaderboard", rows, row_me, rank_me)
			UIGlobals.SPPostRaceLeaderboardDone = false
		end		
	end		

	if GUI.WaitForFinishLoad == true and Amax.IsFrontEndLoading() == false and GUI.reading_leaderboard == false then
		--print("GUI.WaitForFinishLoad == true and Amax.IsFrontEndLoading() == false, call HandleLoad()")
		GUI.WaitForFinishLoad = false
		HandleLoad()
	end
end

function EnterEnd()
	--
	SetupInfoLine()
end

function EndLoop( time )
	--
end

function End()
	Amax.ExitNetworkLoad()
	
	Profile.ForceProfileUpdate()
	Profile.ActOnProfileChanges( true )
	UIGlobals.ActUponFriendChallenges = true
end

function LoadingSPGameSetTabEnabled( Tab, Enabled )
	GUI.TabsActive[Tab] = Enabled
end

function LoadingSPGameSetActiveTab( Tab )
	if GUI.move_cool_down > 0 then
		return false
	end
	GUI.move_cool_down = 0.15

	local left = (Tab > GUI.CurrentTab)
	local a = Select( left,	"_Up", "_Down" )
	local b = Select( not left,	"_Up", "_Down" )
	local t = nil
	local table = 
	{
		[GUI.Tabs.Goal]		= "Goal",
		[GUI.Tabs.FanPar]	= "FanPar",
		[GUI.Tabs.FanRun]	= "FanRun",
		[GUI.Tabs.Map]		= "Map",
	}

	-- End current
	t = table[ GUI.CurrentTab ]
	if IsString( t ) == true then
		UIButtons.TimeLineActive( t .. a, false, 0 )
		UIButtons.TimeLineActive( t .. b, true )
	end
	
	-- Move
	if Tab == GUI.Tabs.Max then
		GUI.CurrentTab = GUI.Tabs.Min + 1
	elseif Tab == GUI.Tabs.Min then
		GUI.CurrentTab = GUI.Tabs.Max - 1
	else
		GUI.CurrentTab = Tab
	end
	
	-- Start current
	t = table[ GUI.CurrentTab ]
	if IsString( t ) == true then
		UIButtons.TimeLineActive( t .. a, true, 1, true )  -- Grr false, 1, true doesn't work
		UIButtons.TimeLineActive( t .. a, false ) 
		UIButtons.TimeLineActive( t .. b, false, 0 )
	end
	
	return true
end


function LoadingSp_HelpTextInit()

	local tips_table = nil

	if GUI.RaceType == "race" then 
		tips_table	= UIGlobals.sp_race_help_text_ids
	elseif GUI.RaceType == "checkpoint" then 
		tips_table	= UIGlobals.sp_checkpoint_help_text_ids
	elseif GUI.RaceType == "destruction" then 
		tips_table	= UIGlobals.sp_destruction_help_text_ids
	elseif GUI.RaceType == "boss" then 
		tips_table	= UIGlobals.sp_boss_help_text_ids
	elseif GUI.RaceType == "fan" then 
		tips_table	= UIGlobals.sp_fan_help_text_ids
	end
	
	if tips_table ~= nil and #tips_table > 1 then
		return
	end

	if GUI.RaceType == "race" then 
		UIGlobals.sp_race_help_text_ids = { }
	elseif GUI.RaceType == "checkpoint" then 
		UIGlobals.sp_checkpoint_help_text_ids = { }
	elseif GUI.RaceType == "destruction" then 
		UIGlobals.sp_destruction_help_text_ids = { }
	elseif GUI.RaceType == "boss" then  
		UIGlobals.sp_boss_help_text_ids = { }
	elseif GUI.RaceType == "fan" then 
		UIGlobals.sp_fan_help_text_ids = { }
	end
	
	UIGlobals.sp_num_event_help = 50
	UIGlobals.sp_num_generic_help = 100
	
	if GUI.RaceType == "race" then 
		tips_table	= UIGlobals.sp_race_help_text_ids
	elseif GUI.RaceType == "checkpoint" then 
		tips_table	= UIGlobals.sp_checkpoint_help_text_ids
	elseif GUI.RaceType == "destruction" then 
		tips_table	= UIGlobals.sp_destruction_help_text_ids
	elseif GUI.RaceType == "boss" then 
		tips_table	= UIGlobals.sp_boss_help_text_ids
	elseif GUI.RaceType == "fan" then 
		tips_table	= UIGlobals.sp_fan_help_text_ids
	end
		
	for i = 1, UIGlobals.sp_num_generic_help, 1 do

		if i <= UIGlobals.sp_num_event_help then
			-- add one event dependant tip
			if GUI.RaceType == "race" then 
				local text = UIText[ "HLP_RACE_" .. i ]
				if text ~= nil then
					tips_table[ #tips_table + 1 ] = text
				end
			elseif GUI.RaceType == "checkpoint" then 
				local text = UIText[ "HLP_CHECKPOINT_" .. i ]
				if text ~= nil then
					tips_table[ #tips_table + 1 ] = text
				end
			elseif GUI.RaceType == "destruction" then
				local text = UIText[ "HLP_DESTRUCTION_" .. i ]
				if text ~= nil then
					tips_table[ #tips_table + 1 ] = text
				end
			elseif GUI.RaceType == "boss" then 
				local text = UIText[ "HLP_BOSS_" .. i ]
				if text ~= nil then
					tips_table[ #tips_table + 1 ] = text
				end
			elseif GUI.RaceType == "fan" then 
				local text = UIText[ "HLP_FAN_" .. i ]
				if text ~= nil then
					tips_table[ #tips_table + 1 ] = text
				end
			end

		end

		-- and one generic tip
		local genericText = UIText[ "HLP_" .. i ]
		if genericText ~= nil then
			tips_table[ #tips_table + 1 ] = genericText
		end
	end
	
	--mix em up	
	for i = 1, #tips_table, 1 do 	
		local randomIndex = math.random( #tips_table )
		local temp = tips_table[i]
		tips_table[i] = tips_table[randomIndex]
		tips_table[randomIndex] = temp
	end
end

function LoadingSp_HelpTextInitLocalTips()
	
	local fanPosition, currentRank = SinglePlayer.FanStatusForTooltip()
	

	if fanPosition == 1 then
		local text = UIText[ "HLP_EARLYFANSTATUS_" .. currentRank ]
		if text ~= nil then
			GUI.local_tips_table[#GUI.local_tips_table + 1] = text
		end
	elseif fanPosition == 2 then
		local text = UIText[ "HLP_MIDFANSTATUS_" .. currentRank ]
		if text ~= nil then
			GUI.local_tips_table[#GUI.local_tips_table + 1] = text
		end
	elseif fanPosition == 3 then
		local text = UIText[ "HLP_LATEFANSTATUS_" .. currentRank ]
		if text ~= nil then
			GUI.local_tips_table[#GUI.local_tips_table + 1] = text
		end
	end
	
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) then
		for i = 1, 20, 1 do
			local text = UIText[ "HLP_ONLINE_" .. i ]
			if text ~= nil then
				GUI.local_tips_table[#GUI.local_tips_table + 1] = text
			end
		end
	end
	
	if GUI.is_fd ~= nil then
		if GUI.is_fd == true then
			for i = 1, 20, 1 do
				local text = UIText[ "HLP_FRIEND_" .. i ]
				if text ~= nil then
					GUI.local_tips_table[#GUI.local_tips_table + 1] = text
				end
			end
		end
	end
	
	--mix em up	
	for i = 1, #GUI.local_tips_table, 1 do 	
		local randomIndex = math.random( #GUI.local_tips_table )
		local temp = GUI.local_tips_table[i]
		GUI.local_tips_table[i] = GUI.local_tips_table[randomIndex]
		GUI.local_tips_table[randomIndex] = temp
	end

end

function LoadingSp_HelpTextNext()
	local tips_table = nil

	if GUI.RaceType == "race" then 
		tips_table	= UIGlobals.sp_race_help_text_ids
	elseif GUI.RaceType == "checkpoint" then 
		tips_table	= UIGlobals.sp_checkpoint_help_text_ids
	elseif GUI.RaceType == "destruction" then 
		tips_table	= UIGlobals.sp_destruction_help_text_ids
	elseif GUI.RaceType == "boss" then 
		tips_table	= UIGlobals.sp_boss_help_text_ids
	elseif GUI.RaceType == "fan" then 
		tips_table	= UIGlobals.sp_fan_help_text_ids
	end

	local next_text_id
	
	if GUI.CurrentlySelectingFromLocalTips == true and (#GUI.local_tips_table > 0) then
		next_text_id = table.remove( GUI.local_tips_table, 1 )
		 GUI.CurrentlySelectingFromLocalTips = false
	else
		next_text_id  = table.remove( tips_table, 1 ) --math.random( #tips_table ) )
		 GUI.CurrentlySelectingFromLocalTips = true
	end 
	
	local text_id = SCUI.name_to_id[ "hint_text" ]
	UIButtons.ChangeText( text_id, next_text_id )
	UIButtons.TimeLineActive( "hint_text_next", true, 0 )
	
	-- Scale bar to fit text
	local x, y = UIButtons.GetSize( text_id )
	local num_lines = UIButtons.GetStaticTextHeight( text_id ) / y
	UIButtons.ChangeScale( SCUI.name_to_id[ "motd_dummy" ], 1, (num_lines) + 0.3 )
	
	if #tips_table == 0 then
		LoadingSp_HelpTextInit()
	end
	
	PlaySfxToggle()
	
	GUI.tip_timer = 0
end

function LoadingSp_HelpTextUpdate( time )
	GUI.tip_timer = GUI.tip_timer + time
	
	if GUI.tip_timer >= GUI.tip_change_time then
		LoadingSp_HelpTextNext()
	end
end
