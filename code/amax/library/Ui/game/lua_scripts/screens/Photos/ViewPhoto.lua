GUI =
{
	finished = false,
		
	CanExit = function( _GUI )
		return false
	end,
	
	spinner_id = -1,
	
	can_save = false,

	is_online = false,
	
	should_unhide_enumerate_screen = true,
}

function Init()
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_2", UIGlobals.CameraLerpTime, 0 )
	
	UIButtons.TimeLineActive( "Hide_LightPaint", true )
		
	StoreInfoLine()
	
	local allowOnline = true
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
		allowOnline = not NetServices.UserHasAgeRestriction()
	end
	
	if Amax.CanUseShare() == true and Amax.CanViewUserContent() == true and allowOnline == true then
		SetupInfoLine( UIText.INFO_B_BACK, "GAME_SHARE_BUTTON" )
	else
		SetupInfoLine( UIText.INFO_B_BACK )
	end
	
	GUI.spinner_id = SCUI.name_to_id[ "uploading_spinner" ]
	
	GUI.can_save  = ( CanSave( Profile.GetPrimaryPad(), true ) == true )
end

function PostInit()	
	UIButtons.TimeLineActive( "hide_asyncenumerate", true )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true then
		return
	end
	
	local allowOnline = true
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
		allowOnline = not NetServices.UserHasAgeRestriction()
	end
	
	if message == UIEnums.Message.MenuBack then
		GUI.should_unhide_enumerate_screen = true
		PlaySfxBack()
		PopScreen()
		
	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then
		if Amax.CanUseShare() == true and Amax.CanViewUserContent() == true and allowOnline == true then
			SetupCustomPopup( UIEnums.CustomPopups.OnlinePhotoSharingOptions )
		end
	
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.OnlinePhotoSharingOptions then
		if arg3 == UIEnums.ShareOptions.FacebookAlbum then
			if Amax.CanUseShare() == true and Amax.CanViewUserContent() == true and allowOnline == true then
				UIHardware.StartKeyboard( arg1, UIText.RBC_MY_PHOTO, UIText.RBC_FACEBOOK_KEYB_HEADER, UIText.RBC_FACEBOOK_KEYB_PHOTO_UPLOAD_HEADER, 128, UIEnums.XboxKeyboardType.StandardHighlight )
			end

		elseif arg3 == UIEnums.ShareOptions.Blurb then
			PushScreen("Photos\\ChoosePhotoSlot.lua")
		end

	elseif message == UIEnums.Message.KeyboardFinished then
		-- upload to facebook
		if UIScreen.IsContextActive(UIScreen.Context()) then
			SetupCustomPopup( UIEnums.CustomPopups.UploadPhotoToFacebook )
		end
	end
end

function FrameUpdate( time )
	--
end

function Render()
	--
end

function EnterEnd()

	if GUI.should_unhide_enumerate_screen == true then
		UIButtons.TimeLineActive( "hide_asyncenumerate", false )
		UIButtons.TimeLineActive( "Hide_LightPaint", false )
	end	
	
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	--
	
end
