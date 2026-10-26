GUI =
{
	finished = false,
	carousel_branch = "Photos",
	
	-- message text
	textId = -1,
	
	ids = {  },
	node_list_id = -1,
	
	num_items = 200,
	
	enumeration_indices = {  },
	
	preview_title = -1,
	total_photos = -1,
	
	photo_count = 0,
	
	photo_preview = { },
	
	Current_Selection = -1,
	
	wait_time = 0.25,
	wait_timer = 0.25,
	waiting = true,
	
	can_view_fullscreen = false,
	
	can_select_nodelist = true,
	
	CanExit = function( _GUI )
		return false
	end,
}
	
function Init()
	net_LockoutFriendsOverlay( true )
	
	Amax.ChangeUiCamera( "Sp_2", UIGlobals.CameraLerpTime, 0 )

	CarouselApp_SetScreenTimers()
	
	net_SetRichPresence( UIEnums.RichPresence.Photos )
	
	-- message text
	GUI.textId = SCUI.name_to_id[ "text" ]	
	GUI.node_list_id = SCUI.name_to_id[ "node_list" ]
	
	-- ***
	-- are we currently enumerating?
	GUI.enumerating = false
	-- return value of the save system when loading in
	GUI.profileReturnCode = UIEnums.ProfileError.Okay
	-- allowed to change storage devices?
	GUI.allowSaveLocationChange = false
	-- did the enumeration bork up?
	GUI.errorWhenEnumerating = false
	-- allow to load a file after enumeration?
	GUI.allowLoad = false
	-- are we currently loading?
	GUI.loading = false
	-- data corrupt after attempting a load?
	GUI.corruptData = false
	-- try again if cancelled on PS3
	GUI.userCancelled = false
	-- ***
	
	StoreInfoLine()
	SetupInfoLine()
	
	local sx, sy, sz = UIButtons.GetSize( SCUI.name_to_id[ "box" ] )
	GUI.default_box_height = sy
	GUI.font_style = UIButtons.GetXtVar( SCUI.name_to_id[ "text" ], "styles.name" )
	sx, sy, sz = UIButtons.GetSize( SCUI.name_to_id[ "text" ] )
	GUI.font_height = sy
	GUI.font_width = sx
end

function PostInit()
	UIButtons.TimeLineActive( "Hide_Carousel", true )
	
	for i = 1, GUI.num_items, 1 do
		-- clone new node
		local node_id = UIButtons.CloneXtGadgetByName( "Profile\\AsyncEnumerate.lua", "_node" )
		UIButtons.AddListItem( GUI.node_list_id, node_id, i )
		
		GUI.ids[ #GUI.ids + 1 ] = 
		{
			node = node_id,
			text = UIButtons.FindChildByName( node_id, "_text" ),
			image = UIButtons.FindChildByName( node_id, "_preview" ),
		}
	end
	
	SetupScreenTitle( UIText.RBU_PHOTO_LOCAL_PHOTOS_PAGE_TITLE, SCUI.name_to_id["screen"], "Local_Photos", "fe_icons"  )
	
	local node_id = SCUI.name_to_id[ "photo_preview_dummy"]
	GUI.photo_preview =  --CreatePhotoPreviewPage( SCUI.name_to_id["screen"] )	 
	{
		node = node_id,
		background = UIButtons.FindChildByName( node_id, "background" ),
		bottom_text = UIButtons.FindChildByName( node_id, "bottom_bar_text" ),
		top_text = UIButtons.FindChildByName( node_id, "top_bar_text" ),
	}
	
	
	
	-- start the enumeration
	GUI.bottom_text_id = SetupBottomHelpBar( UIText.HLP_PHOTO, SCUI.name_to_id["screen"] )
	--show_table(GUI.photo_preview)
	UIButtons.SetParent( SCUI.name_to_id["node_list_dummy"], GUI.photo_preview.background, UIEnums.Justify.MiddleCentre )
	
	StartEnumeration()
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		UIButtons.SetSelected( GUI.node_list_id, false )
		GUI.can_select_nodelist = false
		UIButtons.SetActive(SCUI.name_to_id["Right_Arrow"], false)
		UIButtons.SetActive(SCUI.name_to_id["Left_Arrow"], false)
	end
	
end

function StartLoop( time )
	--
end
	
function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true or GUI.enumerating == true or GUI.loading == true or GUI.waiting_to_enumerate == true then
		return
	end
	
	if SubScreenActive() == true then
		--fix for bug 36826
		if message == UIEnums.Message.MenuBack then
			if GUI.can_select_nodelist == true then
				UIButtons.SetSelected( GUI.node_list_id, true )
			end
		end
			
		return
	end
	
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.NoLocalPhotos then
		Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, UIGlobals.CameraLerpTime, 0 )
		UIButtons.TimeLineActive( "Hide_Carousel", false )
		CloseCurrentApp( true )
		return
	end
	
	-- player in-specific messages
	if message == UIEnums.Message.SaveLocationNotFound then
		GUI.allowSaveLocationChange = true
		AsyncEnumerate_ChangeText( UIText.PRO_NO_STORAGE_DEVICE )
		return
	end
	
	if message == UIEnums.Message.SaveLocationFound then
		GUI.allowSaveLocationChange = false
		Profile.StoreDeviceId( Profile.GetPrimaryPad() )
		-- start the enumeration
		StartEnumeration()
		return
	end
	
	-- try again if cancelled
	if GUI.userCancelled == true then
		if message == UIEnums.Message.MenuNext then
			PlaySfxNext()
			StartEnumeration()
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			AsyncEnumerate_Finished( true )
		end
		return
	end
	
	if GUI.errorWhenEnumerating == true then
		if message == UIEnums.Message.MenuNext then
			GoScreen( "Profile\\AsyncEnumerate.lua" )
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			AsyncEnumerate_Finished( true )
		end
		return
	end
	
	if GUI.allowSaveLocationChange == true then
		if message == UIEnums.Message.MenuNext then
			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
				UIHardware.FindXbox360SaveLocation( UIGlobals.ProfileEnumerateIndex, 0, UIEnums.XboxDeviceSelector.ForceUI )
			end
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			AsyncEnumerate_Finished( true )
		end
		return
	end
	
	if GUI.corruptData == true then
		if message == UIEnums.Message.MenuNext then
			GoScreen( "Profile\\AsyncEnumerate.lua" )
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			AsyncEnumerate_Finished( true )
		end
		-- TODO - allow to delete the corrupt file?
		return
	end
	
	if GUI.can_view_fullscreen == true then

		local allowOnline = true
		if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
			allowOnline = not NetServices.UserHasAgeRestriction()
		end
		
		if message == UIEnums.Message.MenuNext then
			PlaySfxNext()
			PushScreen("Photos\\ViewPhoto.lua")
			
			--fix for bug 36826
			if GUI.can_select_nodelist == true then
				UIButtons.SetSelected( GUI.node_list_id, false )
			end
				
		elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then
			if Amax.CanUseShare() == true and Amax.CanViewUserContent() == true and allowOnline == true then
				SetupCustomPopup( UIEnums.CustomPopups.OnlinePhotoSharingOptions )
			end
		
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			AsyncEnumerate_Finished( true )
			
		elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.OnlinePhotoSharingOptions then
			if arg3 == UIEnums.ShareOptions.FacebookAlbum then
				-- upload to facebook
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
		return
	end
end
	
function FrameUpdate( time )
	if GUI.waiting_to_enumerate == true then
		-- just spin until it's finished
		if UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false then
			GUI.waiting_to_enumerate = false
			StartEnumeration( GUI.waiting_to_enumerate_index )
		end
		return
	end
	
	if GUI.enumerating == true or GUI.loading == true then
		GUI.timer = GUI.timer + time
		
		-- continue the enumeration
		local finished, error = Profile.ContinueAsyncLoad()
		
		if finished == true and GUI.timer > 0 then
			AsyncEnumerate_ChangeText( UIText.CMN_NOWT, true )
			
			-- now check the error code
			if error == UIEnums.ProfileError.Okay then
				UIButtons.TimeLineActive( "Intro", false )
				
				if GUI.can_select_nodelist == true then
					UIButtons.SetSelected( GUI.node_list_id, true )
				end
								
				if GUI.enumerating == true then
					GUI.enumeration_indices = {  }
					
					-- count valid items
					local count = 0
					
					for i = 1, Profile.GetEnumerationCount(), 1 do
						if Profile.IsValidEnumerationFileType( i ) then
							count = count + 1
						end
					end
					
					GUI.photo_count = count

					if count == 0 then
						SetupCustomPopup( UIEnums.CustomPopups.NoLocalPhotos )
						GUI.allowLoad = false
						SetupInfoLine()
					else
						-- fill list
						local index = 0
						
						--print("Filling list")
						
						for i = 1, Profile.GetEnumerationCount(), 1 do
							if Profile.IsValidEnumerationFileType( i ) then
								index = index + 1
								UIButtons.SetNodeItemLocked( GUI.node_list_id, index - 1, false )
								GUI.enumeration_indices[ #GUI.enumeration_indices + 1 ] = i
							end
						end
					
						for i = index + 1, #GUI.ids, 1 do
							UIButtons.SetNodeItemLocked( GUI.node_list_id, i - 1, true )
						end
						
						if GUI.can_select_nodelist == true then
							UIButtons.SetSelection( GUI.node_list_id, 1 )
						end
						
						GUI.allowLoad = true
						--AsyncEnumerate_ChangeText( UIText.IDS_CMN_NOWT )
						SetupInfoLine()-- UIText.INFO_OPEN_A, UIText.INFO_B_BACK )
					end
				else
					-- we've loaded fine
					--AsyncEnumerate_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileEnumerateIndex .. "_ENUMERATION_LOADED" )
					AsyncEnumerate_SetupInfoLine()
					UIGlobals.FileParams.FinishedSuccess = true
					
					-- PR - force end - param for this?
					--AsyncEnumerate_ChangeText( UIText.IDS_CMN_NOWT )
					--AsyncEnumerate_Finished()
					
					for i = 1, #GUI.ids, 1 do
						UIButtons.ChangeTexture( { filename = "" }, 0, GUI.ids[i].image )
					end				
					
					UIButtons.ChangeTexture( { filename = "PHOTOMODE" }, 0, GUI.ids[GUI.Current_Selection].image )
					UIButtons.TimeLineActive( "Photo_Loaded", true, 0 )
					
					GUI.can_view_fullscreen = true
					
					local allowOnline = true
					if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
						allowOnline = not NetServices.UserHasAgeRestriction()
					end
					
					if Amax.CanUseShare() == true and Amax.CanViewUserContent() == true and allowOnline == true then
						SetupInfoLine( UIText.INFO_OPEN_A, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON" )
					else
						SetupInfoLine( UIText.INFO_OPEN_A, UIText.INFO_B_BACK )
					end
				end
			
			elseif error == UIEnums.ProfileError.DeviceNotConnected then
				GUI.allowSaveLocationChange = true
				AsyncEnumerate_SetupInfoLine( UIText.INFO_CHOOSE_STORAGE_A, UIText.INFO_CONTINUE_B )
				
				if GUI.enumerating == true then
					AsyncEnumerate_ChangeText( UIText.PRO_DEVICE_REMOVED_ENUMERATE )
				else
					AsyncEnumerate_ChangeText( UIText.PRO_DEVICE_REMOVED_LOAD )
				end				
				
			elseif GUI.loading == true and error == UIEnums.ProfileError.CannotReadFile then
				-- "profile out of date" (debug) or corrupt -- just display a corrupt message
				GUI.corruptData = true
				AsyncEnumerate_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileEnumerateIndex .. "_ENUMERATION_CORRUPT" )
				AsyncEnumerate_SetupInfoLine( UIText.INFO_TRY_AGAIN_A, UIText.INFO_CONTINUE_B )
				
			elseif error == UIEnums.ProfileError.UserCancelled then	-- PS3				
				if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
					--GUI.userCancelled = true
					--AsyncEnumerate_ChangeText( UIText.PRO_LOAD_USER_CANCELLED )
					--AsyncEnumerate_SetupInfoLine( UIText.INFO_YES_A, UIText.INFO_NO_B )
					AsyncEnumerate_SetupInfoLine()
					AsyncEnumerate_Finished( true )
				else
					print( "*****" )
					print( "ERROR: UIEnums.ProfileError.UserCancelled - got here not on PS3" )
					print( "*****" )
				end
			
			else
				GUI.errorWhenEnumerating = true
				AsyncEnumerate_SetupInfoLine( UIText.INFO_TRY_AGAIN_A, UIText.INFO_CONTINUE_B )
				
				if GUI.enumerating == true then				
					AsyncEnumerate_ChangeText( "PROFILE_PAD" .. UIGlobals.ProfileEnumerateIndex .. "_ENUMERATION_FAILED" )
				else
					print( "ERROR: load return code not handled! How did we get here then?" )
					AsyncEnumerate_ChangeText( UIText.IDS_CMN_NOWT )
				end
			end
			
			if GUI.enumerating == true then
				GUI.enumerating = false
				
				print( "FINISHED ENUMERATION" )
			else
				--GUI.allowLoad = false
				GUI.loading = false
				print( "FINISHED LOAD" )
			end
		end
	end
	
	-- if changed selected
	
	local selection = (UIButtons.GetSelectionIndex(SCUI.name_to_id[ "node_list" ]) + 1)
	if selection ~= GUI.Current_Selection then
		-- arrows
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "Left_Arrow" ], "more", ( selection ~= 1 ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "Left_Arrow" ], "push", ( selection < GUI.Current_Selection ), 0 )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "Right_Arrow" ], "more", ( selection ~= GUI.photo_count ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "Right_Arrow" ], "push", ( selection > GUI.Current_Selection ), 0 )
		
		GUI.Current_Selection = selection
		GUI.waiting = true
		-- reset wait timer
		GUI.wait_timer = GUI.wait_time
		UIButtons.TimeLineActive( "Photo_Loaded", false, 0 )
		GUI.can_view_fullscreen = false
	end
	
	if GUI.waiting == true then
		GUI.wait_timer = GUI.wait_timer - time
	end
		
	
	if GUI.allowLoad == true and UIEnums.CurrentPlatform ~= UIEnums.Platform.PS3 then
		if GUI.waiting == true then
			if GUI.wait_timer <= 0.0 then	
				GUI.waiting	= false
				StartEnumeration( GUI.enumeration_indices[ UIButtons.GetSelection( GUI.node_list_id ) ] )
			end
		end
	end
	
	if GUI.enumerating == false then
		if #GUI.ids > 0 then
			local photo_selected = GUI.enumeration_indices[ UIButtons.GetSelection( GUI.node_list_id ) ] 
			if photo_selected ~= nil then
			
				UIButtons.ChangeText( GUI.photo_preview.top_text, "PROFILE_SAVE_ENUMERATION" .. photo_selected )	
			
				local current_photo = (photo_selected -1)
				local total_photos = GUI.photo_count
						
				UIButtons.ChangeText( GUI.photo_preview.bottom_text, "SPL_PHOTOMODE_TOTAL_PHOTOS_" .. current_photo .. "_" .. total_photos )
			end	
		end	
	end
	
end

function EndLoop( time )
	--
end

function EnterEnd()
	RestoreInfoLine()
	PlaySfxGraphicBack()
end

function End()
	net_LockoutFriendsOverlay( false )
	
	net_SetRichPresence( UIEnums.RichPresence.SpMain )
end

function StartEnumeration( enumerationIndex )
	if UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == true then
		GUI.waiting_to_enumerate = true
		GUI.waiting_to_enumerate_index = enumerationIndex
		return
	end
	
	StartAsyncLoadPhotos()
	
	-- setup the save system
	UIGlobals.FileParams.SetInterface( true, enumerationIndex )
	
	Profile.StartAsyncLoad( UIGlobals.ProfileEnumerateIndex )
	
	if enumerationIndex == nil and UIEnums.CurrentPlatform ~= UIEnums.Platform.PS3 then
		print( "enumerate" )
		AsyncEnumerate_ChangeText( UIText.PRO_ENUMERATING_PHOTOS, true )
		GUI.enumerating = true
	else
		print( "load" )
		if UIEnums.CurrentPlatform ~= UIEnums.Platform.PS3 then
			AsyncEnumerate_ChangeText( UIText.PRO_LOADING, true )	
		else
			UIButtons.TimeLineActive( "Intro", false )
		end
		GUI.loading = true
	end
	
	AsyncEnumerate_SetupInfoLine()
	SetupInfoLine()

	UIButtons.SetSelected( GUI.node_list_id, false )
	
	-- reset
	GUI.timer = 0
	GUI.userCancelled = false
end

function AsyncEnumerate_Finished( cancelled )

	if cancelled == nil then
		cancelled = false
	end
	
	-- Send message
	print("FinishedMessage", UIGlobals.FileParams.FinishedMessage)
	if IsNumber( UIGlobals.FileParams.FinishedMessage ) == true then
		Amax.SendMessage( UIGlobals.FileParams.FinishedMessage  )
	end		
	if UIGlobals.FileParams.FinishedSuccess ~= true then
		UIGlobals.FileParams.FinishedSuccess = false
	end
	
	local do_next = ( ( UIGlobals.FileParams.FinishedSuccess == true ) and ( cancelled == false ) )
	
	if do_next == true then
		PushScreen( GetStoredScreen( UIEnums.ScreenStorage.SAVE_NEXT ) )
	else
		Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, UIGlobals.CameraLerpTime, 0 )
		UIButtons.TimeLineActive( "Hide_Carousel", false )
		CloseCurrentApp()
	end	
end

function AsyncEnumerate_ChangeText( text, not_the_popup )
	if not_the_popup == true then
		UIButtons.ChangeText( SCUI.name_to_id[ "text_small" ], text )
	else
		UIButtons.ChangeText( GUI.textId, text )
		
		-- re-size box
		local box_id = SCUI.name_to_id[ "box" ]
		local num_lines = UISystem.GetNumStringLines( text, GUI.font_height, GUI.font_width, GUI.font_style )
		if num_lines < 1 then
			num_lines = 1
		end
		local sx, sy, sz = UIButtons.GetSize( box_id )
		sy = GUI.default_box_height + ( num_lines * GUI.font_height )
		UIButtons.ChangeSize( box_id, sx, sy, sz )
		UIButtons.ChangeSize( SCUI.name_to_id[ "box_inner" ], sx, sy, sz )
		
		UIButtons.TimeLineActive( "Intro", true, 0 )
	end
end

function AsyncEnumerate_SetupInfoLine( text_one, text_two )
	if text_one == nil then
		text_one = UIText.CMN_NOWT
	end
	if text_two == nil then
		text_two = UIText.CMN_NOWT
	end
	
	UIButtons.ChangeText( SCUI.name_to_id[ "info_1" ], text_one )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_2" ], text_two )
end
