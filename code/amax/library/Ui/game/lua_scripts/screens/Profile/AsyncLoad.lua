GUI =
{
	finished = false,
	
	delay_time = 1.5,
	
	-- message text
	textId = -1,
	success = false,
	
	net_global_update_enabled = false,
}

function Init( )
	AddSCUI_Elements()
	
	InfoLineLock()
	StoreInfoLine_F()
	SetupInfoLine_F()
	
	UISystem.PlaySound( UIEnums.SoundEffect.GraphicTextBox )
		
	local sx, sy, sz = UIButtons.GetSize( SCUI.name_to_id[ "box" ] )
	GUI.default_box_height = sy
	
	-- disable network global update so it doesn't force us off this script until we finish the save process
	GUI.net_global_update_enabled = UIGlobals.Network.GlobalUpdateEnabled
	net_EnableGlobalUpdate( false )
	
	UIScreen.SetScreenTimers( 0.3, 0.3, UIScreen.Context() )
	
	GUI.textId = SCUI.name_to_id[ "text" ]
	
	-- ***
	-- have we blagged any of the profiles?
	GUI.blaggedAProfile = false
	-- have we found a valid profile to load?
	GUI.foundProfileToLoad = false
	-- are we done?
	GUI.finishedLookingForProfiles = false
	-- valid profile found to load, so attempt to find a valid device
	GUI.checkForDevice = false
	-- are we currently loading?
	GUI.loading = false
	-- blocking pad messages
	GUI.allowPadInput = false
	-- allowed to change storage devices?
	GUI.allowSaveLocationChange = false
	-- enough free space to save if we try to load and the file doesn't exist?
	GUI.enoughFreeSpace = true
	-- amount of free space we need - 360 only
	GUI.freeSpaceAmount = ( 1 * 1024 * 1024 ) -- 1MB
	-- return value of the save system when loading in
	GUI.profileReturnCode = UIEnums.ProfileError.Okay
	-- allow to overwrite a corrupt profile?
	GUI.allowProfileOverwrite = false
	-- are we currently deleting?
	GUI.deleting = false
	-- did the deletion bork up?
	GUI.errorWhenDeleting = false
	-- delay info bar if we load very quickly
	GUI.timer = 0
	-- ***
	
	AsyncLoad_ChangeText( UIText.IDS_CMN_NOWT )

	-- setup the save system
	UIGlobals.FileParams.SetInterface( true )
	
	-- no more changes please, or we'll kick you
	Profile.ActOnProfileChanges( true )
	
	-- sort out the kit profiles
	for i = 0, 3, 1 do
		-- need to check for PreLoad profiles
		UIGlobals.ProfileLookingFor = i
		print( "UIGlobals.ProfileLookingFor", UIGlobals.ProfileLookingFor )
		
		-- valid gamer profile?
		if Profile.PadProfileActive( i ) == true then
			print( "found profile on ", UIGlobals.ProfileLookingFor )
			
			-- create a new profile
			Profile.Setup( i )
			GameProfile.ClearGameProfile( i )
			UIGlobals.ProfilesFound = UIGlobals.ProfilesFound + 1
			-- reset the "do not save" flag
			UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
			
			-- valid profile to load ...
			UIGlobals.ProfileState[ i ] = UIEnums.Profile.GamerProfile
			-- ... unless we're not the guy who pressed start
			if i ~= UIGlobals.ProfilePressedStart then
				print( "pre load on slot " .. i )
				UIGlobals.ProfileState[ i ] = UIEnums.Profile.PreLoad
			end
			
			-- have we found a valid profile to load?
			if UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile then
				GUI.foundProfileToLoad = true
			end
			
		-- person who pressed start doesn't have a valid gamer profile
		elseif UIGlobals.ProfileLookingFor == UIGlobals.ProfilePressedStart then
			print( "blagged profile for slot ", UIGlobals.ProfilePressedStart )

			-- create a new blagged profile
			Profile.Setup( UIGlobals.ProfilePressedStart, true )
			GameProfile.ClearGameProfile( UIGlobals.ProfilePressedStart )
			UIGlobals.ProfilesFound = UIGlobals.ProfilesFound + 1
			
			UIGlobals.ProfileState[ i ] = UIEnums.Profile.Blagged
			GUI.blaggedAProfile = true
			
		-- else we're not active
		else
			UIGlobals.ProfileState[ i ] = UIEnums.Profile.None
		end
	end
	
	-- attempt to start a load by checking for a valid storage device
	GUI.checkForDevice = true
	
	-- if we've not found any new profiles then we're done
	if GUI.foundProfileToLoad == false and GUI.blaggedAProfile == false then
		GUI.finishedLookingForProfiles = true
		GUI.checkForDevice = false
	end
	
	if GUI.blaggedAProfile == true then
		-- if we've blagged a profile - then they can't load/save
		GUI.checkForDevice = false
		GUI.allowPadInput = true
		AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfilePressedStart .. "_BLAGGED" )
		AsyncLoad_SetupInfoLine( UIText.INFO_A_NEXT )
	else
		Amax.InitialiseAdverts()
	end
	
	-- reset this as the PreLoad profiles are sorted now
	UIGlobals.ProfileLookingFor = UIGlobals.ProfilePressedStart
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2 )
	if GUI.finished == true or GUI.loading == true or GUI.deleting == true then
		return
	end
	
	-- player in-specific messages
	if message == UIEnums.Message.SaveLocationNotFound then
		GUI.allowPadInput = true
		GUI.allowSaveLocationChange = true
		UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
		AsyncLoad_ChangeText( UIText.PRO_NO_STORAGE_DEVICE )
		AsyncLoad_SetupInfoLine( UIText.INFO_CHOOSE_STORAGE_A, UIText.INFO_CONTINUE_B )
		return
	end
	
	if message == UIEnums.Message.SaveLocationFound then
		GUI.allowPadInput = false
		GUI.allowSaveLocationChange = false
		GUI.enoughFreeSpace = true
		UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
		AsyncLoad_ChangeText( UIText.PRO_LOADING )
		AsyncLoad_SetupInfoLine()
		
		-- start the async load
		print( "start async load" )
		GUI.loading = true
		Profile.StartAsyncLoad( UIGlobals.ProfileLookingFor )
		UIButtons.TimeLineActive( "auto_save_warning_on", true, 0 )
		UIButtons.PrivateTimeLineActive( UIGlobals.boot_bottom_help_id, "Hide_BottomHelp", false )
		
		-- reset timer
		GUI.timer = 0
		return
	end
	
	if GUI.allowPadInput == false then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		PlaySfxNext()
		
		-- choose storage device again?
		if GUI.allowSaveLocationChange == true then
			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
				UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileLookingFor, 0, UIEnums.XboxDeviceSelector.ForceUI )
			end
			return
		end
		
		if GUI.allowProfileOverwrite == true then
			-- start the deletion of the current profile
			GUI.deleting = true
			Profile.StartAsyncDelete( UIGlobals.ProfileLookingFor )
			GUI.allowPadInput = false
			return
		end
		
		-- did we blag this profile?
		if GUI.blaggedAProfile == true then
			Finished()
			return
		end
		
		if GUI.errorWhenDeleting == true then
			-- nope - what you pressing this for?
			return
		end
		
		-- are we a new user?
		if GUI.profileReturnCode == UIEnums.ProfileError.FileDoesNotExist then
			-- enough space to save out?
			if GUI.enoughFreeSpace == false then
				-- restrict to devices with enough free space
				UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileLookingFor, GUI.freeSpaceAmount, UIEnums.XboxDeviceSelector.ForceUI )
				return
			else
				-- save out
				StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, GetStoredScreen( UIEnums.ScreenStorage.LOAD_NEXT ) )
				UIGlobals.ProfileSaveIndex = UIGlobals.ProfileLookingFor
				GoScreen( "Profile\\AsyncSave.lua" )
				GUI.success = nil
				return
			end
		else
			Finished()
			return
		end
	
	elseif message == UIEnums.Message.MenuBack then
		-- cancelling when told we've not chosen a storage device or we don't have enough space to save-out if the file doesn't exist
		if GUI.allowSaveLocationChange == true or GUI.enoughFreeSpace == false then
			Finished()
			return
		end
		
		if GUI.allowProfileOverwrite == true or GUI.errorWhenDeleting == true then
			if UIEnums.CurrentPlatform ~= UIEnums.Platform.PS3 then
				-- they now can't save
				UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
				UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
				Finished()
			end
			return
		end
	end
end

function FrameUpdate( time )
	-- checking for storage device?
	if GUI.checkForDevice == true then
		print( "check for storage device" )
		-- this check fixes the bug with pressing start and then the guide button straight after - which makes the checking for device call fail
		if Profile.GuideActive() == false then
			-- if we've found a valid profile to load then check for storage device
			if GUI.foundProfileToLoad == true then
				if UIEnums.CurrentPlatform == UIEnums.Platform.PC or UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
					-- just send the message to say we've found a storage device
					UIScreen.AddMessage( UIEnums.Message.SaveLocationFound )
				else
					-- attempt to find a device
					UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileLookingFor, 0 )
				end
			elseif GUI.blaggedAProfile == false and UIGlobals.ProfileLookingFor >= 3 then
				-- we've don't need to do anything - finished
				GUI.finishedLookingForProfiles = true
			end
			-- done
			GUI.checkForDevice = false
		end
	end
	
	if GUI.loading == true then
		GUI.timer = GUI.timer + time
		
		-- continue the load
		local finished, error = Profile.ContinueAsyncLoad()
		
		if finished == true then
			-- delay the info bar
			if GUI.timer > GUI.delay_time then
				-- store the error code
				GUI.profileReturnCode = error
				
				GUI.loading = false
				GUI.allowPadInput = true
			
				print( "FINISHED LOAD", error )
				
				-- now check the error code
				if error == UIEnums.ProfileError.Okay then
					-- we've loaded fine
					UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
					--AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_LOADED" )
					--AsyncLoad_SetupInfoLine( UIText.INFO_A_NEXT )
					GUI.success = true
					Finished()
					
				elseif error == UIEnums.ProfileError.FileDoesNotExist then
					-- check free space here
					if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
						local space, ok = UIHardware.SpaceLeftOnDevice()
						
						if ok == false then
							-- pulled MU
							GUI.profileReturnCode = UIEnums.ProfileError.DeviceNotConnected
							GUI.allowSaveLocationChange = true
							UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
							AsyncLoad_ChangeText( UIText.PRO_DEVICE_REMOVED_LOAD )
							AsyncLoad_SetupInfoLine( UIText.INFO_CHOOSE_STORAGE_A, UIText.INFO_CONTINUE_B )
							
						elseif space < GUI.freeSpaceAmount then
							GUI.enoughFreeSpace = false
							UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
							AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_NO_SPACE" )
							AsyncLoad_SetupInfoLine( UIText.INFO_CHOOSE_STORAGE_A, UIText.INFO_CONTINUE_B )
							
						else
							-- we have some space - ready to save out new profile
							UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
							Profile.Setup( UIGlobals.ProfileLookingFor )
							GameProfile.ClearGameProfile( UIGlobals.ProfileLookingFor )
							AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_CREATED" )
							AsyncLoad_SetupInfoLine( UIText.INFO_A_NEXT )
						end
						
					else
						-- save out new profile
						UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
						Profile.Setup( UIGlobals.ProfileLookingFor )
						GameProfile.ClearGameProfile( UIGlobals.ProfileLookingFor )
						AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_CREATED" )
						AsyncLoad_SetupInfoLine( UIText.INFO_A_NEXT )
					end
					
				elseif error == UIEnums.ProfileError.CannotReadFile then
					-- "profile out of date" (debug) or corrupt -- just display a corrupt message
					GUI.allowProfileOverwrite = true
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
					AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_CORRUPT" )
					
					if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
						AsyncLoad_SetupInfoLine( UIText.INFO_DELETE_A )
					else
						AsyncLoad_SetupInfoLine( UIText.INFO_OVERWRITE_A, UIText.INFO_CONTINUE_B )
					end
					
				elseif error == UIEnums.ProfileError.DeviceNotConnected then
					GUI.allowSaveLocationChange = true
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
					AsyncLoad_ChangeText( UIText.PRO_DEVICE_REMOVED_LOAD )
					AsyncLoad_SetupInfoLine( UIText.INFO_CHOOSE_STORAGE_A, UIText.INFO_CONTINUE_B )
					
				elseif error == UIEnums.ProfileError.DifferentOwner then
					-- PS3 only - save data is not owned by this user - allow them to overwrite it though
					GUI.allowProfileOverwrite = true
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
					AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_DIFFERENT_OWNER" )
					AsyncLoad_SetupInfoLine( UIText.INFO_DELETE_A )
				end
			end
		end
	end
	
	if GUI.deleting == true then
		-- continue the deletion
		local finished, error = Profile.ContinueAsyncDelete()
		
		if finished == true then
			GUI.deleting = false
			GUI.allowPadInput = true
			
			print( "FINISHED DELETION" )
			
			-- now check the error code
			if error == UIEnums.ProfileError.Okay then
				-- we deleted the old one fine - so create a new one and save out
				Profile.Setup( UIGlobals.ProfileLookingFor )
				GameProfile.ClearGameProfile( UIGlobals.ProfileLookingFor )
				UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
				UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
				
				-- save out
				StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, GetStoredScreen( UIEnums.ScreenStorage.LOAD_NEXT ) )
				UIGlobals.ProfileSaveIndex = UIGlobals.ProfileLookingFor
				GoScreen( "Profile\\AsyncSave.lua" )
				GUI.success = nil
				
			elseif error == UIEnums.ProfileError.CannotDeleteFile then
				-- eek - manual deletion required
				GUI.errorWhenDeleting = true
				AsyncLoad_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_DELETE_FAILED" )
				AsyncLoad_SetupInfoLine( UIText.INFO_CONTINUE_B )
			end
			
			-- else on PS3 the user cancelled the delete request - so stay in the same state
		end
	end
	
	if GUI.finishedLookingForProfiles == true then
		Finished()
	end
end

function EnterEnd()
	RestoreInfoLine_F()
	InfoLineUnlock( true )
end

function EndLoop( time )
	--
end

function End()
	UIGlobals.FileParams.FinishedSuccess = GUI.success
end

function Finished()
	-- Send message
	if IsNumber( UIGlobals.FileParams.FinishedMessage ) == true then
		Amax.SendMessage( UIGlobals.FileParams.FinishedMessage  )
	end

	Amax.Options( nil )
	SinglePlayer.ProcessUnlocks()
	SinglePlayer.ClearUnlocks()

	net_EnableGlobalUpdate( GUI.net_global_update_enabled )
	UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false

	GoScreen( GetStoredScreen( UIEnums.ScreenStorage.LOAD_NEXT ) )
end

function AsyncLoad_ChangeText( text )
	UIButtons.ChangeText( GUI.textId, text )
	
	-- re-size box
	local box_id = SCUI.name_to_id[ "box" ]
	local text_height = UIButtons.GetStaticTextHeight( GUI.textId )
	local sx, sy, sz = UIButtons.GetSize( box_id )
	sy = GUI.default_box_height + text_height
	UIButtons.ChangeSize( box_id, sx, sy, sz )
	UIButtons.ChangeSize( SCUI.name_to_id[ "box_inner" ], sx, sy, sz )
end

function AsyncLoad_SetupInfoLine( text_one, text_two )
	if text_one == nil then
		text_one = UIText.CMN_NOWT
	end
	if text_two == nil then
		text_two = UIText.CMN_NOWT
	end
	
	UIButtons.ChangeText( SCUI.name_to_id[ "info_1" ], text_one )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_2" ], text_two )
end
