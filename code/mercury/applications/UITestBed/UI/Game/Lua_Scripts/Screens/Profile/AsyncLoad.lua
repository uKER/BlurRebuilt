GUI =
{
	finished = false,
	
	-- message text
	textId = -1,
	
	-- only load the person who pressed start?
	onlyLoadOne = false,
	-- have we blagged any of the profiles?
	blaggedAProfile = false,
	-- have we found a valid profile to load?
	foundProfileToLoad = false,
	-- are we done?
	finishedLookingForProfiles = false,
	-- valid profile found to load, so attempt to find a valid device
	checkForDevice = false,
	-- are we currently loading?
	loading = false,
	-- blocking pad messages
	allowPadInput = false,
	-- allowed to change storage devices?
	allowSaveLocationChange = false,
	-- enough free space to save if we try to load and the file doesn't exist?
	enoughFreeSpace = true,
	-- return value of the save system when loading in
	profileReturnCode = UIEnums.ProfileError.Okay,
	-- allow to overwrite a corrupt profile?
	allowProfileOverwrite = false,
	-- are we currently deleting?
	deleting = false,
	-- did the deletion bork up?
	errorWhenDeleting = false
}

function Init()
	-- setup the save system
	GameProfile.SetProfileSaveInfoInterface()
	
	-- no more changes please, or we'll kick you
	Profile.AllowProfileChanges( false )
	
	-- message text
	local text =
	{
		layer = UIEnums.Layer.TOP,
		type = UIEnums.ButtonTypes.TEXT,
		pos = { x = 320, y = 240 },
		size = { x = 640, y = 40 },
		justify = UIEnums.Justify.MiddleCentre,
		text_id = UIText.IDS_NOWT,
	}
	GUI.textId = UIButtons.AddButton( text )
	
	-- only need to load in the person who pressed start?
	if UIGlobals.ProfileLookingFor == -1 then
		UIGlobals.ProfileLookingFor = 0
		GUI.onlyLoadOne = true
	end
	
	print( "GUI.onlyLoadOne", GUI.onlyLoadOne )
	
	-- sort out the kit profiles
	for i = UIGlobals.ProfileLookingFor, 3, 1 do
		-- if we're loading in them "all", they'll already be flagged - so just load in those ones
		-- if we're only loading in one however we also need to check for PreLoad profiles
		if GUI.onlyLoadOne == true or ( GUI.onlyLoadOne == false and UIGlobals.LoadProfile[ i ] == true ) then
			UIGlobals.ProfileLookingFor = i
			print( "UIGlobals.ProfileLookingFor", UIGlobals.ProfileLookingFor )
			
			if UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged then
				print( "profile ", i, " already blagged - continuing" )
				UIGlobals.ProfilesFound = UIGlobals.ProfilesFound + 1
				GUI.blaggedAProfile = true
			else
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
					-- ... unless we only wanted to load in one
					if GUI.onlyLoadOne == true and i ~= UIGlobals.ProfilePressedStart then
						print( "PreLoad on slot " .. i )
						UIGlobals.ProfileState[ i ] = UIEnums.Profile.PreLoad
					end
					
					-- have we found a valid profile to load?
					if UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile then
						GUI.foundProfileToLoad = true
					end
					
					-- if we're attempting to load them "all" in and we've got a valid profile - then we're done for now
					if GUI.onlyLoadOne == false and GUI.foundProfileToLoad == true then
						break
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
		end
	end
	
	-- if we've not found any new profiles then we're done
	if GUI.foundProfileToLoad == false and GUI.blaggedAProfile == false then
		GUI.finishedLookingForProfiles = true
	end
	
	-- if we're only loading in one then reset this as the PreLoad profiles are sorted now
	if GUI.onlyLoadOne == true then
		UIGlobals.ProfileLookingFor = UIGlobals.ProfilePressedStart
	end
	
	-- attempt to start a load by checking for a valid storage device
	GUI.checkForDevice = true
end

function StartLoop( time )
	return true
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
		UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_NO_STORAGE_DEVICE )
		return
	end
	
	if message == UIEnums.Message.SaveLocationFound then
		GUI.allowPadInput = false
		GUI.allowSaveLocationChange = false
		GUI.enoughFreeSpace = true
		UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
		UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_LOADING )
		
		-- start the async load
		GUI.loading = true
		Profile.StartAsyncLoad( UIGlobals.ProfileLookingFor )
		return
	end
	
	if GUI.allowPadInput == false then
		return
	end
	
	if message == UIEnums.Message.ButtonA then
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
			if GUI.onlyLoadOne == false then
				-- try the next profile
				GUI.finished = true
				UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
				UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
				UIGlobals.ProfileLookingFor = UIGlobals.ProfileLookingFor + 1
				UIScreen.SetNextScreen( UIScreen.GetCurrentScreen() )
			else
				-- we're done
				Finished()
			end
			
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
				UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileLookingFor, UIGlobals.ProfileSize, UIEnums.XboxDeviceSelector.ForceUI )
				return
			else
				-- save out
				if GUI.onlyLoadOne == false then
					-- try the next profile
					UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
					UIGlobals.ProfileLookingFor = UIGlobals.ProfileLookingFor + 1
					StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, UIScreen.GetCurrentScreen() )
				else
					StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, GetStoredScreen( UIEnums.ScreenStorage.FE_NEXT ) )
				end
				
				UIGlobals.ProfileSaveIndex = UIGlobals.ProfileLookingFor
				UIScreen.SetNextScreen( "Profile\\AsyncSave.lua" )
			end
		else
			if GUI.onlyLoadOne == false then
				-- try the next profile
				UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
				UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
				UIGlobals.ProfileLookingFor = UIGlobals.ProfileLookingFor + 1
				UIScreen.SetNextScreen( UIScreen.GetCurrentScreen() )
			else
				-- we're done
				Finished()
			end
		end
	
	elseif message == UIEnums.Message.ButtonB then
		-- cancelling when told we've not chosen a storage device or we don't have enough space to save-out if the file doesn't exist
		if GUI.allowSaveLocationChange == true or GUI.enoughFreeSpace == false then
			if GUI.onlyLoadOne == false then
				-- try the next profile
				GUI.finished = true
				UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
				UIGlobals.ProfileLookingFor = UIGlobals.ProfileLookingFor + 1
				UIScreen.SetNextScreen( UIScreen.GetCurrentScreen() )
			else
				-- we're done
				Finished()
			end
			
			return
		end
		
		if GUI.allowProfileOverwrite == true or GUI.errorWhenDeleting == true then
			-- they now can't save
			UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
			UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
			
			if GUI.onlyLoadOne == false then
				-- try the next profile
				GUI.finished = true				
				UIGlobals.ProfileLookingFor = UIGlobals.ProfileLookingFor + 1
				UIScreen.SetNextScreen( UIScreen.GetCurrentScreen() )
			else
				-- we're done
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
		-- continue the load
		local finished, error = Profile.ContinueAsyncLoad()
		
		if finished == true then
			-- store the error code
			GUI.profileReturnCode = error
			
			GUI.loading = false
			GUI.allowPadInput = true
			
			print( "FINISHED LOAD" )

			-- now check the error code
			
			if error == UIEnums.ProfileError.Okay then
				-- we've loaded fine
				UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
				UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
				UIButtons.ChangeButtonText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_LOADED" )
				
			elseif error == UIEnums.ProfileError.FileDoesNotExist then
				-- check free space here
				local space, ok = UIHardware.SpaceLeftOnDevice()
				
				if UIEnums.CurrentPlatform ~= UIEnums.Platform.Xenon then
					-- we check for free space elsewhere
					space = UIGlobals.ProfileSize
					ok = true
				end
				
				if ok == false then
					-- pulled MU
					GUI.profileReturnCode = UIEnums.ProfileError.DeviceNotConnected
					GUI.allowSaveLocationChange = true
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
					UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_DEVICE_REMOVED_LOAD )
					
				elseif space < UIGlobals.ProfileSize then
					GUI.enoughFreeSpace = false
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
					UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_NO_SPACE )
					
				else
					-- we have some space - ready to save out new profile
					UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = false
					Profile.Setup( UIGlobals.ProfileLookingFor )
					GameProfile.ClearGameProfile( UIGlobals.ProfileLookingFor )
					UIButtons.ChangeButtonText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_CREATED" )					
				end
				
			elseif error == UIEnums.ProfileError.CannotReadFile then
				-- "profile out of date" (debug) or corrupt -- just display a corrupt message
				GUI.allowProfileOverwrite = true
				UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
				UIButtons.ChangeButtonText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_CORRUPT" )
				
			elseif error == UIEnums.ProfileError.DeviceNotConnected then
				GUI.allowSaveLocationChange = true
				UIGlobals.DoNotSave[ UIGlobals.ProfileLookingFor ] = true
				UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_DEVICE_REMOVED_LOAD )
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
				
				if GUI.onlyLoadOne == false then
					-- try next profile after save this one out
					GUI.finished = true
					UIGlobals.ProfileLookingFor = UIGlobals.ProfileLookingFor + 1
					StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, UIScreen.GetCurrentScreen() )
				else
					-- we're done
					StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, GetStoredScreen( UIEnums.ScreenStorage.FE_NEXT ) )
					Finished()
				end
				
				-- save out
				UIGlobals.ProfileSaveIndex = UIGlobals.ProfileLookingFor
				UIScreen.SetNextScreen( "Profile\\AsyncSave.lua" )
				
			elseif error == UIEnums.ProfileError.CannotDeleteFile then
				-- eek - manual deletion required
				GUI.errorWhenDeleting = true
				UIButtons.ChangeButtonText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_DELETE_FAILED" )
			end
			
			-- else on PS3 the user cancelled the delete request - so stay in the same state
		end
	end
	
	if GUI.finishedLookingForProfiles == true then
		-- we're done
		Finished()
	end

	if GUI.blaggedAProfile == true then
		-- if we've blagged a profile - then they can't load/save
		GUI.allowPadInput = true
		UIButtons.ChangeButtonText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileLookingFor .. "_BLAGGED" )
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	Profile.AllowAllPadInput( false )
end

function Finished()
	print( "finished" )
	GUI.finished = true
	UIGlobals.LoadProfile[ UIGlobals.ProfileLookingFor ] = false
	UIScreen.SetNextScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_NEXT ) )
end
