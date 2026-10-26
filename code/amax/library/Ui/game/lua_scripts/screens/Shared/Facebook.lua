GUI =
{
	finished = false,	
	carousel_branch = "Facebook",

	save_on_exit = false,

	stringNo = 0,

	sending_story = false,
	sending_story_timer = 0,

	num_stories = 0,

	ids = {  },

	refresh_screen = false,
}

function Init()
	GUI.old_friends = net_LockoutFriendsOverlay( true )
	
	StoreInfoLine()
	SetupInfoLine()
	
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	AddSCUI_Elements()
	
	UIScreen.Suspend( UIEnums.Context.Main )
	UIScreen.Suspend( UIEnums.Context.CarouselApp )
	UIScreen.Suspend( UIEnums.Context.Subscreen0 )
	UIScreen.Suspend( UIEnums.Context.Subscreen1 )
	UIScreen.Suspend( UIEnums.Context.Subscreen2 )
	
	-- switch over the info lines
	InfoLineSwitchContext( UIScreen.Context() )
	SetupInfoLine( UIText.INFO_FACEBOOK_SEND_A, UIText.INFO_B_BACK, UIText.INFO_FACEBOOK_EDIT_X )
	
	Amax.ClearBlurbMessage()
end

function PostInit()
	Facebook_AddMessageNode()
	RefreshScreen()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.sending_story == true then
		return
	end

	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.ButtonX then
		-- edit
		UIHardware.StartKeyboard( arg1, "GAME_FACEBOOK_MSG" .. 0, UIText.RBC_FACEBOOK_KEYB_HEADER, UIText.RBC_FACEBOOK_KEYB_SUB_HEADER, 160, UIEnums.XboxKeyboardType.Standard )

	elseif message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		EndScreen()

	elseif message == UIEnums.Message.MenuNext then
		-- send
		local allowOnline = true
		if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
			allowOnline = not NetServices.UserHasAgeRestriction()
		end
		
		if allowOnline == true then
			if GUI.num_stories > 0 and Amax.GetFacebookTimeout() == 0 then
				SetupCustomPopup( UIEnums.CustomPopups.FacebookPost )
			end
		else
			SetupCustomPopup( UIEnums.CustomPopups.FailedAgeCheck )
		end

	elseif message == UIEnums.Message.KeyboardFinished then
		Amax.AddBlurbMessage()
		GUI.save_on_exit = true
		RefreshScreen()
		
	end
end

function FrameUpdate( time )
	if UIGlobals.FacebookClose == true then
		UIGlobals.FacebookClose = false
		EndScreen()
	end
	
	if UIGlobals.SendFacebookStory == true then
		GUI.sending_story = true
		GUI.sending_story_timer = 0

		UIGlobals.SendFacebookStory=false
	end
	
	if GUI.refresh_screen == true then
		RefreshScreen()
		GUI.refresh_screen = false
	end

	if GUI.sending_story == true then
		GUI.sending_story_timer = GUI.sending_story_timer + time
		
		if GUI.sending_story_timer > 2 then
			still_pumping, error = Amax.PumpFacebookCurrentTask()

			if still_pumping == false then
				GUI.sending_story = false
				print( "Error ", error )
				
				if error ~= 0 then
					if error == 3601 or error == 3602 then
						UIGlobals.AuthenticateFromPhoto = false
						PopupSpawn( UIEnums.CustomPopups.FacebookPostError )
					elseif error == 3607 then
						UIGlobals.AuthenticateFromPhoto = false
						PopupSpawn( UIEnums.CustomPopups.FacebookConnectionError )
					else
						if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
							SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
						else
							SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
						end
					end
				else
					PopupSpawn( UIEnums.CustomPopups.FacebookPosted )
					UIGlobals.FacebookFailures = 0
					Amax.ClearFacebookStory( UIGlobals.CurrentFacebookStory )
					Amax.CheckStickerProgress( UIEnums.StickerType.ShareFirstExperience )
				end

				RefreshScreen()
			end
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

function RefreshScreen()
	GUI.num_stories = 1
	
	if UIGlobals.CurrentFacebookStory ~= -1 then
		UIGlobals.CurrentFacebookStory = Clamp( UIGlobals.CurrentFacebookStory, 0, GUI.num_stories - 1 )
	end
	
	if GUI.num_stories > 0 then
		UIButtons.SetActive( SCUI.name_to_id[ "facebook_legal" ], true )
	else
		UIButtons.SetActive( SCUI.name_to_id[ "facebook_legal" ], false )
		UIButtons.TimeLineActive( "no_stories", true, 0 )
	end
	
	-- text
	UIButtons.ChangeText( GUI.ids[ 1 ].message, "GAME_FACEBOOK_MSG" .. 0 )
	UIButtons.ChangeText( GUI.ids[ 1 ].story_title, "GAME_FACEBOOK_STORY_NAME" .. 0 )
	UIButtons.ChangeText( GUI.ids[ 1 ].story_caption, "GAME_FACEBOOK_STORY_CAPTION" .. 0 )
	UIButtons.ChangeText( GUI.ids[ 1 ].story_description, "GAME_FACEBOOK_STORY_DESCRIPTION" .. 0 )
	
	-- box sizes
	local text_height = UIButtons.GetStaticTextHeight( GUI.ids[ 1 ].message )
	
	if IsNumber( text_height ) == true then
	
		text_height = text_height + 1.25
		local sx, sy, sz = UIButtons.GetSize( GUI.ids[ 1 ].message_box )

		if text_height > sy then
			UIButtons.ChangeSize( GUI.ids[ 1 ].message_box, sx, text_height, sz )
			UIButtons.ChangeSize( GUI.ids[ 1 ].message_box2, sx - 0.25, text_height - 0.25, sz )
		end
	end
		

end

function SendStory()
	Amax.PublishFacebookStory( UIGlobals.CurrentFacebookStory )
end

function DeleteStory()
	caro_GUI.save_on_exit = true
	Amax.ClearFacebookStory( UIGlobals.CurrentFacebookStory )
	caro_GUI.refresh_screen = true
end

function Facebook_AddMessageNode( )
	GUI.ids[ #GUI.ids + 1 ] = 
	{
		message				= SCUI.name_to_id[ "message" ],
		story_image			= SCUI.name_to_id[ "story_image" ],
		story_title			= SCUI.name_to_id[ "story_title" ],
		story_caption		= SCUI.name_to_id[ "story_caption" ],
		story_description	= SCUI.name_to_id[ "story_description" ],
		message_box			= SCUI.name_to_id[ "message_box" ],
		message_box2		= SCUI.name_to_id[ "message_box2" ],
	}
end
