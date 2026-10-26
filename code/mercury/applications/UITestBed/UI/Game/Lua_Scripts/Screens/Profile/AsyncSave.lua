GUI =
{
	finished = false,
	
	timer = 0,
	
	-- did we start saving ok
	errorStarting = false,
	-- are we currently saving
	saving = false,
	-- everything is done, but may need to wait to display message for a certain amount of time
	finishedSaving = false,
	-- what to display once we've shown the "saving" message for the TCR time
	finishedText = UIText.IDS_NOWT,
	-- for silly people pulling MUs
	noStorageDevice = false,
	-- enough free space - PS3
	noFreeSpace = false,
}

function Init()	
	-- setup the save system
	GameProfile.SetProfileSaveInfoInterface()
	
	-- pad icon for 360 version - might look dodgy if saving out all 4 profiles after split-screen
	local padIcon =
	{
		layer = UIEnums.Layer.TOP,
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos = { x = GUIBank.ScreenRightEdge, y = GUIBank.ScreenTopEdge, },
		size = { x = 40, y = 40, },
		justify = UIEnums.Justify.TOP_RIGHT,
		colour = "!255 255 255 255",
		textures =
		{
			{
				name = UITexture.Textures.xbox360_buttons,
				pos = { u = -1, v = -1, },
				size = { u = 64/512, v = 64/512, },
			},
		},
	}
	
	print( "save index - ", UIGlobals.ProfileSaveIndex )
	
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
	local text =
	{
		layer = UIEnums.Layer.TOP,
		type = UIEnums.ButtonTypes.TEXT,
		pos = { x = 320, y = 240 },
		size = { x = 0, y = 40 },
		justify = UIEnums.Justify.MiddleCentre,
		text_id = UIText.IDS_PROFILE_SAVING,
	}
	GUI.textId = UIButtons.AddButton( text )
	
	-- kick off the save
	if Profile.StartAsyncSave( UIGlobals.ProfileSaveIndex ) == false then
		print( "ERROR starting the save thread, bailing out" )
		GUI.errorStarting = true
	else
		GUI.saving = true
	end
	
	FrameUpdate( 0 )	
end

function StartLoop( time )
	return true
end

function MessageUpdate( message, arg1, arg2 )
	if GUI.finished == true or GUI.saving == true then
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
			-- we're done
			FinishedSaving()
		
		elseif message == UIEnums.Message.SaveLocationFound then
			-- set the storage device in the profile and reset do not save flag
			Profile.StoreDeviceId( UIGlobals.ProfileSaveIndex )
			UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = false
			-- and try again
			GUI.finished = true
			UIScreen.SetNextScreen( UIScreen.GetCurrentScreen() )
		end
	end
	
	if GUI.noFreeSpace == true then
		if message == UIEnums.Message.ButtonB then
			-- flag to never prompt for a save again
			UIGlobals.DoNotSave[ UIGlobals.ProfileSaveIndex ] = true
			-- we're done
			FinishedSaving()
		end
	end
end

function FrameUpdate( time )
	GUI.timer = GUI.timer + time
	
	-- did we fail to start - eek!
	if GUI.errorStarting == true then
		UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_SAVE_ERROR )
		-- we're done
		FinishedSaving()
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
				GUI.finishedText = UIText.IDS_PROFILE_SAVE_COMPLETE
			
			elseif error == UIEnums.ProfileError.DeviceNotConnected then
				-- sill people pulling MUs
				GUI.saving = false
				GUI.noStorageDevice = true
				UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_DEVICE_REMOVED_SAVE )
			
			elseif error == UIEnums.ProfileError.NoFreeSpace then	-- PS3
				GUI.saving = false
				GUI.noFreeSpace = true
				UIButtons.ChangeButtonText( GUI.textId, UIText.IDS_PROFILE_NO_SPACE )
			
			else
				-- something which we don't know about has happened - eek!
				print( "ERROR saving!" )
				GUI.saving = false
				GUI.finishedSaving = true
				GUI.finishedText = UIText.IDS_PROFILE_SAVE_ERROR
			end
		end
	end
	
	-- are we finished?
	local startTimer, endTimer = UIScreen.GetScreenTimers()
	
	-- TODO - change message depending on length of save
	
	-- one second for the TCR
	if GUI.finishedSaving == true and GUI.timer >= ( 1 - startTimer ) then
		UIButtons.ChangeButtonText( GUI.textId, GUI.finishedText )
		-- we're done
		FinishedSaving()
		
		print( "save time - ", GUI.timer )
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end

function FinishedSaving()
	GUI.finished = true
	print( "finished saving" )
	
	--- *****
	-- TODO - set the isSplitScreen flag properly
	local isSplitScreen = false
	--- *****
	
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
			-- reload
			UIScreen.SetNextScreen( UIScreen.GetCurrentScreen() )
		else
			-- we're done
			UIScreen.SetNextScreen( GetStoredScreen( UIEnums.ScreenStorage.SAVE_NEXT ) )
		end
	else
		-- we're done
		UIScreen.SetNextScreen( GetStoredScreen( UIEnums.ScreenStorage.SAVE_NEXT ) )
	end
end
