GUI =
{
	finished = false,
}
	
function Init()
	UIScreen.SetScreenTimers( 0, 0.5 )
	
	-- pad icon for 360 version - might look dodgy if saving out all 4 profiles after split-screen
	local padIcon =
	{
		layer = UIEnums.Layer.TOP,
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos = { x = Screen.safe.right, y = Screen.safe.top, },
		size = { x = 40, y = 40, },
		justify = UIEnums.Justify.TOP_RIGHT,
		colour = "255 255 255 255",
		textures =
		{
			{
				name = UITexture.Textures.xbox360_buttons,
				pos = { u = -1, v = -1, },
				size = { u = 64/512, v = 64/512, },
			},
		},
	}
	
	local padIndex = UIGlobals.ProfileSaveIndex
	
	if padIndex == 0 then
		padIcon.textures[ 1 ].pos.u = 384/512
		padIcon.textures[ 1 ].pos.v = 384/512
	elseif padIndex == 1 then
		padIcon.textures[ 1 ].pos.u = 0
		padIcon.textures[ 1 ].pos.v = 448/512
	elseif padIndex == 2 then
		padIcon.textures[ 1 ].pos.u = 64/512
		padIcon.textures[ 1 ].pos.v = 448/512
	elseif padIndex == 3 then
		padIcon.textures[ 1 ].pos.u = 448/512
		padIcon.textures[ 1 ].pos.v = 384/512
	end
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		UIButtons.AddButton( padIcon )
	end
	
	-- message text
	GUI.textId = SCUI.name_to_id[ "text" ]
	
	-- ***
	GUI.timer = 0
	-- did we start saving ok
	GUI.errorStarting = false
	-- are we currently saving
	GUI.saving = false
	-- everything is done, but may need to wait to display message for a certain amount of time
	GUI.finishedSaving = false
	-- what to display once we've shown the "saving" message for the TCR time
	GUI.finishedText = UIText.CMN_NOWT
	-- for silly people pulling MUs
	GUI.noStorageDevice = false
	-- enough free space - PS3
	GUI.noFreeSpace = false
	-- deleting from a list PS3
	GUI.deleting = false
	-- check overwrites is on and the file exists?
	GUI.fileExists = false
	-- try again if cancelled on PS3
	GUI.userCancelled = false
	-- ***
	
	-- setup the save system
	if UIGlobals.ProfileSaveToList == true and UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		GameProfile.SetProfileSaveInfoInterfaceEnumerate() -- *** GAME SPECIFIC
	else
		GameProfile.SetProfileSaveInfoInterface( UIGlobals.ProfileCheckOverwrites )	-- *** GAME SPECIFIC
	end
	
	print( "save index - ", UIGlobals.ProfileSaveIndex )
	
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
	
	-- try again if cancelled
	if GUI.userCancelled == true then
		if message == UIEnums.Message.ButtonA then
			StartSave()
			return
			
		elseif message == UIEnums.Message.ButtonB then
			UIButtons.ChangeText( GUI.textId, UIText.CMN_NOWT )
			SetupInfoLine( UIText.CMN_NOWT )
			FinishedSaving()
		end
	end
	
	-- confirm overwrite?
	if GUI.fileExists == true then
		if message == UIEnums.Message.ButtonA then
			-- *** GAME SPECIFIC
			UIGlobals.ProfileCheckOverwrites = false
			GoScreen( "Profile\\AsyncSave.lua" )
			
		elseif message == UIEnums.Message.ButtonB then
			-- cancel and continue
			UIButtons.ChangeText( GUI.textId, UIText.CMN_NOWT )
			SetupInfoLine( UIText.CMN_NOWT )
			FinishedSaving()
		end
		
		return
	end
	
	-- have people been daft and pulled their MU?
	if GUI.noStorageDevice == true then
		if message == UIEnums.Message.ButtonA then
			-- choose another storage device
			UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileSaveIndex, UIGlobals.ProfileSize, UIEnums.XboxDeviceSelector.ForceUI )
		
		elseif message == UIEnums.Message.ButtonB then
			-- flag to never prompt for a save again
			UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = true
			FinishedSaving()
		
		elseif message == UIEnums.Message.SaveLocationFound then
			-- set the storage device in the profile and reset do not save flag
			Profile.StoreDeviceId( UIGlobals.ProfileSaveIndex )
			UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = false
			-- and try again
			GoScreen( "Profile\\AsyncSave.lua" )
		end
	end
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		if GUI.noFreeSpace == true then
			if message == UIEnums.Message.ButtonB then
				-- flag to never prompt for a save again
				UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = true
				FinishedSaving()
				
			elseif message == UIEnums.Message.ButtonX then
				GUI.deleting = true
				Profile.StartAsyncDelete( UIGlobals.ProfileLookingFor, true )
			
			elseif message == UIEnums.Message.ButtonA then
				-- try again
				GoScreen( "Profile\\AsyncSave.lua" )
			end
		end
	end
end

FrameUpdate = function( time )
	GUI.timer = GUI.timer + time
	
	-- did we fail to start - eek!
	if GUI.errorStarting == true then
		UIButtons.ChangeText( GUI.textId, UIText.PRO_SAVE_ERROR )
		FinishedSaving()
		SetupInfoLine( UIText.CMN_NOWT )
	end
	
	-- are we currently saving?
	if GUI.saving == true then
		-- continue the save
		local finished, error = Profile.ContinueAsyncSave()
		
		if finished == true then				
			-- check the error
			if error == UIEnums.ProfileError.Okay then
				GUI.saving = false
				GUI.finishedSaving = true
				GUI.finishedText = UIText.PRO_SAVE_COMPLETE
			
			elseif error == UIEnums.ProfileError.DeviceNotConnected then
				-- sill people pulling MUs
				GUI.saving = false
				GUI.noStorageDevice = true
				UIButtons.ChangeText( GUI.textId, UIText.PRO_DEVICE_REMOVED_SAVE )
				SetupInfoLine( UIText.INF_CHOOSE_STORAGE_A, UIText.INF_CONTINUE_B )
				
			elseif error == UIEnums.ProfileError.NoFreeSpace then	-- PS3
				if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
					GUI.saving = false
					GUI.noFreeSpace = true
					UIButtons.ChangeText( GUI.textId, UIText.PRO_NO_SPACE )
					SetupInfoLine( UIText.INF_CONTINUE_B, UIText.INF_TRY_AGAIN_A, UIText.INF_FREE_SPACE_X )
				else
					print( "*****" )
					print( "ERROR: UIEnums.ProfileError.NoFreeSpace - got here not on PS3" )
					print( "THIS IS A TRC FAIL" )
					print( "*****" )
				end
				
			elseif error == UIEnums.ProfileError.FileExists then
				GUI.saving = false
				GUI.fileExists = true
				UIButtons.ChangeText( GUI.textId, UIText.PRO_SAVE_FILE_EXISTS )
				SetupInfoLine( UIText.INF_YES_A, UIText.INF_NO_B )
				
			elseif error == UIEnums.ProfileError.UserCancelled then	-- PS3
				if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
					GUI.saving = false
					GUI.userCancelled = true
					UIButtons.ChangeText( GUI.textId, UIText.PRO_SAVE_USER_CANCELLED )
					SetupInfoLine( UIText.INF_YES_A, UIText.INF_NO_B )
				else
					print( "*****" )
					print( "ERROR: UIEnums.ProfileError.UserCancelled - got here not on PS3" )
					print( "*****" )
				end
			else
				-- something which we don't know about has happened - eek!
				print( "ERROR saving!" )
				GUI.saving = false
				GUI.finishedSaving = true
				GUI.finishedText = UIText.PRO_SAVE_ERROR
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
	
	-- are we finished?
	local startTimer, endTimer = UIScreen.GetScreenTimers()
	
	-- wait at least three seconds for the TCR
	if GUI.finishedSaving == true and GUI.timer >= ( 3 - startTimer ) then
		UIButtons.ChangeText( GUI.textId, GUI.finishedText )
		-- we're done
		SetupInfoLine( UIText.CMN_NOWT )
		FinishedSaving()
		
		print( "save time - ", GUI.timer )
	end
end

function EndLoop( time )
	--
end

function End()
	--
end

function FinishedSaving()
	print( "finished saving" )
	
	-- reset to default
	UIGlobals.ProfileCheckOverwrites = false
	
	local isSplitScreen = false	-- *** GAME SPECIFIC
	
	-- in split-screen we can possible save out all of the profiles
	if isSplitScreen == true then
		-- scan the rest to find any that can save
		local oneCanSave = false
		
		for i = UIGlobals.ProfileSaveIndex + 1, 3, 1 do
			if CanSave( i ) == true then
				oneCanSave = true
				break
			end
		end
		
		if oneCanSave == true then
			GoScreen( "Profile\\AsyncSave.lua" )
		else
			GoScreen( GetStoredScreen( UIEnums.ScreenStorage.SAVE_NEXT ) )
		end
	else
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.SAVE_NEXT ) )
	end
end

function StartSave()
	if Profile.StartAsyncSave( UIGlobals.ProfileSaveIndex ) == false then
		print( "ERROR starting the save thread, bailing out" )
		GUI.errorStarting = true
	else
		GUI.saving = true
	end
	
	SetupInfoLine()
	
	UIButtons.ChangeText( GUI.textId, UIText.PRO_SAVING )
end
