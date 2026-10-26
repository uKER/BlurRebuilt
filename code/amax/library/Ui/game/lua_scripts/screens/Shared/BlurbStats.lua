GUI =
{
	finished = false,
	
	timer = 0,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	DeferCam_Init( "Sp_1" )
		
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_B_BACK, "GAME_SHARE_BUTTON" )
	
	UISystem.PlaySound( UIEnums.SoundEffect.FriendTaleOfTheTape )
end

function PostInit()
	PlaySfxGraphicNext()
	
	local screen_id = SCUI.name_to_id[ "screen" ]
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMANDS, screen_id, "message", "common_icons", 1, UIEnums.Justify.TopCentre, nil, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	--GUI.help_text_id = SetupBottomHelpBar( UIText.CMN_NOWT, screen_id, 1, nil )
	
	-- info you
	UIButtons.ChangeTexture( { filename = "GAMERPIC_" .. Profile.GetPrimaryPad() }, 1, SCUI.name_to_id[ "pic_0" ] )
	UIButtons.ChangeText( SCUI.name_to_id[ "name_0" ], "PROFILE_PAD_NAME" )
	
	-- info them	
	local map = Profile.GetRemoteGamerPictureMap()
	if map[ UIGlobals.FriendDemandFilterFriend ] ~= nil then
		UIButtons.ChangeTexture( { filename = "REMOTE_GAMERPIC_" .. map[ UIGlobals.FriendDemandFilterFriend ] }, 1, SCUI.name_to_id[ "pic_1" ] )
	end
	UIButtons.ChangeText( SCUI.name_to_id[ "name_1" ], "GAME_FRIEND_DEMAND_FRIEND_" .. UIGlobals.FriendDemandFilterFriend )
	
	-- data
	UIButtons.ChangeText( SCUI.name_to_id[ "info_sent_you" ], "GAME_FRIEND_DEMAND_STATS_SENT_YOU_" .. UIGlobals.FriendDemandFilterFriend )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_sent_them" ], "GAME_FRIEND_DEMAND_STATS_SENT_THEM_" .. UIGlobals.FriendDemandFilterFriend )

	UIButtons.ChangeText( SCUI.name_to_id[ "info_won_you" ], "GAME_FRIEND_DEMAND_STATS_WON_YOU_" .. UIGlobals.FriendDemandFilterFriend )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_won_them" ], "GAME_FRIEND_DEMAND_STATS_WON_THEM_" .. UIGlobals.FriendDemandFilterFriend )

	UIButtons.ChangeText( SCUI.name_to_id[ "info_aces_you" ], "GAME_FRIEND_DEMAND_STATS_KOS_YOU_" .. UIGlobals.FriendDemandFilterFriend )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_aces_them" ], "GAME_FRIEND_DEMAND_STATS_KOS_THEM_" .. UIGlobals.FriendDemandFilterFriend )

	UIButtons.ChangeText( SCUI.name_to_id[ "info_most_attempts_you" ], "GAME_FRIEND_DEMAND_STATS_MOST_ATTEMPTS_YOU_" .. UIGlobals.FriendDemandFilterFriend )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_most_attempts_them" ], "GAME_FRIEND_DEMAND_STATS_MOST_ATTEMPTS_THEM_" .. UIGlobals.FriendDemandFilterFriend )

	UIButtons.ChangeText( SCUI.name_to_id[ "info_avg_attempts_you" ], "GAME_FRIEND_DEMAND_STATS_AVG_ATTEMPTS_YOU_" .. UIGlobals.FriendDemandFilterFriend )
	UIButtons.ChangeText( SCUI.name_to_id[ "info_avg_attempts_them" ], "GAME_FRIEND_DEMAND_STATS_AVG_ATTEMPTS_THEM_" .. UIGlobals.FriendDemandFilterFriend )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		GoScreen( "Shared\\BlurbMainMenu.lua" )
		
	elseif message == UIEnums.Message.ButtonLeftShoulder then
		if Amax.CanUseShare() == true then
			SetupCustomPopup( UIEnums.CustomPopups.TaleOfTheTapeSharingOptions )
		end
	
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then
		if arg3 == UIEnums.ShareOptions.Facebook then
			Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.TaleOfTheTapeAccepted + UIGlobals.SharingOptionsChosen, 1, UIGlobals.FriendDemandFilterFriend )
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then
			Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.TaleOfTheTapeAccepted + UIGlobals.SharingOptionsChosen, 0, UIGlobals.FriendDemandFilterFriend )
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )
			
		elseif arg3 == UIEnums.ShareOptions.Blurb then
			Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.TaleOfTheTapeAccepted + UIGlobals.SharingOptionsChosen, 2, UIGlobals.FriendDemandFilterFriend )
		end
	end
end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate( time )
	DeferCam_Update( time )
	
	GUI.timer = GUI.timer + time
	
	-- delay this a little so the read is finished - maybe we need a message for this?
	if GUI.timer > 2 and GUI.has_done_stat_colours ~= true then
		BlurbStats_DoStatColours()
		GUI.has_done_stat_colours = true
	end
	
	if FriendDemand.IsFriend( UIGlobals.FriendDemandFilterFriend ) == false then
		PlaySfxBack()
		GoScreen( "Shared\\BlurbMainMenu.lua" )
	end
	
end

function EnterEnd()
	RestoreInfoLine()
	PlaySfxGraphicBack()
end

function EndLoop( time )
	--
end

function End()
	--
end

function BlurbStats_DoStatColours()
	local data = FriendDemand.GetTaleOfTheTapeData( UIGlobals.FriendDemandFilterFriend )
	
	if data == nil then
		return
	end
	
	local sent_res = Select( ( data.sent_you == data.sent_them ), 0, Select( ( data.sent_you > data.sent_them ), 1, -1 ) )
	local won_res =	Select( ( data.won_you == data.won_them ), 0, Select( ( data.won_you > data.won_them ), 1, -1 ) )
	local aces_res = Select( ( data.aces_you == data.aces_them ), 0, Select( ( data.aces_you > data.aces_them ), 1, -1 ) )
	
	local CompareAttempts = function( you, them )
		local res = 0
		
		if you == 0 and them ~= 0 then
			res = -1
		elseif them == 0 and you ~= 0 then
			res = 1
		else
			res = Select( ( you == them ), 0, Select( ( you < them ), 1, -1 ) )
		end
		
		return res
	end
	
	local most_attempts_res = CompareAttempts( data.most_attempts_you, data.most_attempts_them )
	local avg_attempts_res = CompareAttempts( data.avg_attempts_you, data.avg_attempts_them )
	
	--print( "sent_res", sent_res )
	--print( "won_res", won_res )
	--print( "aces_res", aces_res )
	--print( "most_attempts_res", most_attempts_res )
	--print( "avg_attempts_res", avg_attempts_res )
	
	-- update colours
	local GetColour = function( res, is_you )
		if res == 0 then
			return "Main_1"
		else
			return Select( ( res == Select( is_you, 1, -1 ) ), "Support_0", "Support_3" )
		end
	end
	
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_sent_you" ], GetColour( sent_res, true ) )
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_sent_them" ], GetColour( sent_res, false ) )
	
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_won_you" ], GetColour( won_res, true ) )
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_won_them" ], GetColour( won_res, false ) )
	
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_aces_you" ], GetColour( aces_res, true ) )
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_aces_them" ], GetColour( aces_res, false ) )
	
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_most_attempts_you" ], GetColour( most_attempts_res, true ) )
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_most_attempts_them" ], GetColour( most_attempts_res, false ) )
	
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_avg_attempts_you" ], GetColour( avg_attempts_res, true ) )
	UIButtons.ChangeColour( SCUI.name_to_id[ "icon_avg_attempts_them" ], GetColour( avg_attempts_res, false ) )
end
