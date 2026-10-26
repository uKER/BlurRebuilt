GUI =
{
	finished = false,
	-- message text
	textId = -1,
}
	
function Init()
	-- message text
	GUI.textId = SCUI.name_to_id[ "text" ]
	-- menu list of files
	GUI.menuId = SCUI.name_to_id[ "menu" ]
	
	-- ***
	-- are we currently enumerating?
	GUI.enumerating = false
	-- return value of the save system when loading in
	GUI.profileReturnCode = UIEnums.ProfileError.Okay
	-- allowed to change storage devices?
	GUI.allowSaveLocationChange = false
	-- did the enumeration bork up?
	GUI.errorWhenEnumerating = false
	-- allow to load a file after enumeration?
	GUI.allowLoad = false
	-- are we currently loading?
	GUI.loading = false
	-- data corrupt after attempting a load?
	GUI.corruptData = false
	-- try again if cancelled on PS3
	GUI.userCancelled = false
	-- ***
	
	-- start the enumeration
	StartEnumeration()
end

function StartLoop( time )
	--
end
	
function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true or GUI.enumerating == true or GUI.loading == true then
		return
	end
	
	-- player in-specific messages
	if message == UIEnums.Message.SaveLocationNotFound then
		GUI.allowSaveLocationChange = true
		UIButtons.ChangeText( GUI.textId, UIText.PRO_NO_STORAGE_DEVICE )
		return
	end
	
	if message == UIEnums.Message.SaveLocationFound then
		GUI.allowSaveLocationChange = false
		-- start the enumeration
		StartEnumeration()
		return
	end
	
	-- try again if cancelled
	if GUI.userCancelled == true then
		if message == UIEnums.Message.ButtonA then
			StartEnumeration()
			return
		elseif message == UIEnums.Message.ButtonB then
			Finished()
		end
	end
	
	if GUI.errorWhenEnumerating == true then
		if message == UIEnums.Message.ButtonA then
			GoScreen( "Profile\\AsyncEnumerate.lua" )
		elseif message == UIEnums.Message.ButtonB then
			Finished()
		end
		
		return
	end
	
	if GUI.allowSaveLocationChange == true then
		if message == UIEnums.Message.ButtonA then
			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
				UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileEnumerateIndex, 0, UIEnums.XboxDeviceSelector.ForceUI )
			end
		elseif message == UIEnums.Message.ButtonB then
			Finished()
		end
		
		return
	end
	
	if GUI.allowLoad == true then
		if message == UIEnums.Message.ButtonA then
			StartEnumeration( UIButtons.GetSelection( GUI.menuId ) )
			return
		elseif message == UIEnums.Message.ButtonB then
			Finished()
		end
		
		return
	end
	
	if GUI.corruptData == true then
		if message == UIEnums.Message.ButtonA then
			GoScreen( "Profile\\AsyncEnumerate.lua" )
		elseif message == UIEnums.Message.ButtonB then
			Finished()
		end
		
		-- TODO - allow to delete the corrupt file?
		
		return
	end
	
	if message == UIEnums.Message.ButtonA then
		Finished()
	end
end
	
function FrameUpdate( time )
	if GUI.enumerating == true or GUI.loading == true then
		-- continue the enumeration
		local finished, error = Profile.ContinueAsyncLoad()
		
		if finished == true then
			-- now check the error code
			if error == UIEnums.ProfileError.Okay then
				if GUI.enumerating == true then
					-- fill list
					for i = 1, Profile.GetEnumerationCount(), 1 do
						UIButtons.AddItem( GUI.menuId, i, "PROFILE_SAVE_ENUMERATION" .. i, false )
					end
					
					GUI.allowLoad = true
					UIButtons.ChangeText( GUI.textId, UIText.CMN_NOWT )
					SetupInfoLine( UIText.INF_LOAD_A, UIText.INF_BACK_B )
				else
					-- we've loaded fine
					UIButtons.ChangeText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileEnumerateIndex .. "_ENUMERATION_LOADED" )
					SetupNext()
				end
			
			elseif error == UIEnums.ProfileError.DeviceNotConnected then
				GUI.allowSaveLocationChange = true
				SetupInfoLine( UIText.INF_CHOOSE_STORAGE_A, UIText.INF_CONTINUE_B )
				
				if GUI.enumerating == true then
					UIButtons.ChangeText( GUI.textId, UIText.PRO_DEVICE_REMOVED_ENUMERATE )
				else
					UIButtons.ChangeText( GUI.textId, UIText.PRO_DEVICE_REMOVED_LOAD )
				end				
				
			elseif GUI.loading == true and error == UIEnums.ProfileError.CannotReadFile then
				-- "profile out of date" (debug) or corrupt -- just display a corrupt message
				GUI.corruptData = true
				UIButtons.ChangeText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileEnumerateIndex .. "_ENUMERATION_CORRUPT" )
				SetupInfoLine( UIText.INF_TRY_AGAIN_A, UIText.INF_CONTINUE_B )
				
			elseif error == UIEnums.ProfileError.UserCancelled then	-- PS3				
				if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
					GUI.userCancelled = true
					UIButtons.ChangeText( GUI.textId, UIText.PRO_LOAD_USER_CANCELLED )
					SetupInfoLine( UIText.INF_YES_A, UIText.INF_NO_B )
				else
					print( "*****" )
					print( "ERROR: UIEnums.ProfileError.UserCancelled - got here not on PS3" )
					print( "*****" )
				end
			
			else
				GUI.errorWhenEnumerating = true
				SetupInfoLine( UIText.INF_TRY_AGAIN_A, UIText.INF_CONTINUE_B )
				
				if GUI.enumerating == true then				
					UIButtons.ChangeText( GUI.textId, "PROFILE_PAD" .. UIGlobals.ProfileEnumerateIndex .. "_ENUMERATION_FAILED" )
				else
					print( "ERROR: load return code not handled! How did we get here then?" )
					UIButtons.ChangeText( GUI.textId, UIText.CMN_NOWT )
				end
			end
			
			if GUI.enumerating == true then
				GUI.enumerating = false
				print( "FINISHED ENUMERATION" )
			else
				GUI.allowLoad = false
				GUI.loading = false
				print( "FINISHED LOAD" )
			end
		end
	end
end

function EndLoop( time )
	--
end

function End()
	--
end

function StartEnumeration( enumerationIndex )
	-- setup the save system
	GameProfile.SetProfileSaveInfoInterfaceEnumerate( enumerationIndex )	-- *** GAME SPECIFIC
	Profile.StartAsyncLoad( UIGlobals.ProfileEnumerateIndex )
	UIButtons.ClearItems( GUI.menuId )
	
	if enumerationIndex == nil and UIEnums.CurrentPlatform ~= UIEnums.Platform.PS3 then
		print( "enumerate" )
		UIButtons.ChangeText( GUI.textId, UIText.PRO_ENUMERATING )
		GUI.enumerating = true
	else
		print( "load" )
		UIButtons.ChangeText( GUI.textId, UIText.PRO_LOADING )
		GUI.loading = true
	end
	
	SetupInfoLine()
end

function Finished()
	GoScreen( GetStoredScreen( UIEnums.ScreenStorage.SAVE_NEXT ) )
end
