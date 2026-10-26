GUI =
{
	finished = false,
	ProfileDevice = {  },
	reset_controls_to_sp = false,
	carousel_branch = "MpSplitscreenSignIn",

	CanExit = function( _GUI, arg1 )
		-- blag if nothing found
		if UIGlobals.ProfilesFound == 0 then
			UIGlobals.ProfileState[ arg1 ] = UIEnums.Profile.Blagged
			Profile.ClearProfile( arg1 )
			GameProfile.ClearGameProfile( arg1, true )
			Profile.Setup( arg1, true )
			Profile.SetPrimaryPad( arg1 )
			UIGlobals.ProfilesFound = 1
		end
		
		SplitscreenSignIn_ValidateProfiles( false )
		Splitscreen_Clear()
		
		PlaySfxGraphicBack()
		-- [Blur2PC] Retail restores the single-player controls when leaving sign-in.
		_GUI.reset_controls_to_sp = true
		
		return true
	end,

	finished = false,
	
	ProfileState = {  },				-- local copy of UIGlobals.ProfileState so we can check for changes when we're finished
	primary_pad_inactive_ps3 = false,	-- on ps3, the primary pad isn't allowed to re-assign controller number
	
	can_continue = false,				-- do we have enough profiles?
	
	demo = false,
	
	gamerpic_ids = {  },
	frame_ids = {  },
	name_ids = {  },
	info1_ids = {  },
	info2_ids = {  },
	pad_icon_ids = {  },
	pad_connect_ids = {  },
	
	pad_connected = 
	{
		[ 0 ] = true,
		[ 1 ] = true,
		[ 2 ] = true,
		[ 3 ] = true,
	},
	pad_connected_prev = 
	{
		[ 0 ] = true,
		[ 1 ] = true,
		[ 2 ] = true,
		[ 3 ] = true,
	},
	
	update_player_info = false,
}

function Init()
	-- [Blur2PC] Retail PC maps the second keyboard to the next input slot.
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		local primary = Profile.GetPrimaryPad()
		local primary_device = UIGlobals.ProfileDevice[ primary ]
		if UIGlobals.splitscreen_enter == true then
			primary_device = UIGlobals.LastInputDevice
		end
		Amax.SetSplitscreenKeyboards( primary, ( primary + 1 ) % 4,
			primary_device == UIEnums.Device.Keyboard )
	end
	AddSCUI_Elements()
	DeferCam_Init( UIGlobals.CameraNames.MpAppBase )
	
	CarouselApp_SetScreenTimers()
	
	StoreInfoLine()	
	SetupInfoLine()	
	
	-- TODO - set demo flag
	--GUI.demo == *something*
	
	for i = 0, 3, 1 do
		GUI.ProfileDevice[ i ] = UIGlobals.ProfileDevice[ i ]
		UIGlobals.LoadProfile[ i ] = false						-- reset global load table
		GUI.ProfileState[ i ] = UIGlobals.ProfileState[ i ]		-- store the current global state
		
		-- grab the gadget ids
		GUI.gamerpic_ids[ i ]  = SCUI.name_to_id[ "gamerpic_"  .. i ]
		GUI.frame_ids[ i ]  = SCUI.name_to_id[ "frame_"  .. i ]
		GUI.name_ids[ i ]  = SCUI.name_to_id[ "name_"  .. i ]
		GUI.info1_ids[ i ] = SCUI.name_to_id[ "info1_" .. i ]
		GUI.info2_ids[ i ] = SCUI.name_to_id[ "info2_" .. i ]
		GUI.pad_icon_ids[ i ] = SCUI.name_to_id[ "player_pad_icon_" .. i ]
		GUI.pad_connect_ids[ i ] = SCUI.name_to_id[ "connect_pad_" .. i ]
		
		UIButtons.ChangeColour( GUI.frame_ids[ i ], UIGlobals.Splitscreen.colours[ i ] )
		UIButtons.ChangeColour( GUI.name_ids[ i ], UIGlobals.Splitscreen.colours[ i ] )
		UIButtons.ChangeText( GUI.name_ids[ i ], "PROFILE_PAD" .. i .. "_NAME" )
		UIButtons.ChangeColour( GUI.pad_icon_ids[ i ], UIGlobals.Splitscreen.colours[ i ] )
		
		if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
			UIShape.ChangeObjectName( GUI.pad_icon_ids[ i ], "ps3_controller" )
		end
	end
	
	-- Store
	GUI.info1_size_x, GUI.info1_size_y = UIButtons.GetSize( GUI.info1_ids[ 1 ] )
	
	Profile.AllowProfileChanges( true )
	Profile.ActOnProfileChanges( false )
	Profile.AllowAllPadInput( true )
	Profile.LockToPad( -1 )
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC and UIGlobals.splitscreen_enter == true then
		UIGlobals.splitscreen_enter = false
		UIGlobals.ProfileDevice[ UIGlobals.splitscreen_primary_pad_original ] = UIGlobals.LastInputDevice
	end
	SplitscreenSignIn_UpdatePlayerInfo()
	SplitscreenSignIn_UpdatePadChanges( true )
end

function CalcCanContinue()
	-- how many active profiles?
	local num_active_profiles = 0
	
	for i = 0, 3, 1 do
		if UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile or UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged then
			num_active_profiles = num_active_profiles + 1
		end
	end
	
	GUI.can_continue = ( num_active_profiles > 1 )
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		GUI.can_continue = ( ( num_active_profiles > 1 ) and ( GUI.primary_pad_inactive_ps3 == false ) )
	end
end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate(time )
	DeferCam_Update( time )
	
	if GUI.finished == true then
		return
	end
	
	CalcCanContinue()
	
	if GUI.can_continue == true then
		SetupNextAndBack()
	else
		SetupBack()
	end
	
	if GUI.update_player_info == true then
		GUI.update_player_info = false
		SplitscreenSignIn_UpdatePlayerInfo()
	end
	
	SplitscreenSignIn_UpdatePadChanges()

	-- check for the primary controller being disconnected
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		if GUI.primary_pad_inactive_ps3 == true then
			UIButtons.ChangeText( GUI.info1_ids[ Profile.GetPrimaryPad() ], UIText.PRO_SS_CONNECT_CONTROLLER )
			UIButtons.ChangeSize( GUI.info1_ids[ Profile.GetPrimaryPad() ], GUI.info1_size_x, GUI.info1_size_y * 0.7 )
		else
			UIButtons.ChangeText( GUI.info1_ids[ Profile.GetPrimaryPad() ], UIText.CMN_NOWT )
			UIButtons.ChangeSize( GUI.info1_ids[ Profile.GetPrimaryPad() ], GUI.info1_size_x, GUI.info1_size_y )
		end
	
		-- this is only valid for one frame
		GUI.primary_pad_inactive_ps3 = false
	end
	
	-- these are only valid for one frame
	for i = 0, 3, 1 do
		GUI.pad_connected[ i ] = true
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	-- ps3 pad disconnections on primary pad?
	if ( message == UIEnums.Message.ControllerDisconnected ) and ( UIEnums.CurrentPlatform == UIEnums.Platform.PS3 ) and ( arg1 == Profile.GetPrimaryPad() ) then
		GUI.primary_pad_inactive_ps3 = true
		
	elseif ( message == UIEnums.Message.ControllerDisconnected ) and ( UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.Blagged ) then
		-- reset on controller pullout
		message = UIEnums.Message.ButtonY
	end
	
	if message == UIEnums.Message.ControllerDisconnected then
		GUI.pad_connected[ arg1 ] = false
	end
	
	if message == UIEnums.Message.ProfileChanged and GUI.demo == false then
		print( "ProfileChanged" )
		
		-- re-scan the profiles
		for i = 0, 3, 1 do
			if Profile.PadProfileSignedIn( i ) == true then
				print( "profile active", i )
				
				if ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.None ) or ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged ) then
					print( "profile signed in", i )
					
					if i == UIGlobals.splitscreen_primary_pad_original then
						ProfileSignedInOut( true, i )
					end
					
					UIGlobals.ProfileState[ i ] = UIEnums.Profile.GamerProfile
					UIGlobals.LoadProfile[ i ] = true
				end
				
			elseif Profile.PadProfileActive( i ) == false then
				print( "no profile", i )
				
				if ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile ) or ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.PreLoad ) then
					print( "profile signed out", i )
					
					if i == UIGlobals.splitscreen_primary_pad_original then
						ProfileSignedInOut( false, i )
					end
					
					UIGlobals.ProfileState[ i ] = UIEnums.Profile.None
					
					Profile.ClearProfile( i )
					GameProfile.ClearGameProfile( i, true )
					
					UIGlobals.LoadProfile[ i ] = false
				end
			
			else
				print( "profile blagged", i )
			end
		end
		
		-- re-count
		UIGlobals.ProfilesFound = 0
		
		for i = 0, 3, 1 do
			if UIGlobals.ProfileState[ i ] ~= UIEnums.Profile.None then
				UIGlobals.ProfilesFound = UIGlobals.ProfilesFound + 1
			end
		end
		
		print( "profiles found", UIGlobals.ProfilesFound )
		
		GUI.update_player_info = true
		CalcCanContinue()
	end
	
	if message == UIEnums.Message.ButtonY then
		-- [Blur2PC] Retail PC joins the second keyboard in the first free profile slot.
		if UIEnums.CurrentPlatform == UIEnums.Platform.PC and
		   arg1 ~= UIGlobals.splitscreen_primary_pad_original and
		   arg4 == UIEnums.Device.Keyboard then
			local keyboard_slot = -1
			for i = 0, 3, 1 do
				if UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged and
				   UIGlobals.ProfileDevice[ i ] == UIEnums.Device.Keyboard then
					keyboard_slot = i
					break
				end
			end
			if keyboard_slot == -1 then
				for i = 0, 3, 1 do
					if UIGlobals.ProfileState[ i ] == UIEnums.Profile.None then
						keyboard_slot = i
						break
					end
				end
				if keyboard_slot == -1 then
					PlaySfxError()
				else
					PlaySfxToggle()
					UIGlobals.ProfileState[ keyboard_slot ] = UIEnums.Profile.Blagged
					UIGlobals.ProfileDevice[ keyboard_slot ] = arg4
					Profile.ClearProfile( keyboard_slot )
					GameProfile.ClearGameProfile( keyboard_slot, true )
					Profile.Setup( keyboard_slot, true )
					UIGlobals.ProfilesFound = UIGlobals.ProfilesFound + 1
				end
			else
				PlaySfxToggle()
				UIGlobals.ProfileState[ keyboard_slot ] = UIEnums.Profile.None
				UIGlobals.ProfileDevice[ keyboard_slot ] = -1
				Profile.ClearProfile( keyboard_slot )
				GameProfile.ClearGameProfile( keyboard_slot, true )
				UIGlobals.ProfilesFound = UIGlobals.ProfilesFound - 1
			end
		elseif UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.PreLoad then
			PlaySfxToggle()
			
			-- flag a profile not loaded at start to be loaded now
			UIGlobals.LoadProfile[ arg1 ] = true
			UIGlobals.ProfileState[ arg1 ] = UIEnums.Profile.GamerProfile
			UIGlobals.ProfileDevice[ arg1 ] = arg4
		
		elseif UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.None then
			PlaySfxToggle()
			
			-- new blagged
			UIGlobals.ProfileState[ arg1 ] = UIEnums.Profile.Blagged
			UIGlobals.ProfileDevice[ arg1 ] = arg4
			Profile.ClearProfile( arg1 )
			GameProfile.ClearGameProfile( arg1, true )
			Profile.Setup( arg1, true )
			UIGlobals.ProfilesFound = UIGlobals.ProfilesFound + 1
			
		elseif UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.Blagged then
			PlaySfxToggle()
			
			-- sign out blagged
			UIGlobals.ProfileState[ arg1 ] = UIEnums.Profile.None
			UIGlobals.ProfileDevice[ arg1 ] = -1
			
			Profile.ClearProfile( arg1 )
			GameProfile.ClearGameProfile( arg1, true )
			
			UIGlobals.ProfilesFound = UIGlobals.ProfilesFound - 1
			
		elseif UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.GamerProfile then
			local is_pre_load = ( arg1 ~= UIGlobals.splitscreen_primary_pad_original )
			
			if is_pre_load == true then
				PlaySfxToggle()
				
				-- sign out pre-load
				UIGlobals.LoadProfile[ arg1 ] = false
				UIGlobals.ProfileState[ arg1 ] = UIEnums.Profile.PreLoad
			end
		end
		
		GUI.update_player_info = true
		CalcCanContinue()
	end
	
	if message == UIEnums.Message.MenuNext and GUI.can_continue == true then
		if UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.GamerProfile or UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.Blagged then
			SplitscreenSignIn_ValidateProfiles( true )
		end
	end
end

function EnterEnd()
	RestoreInfoLine()
	
	Profile.AllowProfileChanges( true )
	Profile.ActOnProfileChanges( true )
	Profile.AllowAllPadInput( false )
	Profile.LockToPad( Profile.GetPrimaryPad() )
	if GUI.reset_controls_to_sp == true and UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		Amax.Options( nil )
		GUI.reset_controls_to_sp = false
	end
end

function End()
end

function SplitscreenSignIn_UpdatePlayerInfo()
	local is_ps3 = ( UIEnums.CurrentPlatform == UIEnums.Platform.PS3 )
	local is_demo = GUI.demo
	
	for i = 0, 3, 1 do
		if UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile then
			local is_pre_load = ( i ~= UIGlobals.splitscreen_primary_pad_original )
			
			UIButtons.TimeLineActive( "player_active_" .. i, true )
			
			if is_ps3 == true or is_demo == true then
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.CMN_NOWT )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.CMN_NOWT )
			else
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_SIGN_OUT_J )
				UIButtons.ChangeText( GUI.info2_ids[ i ], Select( is_pre_load, UIText.PRO_SS_LEAVE_Y, UIText.CMN_NOWT ) )
			end
			
		elseif UIGlobals.ProfileState[ i ] == UIEnums.Profile.None then
			UIButtons.TimeLineActive( "player_active_" .. i, false )
			
			if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
				-- [Blur2PC] Retail PC's combined numpad + and pad Y prompt.
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_JOIN_PCP_PCY )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.CMN_NOWT )
			elseif is_ps3 == true or is_demo == true then
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_JOIN_Y )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.CMN_NOWT )
			else
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_SIGN_IN_J )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.PRO_SS_JOIN_Y )
			end
			
		elseif UIGlobals.ProfileState[ i ] == UIEnums.Profile.PreLoad then
			UIButtons.TimeLineActive( "player_active_" .. i, false )
			
			if is_ps3 == true or is_demo == true then
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.CMN_NOWT )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.CMN_NOWT )
			else
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_SIGN_OUT_J )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.PRO_SS_JOIN_Y )
			end
		
		elseif UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged then
			UIButtons.TimeLineActive( "player_active_" .. i, true )
			
			if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_LEAVE_PCP_PCY )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.CMN_NOWT )
			elseif is_ps3 == true or is_demo == true then
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_LEAVE_Y )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.CMN_NOWT )
			else
				UIButtons.ChangeText( GUI.info1_ids[ i ], UIText.PRO_SS_SIGN_IN_J )
				UIButtons.ChangeText( GUI.info2_ids[ i ], UIText.PRO_SS_LEAVE_Y )
			end
			
		else
			print( "*** ERROR: invalid profile state ***" )
		end
		
		SplitscreenSignIn_UpdatePad( i )
	end
end

function SplitscreenSignIn_ValidateProfiles( confirm )
	-- sort out the new primary pad
	local old_primary_pad = Profile.GetPrimaryPad()
	local new_primary_pad = -1
	
	for i = 0, 3, 1 do
		if ( ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile ) or ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged ) ) and ( i == old_primary_pad ) then
			new_primary_pad = i
			break
		end
	end
	
	if new_primary_pad == -1 then
		-- if old one signed out or something just find the first active one
		for i = 0, 3, 1 do
			if ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile ) or ( UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged ) then
				new_primary_pad = i
				break	
			end
		end
	end
	
	if new_primary_pad == -1 then
		print( "new_primary_pad == -1. This is bad - setting to pad0 to be safe" )
		new_primary_pad = 0
	end
	
	Profile.SetPrimaryPad( new_primary_pad )
	
	-- do we need to load any profiles?
	local load_profiles = false
	
	if confirm == true and UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		for i = 0, 3, 1 do
			if UIGlobals.LoadProfile[ i ] == true then
				load_profiles = true
				break
			end
		end
	end
	
	-- setup player info
	UIGlobals.Splitscreen.players = {  }
	UIGlobals.Splitscreen.pad_to_player = {  }
	
	for i = 0, 3, 1 do
		if UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile or UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged then
			-- new player
			local player = {  }
			player.pad = i
			
			-- default car and colour for now
			player.car_id = 0
			player.colour_id = 0
			
			UIGlobals.Splitscreen.players[ #UIGlobals.Splitscreen.players + 1 ] = player
			UIGlobals.Splitscreen.pad_to_player[ i ] = #UIGlobals.Splitscreen.players
		end
	end
	-- [Blur2PC] Retail keeps keyboard controls on the active primary and second player.
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		local primary_keyboard = -1
		local second_keyboard = -1
		for i = 0, 3, 1 do
			if UIGlobals.Splitscreen.pad_to_player[ i ] ~= nil then
				if i == UIGlobals.splitscreen_primary_pad_original then
					primary_keyboard = i
				elseif UIGlobals.ProfileDevice[ i ] == UIEnums.Device.Keyboard then
					second_keyboard = i
				end
			end
		end
		Amax.SetSplitscreenKeyboards( primary_keyboard, second_keyboard,
			UIGlobals.ProfileDevice[ UIGlobals.splitscreen_primary_pad_original ] == UIEnums.Device.Keyboard )
	end
	
	if load_profiles == true then
		-- TODO - do we need this?
		
		--UIGlobals.ProfileLookingFor = 0		-- clear
		--UIGlobals.ProfilesFound = 0			-- reset
		--StoreScreen( UIEnums.ScreenStorage.LOAD_NEXT, "Multiplayer\\MpMain.lua" )
		--GoScreen( "Profile\\AsyncLoad.lua" )

		StoreScreen( UIEnums.ScreenStorage.FE_BACK, UIScreen.GetCurrentScreen( UIScreen.Context() ) ) -- Store current
		net_SetRichPresence( UIEnums.RichPresence.SplitScreen )
		GoScreen( "Multiplayer\\Shared\\MpModeSelect.lua" )
		
		PlaySfxNext()
		PlaySfxGraphicNext()
	else
		if confirm == true then
			StoreScreen( UIEnums.ScreenStorage.FE_BACK, UIScreen.GetCurrentScreen( UIScreen.Context() ) )
			net_SetRichPresence( UIEnums.RichPresence.SplitScreen )
			GoScreen( "Multiplayer\\Shared\\MpModeSelect.lua" )
			
			PlaySfxNext()
			PlaySfxGraphicNext()
		end
	end
end

function SplitscreenSignIn_UpdatePadChanges( initial_init )
	for i = 0, 3, 1 do
		if initial_init == true then
			SplitscreenSignIn_UpdatePad( i )
		else
			GUI.pad_connected_prev[ i ] = GUI.pad_connected[ i ]
			SplitscreenSignIn_UpdatePad( i )
		end
	end
end

function SplitscreenSignIn_UpdatePad( pad_index )
	if GUI.pad_connected[ pad_index ] == true or ( pad_index == Profile.GetPrimaryPad() and GUI.primary_pad_inactive_ps3 == true ) then
		UIButtons.SetActive( GUI.name_ids[ pad_index ], true )
		UIButtons.SetActive( GUI.info1_ids[ pad_index ], true )
		UIButtons.SetActive( GUI.info2_ids[ pad_index ], true )
		UIButtons.SetActive( GUI.pad_connect_ids[ pad_index ], false )
		
		local colour = UIGlobals.Splitscreen.colours[ pad_index ]
		local colour_gamerpic = "!255 255 255 255"
			
		if UIGlobals.ProfileState[ pad_index ] == UIEnums.Profile.None or UIGlobals.ProfileState[ pad_index ] == UIEnums.Profile.PreLoad then
			colour = "Main_Black"
			colour_gamerpic = "Main_Black"
		end
		
		UIButtons.ChangeColour( GUI.gamerpic_ids[ pad_index ], colour_gamerpic )
		UIButtons.ChangeColour( GUI.frame_ids[ pad_index ], colour )
		UIButtons.ChangeColour( GUI.name_ids[ pad_index ], colour )
		UIButtons.ChangeColour( GUI.pad_icon_ids[ pad_index ], colour )
	else
		UIButtons.SetActive( GUI.name_ids[ pad_index ], false )
		UIButtons.SetActive( GUI.info1_ids[ pad_index ], false )
		UIButtons.SetActive( GUI.info2_ids[ pad_index ], false )
		UIButtons.SetActive( GUI.pad_connect_ids[ pad_index ], true )
		
		UIButtons.ChangeColour( GUI.gamerpic_ids[ pad_index ], "Main_Black" )
		UIButtons.ChangeColour( GUI.frame_ids[ pad_index ], "Main_Black" )
		UIButtons.ChangeColour( GUI.name_ids[ pad_index ], "Main_Black" )
		UIButtons.ChangeColour( GUI.pad_icon_ids[ pad_index ], "Main_Black" )
	end
end
