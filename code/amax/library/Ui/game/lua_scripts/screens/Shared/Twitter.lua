GUI =
{
	finished = false,	
	carousel_branch = "Twitter",

	new_tweet = false,

	tweeting = false,
	tweeting_timer = 0,

	num_tweets = 0,
	
	ids = {  },

	CanExit = function( _GUI )
		return false
	end,

	refresh_screen=false,
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
	SetupInfoLine( UIText.INFO_TWITTER_SEND_A, UIText.INFO_B_BACK, UIText.INFO_TWITTER_EDIT_X )
	
	-- store this lot for when adding tweets
	local box = SCUI.elements[ SCUI.name_to_index[ "box" ] ]
	GUI.default_box_height = box.size.y
	GUI.default_box_width = box.size.x
	
	local message = SCUI.elements[ SCUI.name_to_index[ "message" ] ]
	GUI.font_style = message.styles.name
	GUI.font_height = message.size.y
	GUI.font_width = message.size.x
	
	Amax.ClearBlurbMessage()
end

function PostInit()
	Twitter_AddMessageNode()
	
	RefreshScreen()
	Twitter_DoNoTweetsBox()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.tweeting == true then
		return
	end

	if GUI.finished == true then
		return
	end

	if message == UIEnums.Message.ButtonX then
		-- edit
		local message_length=Amax.RemainingTweetSpace()

		UIHardware.StartKeyboard( arg1, "GAME_TWITTER_MSG_TEXT_DIRECT", UIText.RBC_TWITTER_KEYB_HEADER, UIText.RBC_TWITTER_KEYB_SUB_HEADER, message_length, UIEnums.XboxKeyboardType.Standard )
		GUI.new_tweet = false
		
	elseif message == UIEnums.Message.MenuNext then
		-- send
		local allowOnline = true
		if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
			allowOnline = not NetServices.UserHasAgeRestriction()
		end
		
		if allowOnline == true then
			if GUI.num_tweets > 0 and Amax.GetTwitterTimeout() == 0 then
				SetupCustomPopup( UIEnums.CustomPopups.TwitterPost )
			end
		else
			SetupCustomPopup( UIEnums.CustomPopups.FailedAgeCheck )
		end

	elseif message == UIEnums.Message.MenuBack then
		-- Do one!
		EndScreen()
		PlaySfxBack()
		
	elseif message == UIEnums.Message.KeyboardFinished then
		if GUI.new_tweet == true then
			Amax.CreateNewTweet()
		else
			Amax.AddBlurbMessage()
		end

		RefreshScreen()

	end
end

function FrameUpdate( time )
	if UIGlobals.TwitterClose == true then
		UIGlobals.TwitterClose = false
		EndScreen()
	end

	if GUI.refresh_screen == true then
		RefreshScreen()
		GUI.refresh_screen = false
	end

	if GUI.tweeting == true then

		GUI.tweeting_timer = GUI.tweeting_timer + time
		if GUI.tweeting_timer > 2 then
			local still_pumping, error = Amax.PumpTwitterCurrentTask()

			if still_pumping == false then
				GUI.tweeting = false		

				if error ~= 0 then
					if error == 3501 or error == 3502 then
						PopupSpawn( UIEnums.CustomPopups.TwitterPostError )
					else
						if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
							SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
						else
							SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
						end
					end
				else
					PopupSpawn( UIEnums.CustomPopups.TwitterPosted )
					
					UIGlobals.TwitterFailures = 0
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
	--
end

function End()
	net_LockoutFriendsOverlay( GUI.old_friends )
end

function RefreshScreen()
	GUI.num_tweets = 1

	local has_tweets = ( GUI.num_tweets > 0 )
	UIButtons.SetActive( SCUI.name_to_id[ "box_no" ], not has_tweets, true )
	
	-- text
	local text_string_item = "GAME_TWITTER_MSG"
	local text_string_msg = UIText.CMN_NOWT
	UIButtons.ChangeText( GUI.ids[ 1 ].message_item, text_string_item )
	UIButtons.ChangeText( GUI.ids[ 1 ].message, text_string_msg )
	
	-- box
	local num_lines_item = UISystem.GetNumStringLines( text_string_item, GUI.font_height, GUI.font_width, GUI.font_style )
	local num_lines_msg = UISystem.GetNumStringLines( text_string_msg, GUI.font_height, GUI.font_width, GUI.font_style )
	
	local num_lines = num_lines_item --+ num_lines_msg
	
	if num_lines < 1 then
		num_lines = 1
	end
	
	local sx, sy, sz = UIButtons.GetSize( GUI.ids[ 1 ].box )
	UIButtons.ChangeSize( GUI.ids[ 1 ].box, GUI.default_box_width, GUI.default_box_height + ( num_lines - 1 ) * GUI.font_height, sz )
	
	local sx, sy, sz = UIButtons.GetSize( GUI.ids[ 1 ].box2 )
	UIButtons.ChangeSize( GUI.ids[ 1 ].box2, GUI.default_box_width + 0.75, GUI.default_box_height + ( num_lines - 1 ) * GUI.font_height + 0.8125, sz )
	
	local left = true
	
	UIButtons.SetActive( GUI.ids[ 1 ].speech_left, left )
	UIButtons.SetActive( GUI.ids[ 1 ].icon_left, left )
	UIButtons.SetActive( GUI.ids[ 1 ].speech_right, not left )
	UIButtons.SetActive( GUI.ids[ 1 ].icon_right, not left )
end

function Twitter_AddMessageNode( )
	GUI.ids[ #GUI.ids + 1 ] = 
	{
		box				= SCUI.name_to_id[ "box" ],
		box2			= SCUI.name_to_id[ "_box2" ],
		speech_left		= SCUI.name_to_id[ "_speech_left" ],
		speech_right	= SCUI.name_to_id[ "_speech_right" ],
		icon_left		= SCUI.name_to_id[ "_icon_left" ],
		icon_right		= SCUI.name_to_id[ "_icon_right" ],
		message_item	= SCUI.name_to_id[ "_message_item" ],
		message			= SCUI.name_to_id[ "message" ],
	}
end

function Twitter_DoNoTweetsBox()
	-- sort out the size of the no-tweets box
	local box_id = SCUI.name_to_id[ "box_no" ]
	local sx, sy, sz = UIButtons.GetSize( box_id )
	local default_box_height = sy
	local font_style = UIButtons.GetXtVar( SCUI.name_to_id[ "text" ], "styles.name" )
	sx, sy, sz = UIButtons.GetSize( SCUI.name_to_id[ "text" ] )
	local font_height = sy
	local font_width = sx
	local num_lines = UISystem.GetNumStringLines( UIText.RBC_TWITTER_NO_TWEETS, font_height, font_width, font_style )
	if num_lines < 1 then
		num_lines = 1
	end
	sx, sy, sz = UIButtons.GetSize( box_id )
	UIButtons.ChangeSize( box_id, sx, default_box_height + ( num_lines - 1 ) * font_height, sz )
end

function SendTweet()
	local _GUI = ContextTable[ UIEnums.Context.Blurb ].GUI
	if IsTable( _GUI ) ~= true then
		return
	end

	Amax.Tweet()
	_GUI.tweeting = true
	_GUI.tweeting_timer = 0
end

function DeleteTweet()
	local _GUI = ContextTable[ UIEnums.Context.Blurb ].GUI
	if IsTable( _GUI ) ~= true then
		return
	end

	_GUI.save_on_exit = true
	_GUI.refresh_screen=true
end
