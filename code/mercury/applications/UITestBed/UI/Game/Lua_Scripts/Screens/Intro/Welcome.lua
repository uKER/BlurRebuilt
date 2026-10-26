GUI =
{
	finished = false,
}

function Init()
	-- TEMP - you shouldn't do this in a proper game
	Profile.SetPrimaryPad( 0 )
	
	-- clear all cached profiles
	for i = 0, 3, 1 do
		Profile.ClearProfile( i )
		Profile.ReadPadProfile( i )
		GameProfile.ClearGameProfile( i )
	end
		
	-- discard any game invites until we've loaded the profiles
	Profile.SetGameInviteMode( UIEnums.AcceptGameInviteMode.Delay )
	
	-- allow changes to the profile here because we've not loaded any, so it makes no odds.
	-- also we get chucked back here if we change mid-game.
	Profile.AllowProfileChanges( true )
	Profile.AllowAllPadInput( true )
	
	if UIGlobals.ShowUserChangedPopup == true then
		-- TODO - show the "changed users" popup message if it's needed
	end
end

function StartLoop( time )
	return true
end

function MessageUpdate( message, arg1, arg2 )
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.ButtonA or message == UIEnums.Message.ButtonStart then
		GUI.finished = true
		
		-- clear all cached profiles - again - for if we've signed in on this screen
		for i = 0, 3, 1 do
			Profile.ClearProfile( i )
			Profile.ReadPadProfile( i )
			GameProfile.ClearGameProfile( i )
		end
		
		UIGlobals.ProfilePressedStart = arg1
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
		
		-- continue		
		StoreScreen( UIEnums.ScreenStorage.FE_BACK, UIScreen.GetCurrentScreen() )
		StoreScreen( UIEnums.ScreenStorage.FE_NEXT, "Intro\\MainMenu.lua" )
		
		UIScreen.SetNextScreen( "Profile\\AsyncLoad.lua" )
	end
end

function FrameUpdate( time )
	--
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
