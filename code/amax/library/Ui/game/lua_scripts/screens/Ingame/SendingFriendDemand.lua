GUI =
{
	finished = false,
	
	timer = 0,
	
	uploaded = false,
}

function Init()
	AddSCUI_Elements()
	SetupInfoLine()
	
	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
end

function PostInit()
	local screen_id = SCUI.name_to_id[ "screen" ]
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMAND, screen_id, "message", "common_icons", nil, UIEnums.Justify.TopCentre, true, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	local help_text_id, bottom_bar_id = SetupBottomHelpBar( nil, nil, nil, true, nil, 1 )
	GUI.bottom_bar_id = bottom_bar_id
	
	if UIGlobals.FriendDemandIsResend == true then
		UIButtons.ChangeText( SCUI.name_to_id[ "sent_text" ], UIText.FDE_RECHALLENGE_SENT )
	end
	
	UIButtons.SetParent( SCUI.name_to_id[ "help_text" ], help_text_id, UIEnums.Justify.MiddleCentre )
	UIButtons.SetParent( SCUI.name_to_id[ "sent_text" ], help_text_id, UIEnums.Justify.MiddleCentre )
	
	-- ids
	local ids = {  }
	for i = 0, 3, 1 do
		ids[ i ] = { name = SCUI.name_to_id[ "name_" .. i ], pic = SCUI.name_to_id[ "pic_" .. i ], frame = SCUI.name_to_id[ "frame_" .. i ], blob = SCUI.name_to_id[ "blob_" .. i ], }
	end
	
	-- update info
	UIButtons.ChangeTexture( { filename = "GAMERPIC_" .. Profile.GetPrimaryPad() }, 1, ids[ 0 ].pic )
	UIButtons.ChangeText( ids[ 0 ].name, "PROFILE_PAD_NAME" )
	
	local map = Profile.GetRemoteGamerPictureMap()
	local num_friends = 0
	
	-- build friends
	local friends = {  }
	
	for i = 1, 3, 1 do
		if UIGlobals.FriendDemandFriends[ i ] ~= nil then
			if UIGlobals.FriendDemandIsResend == true then
				friends[ #friends + 1 ] = UIGlobals.FriendDemandFriends[ i ]
			else
				friends[ #friends + 1 ] = i - 1
			end
		end
	end
	
	local num_friends = #friends
	
	show_table(friends, true)
	show_table(map, true)
	
	-- add info
	for i, v in ipairs( friends ) do
	
		if map[ v ] ~= nil then
			UIButtons.ChangeTexture( { filename = "REMOTE_GAMERPIC_" .. map[ v ] }, 1, ids[ i ].pic )
		else
			UIButtons.ChangeTexture( { filename = "DEFAULT_GAMERPIC" }, 1, ids[ i ].pic )
		end
		
		if UIGlobals.FriendDemandIsResend == true then
			UIButtons.ChangeText( ids[ i ].name, "GAME_FRIEND_DEMAND_FRIEND_" .. v  )
		else
			UIButtons.ChangeText( ids[ i ].name, "GAME_FRIEND_DEMAND_FRIEND_" .. UIGlobals.FriendDemandFriends[ v + 1 ] )
		end
	end
	
	-- turn off others
	for i = num_friends + 1, 3, 1 do
		UIButtons.SetActive( ids[ i ].frame, false, true )
	end
	
	-- shift and scale	
	if num_friends ~= 3 then
		local sx0, sy0, sz0 = UIButtons.GetSize( ids[ 0 ].frame )
		local sx1, sy1, sz1 = UIButtons.GetSize( ids[ 1 ].frame )
		local sx3, sy3, sz3 = UIButtons.GetSize( ids[ 0 ].blob )
		local sx4, sy4, sz4 = UIButtons.GetSize( ids[ 1 ].blob )
		
		local offset_pic = ( sx0 - sx1 )
		local offset_blob = ( sx3 - sx4 )
		
		if num_friends == 2 then
			offset_pic = offset_pic / 2
			offset_blob = offset_blob / 2
		end
		
		for i = 1, num_friends, 1 do
			if num_friends == 1 then
				UIButtons.ChangePosition( ids[ i ].frame, nil, 0, nil )
			else
				local offset = offset_pic
				UIButtons.ChangePosition( ids[ i ].frame, nil, Select( ( i == 1 ), sy0 - offset, -sy0 + offset ), nil )
			end
			
			UIButtons.ChangeSize( ids[ i ].frame, sx1 + offset_pic, sy1 + offset_pic, sz1 )
			UIButtons.ChangeSize( ids[ i ].pic, sx1 + offset_pic, sy1 + offset_pic, sz1 )
			UIButtons.ChangeSize( ids[ i ].blob, sx4 + offset_blob, sy4 + offset_blob, sz4 )	
		end
	end
	
	-- swirls
	if num_friends == 2 or num_friends == 3 then
		UIButtons.SetActive( SCUI.name_to_id[ "vs_1a" ], true )
		UIButtons.SetActive( SCUI.name_to_id[ "vs_1b" ], true )
	end
	if num_friends == 1 or num_friends == 3 then
		UIButtons.SetActive( SCUI.name_to_id[ "vs_2a" ], true )
		UIButtons.SetActive( SCUI.name_to_id[ "vs_2b" ], true )
	end
	if num_friends == 2 or num_friends == 3 then
		UIButtons.SetActive( SCUI.name_to_id[ "vs_3a" ], true )
		UIButtons.SetActive( SCUI.name_to_id[ "vs_3b" ], true )
	end
	
	-- audio
	if num_friends == 1 then
		UISystem.PlaySound( UIEnums.SoundEffect.FriendVs1 )
	elseif num_friends == 2 then
		UISystem.PlaySound( UIEnums.SoundEffect.FriendVs2 )
	elseif num_friends == 3 then
		UISystem.PlaySound( UIEnums.SoundEffect.FriendVs3 )
	end
	
	-- go
	FriendDemand.Send()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then
		local type = UIEnums.SocialNetworkingItemType.FriendDemand
		
		if UIGlobals.FriendDemandIsResend == true then
			type = UIEnums.SocialNetworkingItemType.FriendReDemand
		end

		if arg3 == UIEnums.ShareOptions.Facebook then
			Amax.CreateBlurb( type, 1, -1 )
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, GetStoredScreen( UIEnums.ScreenStorage.FE_FRIEND_DEMAND ) )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then
			Amax.CreateBlurb( type, 0, -1 )
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, GetStoredScreen( UIEnums.ScreenStorage.FE_FRIEND_DEMAND ) )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )
			
		elseif arg3 == UIEnums.ShareOptions.Blurb then
			if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
				FriendDemand.SendSystemMessage()

			elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
				Amax.CreateBlurb( type, 2, -1 )
			end
		end
		
	elseif GUI.uploaded == true then
		if message == UIEnums.Message.MenuNext then
			PlaySfxNext()
			GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_FRIEND_DEMAND ) )
			
		elseif message == UIEnums.Message.ButtonLeftShoulder then
			if Amax.CanUseShare() == true then
				UIGlobals.ShareFromWhatPopup = -1
				SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
			end
		end
	end
end

function FrameUpdate( time )
	if GUI.uploaded == false then
		GUI.timer = GUI.timer + time

		if GUI.timer > 2 then

			local busy, error = FriendDemand.PumpSend()
			
			if busy == false then
				GUI.uploaded = true
				
				SetupInfoLine( UIText.INFO_A_NEXT, "GAME_SHARE_BUTTON" )
				UIButtons.TimeLineActive( "help_text_on", false )
				
				if error == 0 then
					UIButtons.TimeLineActive( "sent_on", true, 0 )
					UIGlobals.FriendDemandSent = true
					
					if UIGlobals.FriendDemandIsResend == true then
						Amax.CheckStickerProgress( UIEnums.StickerType.SendChallengeBack )
					else
						Amax.CheckStickerProgress( UIEnums.StickerType.FirstFriendDemand )
					end
				else
					if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
						if net_CanReconnectToDemonware() == false then
							SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
						else
							net_StartServiceConnection(true, nil, false)
						end
					elseif LSP.IsConnected() == false then
						SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
					else
						SetupCustomPopup( UIEnums.CustomPopups.FriendDemandSendingError )
					end
				end
			end
		end
	end
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end

function FriendChallengeReloadScreen( result )
	if result == true then
		GoScreen( "Ingame\\SendingFriendDemand.lua" )
	end
end
