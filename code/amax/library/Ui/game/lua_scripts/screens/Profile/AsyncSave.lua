GUI =
{
	finished = false,
	net_global_update_enabled = false,
	first_save = false,
	
	textId = -1,
	
	is_multiplayer = false,
	
	done_info_lines = false,
	
	upload_finished = true,
	local_finished = false,
	
	local_error = 0,
	upload_result = false,
}

function Init( )
	-- [Bone] #37144 - Bloom after changing difficulty
	UIGlobals.AsyncSave_FailMessageActive = nil
	
	AddSCUI_Elements()
	
	GUI.is_multiplayer = Amax.IsGameModeOnline()
	
	local sx, sy, sz = UIButtons.GetSize( SCUI.name_to_id[ "box" ] )
	GUI.default_box_height = sy
	
	UIScreen.SetScreenTimers( 0, 0 )
	
	-- disable network global update so it doesn't force us off this script until we finish the save process
	GUI.net_global_update_enabled = UIGlobals.Network.GlobalUpdateEnabled
	net_EnableGlobalUpdate( false )
	
	-- ***
	GUI.timer = 0
	-- did we start saving ok
	GUI.errorStarting = false
	-- did we finish saving ok
	GUI.errorSaving = false
	-- are we currently saving
	GUI.saving = false
	-- everything is done, but may need to wait to display message for a certain amount of time
	GUI.finishedSaving = false
	-- what to display once we've shown the "saving" message for the TCR time
	GUI.finishedText = UIText.CMN_NOWT
	-- for silly people pulling MUs
	GUI.noStorageDevice = false
	-- enough free space
	GUI.noFreeSpace = false
	-- deleting from a list PS3
	GUI.deleting = false
	-- check overwrites is on and the file exists?
	GUI.fileExists = false
	-- try again if cancelled on PS3
	GUI.userCancelled = false
	-- callback var
	GUI.finishedSuccess = false
	-- ***
	
	print( "save index - ", UIGlobals.ProfileSaveIndex )
end

function PostInit()	
	GUI.textId = SCUI.name_to_id[ "text" ]
	
	GUI.first_save = ( UIScreen.GetCurrentScreen() == "intro\\startscreen.lua" )
	
	if GUI.first_save == false then
		UIButtons.SetActive( SCUI.name_to_id[ "black" ], true )
	end
	
	GUI.bleep_bleep_id = AddTransmitter( 30, 0, 35, 35, UIEnums.Panel._3DAA_1, 5, false, true )
	UIButtons.SetParent( GUI.bleep_bleep_id, SCUI.name_to_id[ "box" ], UIEnums.Justify.MiddleRight )
	
	-- kick off the save
	StartSave()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true or GUI.saving == true then
		return
	end
	
	-- failed to start or failed to save
	if GUI.errorStarting == true or GUI.errorSaving == true then
		if message == UIEnums.Message.MenuBack then
			AsyncSave_FinishedSaving()
		end
		return
	end
	
	-- try again if cancelled
	if GUI.userCancelled == true then
		if message == UIEnums.Message.MenuNext then
			StartSave( true )
			return
			
		elseif message == UIEnums.Message.MenuBack then
			AsyncSave_ChangeText( UIText.CMN_NOWT )
			--SetupInfoLine_F( UIText.CMN_NOWT )
			AsyncSave_SetupInfoLine( UIText.CMN_NOWT )
			AsyncSave_FinishedSaving()
			return
		end
	end
	
	-- confirm overwrite?
	if GUI.fileExists == true then
		if message == UIEnums.Message.MenuNext then
			GoScreen( "Profile\\AsyncSave.lua" )
			
		elseif message == UIEnums.Message.MenuBack then
			-- cancel and continue
			AsyncSave_ChangeText( UIText.CMN_NOWT )
			AsyncSave_SetupInfoLine( UIText.CMN_NOWT )
			AsyncSave_FinishedSaving()
		end
		return
	end
	
	-- have people been daft and pulled their MU?
	if GUI.noStorageDevice == true then
		if message == UIEnums.Message.MenuNext then
			-- choose another storage device
			UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileSaveIndex, 0, UIEnums.XboxDeviceSelector.ForceUI )
		
		elseif message == UIEnums.Message.MenuBack then
			-- flag to never prompt for a save again
			UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = true
			AsyncSave_FinishedSaving()
		
		elseif message == UIEnums.Message.SaveLocationFound then
			-- set the storage device in the profile and reset do not save flag
			Profile.StoreDeviceId( UIGlobals.ProfileSaveIndex )
			UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = false
			-- and try again
			GoScreen( "Profile\\AsyncSave.lua" )
			UIButtons.TimeLineActive( "save_on", false )
		end
		return
	end
	
	if GUI.noFreeSpace == true then
		if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
			if message == UIEnums.Message.MenuNext then
				-- choose another storage device
				UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileSaveIndex, Profile.GetCurrentSaveSize(), UIEnums.XboxDeviceSelector.ForceUI )	
				
			elseif message == UIEnums.Message.MenuBack then
				-- flag to never prompt for a save again
				UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = true
				AsyncSave_FinishedSaving()
				
			elseif message == UIEnums.Message.SaveLocationFound then
				-- set the storage device in the profile and reset do not save flag
				Profile.StoreDeviceId( UIGlobals.ProfileSaveIndex )
				UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = false
				-- and try again
				GoScreen( "Profile\\AsyncSave.lua" )
				UIButtons.TimeLineActive( "save_on", false )
			end
			return
			
		elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
			if message == UIEnums.Message.ButtonX then
				GUI.deleting = true
				Profile.StartAsyncDelete( UIGlobals.ProfileSaveIndex, true )
			
			elseif message == UIEnums.Message.MenuNext then
				-- try again
				GoScreen( "Profile\\AsyncSave.lua" )
			
			elseif message == UIEnums.Message.MenuBack then
				-- flag to never prompt for a save again
				UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = true
				AsyncSave_FinishedSaving()
			end
			return
			
		else
			if message == UIEnums.Message.MenuBack then
				AsyncSave_FinishedSaving()
			end
			return
		end
	end
end

function FrameUpdate( time )
	GUI.timer = GUI.timer + time
	
	-- are we currently saving?
	if GUI.saving == true then
		if GUI.upload_finished == false then
			GUI.upload_result = NetServices.ContinueProfileUpload()
			GUI.upload_finished = ( GUI.upload_result ~= UIEnums.StorageWriterState.InProgress )
		end
		
		-- continue the save
		if GUI.local_finished == false then
			GUI.local_finished, GUI.local_error = Profile.ContinueAsyncSave()
		end
		
		if GUI.local_finished == true and GUI.upload_finished == true then
			print( "FINISHED SAVING" )
			
			-- check the error
			print( "local_error", GUI.local_error, "upload_result", GUI.upload_result )
			
			if GUI.local_error == UIEnums.ProfileError.Okay then	
				GUI.saving = false
				GUI.finishedSaving = true
				GUI.finishedText = UIText.PRO_SAVE_COMPLETE
				GUI.finishedSuccess = true
			
			elseif GUI.local_error == UIEnums.ProfileError.DeviceNotConnected then
				-- sill people pulling MUs
				GUI.saving = false
				GUI.noStorageDevice = true
				AsyncSave_ChangeText( UIText.PRO_DEVICE_REMOVED_SAVE )
				AsyncSave_SetupInfoLine( UIText.INFO_CHOOSE_STORAGE_A, UIText.INFO_CONTINUE_B )
				TransmitterOff( GUI.bleep_bleep_id )
				
			elseif GUI.local_error == UIEnums.ProfileError.NoFreeSpace then
				GUI.saving = false
				GUI.noFreeSpace = true
				AsyncSave_ChangeText( UIText.PRO_NO_SPACE )
				AsyncSave_SetupInfoLine( UIText.INFO_CONTINUE_B )
				TransmitterOff( GUI.bleep_bleep_id )
				
				if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
					AsyncSave_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileSaveIndex .. "_NO_SPACE" )
					AsyncSave_SetupInfoLine( UIText.INFO_CHOOSE_STORAGE_A, UIText.INFO_CONTINUE_B )
					
				elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
					AsyncSave_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileSaveIndex .. "_NO_SPACE_PS3" )
					AsyncSave_SetupInfoLine( UIText.INFO_TRY_AGAIN_A, UIText.INFO_CONTINUE_B, UIText.INFO_FREE_SPACE_X )
				end
				
			elseif GUI.local_error == UIEnums.ProfileError.FileExists then
				GUI.saving = false
				GUI.fileExists = true
				AsyncSave_ChangeText( UIText.PRO_SAVE_FILE_EXISTS )
				AsyncSave_SetupInfoLine( UIText.INFO_YES_A, UIText.INFO_NO_B )
				TransmitterOff( GUI.bleep_bleep_id )
				
			elseif GUI.local_error == UIEnums.ProfileError.UserCancelled then	-- PS3
				if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
					GUI.saving = false
					GUI.userCancelled = true
					AsyncSave_ChangeText( UIText.PRO_SAVE_USER_CANCELLED )
					AsyncSave_SetupInfoLine( UIText.INFO_YES_A, UIText.INFO_NO_B )
					TransmitterOff( GUI.bleep_bleep_id )
				else
					print( "*****" )
					print( "ERROR: UIEnums.ProfileError.UserCancelled - got here not on PS3" )
					print( "*****" )
				end
			
			else
				-- save error
				print( "ERROR saving!" )
				AsyncSave_ChangeText( UIText.PRO_SAVE_ERROR )
				AsyncSave_SetupInfoLine( UIText.INFO_CONTINUE_B )
				GUI.errorSaving = true
				GUI.saving = false
				TransmitterOff( GUI.bleep_bleep_id )
			end	
			
			if GUI.local_error ~= UIEnums.ProfileError.Okay then
				UIButtons.SetActive( SCUI.name_to_id[ "saving_icon" ], false )
			end		
		end	
	end
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		if GUI.deleting == true then
			-- continue the deletion
			local finished, error = Profile.ContinueAsyncDelete()
			
			if finished == true then
				GUI.deleting = false
			end
		end
	end
	
	-- three seconds for the TCR - shouldn't take longer than this on 360
	if GUI.finishedSaving == true and GUI.timer >= 3 then
		TransmitterOff( GUI.bleep_bleep_id )
		--AsyncSave_ChangeText( GUI.finishedText )
		--AsyncSave_SetupInfoLine()
		
		UIGlobals.FileParams.FinishedSuccess = GUI.finishedSuccess
		AsyncSave_FinishedSaving()
		
		print( "save time - ", GUI.timer )
	end
end

function EnterEnd()
	if UIGlobals.AsyncSave_FailMessageActive == true and UIScreen.IsPopupActive() == false then
		UIButtons.TimeLineActive("Popup_Active",false)
		UIGlobals.AsyncSave_FailMessageActive = nil
	end
	
	if GUI.done_info_lines == true then
		RestoreInfoLine_F()
		InfoLineUnlock( true )
	end
	
	UIButtons.TimeLineActive( "show_save_box", false )
end

function EndLoop( time )
	--
end

function End()
	--
end

function AsyncSave_FinishedSaving()
	print( "finished saving" )

	-- Send message
	if IsNumber( UIGlobals.FileParams.FinishedMessage ) == true then
		Amax.SendMessage( UIGlobals.FileParams.FinishedMessage  )
	end
	
	net_EnableGlobalUpdate( GUI.net_global_update_enabled ) 
	
	AsyncSave_Finished()
end

function AsyncSave_Finished()
	if GUI.upload_result ~= UIEnums.StorageWriterState.Successful then
		if UserKickBackActive() == false then
			ReturnToStartScreen( UIEnums.UserKickBackMode.ProfileUploadFailed )
		end
	end
	
	GoScreen( GetStoredScreen( UIEnums.ScreenStorage.SAVE_NEXT ) )
	
	if UIGlobals.FileParams.FinishedSuccess ~= true then
		UIGlobals.FileParams.FinishedSuccess = false
	end

	UIButtons.TimeLineActive( "save_on", false )
end

function StartSave( force_text )
	-- setup the save system
	GUI.upload_finished = true
	GUI.local_finished = false
	GUI.local_error = 0
	GUI.upload_result = UIEnums.StorageWriterState.Successful
	
	UIGlobals.FileParams.SetInterface( false )
	
	if force_text == true then	
		AsyncSave_ChangeText( UIText.PRO_SAVING )
		AsyncSave_SetupInfoLine()	
		TransmitterOn( GUI.bleep_bleep_id )
	else
		UIButtons.SetActive( SCUI.name_to_id[ "saving_icon" ], true )
	end
	
	if Profile.StartAsyncSave( UIGlobals.ProfileSaveIndex ) == false then
		print( "ERROR starting the save thread, bailing out" )
		GUI.errorStarting = true
		AsyncSave_ChangeText( UIText.PRO_SAVE_ERROR )
		AsyncSave_SetupInfoLine( UIText.INFO_CONTINUE_B )
		UIButtons.SetActive( SCUI.name_to_id[ "saving_icon" ], false )
	else
		if GUI.is_multiplayer == true then
			print( "AsyncSave - Uploading Multiplayer Profile" )
			GUI.upload_result = NetServices.StartProfileUpload()
			if GUI.upload_result ~= UIEnums.StorageWriterState.InProgress then	
				GUI.upload_finished = true
			else
				GUI.upload_finished = false
			end		
		end		
		GUI.saving = true
	end
end

function AsyncSave_ChangeText( text )
	UIButtons.ChangeText( GUI.textId, text )
	
	-- [Bone] #37144 - Bloom after changing difficulty
	UIGlobals.AsyncSave_FailMessageActive = true
	UIButtons.TimeLineActive("Popup_Active",true)
	
	-- re-size box
	local box_id = SCUI.name_to_id[ "box" ]
	local text_height = UIButtons.GetStaticTextHeight( GUI.textId )
	local sx, sy, sz = UIButtons.GetSize( box_id )
	
	sy = GUI.default_box_height + text_height
	UIButtons.ChangeSize( box_id, sx, sy, sz )
	UIButtons.ChangeSize( SCUI.name_to_id[ "box_inner" ], sx, sy, sz )
	
	UIScreen.SetScreenTimers( 0, 0.3 )
	UIButtons.TimeLineActive( "show_save_box", true )
	UIScreen.BlockInputToContext( true )
	
	UISystem.PlaySound( UIEnums.SoundEffect.GraphicTextBox )
end

function AsyncSave_SetupInfoLine( text_one, text_two, text_three )
	if text_one == nil then
		text_one = UIText.CMN_NOWT
	end
	if text_two == nil then
		text_two = UIText.CMN_NOWT
	end
	if text_three == nil then
		text_three = UIText.CMN_NOWT
	end
	
	UIButtons.ChangeText( SCUI.name_to_id[ "info_1" ], text_one )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_2" ], text_two )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_3" ], text_three )
	
	if GUI.done_info_lines == false then
		GUI.done_info_lines = true
		
		InfoLineLock()
		StoreInfoLine_F()
		SetupInfoLine_F()
	end
end
