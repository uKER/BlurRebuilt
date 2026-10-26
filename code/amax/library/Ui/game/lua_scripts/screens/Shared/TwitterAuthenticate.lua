GUI =
{
	finished = false,
	carousel_branch = "Options",

	option = 0,

	authenticating = false,

	CanExit = function( _GUI )
		return false
	end,
	
	timeout=0,
}

function Init()
	GUI.old_friends = net_LockoutFriendsOverlay( true )
	
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine()
	
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	
	UIScreen.Suspend( UIEnums.Context.Main )
	UIScreen.Suspend( UIEnums.Context.CarouselApp )
	UIScreen.Suspend( UIEnums.Context.Subscreen0 )
	UIScreen.Suspend( UIEnums.Context.Subscreen1 )
	UIScreen.Suspend( UIEnums.Context.Subscreen2 )
	
	-- switch over the info lines
	InfoLineSwitchContext( UIScreen.Context() )
	SetupMenuOptions()
	
	SetupCustomPopup( UIEnums.CustomPopups.TwitterLegal )
end

function PostInit()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.authenticating == true then
		return
	end

	if GUI.finished == true then
		return
	end

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then 
		PlaySfxBack()

		if UIGlobals.TwitterBackToTweetScreen == false then
			UIScreen.EndScreen()
		else
			GoScreen("Shared\\Twitter.lua", UIEnums.Context.Blurb )
		end
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true then 

		if GUI.option == 0 then
			UIHardware.StartKeyboard( arg1, "GAME_TWITTER_USERNAME", UIText.RBC_TWITTER_KEYB_HEADER, UIText.RBU_TWITTER_KEYB_USERNAME, 256, UIEnums.XboxKeyboardType.Standard )
		elseif GUI.option == 1 then
			UIHardware.StartKeyboard( arg1, UIText.CMN_NOWT, UIText.RBC_TWITTER_KEYB_HEADER, UIText.RBU_TWITTER_KEYB_PASSWORD, 256, UIEnums.XboxKeyboardType.Password )
		elseif GUI.option == 2 then
			GUI.timeout=0
			SetupCustomPopup( UIEnums.CustomPopups.TwitterAuthenticating )
			Amax.TwitterAuthenticate()
			GUI.authenticating=true
		end

	elseif message == UIEnums.Message.ButtonDown and arg2 == true then

		GUI.option = GUI.option + 1
		if GUI.option == 3 then
			GUI.option = 0
		end
		
		PlaySfxDown()

	elseif message == UIEnums.Message.ButtonUp and arg2 == true then

		GUI.option = GUI.option - 1
		if GUI.option == -1 then
			GUI.option = 2
		end
		
		PlaySfxUp()

	elseif message == UIEnums.Message.KeyboardFinished then
		
		if GUI.option == 0 then
			Amax.StoreTwitterUsername()
			UIButtons.ChangeText( SCUI.name_to_id[ "username_text" ], "GAME_TWITTER_USERNAME" )
		elseif GUI.option == 1 then
			Amax.StoreTwitterPassword()
			UIButtons.ChangeText( SCUI.name_to_id[ "password_text" ], "GAME_TWITTER_PASSWORD_ASTERISK" )
		end
	end
end

function FrameUpdate( time )
	if UIGlobals.TwitterClose == true then
		UIGlobals.TwitterClose = false
		StartAsyncSave()
		
		if UIGlobals.TwitterBackToTweetScreen == false then
			UIScreen.EndScreen()
		else
			GoScreen("Shared\\Twitter.lua", UIEnums.Context.Blurb )
		end
	end
	
	if GUI.authenticating == true then

		GUI.timeout=GUI.timeout+time
		
		if GUI.timeout > 2 then
			still_pumping, error = Amax.PumpTwitterCurrentTask()
			
			if GUI.timeout > 59 then
				still_pumping=false
				error=-1
			end
			
			if still_pumping == false then
				GUI.authenticating = false		

				if LSP.IsTwitterConnected() == false or error == -1 then
					if net_CanReconnectToDemonware() == false then
						PopupSpawn( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
					else
						PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
					end
				elseif error ~= 0 then
					Amax.TwitterUnAuthenticate()
					PopupSpawn( UIEnums.CustomPopups.TwitterUserUnknown )
				else
					PopupSpawn( UIEnums.CustomPopups.TwitterAuthenticated )
				end
			end
		end

	else
		if GUI.option == 0 then
			UIButtons.TimeLineActive( "username", true )
			UIButtons.TimeLineActive( "password", false )
			UIButtons.TimeLineActive( "login", false )
		elseif GUI.option == 1 then
			UIButtons.TimeLineActive( "username", false )
			UIButtons.TimeLineActive( "password", true )
			UIButtons.TimeLineActive( "login", false )
		elseif GUI.option == 2 then
			UIButtons.TimeLineActive( "username", false )
			UIButtons.TimeLineActive( "password", false )
			UIButtons.TimeLineActive( "login", true )
		end
	end
end

function Render()
	--
end

function EnterEnd()
	UIButtons.TimeLineActive( "blurb_start", false )
	
	-- switch the info lines back
	InfoLineSwitchContext()	

	UIScreen.Resume( UIEnums.Context.Main )
	UIScreen.Resume( UIEnums.Context.CarouselApp )
	UIScreen.Resume( UIEnums.Context.Subscreen0 )
	UIScreen.Resume( UIEnums.Context.Subscreen1 )
	UIScreen.Resume( UIEnums.Context.Subscreen2 )

	RestoreInfoLine()
end

function EndLoop( time )
end

function End()
	net_LockoutFriendsOverlay( GUI.old_friends )
end
