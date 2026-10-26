GUI =
{
	finished = false,	
	carousel_branch = "Options",

	CanExit = function( _GUI )
		-- Save		
		if UIGlobals.SaveOptions == true then
			if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
				-- [Blur2PC] retail Blur PC stores the keyboard options before applying the options
				Amax.UpdateCustomKeyboardControls( UIGlobals.OptionsTable )
			end
			Amax.Options( UIGlobals.OptionsTable )
			
			StartAsyncSave()
			UIGlobals.SaveOptions = nil
		end
		
		-- Clear
		UIGlobals.OptionsTable = nil
		
		-- Set the audio options in case they where changed but want to be reverted
		Amax.UpdateAudioOptions( Amax.Options() )
		
		PlaySfxGraphicBack()
		
		return true
	end,
	
	fb_disconnecting_timer=0,
}

function Init()
	AddSCUI_Elements()

	if UIGlobals.IsIngame == false then
		Amax.ChangeUiCamera( "Op_1", 0.6, 0 )
	end
	CarouselApp_SetScreenTimers()
	
	if IsTable( UIGlobals.OptionsTable ) == false then
		UIGlobals.OptionsTable = Amax.Options()

		if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
			-- [Blur2PC] retail Blur PC adds the keyboard options (key names; retail leaves Drop out)
			local keys = Amax.UpdateCustomKeyboardControls()
			UIGlobals.OptionsTable.IsCustom		= keys.IsCustom
			UIGlobals.OptionsTable.Throttle		= keys.Throttle
			UIGlobals.OptionsTable.Brake		= keys.Brake
			UIGlobals.OptionsTable.SteerLeft	= keys.SteerLeft
			UIGlobals.OptionsTable.SteerRight	= keys.SteerRight
			UIGlobals.OptionsTable.EBrake		= keys.EBrake
			UIGlobals.OptionsTable.Fire			= keys.Fire
			UIGlobals.OptionsTable.SwitchPerk	= keys.SwitchPerk
			UIGlobals.OptionsTable.PowerUpFwd	= keys.PowerUpFwd
			UIGlobals.OptionsTable.PowerUpBwd	= keys.PowerUpBwd
			UIGlobals.OptionsTable.ChangeView	= keys.ChangeView
			UIGlobals.OptionsTable.Pause		= keys.Pause
			UIGlobals.OptionsTable.RearView		= keys.RearView
		end
	end
	
	if IsTable( UIGlobals.DefaultOptionsTable ) == false then
		UIGlobals.DefaultOptionsTable = Amax.GetOptionsDefaults()
	end
	
	local controller_icon_string = "controller_options"
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		controller_icon_string = "ps3_controller_options"
	end
	
	
	GUI.MenuId = SCUI.name_to_id[ "menu" ]
	
	local num = 0
	
	if UIGlobals.IsIngame == false and Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
		Mp_SetupMenuItemHelpText( GUI.MenuId, 0,	UIText.CMN_OPT_DIFFICULTY,	UIText.CMN_HLP_OPT_DIFFICULTY,			"ai_difficulty",		"common_icons" )
		num = num + 1
	end
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] retail Blur PC: Keyboard (1) then the controller configuration (2), both left out in a
		-- split-screen race; audio and graphics move to 3 and 4
		if UIGlobals.IsIngame == false or IsSplitScreen() == false then
			Mp_SetupMenuItemHelpText( GUI.MenuId, 1,	UIText.CMN_OPT_CONTROL,			UIText.CMN_HLP_OPT_KEYBOARD,			"Keyboard_Mouse",		"common_icons" )
			Mp_SetupMenuItemHelpText( GUI.MenuId, 2,	UIText.CMN_OPT_CONTROLS_CONFIG,	UIText.CMN_HLP_OPT_CONTROLS_CONFIG,		controller_icon_string,	"common_icons" )
		end
		num = num + 2
	else
	Mp_SetupMenuItemHelpText( GUI.MenuId, 1,	UIText.CMN_OPT_CONTROLS_CONFIG,	UIText.CMN_HLP_OPT_CONTROLS_CONFIG,		controller_icon_string,	"common_icons" )
	num = num + 1
	end
	
	local show_audio_image = true
	
	if UIGlobals.IsIngame == true and IsSplitScreen() == true and Profile.GetPrimaryPad() ~= UIGlobals.splitscreen_primary_pad_original then
		show_audio_image = false
	end
	
	if show_audio_image == true then
		if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
			-- [Blur2PC] retail Blur PC offers Audio (3) and Graphics (4); image adjust (gamma) is reached from the graphics screen
			Mp_SetupMenuItemHelpText( GUI.MenuId, 3,	UIText.CMN_OPT_AUDIO,			UIText.CMN_HLP_OPT_AUDIO,				"large_speaker",		"common_icons" )
			Mp_SetupMenuItemHelpText( GUI.MenuId, 4,	UIText.CMN_OPT_GRAPHICS,		UIText.CMN_HLP_OPT_GRAPHICS,			"Graphics",				"common_icons" )
		else
		Mp_SetupMenuItemHelpText( GUI.MenuId, 2,	UIText.CMN_OPT_IMAGE_ADJUST,	UIText.CMN_HLP_OPT_IMAGE_ADJUST,		"brightness",			"common_icons" )
		Mp_SetupMenuItemHelpText( GUI.MenuId, 3,	UIText.CMN_OPT_AUDIO,			UIText.CMN_HLP_OPT_AUDIO,				"large_speaker",		"common_icons" )
		end
		num = num + 2
	end
	
	-- Remove save location on Ps3
	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon and CanSave( Profile.GetPrimaryPad(), true ) == true then
		Mp_SetupMenuItemHelpText( GUI.MenuId, 4,	UIText.CMN_OPT_SAVE_LOCATION,	UIText.CMN_HLP_OPT_SAVE_LOCATION,	"load_save",			"common_icons" )
		num = num + 1
	end
	
	if UIGlobals.IsIngame == false then

		if GameProfile.HasLinkCode() == true then
			Mp_SetupMenuItemHelpText( GUI.MenuId, 9,	UIText.CMN_LINK_YOUR_ACCOUNT,	UIText.CMN_HLP_LINK_YOUR_ACCOUNT,	"powered_up_racing",	"common_icons" )
			num = num + 1
		end
		
		Mp_SetupMenuItemHelpText( GUI.MenuId, 5,	UIText.CMN_OPT_SHOW_CREDITS,	UIText.CMN_HLP_OPT_SHOW_CREDITS,	"summery",				"common_icons" )
		num = num + 1
	end
	
	if Amax.GetGameMode() ~= UIEnums.GameMode.SinglePlayer and IsSplitScreen() == false then
		Mp_SetupMenuItemHelpText( GUI.MenuId, 6,	UIText.CMN_OPT_MULTIPLAYER,		UIText.CMN_HLP_OPT_MULTIPLAYER,		"online",				"common_icons" )
		num = num + 1
	end
	
	if UIGlobals.IsIngame == false and Amax.CanUseShare() == true then
	
		if Amax.FacebookHasPublisherFile() == true then
			local fb_text=UIText.CMN_OPT_AUTHENTICATE_FACEBOOK
			if Amax.FacebookIsRegistered() == true then
				fb_text=UIText.CMN_OPT_UNAUTHENTICATE_FACEBOOK
			end
		
			local facebook_id = Mp_SetupMenuItemHelpText( GUI.MenuId, 7,	fb_text,	UIText.CMN_HLP_OPT_AUTHENTICATE_FACEBOOK, "facebook",	"common_icons" )
			local fb_icon_id = UIButtons.FindChildByName( facebook_id, "icon" )
			UIButtons.ChangeColour( fb_icon_id, "NonBloomWhite" )
			UIButtons.SetXtVar( fb_icon_id, "time_lines.0.label", "hacksaw" ) -- 
			num = num + 1
		end

		if Amax.TwitterHasPublisherFile() == true then
			Mp_SetupMenuItemHelpText( GUI.MenuId, 8,	UIText.CMN_OPT_AUTHENTICATE_TWITTER,	UIText.CMN_HLP_OPT_AUTHENTICATE_TWITTER, "twitter", "common_icons" )
			num = num + 1
		end
	end

	-- Reduce scale so we can fit all the options on
	local scale = 1-((num - 6) * 0.06)
	if scale < 1 then
		UIButtons.ChangeScale( SCUI.name_to_id["ScaleDummy"], scale, scale, scale )
	end
	
	StoreInfoLine()
	SetupMenuOptions()	
	
	SetupScreenTitle( UIText.CMN_OPT_OPTIONS, SCUI.name_to_id["_CarouselDummy"], "option cog" )
end

function PostInit()

	-- Remove game options ingame
	local start_index = 0
	if UIGlobals.IsIngame == true or Amax.GetGameMode() ~= UIEnums.GameMode.SinglePlayer then
		start_index = 1
	end
	
	UIButtons.NodeListScan( GUI.MenuId )
	local s = GetScreenOtions().selection
	
	-- Set to previous selection if it exists
	UIButtons.SetSelection( GUI.MenuId, start_index )
	if IsNumber( s ) == true then
		UIButtons.SetSelection( GUI.MenuId, s )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if IsControllerLocked() == true then
		return
	end
	
	if message == UIEnums.Message.SaveLocationFound then
		SetupCustomPopup( UIEnums.CustomPopups.OptionsChangeStorageDevice )
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext()
		local selected = UIButtons.GetSelection( GUI.MenuId )
		if selected == 0 then
			GoScreen("Shared\\Options_DrivingAssists.lua")
		elseif UIEnums.CurrentPlatform == UIEnums.Platform.PC and selected >= 1 and selected <= 4 then
			-- [Blur2PC] retail Blur PC's menu: Keyboard, controller configuration, Audio, Graphics
			if selected == 1 then
				GoScreen("Shared\\Options_Control.lua")
			elseif selected == 2 then
				GoScreen("Shared\\Options_Controller.lua")
			elseif selected == 3 then
				UIGlobals.SaveOptions = true
				GoScreen("Shared\\Options_Audio.lua")
			else
				UIGlobals.SaveGraphicOptions = true
				GoScreen("Shared\\Options_Graphics.lua")
			end
		elseif selected == 1 then
			GoScreen("Shared\\Options_Controller.lua")
		elseif selected == 2 then
			GoScreen("Shared\\Options_ImageAdjust.lua")
		elseif selected == 3 then
			-- Just by looking at the audio options means we're going to save
			UIGlobals.SaveOptions = true
			GoScreen("Shared\\Options_Audio.lua")
		elseif selected == 4 then
			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
				-- change storage device
				UIHardware.FindXbox360SaveLocation( Profile.GetPrimaryPad(), 0, UIEnums.XboxDeviceSelector.ForceUI )
			end
		elseif selected == 5 then
			GoScreen("Shared\\Options_Credits.lua")
		elseif selected == 6 then
			UIGlobals.SaveOptions = true
			GoScreen("Shared\\Options_Multiplayer.lua") 
		elseif selected == 7 then
		
			if Amax.FacebookIsRegistered() == true then
				GUI.fb_disconnecting_timer=0
				SetupCustomPopup( UIEnums.CustomPopups.DisconnectFromFacebook )
			else
				UIGlobals.FacebookUploadingPhoto = false
				UIGlobals.FacebookBackToStoryScreen = false
				GoScreen("Shared\\FacebookAuthenticate.lua", UIEnums.Context.Blurb )
			end
			
		elseif selected == 8 then
			UIGlobals.TwitterBackToTweetScreen = false
			GoScreen("Shared\\TwitterAuthenticate.lua", UIEnums.Context.Blurb )
		elseif selected == 9 then
			GoScreen("Shared\\Options_LinkYourAccount.lua")
		else
			print("whaatisit?",selected)
		end
	end
end

function FrameUpdate( time )
	
	if UIGlobals.RefreshOptions==true then
		UIGlobals.RefreshOptions=nil

		GoScreen("Shared\\Options.lua")
	end

	if UIGlobals.fb_disconnecting == true then
		GUI.fb_disconnecting_timer = GUI.fb_disconnecting_timer + time
		
		if GUI.fb_disconnecting_timer > 2 then
			still_pumping, error = Amax.PumpFacebookCurrentTask()

			if still_pumping == false then
				UIGlobals.fb_disconnecting = nil
				
				if error ~= 0 then
					PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
				else
					PopupSpawn( UIEnums.CustomPopups.FacebookDisconnected )
					GoScreen("Shared\\Options.lua")
				end
			end
		end
	end
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	GetScreenOtions().selection = UIButtons.GetSelection( GUI.MenuId )
end
