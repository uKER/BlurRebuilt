GUI =
{
}

function Run( func, arg1 )
	print("Func:", func, "Arg:", arg1)
	
	if UIScreen.Context() ~= 0 then
		return
	end
	
	-- Grab local GUI because this can be called from Sorcery
	local GUI_Main	= ContextTable[ UIEnums.Context.Main ].GUI
	local SCUI_Main = ContextTable[ UIEnums.Context.Main ].SCUI
	
	if GUI_Main == nil or SCUI_Main == nil then
		print( "Command.lua Run() has no context[0].GUI / .SCUI table" )
		return
	end
	
	-- These could be nil if their is no active context so you might want to be carfull when calling subscreen commands? Or check it
	local GUI_Active = nil
	local SCUI_Active = nil
	
	if ContextTable[ GUI_Main.active_context ] ~= nil then
		GUI_Active = ContextTable[ GUI_Main.active_context ].GUI
		SCUI_Active = ContextTable[ GUI_Main.active_context ].SCUI
	end
		
	------------------------
	-- Common commands
	------------------------
	if func == "open_app" then
		local ftime, selection = UIButtons.GetSelection( GUI_Main.carousel_id, 2 )
		OpenApp( selection )
		
	elseif func == "pan_in" then
		UIButtons.TimeLineActive( "start_load", false )

	elseif func == "pan_out" then
		UIButtons.TimeLineActive( "start_load", true )
	
	elseif func == "close_app" then
		local ftime, selection = UIButtons.GetSelection( GUI_Main.carousel_id, 2 )
		CloseApp( selection )
		
	elseif func == "lock_controller" then
		LockController()
		UIButtons.SetSelected( GUI_Main.carousel_id, false )
		
	elseif func == "unlock_controller" then
		UnlockController()
		
	elseif func == "select" then
		UIButtons.SetSelection( GUI_Main.carousel_id, arg1 )
	end
	
	------------------------
	-- Group specific commands
	------------------------ 
	if func == "select_group" then
		UIButtons.SetSelected( GUI_Active.GroupListId, true )
		UIButtons.SetSelection( GUI_Active.GroupListId, UIGlobals.JumpGroupId )
	end
	
	------------------------
	-- Profile specific commands
	------------------------ 
	if func == "profile_update_start" then
	
	elseif func == "profile_update_show_icon" then
		UIButtons.TimeLineActive( "show_fan_icon", true )
		UIButtons.TimeLineActive( "hide_fan_icon", false )
		
	elseif func == "profile_update_hide_icon" then
		UIButtons.TimeLineActive( "hide_fan_icon", true )
		UIButtons.TimeLineActive( "show_fan_icon", false )
	
	elseif func == "profile_update_show_fans" then
		UIButtons.TimeLineActive( "show_fans_earned", true )
		
	elseif func == "profile_update_hide_fans" then
		UIButtons.TimeLineActive( "show_fans_earned", false )
		
	elseif func == "profile_update_show_cash" then
		UIButtons.TimeLineActive( "show_cash_earned", true )
		
	elseif func == "profile_update_hide_cash" then
		UIButtons.TimeLineActive( "show_cash_earned", false )
	
	elseif func == "profile_update_fans_start" then
		GUI_Active.UpdateFans = true
	
	elseif func == "profile_update_fans_end" then
		GUI_Active.UpdateFans = false
		
	elseif func == "profile_update_cash_start" then
		GUI_Active.UpdateCash = true
	
	elseif func == "profile_update_cash_end" then
		GUI_Active.UpdateCash = false
		
	elseif func == "profile_update_end" then
		-- Turn fan animation off
		UIButtons.SetActive( SCUI_Active.name_to_id[ "FanBody" ], false, true )
		
		-- Turn friends wall back on...
		UIButtons.SetActive( GUI_Active.FriendsListId, true )
		UIButtons.SetActive( SCUI_Active.name_to_id[ "Friends" ], true )
		
		SetupInfoLine( UIText.INFO_VEW_DETAILS, UIText.INFO_B_BACK, UIText.INFO_TOGGLE_PERKS, UIText.INFO_EDIT_MEDIA )
	elseif func == "open_game_options" then
		-- Get active context GUI
		if IsNumber( GUI_Main.active_context ) == true then
			GoScreen("Shared\\Options_DrivingAssists.lua", GUI_Main.active_context )
		end
	elseif func == "open_options_gamma" then
		-- Get active context GUI
		if IsNumber( GUI_Main.active_context ) == true then
			GoScreen("Shared\\Options_ImageAdjust.lua", GUI_Main.active_context )
		end
	end	

	------------------------
	-- MpRaceResults
	------------------------ 
	if func == "mp_raceresults_start" then
		UIButtons.TimeLineActive( "show_finished", true )
		UIButtons.SetActive( SCUI_Main.name_to_id[ "icon" ], true, true )

	elseif func == "mp_raceresults_hide_finished" then
		UIButtons.TimeLineActive( "hide_finished", true )

	elseif func == "mp_raceresults_update_fans" then
		UIButtons.TimeLineActive( "show_fan_icon", true )
		UIButtons.SetActive( SCUI_Main.name_to_id[ "fan_icon" ], true, true )

		GUI_Main.UpdatingFans = true
		SetupInfoLine( UIText.INFO_SKIP_A )
	end
	
	------------------------------
	-- Tutorial specific commands
	------------------------------
	if GUI_Main.do_tutorial == false then
		return
	end
	
	if func == "avatar_on" then
		UIButtons.TimeLineActive( "avatar_scale", true )
	elseif func == "avatar_off" then
		UIButtons.TimeLineActive( "avatar_scale", false )
		UIButtons.SetActive( GUI_Main.tutorial_text_id, false )
	elseif func == "next_text" then
		UIButtons.SetActive( GUI_Main.tutorial_text_id, true )
		UIButtons.ResetTextPrint( GUI_Main.tutorial_text_id )
		UIButtons.ChangeText( GUI_Main.tutorial_text_id, GUI_Main.tutorial_step_text[ GUI_Main.tutorial_text_index ] )
		UISystem.PlaySound( GUI_Main.tutorial_step_vo[ GUI_Main.tutorial_text_index ] )
		GUI_Main.tutorial_text_index = GUI_Main.tutorial_text_index + 1
	elseif func == "init_app" then
		local ftime, selection = UIButtons.GetSelection( GUI_Main.carousel_id, 2 )
		UIButtons.TimeLineActive( "fade_" .. GUI_Main.branch_name[ selection ], true )
	elseif func == "enable" then
		EndTutorial()
	end
end
