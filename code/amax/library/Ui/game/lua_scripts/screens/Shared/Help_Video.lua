GUI =
{
	finished = false,

	CanExit = function( _GUI )
		return false
	end,

	VideoEnums = 
	{
		UIEnums.SPVideoConfig.VIDEO_TUTORIAL_1,
		UIEnums.SPVideoConfig.VIDEO_POWER_UPS,
		UIEnums.SPVideoConfig.VIDEO_TUTORIAL_2,
		UIEnums.SPVideoConfig.VIDEO_GAME_MODE_RACE,
		UIEnums.SPVideoConfig.VIDEO_GAME_MODE_DESTRUCTION,
		UIEnums.SPVideoConfig.VIDEO_GAME_MODE_CHECKPOINT,
		UIEnums.SPVideoConfig.VIDEO_GAME_MODE_ONEONONE,
		UIEnums.SPVideoConfig.VIDEO_FAN_PAR,
		UIEnums.SPVideoConfig.VIDEO_FAN_RUN,
		UIEnums.SPVideoConfig.VIDEO_FAN_DEMAND,
	},
	
	VideoIcons = 
	{
		"bio",
		"perk_question",
		"garage",
		"style_racing",
		"survivor",
		"quick_race",
		"deathmatch",
		"fan",
		"fan_attack",
		"fan",
	},
	
	VideoTextures = 
	{
		"tut_thumb_1", -- welcome
		"tut_thumb_7", -- power-ups
		"tut_thumb_2", -- advanced tactics
		"tut_thumb_6", -- race
		"tut_thumb_8", -- destruction
		"tut_thumb_9", -- checkpoint
		"tut_thumb_10", -- one on one
		"tut_thumb_4", -- fan target
		"tut_thumb_5", -- fan run
		"tut_thumb_3", -- fan demands	
	},
	
	VideoTitles = 
	{
		UIText.CMN_HELP_VIDEO_VID1,
		UIText.CMN_HELP_VIDEO_VID2,
		UIText.CMN_HELP_VIDEO_VID3,
		UIText.CMN_HELP_VIDEO_VID4,
		UIText.CMN_HELP_VIDEO_VID5,
		UIText.CMN_HELP_VIDEO_VID6,
		UIText.CMN_HELP_VIDEO_VID7,
		UIText.CMN_HELP_VIDEO_VID8,
		UIText.CMN_HELP_VIDEO_VID9,
		UIText.CMN_HELP_VIDEO_VID10,
	},
	
	VideoBottomHelp = 
	{
		UIText.CMN_HELP_VIDEO_VID1_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID2_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID3_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID4_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID5_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID6_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID7_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID8_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID9_BOTTOM,
		UIText.CMN_HELP_VIDEO_VID10_BOTTOM,
	},
	
	num_videos = 10,
	do_update = true,
}

function Init()
	AddSCUI_Elements()
	
	GUI.NodeListId = SCUI.name_to_id[ "video_list" ]
	
	for i=1,GUI.num_videos do
		SetupVideoNode( i )
	end
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_PLAY_A, UIText.INFO_B_BACK )
	
	SetupScreenTitle( UIText.CMN_HELP_VIDEO_TITLE, SCUI.name_to_id["_CarouselDummy"], "video" )
	GUI.bottom_bar_text_id = SetupBottomHelpBar( GUI.VideoBottomHelp[1] )
end

function PostInit()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		local selection = UIButtons.GetSelection( GUI.NodeListId )
		
		PlaySPMovieFullScreen( GUI.VideoEnums[selection], true )
	end
		
	if message == UIEnums.Message.MenuBack then
		PlaySfxBack()
		GoScreen("Shared\\Help.lua")
	end
	
	if message == UIEnums.Message.ButtonLeft then
		GUI.do_update = true
		UIButtons.TimeLineActive( "move_left", true, 0 )
		if UIButtons.GetSelectionIndex( GUI.NodeListId ) == GUI.num_videos - 1 then
			UIButtons.TimeLineActive( "right_fade", true )
		end
	elseif message == UIEnums.Message.ButtonRight then
		GUI.do_update = true
		UIButtons.TimeLineActive( "move_right", true, 0 )
		if UIButtons.GetSelectionIndex( GUI.NodeListId ) == 0 then
			UIButtons.TimeLineActive( "left_fade", true )
		end
	end
end

function FrameUpdate( time )
	if GUI.do_update == true then
		GUI.do_update = false
		GUI.update_arrows = false
		local index = UIButtons.GetSelectionIndex( GUI.NodeListId )
		UIButtons.ChangeText( GUI.bottom_bar_text_id, GUI.VideoBottomHelp[index + 1] )
		
		SetupInfoLine( UIText.INFO_PLAY_A, UIText.INFO_B_BACK )
		
		if index == GUI.num_videos - 1 then
			UIButtons.TimeLineActive( "right_fade", false )
		elseif index == 0 then
			UIButtons.TimeLineActive( "left_fade", false )
		end
	end
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	--
end

function SetupVideoNode( index )
	local node_id = UIButtons.CloneXtGadgetByName( "Shared\\Help_Video.lua", "video_node" )
	local image_id = UIButtons.FindChildByName( node_id, "image" )
	local lock_id = UIButtons.FindChildByName( node_id, "lock" )
	local title_id = UIButtons.FindChildByName( node_id, "title" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )
	
	UIButtons.AddListItem( GUI.NodeListId, node_id, index )
	UIButtons.ChangeTexture( { filename = GUI.VideoTextures[index] }, 0, image_id )
--	UIShape.ChangeObjectName( icon_id, GUI.VideoIcons[index] )
	UIButtons.ChangeText( title_id, GUI.VideoTitles[index] )
	
	UIButtons.SetActive( node_id, true )
end
