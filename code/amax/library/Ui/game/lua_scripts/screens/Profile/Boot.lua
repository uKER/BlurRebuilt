GUI =
{
	finished = false,
	timer = 0,
	
	checking_for_signin_wait_time = 0.25,
	checking_for_signin = false,
	waiting_for_signin = false,
	signin_wait_time = 1,	-- for the MS workaround *sigh* - else it still kicks us back sometimes
	signin_wait_timer = 0,
	
	waiting_for_load = false,
	
	check_for_dlc = false,
	check_for_dlc_time = 0.25,
	check_for_dlc_timer = 0,
	
	check_for_connection = false,
	check_for_connection_time = 0.25,
	check_for_connection_timer = 0,
	
	waiting_to_finish = false,
	waiting_to_finish_time = 5.6,
}

function Init()
	AddSCUI_Elements()
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	
	StoreInfoLine()
	SetupInfoLine()
	
	UIGlobals.ProfileBootContext = UIScreen.Context()
	
	-- reset DLC callback
	UIGlobals.CallbackDLC = nil
	
	GUI.checking_for_signin = true
end

function PostInit()
	-- [Bone] #37663 - Remove ESRB from boot screen
	-- if UIGlobals.CurrentRegion == UIEnums.Region.NorthAmerica then
	--	UIButtons.SetActive( SCUI.name_to_id[ "esrb_text" ], true )
	-- end
	
	UIGlobals.boot_help_text_id, UIGlobals.boot_bottom_help_id = SetupBottomHelpBar( nil, nil, nil, nil, nil, 1.8 )
	UIButtons.PrivateTimeLineActive( UIGlobals.boot_bottom_help_id, "Hide_BottomHelp", true, 0 )	-- hide at start
	
	-- needed here as platform specific
	UIButtons.ChangeText( SCUI.name_to_id[ "auto_saving_text" ], UIText.POP_AUTO_SAVE_WARNING )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	--
end

function FrameUpdate( time )
	if GUI.checking_for_signin == true then
		if Profile.PadProfileActive( UIGlobals.ProfilePressedStart ) == false then
			UIHardware.StartSigninUI( 1 )
		else
			GUI.checking_for_signin_wait_time = 0
			GUI.signin_wait_time = 0
		end
		
		GUI.checking_for_signin = false
		GUI.waiting_for_signin = true
	end
	
	-- wait for guide to close if someone is signing in
	if GUI.waiting_for_signin == true and GUI.timer > GUI.checking_for_signin_wait_time then	-- guide doesn't come up instantly
		if Profile.GuideActive() == false then
			if GUI.signin_wait_timer > GUI.signin_wait_time then
				GUI.waiting_for_signin = false
			
				-- start profile load
				Profile.ActOnProfileChanges( true )	-- any more changes and we'll kick you
				StartAsyncLoad()
				
				GUI.waiting_for_load = true
			end
			
			GUI.signin_wait_timer = GUI.signin_wait_timer + time
		end
	end
	
	-- finished load
	if GUI.waiting_for_load == true and UIGlobals.FileParams.FinishedSuccess ~= nil then
		-- attempt DLC
		if Boot_TryDLC() == false then
			-- attempt net connection
			if Boot_TryConnection() == false then
				Boot_Finished()
			end
		end
		
		GUI.waiting_for_load = false
	end
	
	-- wait to check for DLC
	if GUI.check_for_dlc == true then
		if GUI.check_for_dlc_timer > GUI.check_for_dlc_time then
			Boot_GoDLC()
			GUI.check_for_dlc = false			
		else
			GUI.check_for_dlc_timer = GUI.check_for_dlc_timer + time
		end
	end
	
	-- wait to check for net connection
	if GUI.check_for_connection == true then
		if GUI.check_for_connection_timer > GUI.check_for_connection_time then
			Boot_GoConnection()
			GUI.check_for_connection = false			
		else
			GUI.check_for_connection_timer = GUI.check_for_connection_timer + time
		end
	end
	
	-- wait to finish
	if GUI.waiting_to_finish == true and GUI.timer > GUI.waiting_to_finish_time then
		UIGlobals.ProfileBootFinished = true
		PopScreen()
	end
	
	GUI.timer = GUI.timer + time
end

function EnterEnd()
	RestoreInfoLine()
	UIButtons.TimeLineActive( "auto_save_warning_on", false )
	UIButtons.PrivateTimeLineActive( UIGlobals.boot_bottom_help_id, "Hide_BottomHelp", true )
end

function EndLoop( time )
	--
end

function End()
	--
end

-- check whether DLC is possible
function Boot_TryDLC()
	-- [Blur2PC] The PC DLC worker is empty; continue boot without its popup/thread.
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		return false
	end

	if IsTable( ContextTable[ UIGlobals.ProfileBootContext ] ) == false then
		return false
	end
	
	ContextTable[ UIGlobals.ProfileBootContext ].GUI.check_for_dlc = ( UIGlobals.ProfileState[ Profile.GetPrimaryPad() ] == UIEnums.Profile.GamerProfile )
	return ContextTable[ UIGlobals.ProfileBootContext ].GUI.check_for_dlc
end

-- start DLC
function Boot_GoDLC()
	UIGlobals.CallbackDLC = Boot_CallbackDLC
	SetupCustomPopup( UIEnums.CustomPopups.CheckForDLC )
end

-- DLC finished
function Boot_CallbackDLC( success )
	print( "Boot_CallbackDLC - success", success )
	
	-- attempt net connection
	if Boot_TryConnection() == false then
		Boot_Finished()
	end
end

-- check whether net connection is possible
function Boot_TryConnection()
	if IsTable( ContextTable[ UIGlobals.ProfileBootContext ] ) == false then
		return false
	end
	
	ContextTable[ UIGlobals.ProfileBootContext ].GUI.check_for_connection = ( net_CanReconnectToDemonware() == true )
	return ContextTable[ UIGlobals.ProfileBootContext ].GUI.check_for_connection
end

-- start new connection
function Boot_GoConnection()
	UIGlobals.NetConnectionDarken = false
	net_StartServiceConnection( true, Boot_CallbackConnection )
end

-- connection finished
function Boot_CallbackConnection( success )
	print( "Boot_CallbackConnection - success", success )
	UIGlobals.NetConnectionDarken = true
	Boot_Finished()
end

-- profile boot flow finished
function Boot_Finished()
	if IsTable( ContextTable[ UIGlobals.ProfileBootContext ] ) == false then
		return false
	end
	
	ContextTable[ UIGlobals.ProfileBootContext ].GUI.waiting_to_finish = true
end
