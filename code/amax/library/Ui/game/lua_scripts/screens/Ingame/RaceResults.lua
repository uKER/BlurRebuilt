GUI =
{
	finished = false,

	anim_finished = false,
	wait_retry = false,
}

function Init()
	AddSCUI_Elements( )
	
	if Amax.FansInRace() == false then
		UIButtons.SetActive( SCUI.name_to_id[ "Bra_Fans" ], false )
	end
	
	-- Status bar
	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	
	local statusBarId = UIButtons.CloneXtGadgetByName( "SCUIBank", "Dmy_StatusBar" )
	UIButtons.SetParent( statusBarId, GUI.camera_id, UIEnums.Justify.MiddleCentre )

	local obj_table = SCUI.elements[ SCUI.name_to_index[ "_objectives" ] ]
	GUI.overall_pass = GameProfile.HasPassedEvent()
	
	local col_pass = "Support_0"
	local col_fail = "Support_3"
	
	local current_event = Amax.GetCurrentEvent()
	
	-- chanage event name and icon
--	UIButtons.ChangeText( SCUI.name_to_id[ "event_name" ], current_event.name )
--	UIShape.ChangeObjectName( SCUI.name_to_id[ "event_icon" ], current_event.icon )
end

function FinishedGoal( goal_num, is_dark )
	local has_passed
		
	-- #18846 results show achieved if wrecked
	if GUI.has_wrecked == true then
		has_passed = false
	else
		has_passed = Amax.HasPassedRule( goal_num+1 )
	end
	
	if has_passed == true then
		if is_dark == true then
			goal_num = goal_num + 1
			if goal_num == Amax.GetNumRules() then
				UIButtons.TimeLineActive( "pass_fail_in", true )
			else
				UIButtons.TimeLineActive( "slide_in_goal" .. goal_num, true )
			end
		else
			UIButtons.TimeLineActive( "darken_goal" .. goal_num, true )
		end
	else
		goal_num = goal_num + 1
		if goal_num == Amax.GetNumRules() then
			UIButtons.TimeLineActive( "pass_fail_in", true )
		else
			UIButtons.TimeLineActive( "slide_in_goal" .. goal_num, true )
		end
	end
end

function EndAnim()
	if GUI.anim_finished == true then
		return
	end
	
	GUI.anim_finished = true

	GUI.dialogue_box_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Dmy_DlgBox", 0 )
	UIButtons.SetParent( GUI.dialogue_box_id, SCUI.name_to_id[ "Dmy_RetryDialogue" ], UIEnums.Justify.MiddleCentre )
	
	local body
	local info
	if GUI.overall_pass == true then
		Amax.PlayPostEventCheer( 0 )

		UIButtons.TimeLineActive( "cash_added", true )
		UIButtons.TimeLineActive( "cash_added2", true )
		
		if UIGlobals.FirstRace == true then
			info = UIText.INFO_A_ACCEPT_B_RETRY
			body = "GAME_END_OF_FIRST_RACE_DIFF"
		else
			info = UIText.INFO_A_ACCEPT_Y_CREATE_FRIEND_DEMAND
			body = UIText.CMN_RETURN_TO_BLUR
		end
	else
		info = UIText.INFO_A_RETRY_B_BACK_TO_MENU
		body = UIText.CMN_WOULD_YOU_LIKE_TO_RETRY
	end

	local dlg_msg = UIButtons.FindChildByName( GUI.dialogue_box_id, "Txt_DlgBoxMessage" )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.dialogue_box_id, "Txt_DlgBoxInfoLine" ), info )
	UIButtons.ChangeText( dlg_msg, body )
	
	DialogueBox_Resize( GUI.dialogue_box_id, body )
	
	UIButtons.TimeLineActive( "fan_out", true )
end

function PostInit()
	DarkenScreen()
	
	GUI.has_wrecked = (UIGlobals.Ingame.RaceFinishedMsg == UIEnums.GameFlowMessage.Wrecked)
	if GUI.has_wrecked == true then
		GUI.overall_pass = false
	end
	
	if UIGlobals.FirstRace == true or Amax.ValidGoalInRace() == false then
		UIButtons.SetActive( SCUI.name_to_id[ "Bra_Dummy" ], false )
		EndAnim()
	else
		local num_rules = Amax.GetNumRules()
	
		-- Goal or goals
		if num_rules == 1 then
			UIButtons.ChangeText( SCUI.name_to_id[ "Txt_GoalsTitle" ], UIText.HUD_TARGET )
		else
			UIButtons.ChangeText( SCUI.name_to_id[ "Txt_GoalsTitle" ], UIText.HUD_TARGETS )
		end
		
		-- Resize goals box
		local box_id = SCUI.name_to_id[ "Box_GoalsBox" ]
		if IsNumber( box_id ) == true then
			local w, h = UIButtons.GetSize( box_id )
			UIButtons.ChangeSize( box_id, UIButtons.GetStaticTextLength( SCUI.name_to_id[ "Txt_GoalsTitle" ] ) + 1.5, h )
		end


		-- Add race rules
		for i = 1, 2, 1 do
			local shape_id = SCUI.name_to_id[ "Shp_GoalsText" .. i - 1 ]
			local text_id =  SCUI.name_to_id[ "Txt_GoalsText" .. i - 1 ]
			if i > num_rules then
				UIButtons.SetActive( text_id, false )
				UIButtons.SetActive( shape_id, false )
			else
				-- #18846 results show passed if wrecked
				local has_passed = false
				if GUI.has_wrecked == false then
					has_passed = Amax.HasPassedRule( i )
				end
				if has_passed == true then
					UIShape.ChangeObjectName( shape_id, "tick" )
				end
			end
		end
	end
	
	if GUI.has_wrecked == true then
		
		UIButtons.SetActive( SCUI.name_to_id[ "icon_happy" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "Txt_Pass" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "cash_added" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "cash_added2" ], false )
		
		UIButtons.ChangeText( SCUI.name_to_id[ "Txt_Fail" ], UIText.CMN_WRECKED )
	else
		if GUI.overall_pass == true then
			UIButtons.SetActive( SCUI.name_to_id[ "icon_sad" ], false )
			UIButtons.SetActive( SCUI.name_to_id[ "Txt_Fail" ], false )
		else
			UIButtons.SetActive( SCUI.name_to_id[ "icon_happy" ], false )
			UIButtons.SetActive( SCUI.name_to_id[ "Txt_Pass" ], false )
			UIButtons.SetActive( SCUI.name_to_id[ "cash_added" ], false )
			UIButtons.SetActive( SCUI.name_to_id[ "cash_added2" ], false )
		end
	end
	
	if UIGlobals.LaunchMode == UIEnums.LaunchMode.Demo then
		local turn_it_off = 
		{
			"Box_GoalsBox",
			"Txt_GoalsTitle",
			"Txt_GoalsText0",
			"Txt_GoalsText1",
			"Shp_GoalsText0",
			"Shp_GoalsText1",
		}
	
		for i,v in ipairs( turn_it_off ) do
			UIButtons.SetActive( SCUI.name_to_id[ v ], false )
		end		
	end
end

function StartLoop( time )
	--
end

function GoRetry()
	StartAsyncSave()
	StopIngameMusic()
	
	Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)
	Amax.SendMessage(UIEnums.GameFlowMessage.StopGameUpdate)
								
	UIGlobals.IsQuickRestart = true
	GoLoadingScreen("Loading\\LoadingSpGame.lua")
	Amax.GameDataLogRetry()
end

function GoOnwards()
	if RaceBook.PostEventAnimaticExists() == true then
		GoScreen( "Ingame\\PlayAnimatic.lua" )
	elseif RaceBook.PostEventMessageExists() == true then
		GoScreen("Ingame\\RaceOutro.lua")
	else
		GoScreenSpPostCutScene()
	end
end

function GoMenu()
	StartAsyncSave()
	GoScreen("Loading\\LoadingUi.lua")
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.wait_retry == true then
		if message == UIEnums.GameFlowMessage.LevelDumped then
			UIGlobals.Ingame = { }
			StartGameLoad()
		end
	end
	
	if GUI.wait_retry == true then
		return
	end

	if GUI.finished == true then
		return
	end
	
	if GUI.anim_finished == false then
		if message == UIEnums.Message.MenuNext and arg2 == true then
			UIButtons.TimeLineActive( "fan_in", true, 1 )
			UIButtons.TimeLineActive( "fan_out", true, 1 )
			UIButtons.TimeLineActive( "pass_fail_in", true )
			UIButtons.TimeLineActive( "goal_box_in", true )
			UIButtons.TimeLineActive( "slide_in_goal0", true )
			UIButtons.TimeLineActive( "slide_in_goal1", true )
			UIButtons.TimeLineActive( "fade_in_goal0", true, 1 )
			UIButtons.TimeLineActive( "fade_in_goal1", true, 1 )
			
			EndAnim()
		end
		
		return
	end
	
	if GUI.overall_pass == false then
		if message == UIEnums.Message.MenuNext and arg2 == true then
			GoRetry()
		elseif message == UIEnums.Message.MenuBack and arg2 == true then
			GoMenu()
		end
	else
		-- The only option if you pass
		if message == UIEnums.Message.MenuNext and arg2 == true then
			GoOnwards()
		elseif message == UIEnums.Message.ButtonY and arg2 == true then
			GoScreen("Ingame\\CreateFriendDemand.lua")
		elseif message == UIEnums.Message.MenuBack and arg2 == true and UIGlobals.FirstRace == true then
			GoRetry()
		end
	end

end

function FrameUpdate( time )
	--
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	GoRetry = nil
	GoMenu = nil
	GoOnwards = nil
	EndAnim = nil
	FinishedGoal = nil
end

