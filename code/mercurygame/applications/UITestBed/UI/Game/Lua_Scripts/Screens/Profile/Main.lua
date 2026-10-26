GUI =
{
	finished = false,
}

function Init()
	--
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.ButtonA then
		local selection = UIButtons.GetSelection( SCUI.name_to_id[ "menu" ] )
		
		-- test
		StoreScreen( UIEnums.ScreenStorage.FE_NEXT, "Profile\\Main.lua" )
		StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, "Profile\\Main.lua" )
		--
		
		-- clear all cached profiles
		for i = 0, 3, 1 do
			Profile.ClearProfile( i )
			Profile.ReadPadProfile( i )
			GameProfile.ClearGameProfile( i )	-- *** GAME SPECIFIC
		end
		
		UIGlobals.ProfilePrimaryPad = -1		-- clear
		UIGlobals.ProfilesFound = 0				-- reset
		
		-- clear global profile state table
		for i = 0, 3, 1 do
			UIGlobals.ProfileState[ i ] = UIEnums.Profile.None
		end
		
		-- reset global load table
		for i = 0, 3, 1 do
			UIGlobals.LoadProfile[ i ] = false
		end
		
		-- reset to only load in the one profile - the person that presses start
		UIGlobals.ProfileLookingFor = -1
			
		if selection == 0 then
			GoScreen( "Profile\\AsyncLoad.lua" )
		elseif selection == 1 then
			-- *** NOTE : in a proper game you'd wrap this up with CanSave( profileIndex )
			UIGlobals.ProfileSaveToList = false
			GoScreen( "Profile\\AsyncSave.lua" )
		elseif selection == 2 then
			-- *** NOTE : in a proper game you'd wrap this up with CanEnumerate( profileIndex )
			GoScreen( "Profile\\AsyncEnumerate.lua" )
		elseif selection == 3 then
			-- *** NOTE : in a proper game you'd wrap this up with CanSave( profileIndex )
			UIGlobals.ProfileSaveToList = true
			GoScreen( "Profile\\AsyncSave.lua" )
		end
	end
end

function FrameUpdate( time )
	--
end

function EndLoop( time )
	--
end

function End()
	--
end
